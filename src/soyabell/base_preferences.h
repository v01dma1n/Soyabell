#ifndef BASE_PREFERENCES_H
#define BASE_PREFERENCES_H

#include "enc_types.h"
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