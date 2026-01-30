#include "SoyabellManager.h"

SoyabellManager::SoyabellManager() 
    : _prefs(_config), 
      _apManager(_prefs, _config),
      _FFT(_vReal, _vImag, SAMPLES, SAMPLE_RATE),
      _beep_hits(0),
      _last_hit_time(0) 
{
}

void SoyabellManager::begin() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Load Configuration
    _prefs.getPreferences();

    // WiFi Connection Strategy
    if (!WiFiConnect(_config.hostname, _config.ssid, _config.password, 20)) {
        Serial.println("\nConnection Failed. Starting AP.");
        _apManager.setup("Soyabell-Setup");
        
        // AP Mode Blocking Loop
        _apManager.runBlocking([](bool clientConnected) {
            static unsigned long last = 0;
            if (millis() - last > (clientConnected ? 200 : 1000)) {
                digitalWrite(LED_PIN, !digitalRead(LED_PIN));
                last = millis();
            }
        });
    }

    Serial.println("\nWiFi Connected.");
    
    // Time Sync
    TimeManager::begin(_config.time_zone);
    
    Serial.println("Starting Listener.");
    setupI2S();
}

void SoyabellManager::setupI2S() {
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
        .mclk_multiple = I2S_MCLK_MULTIPLE_256, 
        .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
    };
    
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

void SoyabellManager::sendSms() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    // Time Debug
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)){
        Serial.printf("Time: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
    } else {
        Serial.println("Time: Not Set (HTTPS might fail)");
    }

    HTTPClient http;
    http.setTimeout(5000); 

    String url = "https://voip.ms/api/v1/rest.php?method=sendSMS&api_username=" + String(_config.api_user) + 
                 "&api_password=" + String(_config.api_pass) + 
                 "&did=" + String(_config.did) + 
                 "&dst=" + String(_config.dst) + 
                 "&message=Soymilk+is+Ready";
                 
    http.begin(url);
    int code = http.GET();
    if(code > 0) Serial.println("SMS Sent");
    else Serial.printf("SMS Fail: %s\n", http.errorToString(code).c_str());
    http.end();
}

void SoyabellManager::blinkLed() {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
}

void SoyabellManager::loop() {
    size_t bytes_read;
    int32_t raw_samples[SAMPLES];
    
    // 1. Read Audio
    i2s_read(I2S_NUM_0, &raw_samples, sizeof(raw_samples), &bytes_read, portMAX_DELAY);

    // 2. Prepare FFT
    double total_mag = 0;
    for (int i = 0; i < SAMPLES; i++) {
        _vReal[i] = (double)raw_samples[i];
        _vImag[i] = 0;
    }

    // 3. Compute FFT
    _FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    _FFT.compute(FFT_FORWARD);
    _FFT.complexToMagnitude();
    
    // 4. Analyze Data
    for (int i = 2; i < (SAMPLES / 2); i++) total_mag += _vReal[i];
    double avg_noise = total_mag / (SAMPLES / 2);

    double peak = _FFT.majorPeak();
    double peak_mag = _vReal[(int)(peak * SAMPLES / SAMPLE_RATE)];
    double snr = peak_mag / (avg_noise + 1.0);

    // 5. Logic
    if (peak > (TARGET_FREQ - TOLERANCE) && peak < (TARGET_FREQ + TOLERANCE)) {
        if (peak_mag > MIN_ABS_MAG && snr > SNR_THRESHOLD) {
            if (millis() - _last_hit_time > 800) { 
                Serial.printf("Beep! F:%.0f Mag:%.0f SNR:%.1f\n", peak, peak_mag, snr);
                _beep_hits++;
                _last_hit_time = millis();
                blinkLed();
            }
        }
    }

    // Timeout Reset
    if (_beep_hits > 0 && (millis() - _last_hit_time > 15000)) {
        _beep_hits = 0;
        Serial.println("Reset hits (timeout)");
    }

    // Trigger
    if (_beep_hits >= TRIGGER_THRESHOLD) {
        Serial.println("Threshold Reached -> Sending Notification");
        sendSms();
        _beep_hits = 0;
        delay(60000); 
    }
}