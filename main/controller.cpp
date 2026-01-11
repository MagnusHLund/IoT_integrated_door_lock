#include <cstring>
#include "controller.hpp"

Controller* Controller::instance_ = nullptr;

Controller::Controller(Buzzer& buzzer, Lock& lock, MatterManager& matterManager)
    : buzzer_(buzzer), lock_(lock), matterManager_(matterManager) {
    instance_ = this;
}

Controller* Controller::getInstance() {
    return instance_;
}

/// @brief Called by matter, when lock state needs to be updated
void Controller::staticCallbackUpdateLockState(const char* newState) {
    if (instance_) {
        instance_->updateLockState(newState);
    }
}

/// @brief Update lock state based on Matter commands
void Controller::updateLockState(const char* newState) {
    if (strcmp(newState, "LOCK") == 0) {
        lockDoor();
    } else if (strcmp(newState, "UNLOCK") == 0) {
        unlockDoor();
    }
}

void Controller::lockDoor() {
    lock_.lock();
    buzzer_.beep(200);
}

void Controller::unlockDoor() {
    lock_.unlock();
    buzzer_.beep(200);
}