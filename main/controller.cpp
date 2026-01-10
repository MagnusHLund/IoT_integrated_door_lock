#pragma once

#include "controller.hpp"

Controller* Controller::instance_ = nullptr;

Controller::Controller(Buzzer& buzzer, Lock& lock, MatterController& matterController)
    : buzzer_(buzzer), lock_(lock), matterController_(matterController) {
    instance_ = this;
}

Controller* Controller::getInstance() {
    return instance_;
}

void Controller::updateLockState() {
}

void Controller::lockDoor() {
    lock_.lock();
    buzzer_.beep(200);
}

void Controller::unlockDoor() {
    lock_.unlock();
    buzzer_.beep(200);
}