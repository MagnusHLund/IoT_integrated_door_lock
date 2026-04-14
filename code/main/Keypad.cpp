#pragma once

#include "Keypad.h"

Keypad::Keypad(const byte rowPins[ROWS], const byte colPins[COLS]) {
    for (byte i = 0; i < ROWS; i++) {
        this->rowPins[i] = rowPins[i];
    }
    for (byte i = 0; i < COLS; i++) {
        this->colPins[i] = colPins[i];
    }
}

void Keypad::begin() {
    for (byte r = 0; r < ROWS; r++) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }
    for (byte c = 0; c < COLS; c++) {
        pinMode(colPins[c], INPUT_PULLUP);
    }
}

char Keypad::scan() {
    for (byte r = 0; r < ROWS; r++) {
        digitalWrite(rowPins[r], LOW);

        for (byte c = 0; c < COLS; c++) {
            if (digitalRead(colPins[c]) == LOW) {
                digitalWrite(rowPins[r], HIGH);
                return keymap[r][c];
            }
        }

        digitalWrite(rowPins[r], HIGH);
    }
    return 0;
}