#include "button.hpp"

Button::Button(int gpioPin) : pin_(gpioPin) {
}

bool Button::isPressed() {
    return false;
}

bool Button::isHeld(int holdDurationMs) {
    return false;
}