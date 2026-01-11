#include "button.hpp"
#include "driver/gpio.h"
#include "esp_timer.h"

Button::Button(int gpioPin) : pin_(gpioPin) {
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << pin_;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    
    gpio_config(&io_conf);
}

bool Button::isPressed() {
    return gpio_get_level(static_cast<gpio_num_t>(pin_)) == 0;
}

bool Button::isHeld(int holdDurationMs) {
    int64_t timeInMs = esp_timer_get_time() / 1000; // convert to ms

    if (isPressed()) {
        if (pressStartMs_ == 0) {
            pressStartMs_ = timeInMs;
            heldTriggered_ = false;
        } else if (!heldTriggered_ && (timeInMs - pressStartMs_) >= holdDurationMs) {
            heldTriggered_ = true;
            return true; 
        }
    } else {
        pressStartMs_ = 0;
        heldTriggered_ = false;
    }

    return false;
}