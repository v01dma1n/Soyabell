#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

bool WiFiConnect(const char *host, const char *ssid, const char *pass,
                 const int attempts);

#endif // WIFI_CONNECT_H
