#include "pairing.hpp"
#include "esp_log.h"
#include <cstring>
#include <stdio.h>

static const char* TAG = "Pairing";

Pairing::Pairing(WiFiManager& wiFiManager, MqttManager& mqttManager)
    : wiFiManager(wiFiManager), mqttManager(mqttManager) {
}

void Pairing::pairToHomeAssistant() {
    const char* macAddress = wiFiManager.getMacAddress(true);
    char* message = formatDiscoveryMessageJson(macAddress);
    if (message) {
        mqttManager.publishMessage(message, mqttManager.getDiscoveryTopic());
        ESP_LOGI(TAG, "Sent discovery message");
        free(message);
    }
}

char* Pairing::formatDiscoveryMessageJson(const char* macAddress) {
    const char* commandTopic = mqttManager.getCommandTopic();
    const char* stateTopic = mqttManager.getStateTopic();

    // Manually construct JSON string
    char* buffer = (char*)malloc(512);
    if (!buffer) return nullptr;

    snprintf(buffer, 512,
        "{"
        "\"name\":\"ESP32 Door Lock\","
        "\"unique_id\":\"%s\","
        "\"command_topic\":\"%s\","
        "\"state_topic\":\"%s\","
        "\"payload_unlock\":\"UNLOCK\","
        "\"payload_lock\":\"LOCK\","
        "\"state_unlocked\":\"UNLOCKED\","
        "\"state_locked\":\"LOCKED\","
        "\"device\":{"
        "\"identifiers\":\"%s\","
        "\"manufacturer\":\"Magnus Lund\","
        "\"model\":\"ESP32-C6\""
        "}"
        "}",
        macAddress, commandTopic, stateTopic, macAddress
    );

    return buffer;
}
