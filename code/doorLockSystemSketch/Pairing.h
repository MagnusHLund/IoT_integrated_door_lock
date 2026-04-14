#pragma once

#include "WiFiManager.h"
#include "MqttManager.h"
#include "Buzzer.h"

class Pairing {
  private:
    WiFiManager& wifiManager;
    MqttManager& mqttManager;
    Buzzer& buzzer;

    char* formatDiscoveryMessageJson(char* macAddress);

  public:
    Pairing(WiFiManager& wifiManager, MqttManager& mqttManager, Buzzer& buzzer);

    void init();
    void pairToHomeAssistant();
};