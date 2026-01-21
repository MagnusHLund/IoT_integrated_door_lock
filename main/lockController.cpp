#include <cstring>
#include "lockController.hpp"

LockController* LockController::instance_ = nullptr;

LockController::LockController(Buzzer& buzzer, Lock& lock, MqttManager& mqttManager)
    : buzzer_(buzzer), lock_(lock), mqttManager_(mqttManager) {
    instance_ = this;
    mqttManager_.setCallback([this](char* topic, uint8_t* data, unsigned int size) {
        staticCallbackUpdateLockState(topic, data, size);
    });
}

LockController* LockController::getInstance() {
    return instance_;
}

/// @brief Called by MQTT, when lock state needs to be updated
void LockController::staticCallbackUpdateLockState(char* topic, uint8_t* data, unsigned int size) {
    (void)topic;  // Topic currently unused
    if (instance_ && data) {
        // Convert payload to null-terminated string
        char newState[size + 1];
        std::memcpy(newState, data, size);
        newState[size] = '\0';
        instance_->updateLockState(newState);
    }
}

/// @brief Update lock state based on MQTT commands
void LockController::updateLockState(const char* newState) {
    if (strcmp(newState, "LOCK") == 0) {
        lockDoor();
    } else if (strcmp(newState, "UNLOCK") == 0) {
        unlockDoor();
    }
}

void LockController::lockDoor() {
    lock_.lock();
    buzzer_.beep(200);
    mqttManager_.publishMessage("LOCKED");
}

void LockController::unlockDoor() {
    lock_.unlock();
    buzzer_.beep(200);
    mqttManager_.publishMessage("UNLOCKED");
}