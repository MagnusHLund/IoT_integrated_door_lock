#pragma once

#include "Buzzer.h"

Buzzer::Buzzer(byte pin) : _pin(pin) {
  pinMode(_pin, OUTPUT);
}

void Buzzer::buzz(unsigned long durationMs) {
    _active = true;
    _endTime = millis() + durationMs;
    digitalWrite(_pin, HIGH);
}

// Makes the buzzer not have to use delay. Non blocking.
void Buzzer::update() {
    if (_active && millis() >= _endTime) {
        digitalWrite(_pin, LOW);
        _active = false;
    }
}