#include <iostream>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static constexpr int PIN_NUM = 2;
static constexpr int BEEP_DURATION_MS = 200;
static constexpr int PAUSE_DURATION_MS = 2000;

extern "C" void app_main(void)
{
    // Configure GPIO pin as output
    gpio_reset_pin((gpio_num_t)PIN_NUM);
    gpio_set_direction((gpio_num_t)PIN_NUM, GPIO_MODE_OUTPUT);

    std::cout << "Buzzer started on GPIO pin " << PIN_NUM << std::endl;

    while (true) {
        // Turn on buzzer
        gpio_set_level((gpio_num_t)PIN_NUM, 1);
        vTaskDelay(pdMS_TO_TICKS(BEEP_DURATION_MS));

        // Turn off buzzer
        gpio_set_level((gpio_num_t)PIN_NUM, 0);

        // Wait 5 seconds
        vTaskDelay(pdMS_TO_TICKS(PAUSE_DURATION_MS));
    }
}
