#include "mqttManager.hpp"
#include <cstring>

static const char* TAG = "MqttManager";

MqttManager::MqttManager(const char* serverHostname, int serverPort,
                         const char* mqttUsername, const char* mqttPassword,
                         WiFiManager& wiFiManager)
    : client(nullptr),
      wiFiManager(wiFiManager),
      serverHostname(serverHostname),
      serverPort(serverPort),
      mqttUsername(mqttUsername),
      mqttPassword(mqttPassword),
      connected(false) {
}

MqttManager::~MqttManager() {
    if (client) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
    }
}

void MqttManager::mqttEventHandler(void* handlerArgs, esp_event_base_t base,
                                   int32_t eventId, void* eventData) {
    MqttManager* mqttManager = static_cast<MqttManager*>(handlerArgs);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(eventData);
    
    switch ((esp_mqtt_event_id_t)eventId) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "connected");
            mqttManager->connected = true;
            
            // Subscribe to command topic
            esp_mqtt_client_subscribe(mqttManager->client, 
                                     mqttManager->getCommandTopic(), 0);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            mqttManager->connected = false;
            break;
            
        case MQTT_EVENT_DATA:
            // Call user callback if set
            if (mqttManager->messageCallback) {
                // Create null-terminated strings
                char* topic = (char*)malloc(event->topic_len + 1);
                uint8_t* data = (uint8_t*)malloc(event->data_len + 1);
                
                if (topic && data) {
                    memcpy(topic, event->topic, event->topic_len);
                    topic[event->topic_len] = '\0';
                    memcpy(data, event->data, event->data_len);
                    data[event->data_len] = '\0';
                    
                    mqttManager->messageCallback(topic, data, event->data_len);
                    
                    free(topic);
                    free(data);
                }
            }
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;
            
        default:
            break;
    }
}

void MqttManager::connect() {
    // Build MQTT URI
    char uri[128];
    snprintf(uri, sizeof(uri), "mqtt://%s:%d", serverHostname, serverPort);
    
    // Configure MQTT client
    esp_mqtt_client_config_t mqttConfig = {};
    mqttConfig.broker.address.uri = uri;
    mqttConfig.credentials.username = mqttUsername;
    mqttConfig.credentials.authentication.password = mqttPassword;
    mqttConfig.buffer.size = 512;
    mqttConfig.buffer.out_size = 512;
    
    // Use MAC address as client ID
    char* macAddress = wiFiManager.getMacAddress(true);
    mqttConfig.credentials.client_id = macAddress;
    
    // Create MQTT client
    client = esp_mqtt_client_init(&mqttConfig);
    if (client == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }
    
    // Register event handler
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqttEventHandler, this);
    
    // Start MQTT client
    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client");
        return;
    }
}

void MqttManager::setupTopics(const char* macAddress) {
    snprintf(this->stateTopic, sizeof(this->stateTopic),
             "homeassistant/lock/%s/state", macAddress);
    snprintf(this->commandTopic, sizeof(this->commandTopic),
             "homeassistant/lock/%s/set", macAddress);
    snprintf(this->discoveryTopic, sizeof(this->discoveryTopic),
             "homeassistant/lock/%s/config", macAddress);
}

void MqttManager::awaitConnectivity() {
    while (!connected) {
        ESP_LOGI(TAG, "Waiting for MQTT connection...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

const char* MqttManager::getDiscoveryTopic() {
    return discoveryTopic;
}

const char* MqttManager::getStateTopic() {
    return stateTopic;
}

const char* MqttManager::getCommandTopic() {
    return commandTopic;
}

void MqttManager::publishMessage(const char* message, const char* topic) {
    if (topic == nullptr) {
        topic = stateTopic;
    }
    
    if (!connected) {
        ESP_LOGW(TAG, "Cannot publish, MQTT not connected");
        return;
    }
    
    bool success = (esp_mqtt_client_publish(client, topic, message, 0, 1, 1) != -1);
    
    if (!success) {
        ESP_LOGE(TAG, "Failed to publish message");
    }
}

void MqttManager::setCallback(std::function<void(char*, uint8_t*, unsigned int)> callback) {
    messageCallback = callback;
}
