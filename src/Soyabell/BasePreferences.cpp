#include "BasePreferences.h"
#include "EncDebug.h"

#define APP_PREF_WIFI_SSID "wifi_ssid"
#define APP_PREF_PASSWORD "password"
#define APP_PREF_TIME_ZONE "time_zone"
#define APP_PREF_LOG_LEVEL "log_level"

BasePreferences::BasePreferences(BaseConfig& config) : _config(config) {}

void BasePreferences::setup() {
  _prefs.begin(PREF_NAMESPACE);
  getPreferences();
}

void BasePreferences::getPreferences() {
  _prefs.begin(PREF_NAMESPACE, true);
  _prefs.getString(APP_PREF_WIFI_SSID, _config.ssid, MAX_PREF_STRING_LEN);
  _prefs.getString(APP_PREF_PASSWORD, _config.password, MAX_PREF_STRING_LEN);
  _prefs.getString(APP_PREF_TIME_ZONE, _config.time_zone, MAX_PREF_STRING_LEN);
  _config.logLevel = static_cast<AppLogLevel>(_prefs.getInt(APP_PREF_LOG_LEVEL, APP_LOG_INFO));
  _prefs.end();
}

void BasePreferences::putPreferences() {
  _prefs.begin(PREF_NAMESPACE, false);
  _prefs.putString(APP_PREF_WIFI_SSID, _config.ssid);
  _prefs.putString(APP_PREF_PASSWORD, _config.password);
  _prefs.putString(APP_PREF_TIME_ZONE, _config.time_zone);
  _prefs.putInt(APP_PREF_LOG_LEVEL, _config.logLevel);
  _prefs.end();
}

void BasePreferences::dumpPreferences() {
  ENC_LOG("Pref=%s: %s", APP_PREF_WIFI_SSID, _config.ssid);
  ENC_LOG("Pref=%s: %s", APP_PREF_PASSWORD, "***");
  ENC_LOG("Pref=%s: %s", APP_PREF_TIME_ZONE, _config.time_zone);
  ENC_LOG("Pref=%s: %d", APP_PREF_LOG_LEVEL, _config.logLevel);
}