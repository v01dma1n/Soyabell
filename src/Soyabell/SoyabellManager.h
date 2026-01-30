#ifndef SOYABELL_SOYABELLMANAGER_H
#define SOYABELL_SOYABELLMANAGER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "SoyaConfig.h"
#include "SoyaPreferences.h"
#include "SoyaAccessPointManager.h"
#include "WifiConnector.h"
#include "TimeManager.h"

// Hardware Constants
#define I2S_SD  32
#define I2S_WS  33
#define I2S_SCK 25
#define LED_PIN 2 

// Audio Constants
#define SAMPLES 512
#define SAMPLE_RATE 16000

class SoyabellManager {
public:
    SoyabellManager();
    void begin();
    void loop();

private:
    // Core Infrastructure
    SoyaConfig _config;
    SoyaPreferences _prefs;
    SoyaAccessPointManager _apManager;

    // FFT & Audio
    double _vReal[SAMPLES];
    double _vImag[SAMPLES];
    ArduinoFFT<double> _FFT;

    // Detection State
    int _beep_hits;
    unsigned long _last_hit_time;

    // Detection Constants
    const double TARGET_FREQ = 2360.0;
    const double TOLERANCE = 100.0;
    const double SNR_THRESHOLD = 30.0;
    const double MIN_ABS_MAG = 500.0;
    const int TRIGGER_THRESHOLD = 5;

    // Private Helpers
    void setupI2S();
    void sendSms();
    void blinkLed();
};

#endif