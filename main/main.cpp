#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.hpp"
#include "buzzer.hpp"
#include "lock.hpp"
#include "lockController.hpp"
#include "threadManager.hpp"
#include "matterManager.hpp"
#include "bleCommissioning.hpp"
#include "button.hpp"
#include "reset.hpp"

// Forward declarations to ensure pointer types are known
class Buzzer;
class Lock;
class LockController;
class ThreadManager;
class MatterManager;
class BLECommissioning;
class Button;
class Reset;

Buzzer* buzzer = nullptr;
Lock* lock = nullptr;
LockController* lockController = nullptr;
ThreadManager* threadManager = nullptr;
MatterManager* matterManager = nullptr;
BLECommissioning* bleCommissioning = nullptr;
Button* button = nullptr;
Reset* reset = nullptr;

static const char* TAG = "MAIN";

bool hasPaired = false;
bool matterInitialized = false;

void setup() {
    ESP_LOGI(TAG, "Running setup...");

    buzzer = new Buzzer(BUZZER_PIN);
    lock = new Lock(RELAY_LOCK_PIN);
    button = new Button(RESET_BUTTON_PIN);
    reset = new Reset();
    
    threadManager = new ThreadManager();
    matterManager = new MatterManager();
    bleCommissioning = new BLECommissioning();

    lockController = new LockController(*buzzer, *lock, *matterManager);

    // Initialize Thread Manager
    if (!threadManager->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize Thread Manager");
    }

    // Initialize Matter Manager with Thread reference
    if (!matterManager->initialize(*threadManager)) {
        ESP_LOGE(TAG, "Failed to initialize Matter Manager");
    } else {
        matterInitialized = true;
    }

    // Initialize BLE Commissioning
    if (!bleCommissioning->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize BLE Commissioning");
    }

    ESP_LOGI(TAG, "Setup complete.");
}

void loop() {
    // Handle factory reset
    if(button->isHeld(5000)) {
        ESP_LOGW(TAG, "Factory reset initiated.");
        
        // Factory reset all subsystems
        if (matterManager) {
            matterManager->factoryReset();
        }
        if (bleCommissioning) {
            bleCommissioning->factoryReset();
        }
        if (threadManager) {
            threadManager->disable();
        }
        
        reset->performFactoryReset();
        return;
    }

    // Handle commissioning
    if (!hasPaired) {
        // Start BLE advertising for commissioning
        if (bleCommissioning && !bleCommissioning->isCommissioned()) {
            bleCommissioning->startAdvertising();
            ESP_LOGI(TAG, "Waiting for commissioning...");
        }

        // Check if commissioning is complete
        if (bleCommissioning && bleCommissioning->isCommissioned()) {
            ESP_LOGI(TAG, "Device commissioned successfully!");
            hasPaired = true;

            // Enable Thread network after commissioning
            if (threadManager) {
                threadManager->enable();
            }

            // Start Matter protocol
            if (matterManager) {
                matterManager->start();
            }

            // Stop BLE advertising after successful commissioning
            bleCommissioning->stopAdvertising();

            buzzer->beep(500);  // Success beep
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Yield to avoid watchdog while unpaired
        return;
    }

    // Normal operation - device is paired
    if (matterManager) {
        // Check pairing state
        if (matterManager->getPairingState() != MatterManager::PairingState::Paired) {
            // Still attempting to pair
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;
        }

        // Run periodic diagnostics (every 30 seconds)
        static uint32_t diagnostic_counter = 0;
        if (++diagnostic_counter >= 30) {
            diagnostic_counter = 0;
            
            threadManager->runDiagnostics();
            matterManager->runDiagnostics();
            
            ESP_LOGI(TAG, "Device paired: %s", matterManager->isPaired() ? "YES" : "NO");
            ESP_LOGI(TAG, "Thread connected: %s", threadManager->isConnected() ? "YES" : "NO");
        }
    }

    vTaskDelay(pdMS_TO_TICKS(1000)); // Main loop runs every second
}

extern "C" void app_main(void)
{
    setup();
    
    while (true) {
        loop();
    }
}