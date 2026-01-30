#include "WifiConnector.h"
#include <Arduino.h>
#include <WiFi.h>

bool WiFiConnect(const char *host, const char *ssid, const char *pass, const int attempts) {
  delay(10);
  Serial.printf("------------\nConnecting to %s\n", ssid);
  WiFi.setHostname(host);
  WiFi.begin(ssid, pass);
  
  unsigned waitCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf(".");
    if (++waitCnt > attempts) {
      Serial.printf("\nConnection timed out\n");
      return false;
    }
  }
  Serial.printf("\nWiFi connected\nIP address: %s\n", WiFi.localIP().toString().c_str());
  return true;
}