#ifndef BASE_ACCESS_POINT_MANAGER_H
#define BASE_ACCESS_POINT_MANAGER_H

#include "enc_types.h"

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <DNSServer.h> 
#include <vector>


#define PASSWORD_MASKED "********" 

class DisplayManager;
class BasePreferences;

class BaseAccessPointManager {
public:
    BaseAccessPointManager(BasePreferences& prefs);
    virtual ~BaseAccessPointManager() = default;

    virtual void setup(const char* hostName);
    void loop();
    bool isClientConnected() const { return _isClientConnected; } 
    void runBlockingLoop(DisplayManager& display, const char* waitingMsg, const char* connectedMsg);


protected:
    friend void onWifiEvent(WiFiEvent_t event); // Allow callback to access private members
    void setClientConnected(bool connected) { _isClientConnected = connected; } 

    BasePreferences& _prefs;
    std::vector<FormField> _formFields;
    virtual void initializeFormFields();

private:
    DNSServer _dnsServer;
    AsyncWebServer _server;
    bool _isClientConnected = false;
    String _pageTitle;

    String generateForm();
    String generateJavascript();
    String assembleHtml();
    void setupServer();
};

#endif // BASE_ACCESS_POINT_MANAGER_H
