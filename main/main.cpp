#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.hpp"
#include "buzzer.hpp"

Buzzer* buzzer = nullptr;

static constexpr int BEEP_DURATION_MS = 200;
static constexpr int PAUSE_DURATION_MS = 2000;

void setup() {
    std::cout << "Setting up Smart Door Lock..." << std::endl;

    buzzer = new Buzzer(BUZZER_PIN);

    std::cout << "Setup complete." << std::endl;
}

void loop() {
    buzzer->beep(BEEP_DURATION_MS);
    vTaskDelay(pdMS_TO_TICKS(PAUSE_DURATION_MS));
}

extern "C" void app_main(void)
{
    setup();
    
    while (true) {
        loop();
    }
}