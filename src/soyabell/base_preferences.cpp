#include "base_preferences.h"
#include "enc_debug.h"

#define APP_PREF_WIFI_SSID "wifi_ssid"
#define APP_PREF_PASSWORD "password"
#define APP_PREF_TIME_ZONE "time_zone"
#define APP_PREF_LOG_LEVEL "log_level"

BasePreferences::BasePreferences(BaseConfig& config) : _config(config) {}

void BasePreferences::setup() {
  prefs.begin(PREF_NAMESPACE);
  getPreferences();
}

void BasePreferences::getPreferences() {
  prefs.begin(PREF_NAMESPACE, true); // true = read-only
  prefs.getString(APP_PREF_WIFI_SSID, _config.ssid, MAX_PREF_STRING_LEN);
  prefs.getString(APP_PREF_PASSWORD, _config.password, MAX_PREF_STRING_LEN);
  prefs.getString(APP_PREF_TIME_ZONE, _config.time_zone, MAX_PREF_STRING_LEN);

  int32_t savedLogLevel = prefs.getInt(APP_PREF_LOG_LEVEL, APP_LOG_INFO);
  _config.logLevel = static_cast<AppLogLevel>(savedLogLevel);
  prefs.end();
}

void BasePreferences::putPreferences() {
  prefs.begin(PREF_NAMESPACE, false); // false = read-write
  prefs.putString(APP_PREF_WIFI_SSID, _config.ssid);
  prefs.putString(APP_PREF_PASSWORD, _config.password);
  prefs.putString(APP_PREF_TIME_ZONE, _config.time_zone);
  prefs.putInt(APP_PREF_LOG_LEVEL, _config.logLevel);
  prefs.end(); // close the connection to the storage namespace 
}

void BasePreferences::dumpPreferences() {
  ENC_LOG("Pref=%s: %s", APP_PREF_WIFI_SSID, _config.ssid);
  ENC_LOG("Pref=%s: %s", APP_PREF_PASSWORD, "***");
  ENC_LOG("Pref=%s: %s", APP_PREF_TIME_ZONE, _config.time_zone);
  ENC_LOG("Pref=%s: %d", APP_PREF_LOG_LEVEL, _config.logLevel);
}
