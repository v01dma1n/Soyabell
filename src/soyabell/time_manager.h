#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include "enc_types.h"

class TimeManager {
public:
    static void begin(const char* timezone) {
        if (strlen(timezone) > 0) {
            // Configures system time using standard ESP32 SNTP
            configTzTime(timezone, "pool.ntp.org", "time.nist.gov");
        }
    }

    static bool isTimeSet() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) return false;
        return timeinfo.tm_year > (2016 - 1900);
    }

    static void syncBlocking(int timeout_sec = 10) {
        Serial.print("Syncing Time");
        int count = 0;
        while (!isTimeSet() && count < timeout_sec * 2) {
            Serial.print(".");
            delay(500);
            count++;
        }
        Serial.println(isTimeSet() ? " OK" : " Fail");
    }
};

#endif