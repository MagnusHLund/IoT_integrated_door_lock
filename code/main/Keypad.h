#pragma once

class Keypad {
  private:
    unsigned long debounceTime = 25;
    unsigned long lastChange = 0;
    
    char lastKey = 0;

    static constexpr byte ROWS = 4;
    static constexpr byte COLS = 4;

    byte rowPins[ROWS];
    byte colPins[COLS];

    static constexpr char keys[ROWS][COLS] = {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    };

  public:
    Keypad(const byte rowPins[ROWS], const byte colPins[COLS]);
    void init();
    char scan();
}