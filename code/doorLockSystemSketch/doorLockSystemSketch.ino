#include <Arduino.h>
#include "Config.h"
#include "WiFiManager.h"
#include "MqttManager.h"
#include "LockController.h"
#include "Pairing.h"

WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD, WIFI_STATIC_IP, WIFI_GATEWAY, WIFI_SUBNET_MASK, WIFI_DNS_SERVER);
MqttManager mqttManager(MQTT_HOSTNAME, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, wifiManager);

Buzzer buzzer(BUZZER_PIN);
Pairing pairing(wifiManager, mqttManager, buzzer);

Lock lock(RELAY_LOCK_PIN);
LockController lockController(lock, mqttManager);

byte keypadRowPins[4] = {KEYPAD_ROW_1_PIN, KEYPAD_ROW_2_PIN, KEYPAD_ROW_3_PIN, KEYPAD_ROW_4_PIN};
byte keypadColPins[4] = {KEYPAD_COL_1_PIN, KEYPAD_COL_2_PIN, KEYPAD_COL_3_PIN, KEYPAD_COL_4_PIN};
Keypad keypad(keypadRowPins, keypadColPins);
KeypadController keypadController(keypad, mqttManager);

unsigned long lastCheck = 0;

void setup() {
  Serial.begin(9600);

  // TODO: Buzz once

  wifiManager.connect();
  mqttManager.connect();

  const char* macAddress = wifiManager.getMacAddress(true);
  mqttManager.setupTopics(macAddress);

  pairing.init();
}

void loop() {
  buzzer.update();
  mqttManager.ensureConnectivity();

  // WiFi can be kept alive longer than MQTT
  if (millis() - lastCheck > 5000) {
    wifiManager.ensureConnectivity();
    lastCheck = millis();
  }
}
