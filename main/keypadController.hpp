#pragma once

#include "zigbeeManager.hpp"

KeypadController* KeypadController::instance_ = nullptr;

KeypadController::KeypadController(Keypad& keypad, ZigbeeManager& zigbeeManager)
    : keypad_(keypad), zigbeeManager_(zigbeeManager) {
    instance_ = this;
    zigbeeManager_.setCommandCallback([this](char* command) {
        staticCallbackUpdateLockState(command);
    });
}