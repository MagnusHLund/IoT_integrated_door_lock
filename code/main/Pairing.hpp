#pragma once

#include "WiFiManager.hpp"
#include "MqttManager.hpp"
#include "Buzzer.hpp"

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