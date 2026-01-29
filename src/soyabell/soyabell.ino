#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

#define I2S_SD  32  
#define I2S_WS  33  
#define I2S_SCK 25  
#define LED_PIN 2    
#define SAMPLE_RATE 16000
#define SAMPLES 512

// --- Calibrated Settings from basement samples ---
const double TARGET_FREQ = 2360.0; 
const double TOLERANCE = 100.0;    
const double SNR_THRESHOLD = 30.0;  // Increased based on ~80 SNR hits
const double MIN_ABS_MAG = 500.0;   // Adjusted to match  scaled plotter output

int beep_hits = 0;
const int TRIGGER_THRESHOLD = 5;    
unsigned long last_hit_time = 0;
const int MIN_BEEP_GAP = 800; // Rhythm filter to ignore other tones

double vReal[SAMPLES];
double vImag[SAMPLES];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLE_RATE);

void sendSms() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(3000); // Prevent long freezes
    String url = "https://voip.ms/api/v1/rest.php?method=sendSMS&api_username=" + api_user + 
                 "&api_password=" + api_pass + "&did=" + did + "&dst=" + dst + 
                 "&message=Soymilk+is+Ready";
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) Serial.println("SMS Processed");
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  WiFi.begin(ssid, password);
  
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void loop() {
  size_t bytes_read;
  int32_t raw_samples[SAMPLES];
  i2s_read(I2S_NUM_0, &raw_samples, sizeof(raw_samples), &bytes_read, portMAX_DELAY);

  double total_mag = 0;
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = (double)raw_samples[i];
    vImag[i] = 0;
  }

  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
  
  for (int i = 2; i < (SAMPLES / 2); i++) {
    total_mag += vReal[i];
  }
  double avg_noise = total_mag / (SAMPLES / 2);

  double peak = FFT.majorPeak();
  double peak_mag = vReal[(int)(peak * SAMPLES / SAMPLE_RATE)];
  double snr = peak_mag / (avg_noise + 1.0);

  // Serial Plotter Output: Frequency, Magnitude (scaled), SNR
  Serial.print(millis()); Serial.print(",");
  Serial.print(peak); Serial.print(",");
  Serial.print(peak_mag); Serial.print(",");
  Serial.println(snr);

  if (peak > (TARGET_FREQ - TOLERANCE) && peak < (TARGET_FREQ + TOLERANCE)) {
    if (peak_mag > MIN_ABS_MAG && snr > SNR_THRESHOLD) {
      if (millis() - last_hit_time > MIN_BEEP_GAP) {
        Serial.println("Beep Hit!");
        beep_hits++;
        last_hit_time = millis();
        
        // Visual confirmation on ESP32 LED
        digitalWrite(LED_PIN, HIGH);
        delay(150); 
        digitalWrite(LED_PIN, LOW);
      }
    }
  }

  if (beep_hits > 0 && (millis() - last_hit_time > 15000)) {
    Serial.println("Beep Hits Timeout");
    beep_hits = 0;
  }

  if (beep_hits >= TRIGGER_THRESHOLD) {
    Serial.println("Sending SMS");
    sendSms();
    beep_hits = 0;
    delay(60000); 
  }
}