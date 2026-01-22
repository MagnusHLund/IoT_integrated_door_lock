#pragma once

#include "mqtt_client.h"
#include "esp_log.h"
#include "wiFiManager.hpp"
#include <functional>

class MqttManager {
private:
    char stateTopic[64];      // State of the Arduino
    char commandTopic[64];    // State change from Home Assistant
    char discoveryTopic[64];  // Used for Home Assistant discovery
    
    esp_mqtt_client_handle_t client;
    WiFiManager& wiFiManager;
    
    const char* serverHostname;
    int serverPort;
    const char* mqttUsername;
    const char* mqttPassword;
    
    bool useTLS;
    const uint8_t* caCert;
    int caCertLen;
    
    bool connected;
    
    // Callback function pointer for message handling
    std::function<void(char*, uint8_t*, unsigned int)> messageCallback;
    
    static void mqttEventHandler(void* handlerArgs, esp_event_base_t base,
                                int32_t eventId, void* eventData);

public:
    MqttManager(const char* serverHostname, int serverPort, 
                const char* mqttUsername, const char* mqttPassword,
                WiFiManager& wiFiManager);
    
    ~MqttManager();
    
    void enableTLS(const uint8_t* caCert, int caCertLen);
    void connect();
    void awaitConnectivity();
    void setupTopics(const char* macAddress);
    const char* getDiscoveryTopic();
    const char* getStateTopic();
    const char* getCommandTopic();
    void publishMessage(const char* message, const char* topic = nullptr);
    void setCallback(std::function<void(char*, uint8_t*, unsigned int)> callback);
};