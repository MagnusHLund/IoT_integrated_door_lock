#pragma once

#include <cstdint>

class Button {
public:
    Button(int gpioPin);
    bool isPressed();
    bool isHeld(int holdDurationMs);

private:
    int pin_;
    int64_t pressStartMs_ = 0;
    bool heldTriggered_ = false;
};
