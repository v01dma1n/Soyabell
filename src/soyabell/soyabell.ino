#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "soya_config.h"
#include "soya_preferences.h"
#include "soya_access_point_manager.h"
#include "wifi_connector.h"
#include "time_manager.h" 

// --- Hardware Pins ---
#define I2S_SD  32  
#define I2S_WS  33  
#define I2S_SCK 25  
#define LED_PIN 2     // Built-in LED

// --- Audio Constants ---
#define SAMPLE_RATE 16000
#define SAMPLES 512
const double TARGET_FREQ = 2360.0;
const double TOLERANCE = 100.0;    
const double SNR_THRESHOLD = 30.0;
const double MIN_ABS_MAG = 500.0;

// --- Globals ---
SoyaConfig config;
SoyaPreferences prefs(config);
SoyaAccessPointManager apManager(prefs, config);

double vReal[SAMPLES];
double vImag[SAMPLES];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLE_RATE);

int beep_hits = 0;
unsigned long last_hit_time = 0;
const int TRIGGER_THRESHOLD = 5;

// --- I2S Setup ---
void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
        .mclk_multiple = I2S_MCLK_MULTIPLE_256, // Changed from DEFAULT to 256
        .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
    };
    
    // Fixed Order: mck_io_num must come first in the struct definition
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE, 
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

// --- SMS Notification ---
void sendSms() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    // Debug time status
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)){
        Serial.printf("Time: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
    } else {
        Serial.println("Time: Not Set (HTTPS might fail)");
    }

    HTTPClient http;
    http.setTimeout(5000); 

    // Construct URL
    String url = "https://voip.ms/api/v1/rest.php?method=sendSMS&api_username=" + String(config.api_user) + 
                 "&api_password=" + String(config.api_pass) + 
                 "&did=" + String(config.did) + 
                 "&dst=" + String(config.dst) + 
                 "&message=Soymilk+is+Ready";
                 
    http.begin(url);
    int code = http.GET();
    if(code > 0) Serial.println("SMS Sent");
    else Serial.printf("SMS Fail: %s\n", http.errorToString(code).c_str());
    http.end();
}

// --- Main Setup ---
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    // Load Preferences
    prefs.getPreferences(); 

    // Connect to WiFi or Start AP
    if (!WiFiConnect(config.hostname, config.ssid, config.password, 20)) {
        Serial.println("\nConnection Failed. Starting AP.");
        apManager.setup("Soyabell-Setup");
        
        apManager.runBlocking([](bool clientConnected) {
            static unsigned long last = 0;
            if (millis() - last > (clientConnected ? 200 : 1000)) {
                digitalWrite(LED_PIN, !digitalRead(LED_PIN));
                last = millis();
            }
        });
    }

    Serial.println("\nWiFi Connected.");
    
    // Sync Time for SSL
    TimeManager::begin(config.time_zone);
    
    Serial.println("Starting Listener.");
    digitalWrite(LED_PIN, LOW); 
    setupI2S();
}

// --- Main Loop ---
void loop() {
    size_t bytes_read;
    int32_t raw_samples[SAMPLES];
    
    // Read Audio
    i2s_read(I2S_NUM_0, &raw_samples, sizeof(raw_samples), &bytes_read, portMAX_DELAY);

    // Prepare FFT
    double total_mag = 0;
    for (int i = 0; i < SAMPLES; i++) {
        vReal[i] = (double)raw_samples[i];
        vImag[i] = 0;
    }

    // Run FFT
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();
    
    // Noise Floor Calculation
    for (int i = 2; i < (SAMPLES / 2); i++) total_mag += vReal[i];
    double avg_noise = total_mag / (SAMPLES / 2);

    // Peak Detection
    double peak = FFT.majorPeak();
    double peak_mag = vReal[(int)(peak * SAMPLES / SAMPLE_RATE)];
    double snr = peak_mag / (avg_noise + 1.0);

    // Frequency Matching
    if (peak > (TARGET_FREQ - TOLERANCE) && peak < (TARGET_FREQ + TOLERANCE)) {
        if (peak_mag > MIN_ABS_MAG && snr > SNR_THRESHOLD) {
            // Debounce hits
            if (millis() - last_hit_time > 800) { 
                Serial.printf("Beep! F:%.0f Mag:%.0f SNR:%.1f\n", peak, peak_mag, snr);
                beep_hits++;
                last_hit_time = millis();
                
                // Blink LED
                digitalWrite(LED_PIN, HIGH);
                delay(150);
                digitalWrite(LED_PIN, LOW);
            }
        }
    }

    // Reset logic if silence for too long
    if (beep_hits > 0 && (millis() - last_hit_time > 15000)) {
        beep_hits = 0;
        Serial.println("Reset hits (timeout)");
    }

    // Trigger Notification
    if (beep_hits >= TRIGGER_THRESHOLD) {
        Serial.println("Threshold Reached -> Sending Notification");
        sendSms();
        beep_hits = 0;
        
        // Cooldown to prevent spam
        delay(60000); 
    }
}