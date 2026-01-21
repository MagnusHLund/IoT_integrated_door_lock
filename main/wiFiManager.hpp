#pragma once

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

class WiFiManager {
private:
    const char* wifiSsid;
    const char* wifiPassword;
    
    esp_netif_ip_info_t ipInfo;
    esp_ip4_addr_t dnsServer;
    
    static EventGroupHandle_t wifiEventGroup;
    static const int WIFI_CONNECTED_BIT = BIT0;
    static const int WIFI_FAIL_BIT = BIT1;
    static int retryCount;
    static const int MAX_RETRY = 5;
    
    static void eventHandler(void* arg, esp_event_base_t eventBase, 
                            int32_t eventId, void* eventData);

public:
    WiFiManager(const char* wifiSsid, const char* wifiPassword, 
                const char* localIP, const char* gateway, 
                const char* subnet, const char* dnsServer = "1.1.1.1");
    
    ~WiFiManager();
    
    void connect();
    void ensureConnectivity();
    char* getMacAddress(bool removeColons = false);
    bool isConnected();
};
