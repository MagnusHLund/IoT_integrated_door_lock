#include "Config.hpp"
#include "WiFiManager.hpp"
#include "MqttManager.hpp"
#include "LockController.hpp"
#include "Pairing.hpp"

WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD, WIFI_STATIC_IP, WIFI_GATEWAY, WIFI_SUBNET_MASK, WIFI_DNS_SERVER);
MqttManger mqttManager(MQTT_HOSTNAME, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, wifiManger);

Buzzer buzzer(BUZZER_PIN);
Pairing pairing(wifiManager, mqttManager, buzzer)

Lock lock(RELAY_LOCK_PIN);
LockController lockController(lock, mqttManager);

Keypad keypad(KEYPAD_ROW_1_PIN, KEYPAD_ROW_2_PIN, KEYPAD_ROW_3_PIN, KEYPAD_ROW_4_PIN, KEYPAD_COL_1_PIN, KEYPAD_COL_2_PIN, KEYPAD_COL_3_PIN, KEYPAD_COL_4_PIN);
KeypadController keypadController(keypad, mqttManager);

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
  mqttManager.ensureConnectivity();

  // WiFi can be kept alive longer than MQTT
  if (millis() - lastCheck > 5000) {
    wiFiManager.ensureConnectivity();
    lastCheck = millis();
  }
}
