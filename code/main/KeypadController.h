#pragma once

class KeypadController {
  public:
    KeypadController(Keypad& keypad, MqttManager& mqttManager) : keypad(keypad), mqttManager(mqttManager) {};

    void init();
    char getKey();

  private:
    Keypad& keypad;
    MqttManager& mqttManager;
};