#include "lock.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Lock::Lock(int gpioPin) : pin_(gpioPin) {
    gpio_reset_pin(static_cast<gpio_num_t>(pin_));
    gpio_set_direction(static_cast<gpio_num_t>(pin_), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(pin_), 0);
}

void Lock::lock() {
    if (state_ == State::Locked || state_ == State::Locking) {
        return;
    }
    state_ = State::Locking;
    gpio_set_level(static_cast<gpio_num_t>(pin_), 0);
    vTaskDelay(pdMS_TO_TICKS(100)); // Simulate time taken to lock
    state_ = State::Locked;
}

void Lock::unlock() {
    if (state_ == State::Unlocked || state_ == State::Unlocking) {
        return; 
    }
    state_ = State::Unlocking;
    gpio_set_level(static_cast<gpio_num_t>(pin_), 1);
    vTaskDelay(pdMS_TO_TICKS(100)); 
    state_ = State::Unlocked;
}