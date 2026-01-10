#pragma once

class Buzzer {
public:
    Buzzer(int gpioPin);
    void beep(int durationMs);

private:
    int pin_;
};
