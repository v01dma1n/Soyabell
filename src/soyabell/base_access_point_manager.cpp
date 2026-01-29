#include "base_access_point_manager.h"
#include "base_preferences.h"
#include "enc_debug.h"
#include "tz_data.h"
#include "log_level_data.h"
#include "display_manager.h"      
#include "anim_scrolling_text.h"  

#include <WiFi.h>
#include <DNSServer.h> 
#include <memory>                 

static BaseAccessPointManager* _baseApInstance = nullptr;

void onWifiEvent(WiFiEvent_t event) {
    if (!_baseApInstance) return;
    // The friend declaration in the header allows this global function
    // to call a protected method on the instance.
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

// The destructor is defaulted in the header, so the empty body is removed from here.

void BaseAccessPointManager::setup(const char* hostName) {
    _pageTitle = String(hostName) + " Settings";
    initializeFormFields();
    setupServer();

    ENC_LOG("Setting up AP Mode");
    WiFi.onEvent(onWifiEvent); // Register the event handler
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostName);

    ENC_LOG("AP IP address: %s", WiFi.softAPIP().toString().c_str());
    _dnsServer.start(DNS_DEFAULT_PORT, "*", WiFi.softAPIP());
    _server.begin();
    ENC_LOG("Base AP Setup Complete.");
}

void BaseAccessPointManager::loop() {
    _dnsServer.processNextRequest();
}

void BaseAccessPointManager::runBlockingLoop(DisplayManager& display, const char* waitingMsg, const char* connectedMsg) {
    enum ApDisplayState { WAITING_FOR_CLIENT, CLIENT_CONNECTED };
    ApDisplayState apState = WAITING_FOR_CLIENT;

    auto waitingAnimation = std::make_unique<ScrollingTextAnimation>(waitingMsg);
    display.setAnimation(std::move(waitingAnimation));

    while (true) {
        if (apState == WAITING_FOR_CLIENT && isClientConnected()) {
            apState = CLIENT_CONNECTED;
            auto connectedAnimation = std::make_unique<ScrollingTextAnimation>(connectedMsg);
            display.setAnimation(std::move(connectedAnimation));
        } else if (apState == CLIENT_CONNECTED && !isClientConnected()) {
            apState = WAITING_FOR_CLIENT;
            auto waitingAnimationRetry = std::make_unique<ScrollingTextAnimation>(waitingMsg);
            display.setAnimation(std::move(waitingAnimationRetry));
        }

        display.update();
        loop(); // Process DNS requests
        delay(10);
    }
}

void BaseAccessPointManager::setupServer() {
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", assembleHtml());
    });

    _server.on("/get", HTTP_GET, [this](AsyncWebServerRequest *request) {
        ENC_LOG("Received new settings via AP.");
        bool restart = false;

        for (FormField &field : _formFields) {
            if (field.prefType == PREF_BOOL) {
                bool isChecked = request->hasParam(field.name);
                *(field.pref.bool_pref) = isChecked;
                restart = true;
            } else {
                if (request->hasParam(field.name)) {
                    String val = request->getParam(field.name)->value();
                    if (field.isMasked && (val.isEmpty() || val == PASSWORD_MASKED)) {
                        continue;
                    }
                    if (field.prefType == PREF_STRING || field.prefType == PREF_SELECT) {
                        strncpy(field.pref.str_pref, val.c_str(), MAX_PREF_STRING_LEN - 1);
                        field.pref.str_pref[MAX_PREF_STRING_LEN - 1] = '\0';
                    } else if (field.prefType == PREF_ENUM) {
                        *(field.pref.int_pref) = val.toInt();
                    }
                    restart = true;
                }
            }
        }

        if (restart) {
            ENC_LOG("Settings saved. Restarting device.");
            _prefs.putPreferences();
            delay(200);
            ESP.restart();
        }
        request->send(200, "text/html", "Settings saved. The device will now restart.");
    });
}

// This is the base implementation that only adds the generic fields.
// The subclass will call this and then add its own specific fields.
void BaseAccessPointManager::initializeFormFields() {
    _formFields.clear(); // Ensure the vector is empty before starting

    FormField wifiSsidField;
    wifiSsidField.id = "WiFiSSIDInput";
    wifiSsidField.name = "WiFi SSID";
    wifiSsidField.isMasked = false;
    wifiSsidField.prefType = PREF_STRING;
    wifiSsidField.pref.str_pref = _prefs.getConfig().ssid;
    _formFields.push_back(wifiSsidField);

    FormField passwordField;
    passwordField.id = "PasswordInput";
    passwordField.name = "Password";
    passwordField.isMasked = true;
    passwordField.prefType = PREF_STRING;
    passwordField.pref.str_pref = _prefs.getConfig().password;
    _formFields.push_back(passwordField);

    FormField timeZoneField;
    timeZoneField.id = "TimeZoneInput";
    timeZoneField.name = "Time Zone";
    timeZoneField.isMasked = false;
    timeZoneField.prefType = PREF_SELECT;
    timeZoneField.pref.str_pref = _prefs.getConfig().time_zone;
    timeZoneField.select_options = timezones;
    timeZoneField.num_select_options = num_timezones;
    _formFields.push_back(timeZoneField);

    FormField logLevelField;
    logLevelField.id = "logLevel";
    logLevelField.name = "Log Level";
    logLevelField.isMasked = false;
    logLevelField.prefType = PREF_ENUM;
    logLevelField.pref.int_pref = reinterpret_cast<int32_t*>(&_prefs.getConfig().logLevel);
    _formFields.push_back(logLevelField);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>%PAGE_TITLE%</title>
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Old+Standard+TT&display=swap"> 
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Cormorant+Garamond&display=swap">
    %JAVASCRIPT_PLACEHOLDER%
    <style>
        body {
            background-image: url('https://plus.unsplash.com/premium_photo-1667761637876-e704c906927d'); 
            background-color: #f0e68c;
            background-repeat: no-repeat;
            background-attachment: fixed;
            background-size: cover;
        }
        h1:first-child {
            font-family: "Cormorant Garamond", "Old Standard TT", "Times New Roman", serif;
            text-align: center;
            font-weight: bold;
        }
        td:first-child, input[type="submit"] {
            text-align: right;
            font-family: "Cormorant Garamond", "Old Standard TT", "Times New Roman", serif;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>%PAGE_TITLE%</h1>
    %FORM_PLACEHOLDER%
</body>
</html>)rawliteral";


String BaseAccessPointManager::generateForm() {
  String form = "<form action=\"/get\"><table>";
  for (const auto& field : _formFields) {
    form += "<tr>";
    form += "<td>" + String(field.name) + ":</td>";
    if (field.prefType == PREF_STRING) {
        String inputTag = "<input type=\"";
        inputTag += (field.isMasked ? "password" : "text");
        inputTag += "\" name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\"";
        if (field.isMasked && strlen(field.pref.str_pref) > 0) {
            inputTag += " value=\"" + String(PASSWORD_MASKED) + "\"";
        }
        inputTag += ">";
        form += "<td>" + inputTag + "</td>";
    } else if (field.prefType == PREF_BOOL) {
      form += "<td><input type=\"checkbox\" name=\"" + String(field.name) + 
              "\" id=\"" + String(field.id) + "\" " +
              (*(field.pref.bool_pref) ? "checked" : "") + "></td>";
    } else if (field.prefType == PREF_ENUM) {
      form += "<td><select name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\">";
      String savedValue = String(*(field.pref.int_pref));
      for (int j = 0; j < numLogLevels; ++j) {
          form += "<option value=\"" + String(logLevels[j].value) + "\"";
          if (savedValue == logLevels[j].value) {
              form += " selected";
          }
          form += ">" + String(logLevels[j].name) + "</option>";
      }
      form += "</select></td>";
    } else if (field.prefType == PREF_SELECT) {
        form += "<td><select name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\">";
        for (int j = 0; j < field.num_select_options; ++j) {
            form += "<option value=\"" + String(field.select_options[j].value) + "\"";
            if (strcmp(field.pref.str_pref, field.select_options[j].value) == 0) {
                form += " selected";
            }
            form += ">" + String(field.select_options[j].name) + "</option>";
        }
        form += "</select></td>";
    }
    form += "</tr>";
  }
  form += "<tr><td colspan=\"2\"><input type=\"submit\" value=\"Save and Restart\"></td></tr>";
  form += "</table></form>";
  return form;
}

String BaseAccessPointManager::generateJavascript() {
    String script = "<script>window.onload = function() {";
    for (size_t i = 0; i < _formFields.size(); ++i) {
        const FormField &field = _formFields[i];
        if (field.isMasked) continue;
        script += "const e" + String(i) + " = document.getElementById(\"" + String(field.id) + "\");";
        if (field.prefType == PREF_STRING) {
            script += "e" + String(i) + ".value = \"" + String(field.pref.str_pref) + "\";";
        } else if (field.prefType == PREF_BOOL) {
            script += "e" + String(i) + ".checked = " + String(*(field.pref.bool_pref) ? "true" : "false") + ";";
        } else if (field.prefType == PREF_ENUM) {
            script += "e" + String(i) + ".value = \"" + String(*(field.pref.int_pref)) + "\";";
        }
    }
    script += "};</script>";
    return script;
}

String BaseAccessPointManager::assembleHtml() {
    String html = String(index_html);

    html.replace("%PAGE_TITLE%", _pageTitle); 

    // Replace Javascript Placeholder
    String generatedJavascript = generateJavascript();
    html.replace("%JAVASCRIPT_PLACEHOLDER%", generatedJavascript);

    // Generate and replace Form Placeholder
    String formHtml = generateForm();
    html.replace("%FORM_PLACEHOLDER%", formHtml);

    return html;
}


