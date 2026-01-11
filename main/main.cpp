#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.hpp"
#include "buzzer.hpp"
#include "lock.hpp"
#include "lockController.hpp"
#include "threadManager.hpp"
#include "matterManager.hpp"
#include "button.hpp"
#include "reset.hpp"

// Forward declarations to ensure pointer types are known
class Buzzer;
class Lock;
class LockController;
class ThreadManager;
class MatterManager;
class Button;
class Reset;

Buzzer* buzzer = nullptr;
Lock* lock = nullptr;
LockController* lockController = nullptr;
ThreadManager* threadManager = nullptr;
MatterManager* matterManager = nullptr;
Button* button = nullptr;
Reset* reset = nullptr;

static const char* TAG = "MAIN";

bool hasPaired = false;

void setup() {
    ESP_LOGI(TAG, "Running setup...");

    buzzer = new Buzzer(BUZZER_PIN);
    lock = new Lock(RELAY_LOCK_PIN);
    button = new Button(RESET_BUTTON_PIN);
    reset = new Reset();
    
    threadManager = new ThreadManager();
    matterManager = new MatterManager();

    lockController = new LockController(*buzzer, *lock, *matterManager);

    ESP_LOGI(TAG, "Setup complete.");
}

void loop() {
    if(button->isHeld(5000)) {
        ESP_LOGW(TAG, "Factory reset initiated.");
        reset->performFactoryReset();
        return;
    }

    if(!hasPaired) {
        // TODO: Pairing logic for Matter
        vTaskDelay(pdMS_TO_TICKS(500)); // Yield to avoid watchdog while unpaired
        return;
    }

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