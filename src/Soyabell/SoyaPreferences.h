#ifndef SOYABELL_SOYAPREFERENCES_H
#define SOYABELL_SOYAPREFERENCES_H

#include "BasePreferences.h"
#include "SoyaConfig.h"

class SoyaPreferences : public BasePreferences {
public:
    SoyaPreferences(SoyaConfig& config) : BasePreferences(config), _soyaConfig(config) {}

    void getPreferences() override {
        BasePreferences::getPreferences(); // Load generic first
        _prefs.begin(PREF_NAMESPACE, true);
        _prefs.getString("api_user", _soyaConfig.api_user, MAX_PREF_STRING_LEN);
        _prefs.getString("api_pass", _soyaConfig.api_pass, MAX_PREF_STRING_LEN);
        _prefs.getString("did", _soyaConfig.did, MAX_PREF_STRING_LEN);
        _prefs.getString("dst", _soyaConfig.dst, MAX_PREF_STRING_LEN);
        _prefs.end();
    }

    void putPreferences() override { 
        BasePreferences::putPreferences(); // Save generic first
        _prefs.begin(PREF_NAMESPACE, false);
        _prefs.putString("api_user", _soyaConfig.api_user);
        _prefs.putString("api_pass", _soyaConfig.api_pass);
        _prefs.putString("did", _soyaConfig.did);
        _prefs.putString("dst", _soyaConfig.dst);
        _prefs.end();
    }

private:
    SoyaConfig& _soyaConfig;
};
#endif