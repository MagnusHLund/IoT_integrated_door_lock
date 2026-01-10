#pragma once

#include "lock.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Lock::Lock(int gpioPin) : pin_(gpioPin) {
    gpio_reset_pin(static_cast<gpio_num_t>(pin_));
    gpio_set_direction(static_cast<gpio_num_t>(pin_), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(pin_), 1); // Start locked
}

void Lock::lock() {
    if (state_ == State::Locked || state_ == State::Locking) {
        return; // Already locked or in the process of locking
    }
    state_ = State::Locking;
    gpio_set_level(static_cast<gpio_num_t>(pin_), 1); // Activate relay to lock
    vTaskDelay(pdMS_TO_TICKS(500)); // Simulate time taken to lock
    state_ = State::Locked;
}

void Lock::unlock() {
    if (state_ == State::Unlocked || state_ == State::Unlocking) {
        return; // Already unlocked or in the process of unlocking
    }
    state_ = State::Unlocking;
    gpio_set_level(static_cast<gpio_num_t>(pin_), 0); // Activate relay to unlock
    vTaskDelay(pdMS_TO_TICKS(500)); // Simulate time taken to unlock
    state_ = State::Unlocked;
}