#ifndef SOYABELL_SOYAAPMANAGER_H
#define SOYABELL_SOYAAPMANAGER_H

#include "BaseAccessPointManager.h"
#include "SoyaConfig.h"
#include "SoyaPreferences.h"

class SoyaAccessPointManager : public BaseAccessPointManager {
public:
    SoyaAccessPointManager(SoyaPreferences& prefs, SoyaConfig& config) 
        : BaseAccessPointManager(prefs), _soyaConfig(config) {}

protected:
    void initializeFormFields() override {
        BaseAccessPointManager::initializeFormFields();
        
        // Use explicit FormField{...} to fix compiler vector error
        _formFields.push_back(FormField{"api_user", "API Username", false, PREF_STRING, { .str_pref = _soyaConfig.api_user }, nullptr, 0});
        _formFields.push_back(FormField{"api_pass", "API Password", true, PREF_STRING, { .str_pref = _soyaConfig.api_pass }, nullptr, 0});
        _formFields.push_back(FormField{"did", "Source Number (DID)", false, PREF_STRING, { .str_pref = _soyaConfig.did }, nullptr, 0});
        _formFields.push_back(FormField{"dst", "Dest Number", false, PREF_STRING, { .str_pref = _soyaConfig.dst }, nullptr, 0});
    }
    
private:
    SoyaConfig& _soyaConfig;
};
#endif