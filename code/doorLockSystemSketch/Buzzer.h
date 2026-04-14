#pragma once

#include <Arduino.h>

class Buzzer {
  private:
    byte _pin;
    bool _active = false;
    unsigned long _endTime = 0;

  public:
    Buzzer(byte pin);
    void buzz(unsigned long durationMs);
    void update();
};