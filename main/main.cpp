#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.hpp"
#include "buzzer.hpp"
#include "lock.hpp"
#include "lockController.hpp"
#include "button.hpp"
#include "reset.hpp"

// Forward declarations to ensure pointer types are known
class Buzzer;
class Lock;
class LockController;
class Button;
class Reset;

Buzzer* buzzer = nullptr;
Lock* lock = nullptr;
LockController* lockController = nullptr;
Button* button = nullptr;
Reset* reset = nullptr;

static const char* TAG = "MAIN";

bool hasPaired = false;

void setup() {
    ESP_LOGI(TAG, "Running setup...");

    buzzer = new Buzzer(BUZZER_PIN);
    lock = new Lock(RELAY_LOCK_PIN);

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