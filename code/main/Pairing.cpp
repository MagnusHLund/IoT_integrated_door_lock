class Pairing {
  private: 
    WiFiManager& wiFiManager;
    MqttManager& mqttManager;

    Button& button;
    Light& light;

  public:
    Pairing(WiFiManager& wiFiManager, MqttManager& mqttManager, Button& button, Light& light) 
      : wiFiManager(wiFiManager), mqttManager(mqttManager), button(button), light(light) {}

    void init() {
      pairToHomeAssistant();
    }

    void handlePairingButton() {
      int fiveSeconds = 5000;

      if (button.isHeld(fiveSeconds)) {
        pairToHomeAssistant();
        light.blink(125, 5);
      }
    }

    void pairToHomeAssistant() {
      char* macAddress = wiFiManager.getMacAddress(true);
      char* message = formatDiscoveryMessageJson(macAddress);

      mqttManager.publishMessage(message, mqttManager.getDiscoveryTopic());
      Serial.println("Sent discovery message");

      // TODO: Buzz twice.
    }

  private:
    char* formatDiscoveryMessageJson(char* macAddress) {
      StaticJsonDocument<512> doc;

      const char* commandTopic = mqttManager.getCommandTopic();
      const char* stateTopic = mqttManager.getStateTopic();

      doc["name"]           = "Arduino Door Lock";
      doc["unique_id"]      = macAddress; 
      doc["command_topic"]  = commandTopic;
      doc["state_topic"]    = stateTopic;
      doc["payload_unlock"] = "UNLOCK";
      doc["payload_lock"]   = "LOCK";
      doc["state_unlocked"] = "UNLOCKED";
      doc["state_locked"]   = "LOCKED";

      JsonObject device = doc.createNestedObject("device");
      device["identifiers"] = macAddress;
      device["manufacturer"] = "Arduino";
      device["model"] = "Uno R4 WiFi";

      static char buffer[512];
      serializeJson(doc, buffer);

      return buffer;
    }
};