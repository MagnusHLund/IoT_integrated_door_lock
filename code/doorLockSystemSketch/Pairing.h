#pragma once

#include "WiFiManager.h"
#include "MqttManager.h"
#include "Buzzer.h"

class Pairing {
  private:
    WiFiManager& wiFiManager;
    MqttManager& mqttManager;
    Buzzer& buzzer;

    char* formatDiscoveryMessageJson(char* macAddress);

  public:
    Pairing(WiFiManager& wiFiManager, MqttManager& mqttManager, Buzzer& buzzer);

    void init();
    void pairToHomeAssistant();
};