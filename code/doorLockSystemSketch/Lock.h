#pragma once

#include <Arduino.h>

class Lock {
  private:
    byte _pin;

  public:
    Lock(byte pin);
    void unlock(unsigned long durationMs);
};