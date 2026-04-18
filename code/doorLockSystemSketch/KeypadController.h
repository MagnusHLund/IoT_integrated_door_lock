#pragma once

#include "Keypad.h"
#include "MqttManager.h"

// TODO: Reset inputted code, after 30 seconds since last input

class KeypadController {
public:
    KeypadController(Keypad& keypad, MqttManager& mqttManager);
    void init();
    void update();

private:
    static constexpr size_t MAX_CODE_LENGTH = 16;

    Keypad& _keypad;
    MqttManager& _mqttManager;

    char _submissionTopic[64];
    char _codeBuffer[MAX_CODE_LENGTH + 1] = {0};
    size_t _codeLength = 0;
    char _lastObservedKey = 0;

    void appendKey(char key);
    void submitCode(const char* specialCharacter = "");
};
