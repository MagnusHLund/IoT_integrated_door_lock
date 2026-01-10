#pragma once

#include "buzzer.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Buzzer::Buzzer(int gpioPin) : pin_(gpioPin) {
    gpio_reset_pin(static_cast<gpio_num_t>(pin_));
    gpio_set_direction(static_cast<gpio_num_t>(pin_), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(pin_), 0);
}

void Buzzer::beep(int durationMs) {
    gpio_set_level(static_cast<gpio_num_t>(pin_), 1);
    vTaskDelay(pdMS_TO_TICKS(durationMs));
    gpio_set_level(static_cast<gpio_num_t>(pin_), 0);
}
