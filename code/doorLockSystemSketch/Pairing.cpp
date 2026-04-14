#pragma once

#include "Pairing.h"

Pairing::Pairing(WiFiManager& wifiManager, MqttManager& mqttManager, Buzzer& buzzer) 
  : wifiManager(wifiManager), mqttManager(mqttManager), buzzer(buzzer) {}

void Pairing::init() {
  buzzer.buzz(200);
  pairToHomeAssistant();
}

void Pairing::pairToHomeAssistant() {
  char* macAddress = wifiManager.getMacAddress(true);
  char* message = formatDiscoveryMessageJson(macAddress);

  mqttManager.publishMessage(message, mqttManager.getDiscoveryTopic());
  Serial.println("Sent discovery message");
}


char* Pairing::formatDiscoveryMessageJson(char* macAddress) {
  StaticJsonDocument<512> doc;

  const char* commandTopic = mqttManager.getCommandTopic();
  const char* stateTopic = mqttManager.getStateTopic();

  doc["name"]           = "Magnus Lund Door Lock";
  doc["unique_id"]      = macAddress; 
  doc["command_topic"]  = commandTopic;
  doc["state_topic"]    = stateTopic;
  doc["payload_unlock"] = "UNLOCK";
  doc["payload_lock"]   = "LOCK";
  doc["state_unlocked"] = "UNLOCKED";
  doc["state_locked"]   = "LOCKED";

  JsonObject device = doc.createNestedObject("device");
  device["identifiers"] = macAddress;
  device["manufacturer"] = "Magnus Lund";
  device["model"] = "ESP32-C6";

  static char buffer[512];
  serializeJson(doc, buffer);

  return buffer;
}