#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
	private:
		const char* wifiSsid;
		const char* wifiPassword;

		IPAddress localIP;
		IPAddress dnsServer;
		IPAddress gateway;
		IPAddress subnet;

	public:
		WiFiManager(const char* wifiSsid, const char* wifiPassword, String localIP, String gateway, String subnet, String dnsServer = "1.1.1.1");

		void connect();
		void ensureConnectivity();
		char* getMacAddress(bool removeColons = false);
};
