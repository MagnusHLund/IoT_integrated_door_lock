#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PIN_NUM 2
#define BEEP_DURATION_MS 200
#define PAUSE_DURATION_MS 5000

void app_main(void)
{
    // Configure GPIO pin 2 as output
    gpio_reset_pin(PIN_NUM);
    gpio_set_direction(PIN_NUM, GPIO_MODE_OUTPUT);
    
    printf("Buzzer started on GPIO pin %d\n", PIN_NUM);
    
    while(1) {
        // Turn on buzzer (beep)
        gpio_set_level(PIN_NUM, 1);
        vTaskDelay(pdMS_TO_TICKS(BEEP_DURATION_MS));
        
        // Turn off buzzer
        gpio_set_level(PIN_NUM, 0);
        
        // Wait 5 seconds
        vTaskDelay(pdMS_TO_TICKS(PAUSE_DURATION_MS));
    }
}
