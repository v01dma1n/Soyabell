#ifndef ESP32WIFI_BASEPREFERENCES_H
#define ESP32WIFI_BASEPREFERENCES_H

#include "EncTypes.h"
#include <Preferences.h>

class BasePreferences {
public:
    BasePreferences(BaseConfig& config);
    virtual ~BasePreferences() = default;

    virtual void setup();
    virtual void getPreferences();
    virtual void putPreferences();
    virtual void dumpPreferences();
    
    BaseConfig& getConfig() { return _config; }

protected:
    Preferences _prefs;
    BaseConfig& _config;
};

#endif