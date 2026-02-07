#include <cstring>
#include "lockController.hpp"

LockController* LockController::instance_ = nullptr;

LockController::LockController(Lock& lock, ZigbeeManager& zigbeeManager)
    : lock_(lock), zigbeeManager_(zigbeeManager) {
    instance_ = this;
    zigbeeManager_.setCommandCallback([this](char* command) {
        staticCallbackUpdateLockState(command);
    });
}

LockController* LockController::getInstance() {
    return instance_;
}

/// @brief Called by Zigbee when lock state needs to be updated
void LockController::staticCallbackUpdateLockState(char* command) {
    if (instance_ && command) {
        instance_->updateLockState(command);
    }
}

/// @brief Update lock state based on Zigbee commands
void LockController::updateLockState(const char* newState) {
    if (strcmp(newState, "LOCK") == 0) {
        lockDoor();
    } else if (strcmp(newState, "UNLOCK") == 0) {
        unlockDoor();
    }
}

void LockController::lockDoor() {
    lock_.lock();
    zigbeeManager_.publishLockState("LOCKED");
}

void LockController::unlockDoor() {
    lock_.unlock();
    zigbeeManager_.publishLockState("UNLOCKED");
}