#pragma once

#include "wiFiManager.hpp"
#include "mqttManager.hpp"

class Pairing {
private:
    WiFiManager& wiFiManager;
    MqttManager& mqttManager;

    char* formatDiscoveryMessageJson(const char* macAddress);

public:
    Pairing(WiFiManager& wiFiManager, MqttManager& mqttManager);
    
    void pairToHomeAssistant();
};
