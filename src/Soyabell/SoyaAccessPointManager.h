#ifndef SOYABELL_SOYAACCESSPOINTMANAGER_H
#define SOYABELL_SOYAACCESSPOINTMANAGER_H

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
        
        // Add VOIP.ms fields
        _formFields.push_back({"api_user", "API Username", false, PREF_STRING, { .str_pref = _soyaConfig.api_user }, nullptr, 0});
        _formFields.push_back({"api_pass", "API Password", true, PREF_STRING, { .str_pref = _soyaConfig.api_pass }, nullptr, 0});
        _formFields.push_back({"did", "Source Number (DID)", false, PREF_STRING, { .str_pref = _soyaConfig.did }, nullptr, 0});
        _formFields.push_back({"dst", "Dest Number", false, PREF_STRING, { .str_pref = _soyaConfig.dst }, nullptr, 0});   }
    
private:
    SoyaConfig& _soyaConfig;
};
#endif