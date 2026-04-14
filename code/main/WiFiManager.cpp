#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* wifiSsid, const char* wifiPassword, String localIP, String gateway, String subnet, String dnsServer)
  : wifiSsid(wifiSsid), wifiPassword(wifiPassword) {
  if (!this->localIP.fromString(localIP)) {
    Serial.println("Invalid static IP!");
  }
  if (!this->subnet.fromString(subnet)) {
    Serial.println("Invalid subnet!");
  }
  if (!this->gateway.fromString(gateway)) {
    Serial.println("Invalid gateway!");
  }
  if (!this->dnsServer.fromString(dnsServer)) {
    Serial.println("Invalid DNS!");
  }
}

void WiFiManager::connect() {
  delay(10);
  Serial.println("Connecting to WiFi...");

  if (!WiFi.config(localIP, gateway, subnet, dnsServer)) {
    Serial.println("WiFi.config() failed!");
  }

  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiManager::ensureConnectivity() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    connect();
  }
}

char* WiFiManager::getMacAddress(bool removeColons) {
  static char macStr[18];
  byte mac[6];
  WiFi.macAddress(mac);

  if (removeColons) {
    snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  } else {
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  return macStr;
}