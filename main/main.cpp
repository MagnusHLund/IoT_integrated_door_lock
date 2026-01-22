#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.hpp"
#include "lock.hpp"
#include "wiFiManager.hpp"
#include "mqttManager.hpp"
#include "lockController.hpp"
#include "pairing.hpp"

// CA certificate embedded from file
extern const uint8_t ca_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t ca_cert_pem_end[] asm("_binary_ca_cert_pem_end");

// Forward declarations to ensure pointer types are known
class Lock;
class Pairing;
class WiFiManager;
class MqttManager;
class LockController;


Lock* lock = nullptr;
Pairing* pairing = nullptr;
WiFiManager* wiFiManager = nullptr;
MqttManager* mqttManager = nullptr;
LockController* lockController = nullptr;


static const char* TAG = "MAIN";

void setup() {
    ESP_LOGI(TAG, "Running setup...");

    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");

    lock = new Lock(RELAY_LOCK_PIN);

    WiFiManager* wiFiManager = new WiFiManager(
        WIFI_SSID,
        WIFI_PASSWORD,
        WIFI_STATIC_IP,
        WIFI_GATEWAY,
        WIFI_SUBNET_MASK,
        WIFI_DNS_SERVER
    );

    mqttManager = new MqttManager(
        MQTT_HOSTNAME,
        MQTT_PORT,
        MQTT_USERNAME,
        MQTT_PASSWORD,
        *wiFiManager
    );

    // Enable TLS for MQTT connection
    int caCertLen = ca_cert_pem_end - ca_cert_pem_start;
    mqttManager->enableTLS(ca_cert_pem_start, caCertLen);

    wiFiManager->connect();
    const char* macAddress = wiFiManager->getMacAddress(true);

    mqttManager->setupTopics(macAddress);
    mqttManager->connect();
    mqttManager->awaitConnectivity(); 

    pairing = new Pairing(*wiFiManager, *mqttManager);
    pairing->pairToHomeAssistant();

    lockController = new LockController(*lock, *mqttManager);

    ESP_LOGI(TAG, "Setup complete.");
}

void loop() {
    wiFiManager->ensureConnectivity();
    vTaskDelay(pdMS_TO_TICKS(1000));
}

extern "C" void app_main(void)
{
    setup();
    
    while (true) {
        loop();
    }
}