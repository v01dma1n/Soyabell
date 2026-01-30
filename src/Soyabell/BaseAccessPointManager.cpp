#include "BaseAccessPointManager.h"
#include "BasePreferences.h"
#include "EncDebug.h"
#include "TzData.h"
#include "LogLevelData.h"

// Global pointer for WiFi event
static BaseAccessPointManager* _baseApInstance = nullptr;

void onWifiEvent(WiFiEvent_t event) {
    if (!_baseApInstance) return;
    if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
        _baseApInstance->setClientConnected(true);
    } else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
        _baseApInstance->setClientConnected(false);
    }
}

BaseAccessPointManager::BaseAccessPointManager(BasePreferences& prefs)
    : _prefs(prefs), _server(80) {
    _baseApInstance = this;
}

void BaseAccessPointManager::setup(const char* hostName) {
    _pageTitle = String(hostName) + " Settings";
    initializeFormFields();
    setupServer();

    ENC_LOG("Setting up AP Mode");
    WiFi.onEvent(onWifiEvent);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostName);

    ENC_LOG("AP IP: %s", WiFi.softAPIP().toString().c_str());
    _dnsServer.start(53, "*", WiFi.softAPIP());
    _server.begin();
    ENC_LOG("Base AP Setup Complete.");
}

void BaseAccessPointManager::loop() {
    _dnsServer.processNextRequest();
}

void BaseAccessPointManager::runBlocking(std::function<void(bool)> callback) {
    while (true) {
        loop();
        if(callback) callback(_isClientConnected);
        delay(10);
    }
}

void BaseAccessPointManager::setupServer() {
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", assembleHtml());
    });
    
    _server.on("/get", HTTP_GET, [this](AsyncWebServerRequest *request) {
        bool restart = false;
        for (FormField &field : _formFields) {
            if (field.prefType == PREF_BOOL) {
                *(field.pref.bool_pref) = request->hasParam(field.name);
                restart = true;
            } else {
                if (request->hasParam(field.name)) {
                    String val = request->getParam(field.name)->value();
                    if (field.isMasked && (val.isEmpty() || val == PASSWORD_MASKED)) continue;
                    
                    if (field.prefType == PREF_STRING) {
                        strncpy(field.pref.str_pref, val.c_str(), MAX_PREF_STRING_LEN - 1);
                        field.pref.str_pref[MAX_PREF_STRING_LEN - 1] = '\0';
                    } else if (field.prefType == PREF_ENUM) {
                        *(field.pref.int_pref) = val.toInt();
                    } else if (field.prefType == PREF_SELECT) {
                        strncpy(field.pref.str_pref, val.c_str(), MAX_PREF_STRING_LEN - 1);
                    }
                    restart = true;
                }
            }
        }
        if (restart) {
            _prefs.putPreferences();
            request->send(200, "text/html", "Settings saved. Restarting...");
            delay(1000);
            ESP.restart();
        }
        request->send(200, "text/html", "No changes detected.");
    });
}

void BaseAccessPointManager::initializeFormFields() {
    _formFields.clear();
    
    // Explicit FormField constructor calls to fix compiler ambiguity
    _formFields.push_back(FormField{"WiFiSSIDInput", "WiFi SSID", false, PREF_STRING, {.str_pref = _prefs.getConfig().ssid}, nullptr, 0});
    
    _formFields.push_back(FormField{"PasswordInput", "Password", true, PREF_STRING, {.str_pref = _prefs.getConfig().password}, nullptr, 0});
    
    _formFields.push_back(FormField{"TimeZoneInput", "Time Zone", false, PREF_SELECT, 
       {.str_pref = _prefs.getConfig().time_zone}, timezones, num_timezones});

    _formFields.push_back(FormField{"logLevel", "Log Level", false, PREF_ENUM, 
       {.int_pref = reinterpret_cast<int32_t*>(&_prefs.getConfig().logLevel)}, logLevels, numLogLevels});
}

String BaseAccessPointManager::generateForm() {
    String form = "<form action=\"/get\"><table>";
    for (const auto& field : _formFields) {
        form += "<tr><td>" + String(field.name) + ":</td>";
        if (field.prefType == PREF_STRING || field.prefType == PREF_SELECT) {
             if(field.prefType == PREF_SELECT) {
                form += "<td><select name=\"" + String(field.name) + "\">";
                for(int i=0; i<field.num_select_options; i++) {
                   bool sel = strcmp(field.pref.str_pref, field.select_options[i].value) == 0;
                   form += "<option value='" + String(field.select_options[i].value) + "'" + (sel ? " selected" : "") + ">" + String(field.select_options[i].name) + "</option>";
                }
                form += "</select></td>";
             } else {
                String val = field.isMasked && strlen(field.pref.str_pref) > 0 ? PASSWORD_MASKED : String(field.pref.str_pref);
                form += "<td><input type=\"" + String(field.isMasked ? "password" : "text") + "\" name=\"" + String(field.name) + "\" value=\"" + val + "\"></td>";
             }
        } else if (field.prefType == PREF_BOOL) {
             form += "<td><input type=\"checkbox\" name=\"" + String(field.name) + "\" " + (*(field.pref.bool_pref) ? "checked" : "") + "></td>";
        } else if (field.prefType == PREF_ENUM) {
             form += "<td><select name=\"" + String(field.name) + "\">";
             for(int i=0; i<field.num_select_options; i++) {
                bool sel = String(*(field.pref.int_pref)) == field.select_options[i].value;
                form += "<option value='" + String(field.select_options[i].value) + "'" + (sel ? " selected" : "") + ">" + String(field.select_options[i].name) + "</option>";
             }
             form += "</select></td>";
        }
        form += "</tr>";
    }
    form += "<tr><td colspan=\"2\"><input type=\"submit\" value=\"Save and Restart\"></td></tr></table></form>";
    return form;
}

String BaseAccessPointManager::generateJavascript() { return ""; } 
String BaseAccessPointManager::assembleHtml() {
    String html = R"(<!DOCTYPE html><html><head><title>%PAGE_TITLE%</title>
    <style>body{background-color:#f0e68c;font-family:serif;text-align:center}table{margin:auto}</style>
    </head><body><h1>%PAGE_TITLE%</h1>%FORM_PLACEHOLDER%</body></html>)";
    html.replace("%PAGE_TITLE%", _pageTitle);
    html.replace("%FORM_PLACEHOLDER%", generateForm());
    return html;
}