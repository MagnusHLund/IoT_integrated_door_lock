#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.hpp"
#include "buzzer.hpp"
#include "lock.hpp"
#include "controller.hpp"
#include "threadManager.hpp"
#include "matterManager.hpp"

// Forward declarations to ensure pointer types are known
class Buzzer;
class Lock;
class Controller;
class ThreadManager;
class MatterManager;

Buzzer* buzzer = nullptr;
Lock* lock = nullptr;
Controller* controller = nullptr;
ThreadManager* threadManager = nullptr;
MatterManager* matterManager = nullptr;

static const char* TAG = "MAIN";

void setup() {
    ESP_LOGI(TAG, "Running setup...");

    buzzer = new Buzzer(BUZZER_PIN);
    lock = new Lock(RELAY_PIN);
    
    threadManager = new ThreadManager();
    matterManager = new MatterManager();

    controller = new Controller(*buzzer, *lock, *matterManager);

    ESP_LOGI(TAG, "Setup complete.");
}

void loop() {
    controller->staticCallbackUpdateLockState("LOCK");
    vTaskDelay(pdMS_TO_TICKS(5000));
    controller->staticCallbackUpdateLockState("UNLOCK");
    vTaskDelay(pdMS_TO_TICKS(5000));
}

extern "C" void app_main(void)
{
    setup();
    
    while (true) {
        loop();
    }
}