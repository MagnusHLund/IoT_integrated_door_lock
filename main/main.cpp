#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.hpp"
#include "buzzer.hpp"
#include "lock.hpp"
#include "wiFiManager.hpp"
#include "mqttManager.hpp"
#include "lockController.hpp"
#include "button.hpp"
#include "reset.hpp"

// Forward declarations to ensure pointer types are known
class Buzzer;
class Lock;
class WiFiManager;

class LockController;
class Button;
class Reset;

Buzzer* buzzer = nullptr;
Lock* lock = nullptr;

WiFiManager* wiFiManager = nullptr;
MqttManager* mqttManager = nullptr;

LockController* lockController = nullptr;
Button* button = nullptr;
Reset* reset = nullptr;

static const char* TAG = "MAIN";

bool hasPaired = false;

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

    buzzer = new Buzzer(BUZZER_PIN);
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

    wiFiManager->connect();
    mqttManager->connect();

    lockController = new LockController(*buzzer, *lock);

    ESP_LOGI(TAG, "Setup complete.");
}

void loop() {
    lockController->staticCallbackUpdateLockState("LOCK");
    vTaskDelay(pdMS_TO_TICKS(5000));
    lockController->staticCallbackUpdateLockState("UNLOCK");
    vTaskDelay(pdMS_TO_TICKS(5000));
}

extern "C" void app_main(void)
{
    setup();
    
    while (true) {
        loop();
    }
}