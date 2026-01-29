#include "wifi_connector.h"
#include <Arduino.h>
#include <WiFi.h>

bool WiFiConnect(const char *host, const char *ssid, const char *pass,
                 const int attempts) {
  delay(10);
  Serial.printf("------------\n");
  Serial.printf("Connecting to %s\n", ssid);

  WiFi.setHostname(host);
  wl_status_t stat = WiFi.begin(ssid, pass);
  Serial.printf("WiFi Status: %d\n", stat);

  unsigned waitCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf(".");
    waitCnt++;
    if (waitCnt > attempts) {
      Serial.printf("\nConnection timed out\n");
      return false;
    }
  }

  IPAddress ip = WiFi.localIP();
  Serial.printf("\nWiFi connected\n");
  Serial.printf("IP address: %s\n", ip.toString().c_str());

  return true;
}
