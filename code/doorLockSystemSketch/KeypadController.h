#pragma once

#include "Keypad.h"
#include "MqttManager.h"

class KeypadController {
public:
    KeypadController(Keypad& keypad, MqttManager& mqttManager);
    void update();          // call every loop

private:
    Keypad& _keypad;
    MqttManager& _mqttManager;
};
