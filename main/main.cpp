#include "esp_log.h"
#include "nvs_flash.h"
#include "config.hpp"
#include "lock.hpp"
#include "zigbeeManager.hpp"
#include "lockController.hpp"

// Forward declarations to ensure pointer types are known
class Lock;
class ZigbeeManager;
class LockController;

Lock* lock = nullptr;
ZigbeeManager* zigbeeManager = nullptr;
LockController* lockController = nullptr;

static const char* TAG = "MAIN";

static void init_nvs_default() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Running setup...");

    init_nvs_default();
    ESP_LOGI(TAG, "NVS initialized");

    lock = new Lock(RELAY_LOCK_PIN);
    zigbeeManager = new ZigbeeManager();
    lockController = new LockController(*lock, *zigbeeManager);

    zigbeeManager->start();

    ESP_LOGI(TAG, "Setup complete.");
}