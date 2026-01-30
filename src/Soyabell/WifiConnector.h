#ifndef ESP32WIFI_WIFICONNECTOR_H
#define ESP32WIFI_WIFICONNECTOR_H

bool WiFiConnect(const char *host, const char *ssid, const char *pass,
                 const int attempts);

#endif // ESP32WIFI_WIFICONNECTOR_H
