#pragma once

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFiManager.h"
#include "WiFi.h"

class MqttManager {
  private:
    char stateTopic[64]; // Current state of the lock
    char commandTopic[64]; // State change from Home Assistant
    char discoveryTopic[64]; // Used for Home Assistant discovery
    char codeSubmissionTopic[64]; // Used for submitting codes from the keypad

    WiFiClient wifiClient;
    PubSubClient client;

    WiFiManager& wifiManager;

    const char* serverHostname;
    int serverPort;
    const char* mqttUsername;
    const char* mqttPassword;

  public:
    MqttManager(const char* serverHostname, int serverPort, const char* mqttUsername, const char* mqttPassword, WiFiManager& wifiManager);
    void connect();
    void setupTopics(const char* macAddress);
    const char* getDiscoveryTopic();
    const char* getStateTopic();
    const char* getCommandTopic();
    const char* getCodeSubmissionTopic();
    void ensureConnectivity();
    void publishMessage(const char* message, const char* topic, bool retain = true);
    void setCallback(void (*callback)(char* topic, byte* payload, unsigned int length));
};