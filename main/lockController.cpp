#include <cstring>
#include "lockController.hpp"

LockController* LockController::instance_ = nullptr;

LockController::LockController(Buzzer& buzzer, Lock& lock)
    : buzzer_(buzzer), lock_(lock) {
    instance_ = this;
}

LockController* LockController::getInstance() {
    return instance_;
}

/// @brief Called by MQTT, when lock state needs to be updated
void LockController::staticCallbackUpdateLockState(const char* newState) {
    if (instance_) {
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
}

void LockController::unlockDoor() {
    lock_.unlock();
    buzzer_.beep(200);
}