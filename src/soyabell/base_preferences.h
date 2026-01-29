#ifndef BASE_PREFERENCES_H
#define BASE_PREFERENCES_H

#include "enc_types.h"
#include <Preferences.h>

#define PREF_NAMESPACE "config"

#define MAX_PREF_STRING_LEN 64

// The base configuration struct with only generic fields
struct BaseConfig {
  char ssid[MAX_PREF_STRING_LEN];
  char password[MAX_PREF_STRING_LEN];
  char time_zone[MAX_PREF_STRING_LEN];
  AppLogLevel logLevel;
};

// The base class for managing preferences
class BasePreferences {
public:
    // The constructor takes a reference to a config struct that it will manage.
    BasePreferences(BaseConfig& config);
    virtual ~BasePreferences() = default;

    // These methods are virtual so the application can extend them.
    virtual void setup();
    virtual void getPreferences();
    virtual void putPreferences();
    virtual void dumpPreferences();

    // Provides access to the managed config struct
    BaseConfig& getConfig() { return _config; }

protected:
    Preferences prefs;
    BaseConfig& _config;
};

#endif // BASE_PREFERENCES_H
