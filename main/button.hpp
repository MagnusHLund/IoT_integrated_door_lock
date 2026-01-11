#pragma once

class Button {
public:
    Button(int gpioPin);
    bool isPressed();
    bool isHeld(int holdDurationMs);

private:
    int pin_;
};