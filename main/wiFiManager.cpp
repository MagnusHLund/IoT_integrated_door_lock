#include "wiFiManager.hpp"
#include <cstring>

static const char* TAG = "WiFiManager";

// Static member initialization
EventGroupHandle_t WiFiManager::wifiEventGroup = nullptr;
int WiFiManager::retryCount = 0;

WiFiManager::WiFiManager(const char* wifiSsid, const char* wifiPassword,
                         const char* localIP, const char* gateway,
                         const char* subnet, const char* dnsServerStr)
    : wifiSsid(wifiSsid), wifiPassword(wifiPassword) {
    
    // Parse IP addresses
    if (esp_netif_str_to_ip4(localIP, &ipInfo.ip) != ESP_OK) {
        ESP_LOGE(TAG, "Invalid static IP: %s", localIP);
    }
    if (esp_netif_str_to_ip4(subnet, &ipInfo.netmask) != ESP_OK) {
        ESP_LOGE(TAG, "Invalid subnet: %s", subnet);
    }
    if (esp_netif_str_to_ip4(gateway, &ipInfo.gw) != ESP_OK) {
        ESP_LOGE(TAG, "Invalid gateway: %s", gateway);
    }
    if (esp_netif_str_to_ip4(dnsServerStr, &dnsServer) != ESP_OK) {
        ESP_LOGE(TAG, "Invalid DNS: %s", dnsServerStr);
    }
    
    // Create event group
    wifiEventGroup = xEventGroupCreate();
}

WiFiManager::~WiFiManager() {
    if (wifiEventGroup) {
        vEventGroupDelete(wifiEventGroup);
        wifiEventGroup = nullptr;
    }
}

void WiFiManager::eventHandler(void* arg, esp_event_base_t eventBase,
                               int32_t eventId, void* eventData) {
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (retryCount < MAX_RETRY) {
            esp_wifi_connect();
            retryCount++;
            ESP_LOGI(TAG, "Retry connecting to WiFi... (%d/%d)", retryCount, MAX_RETRY);
        } else {
            xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Connection to WiFi failed");
    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
        ESP_LOGI(TAG, "WiFi connected. IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

void WiFiManager::connect() {
    ESP_LOGI(TAG, "Connecting to WiFi...");
    
    // Initialize TCP/IP network interface (only if not already initialized)
    static bool netif_initialized = false;
    if (!netif_initialized) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        netif_initialized = true;
    }
    
    esp_netif_t* netif = esp_netif_create_default_wifi_sta();
    
    // Configure static IP
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ipInfo));
    
    // Set DNS server
    esp_netif_dns_info_t dnsInfo;
    dnsInfo.ip.u_addr.ip4 = dnsServer;
    dnsInfo.ip.type = ESP_IPADDR_TYPE_V4;
    ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dnsInfo));
    
    // Initialize WiFi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Register event handlers
    esp_event_handler_instance_t instanceAnyId;
    esp_event_handler_instance_t instanceGotIp;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &eventHandler,
                                                        NULL,
                                                        &instanceAnyId));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &eventHandler,
                                                        NULL,
                                                        &instanceGotIp));
    
    // Configure WiFi
    wifi_config_t wifiConfig = {};
    strncpy((char*)wifiConfig.sta.ssid, wifiSsid, sizeof(wifiConfig.sta.ssid) - 1);
    strncpy((char*)wifiConfig.sta.password, wifiPassword, sizeof(wifiConfig.sta.password) - 1);
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi initialization finished.");
    
    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to SSID: %s", wifiSsid);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to SSID: %s", wifiSsid);
    } else {
        ESP_LOGE(TAG, "Unexpected event");
    }
}

void WiFiManager::ensureConnectivity() {
    wifi_ap_record_t apInfo;
    if (esp_wifi_sta_get_ap_info(&apInfo) != ESP_OK) {
        ESP_LOGW(TAG, "WiFi lost, reconnecting...");
        retryCount = 0;
        xEventGroupClearBits(wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
        esp_wifi_connect();
        
        // Wait for reconnection
        xEventGroupWaitBits(wifiEventGroup,
                           WIFI_CONNECTED_BIT,
                           pdFALSE,
                           pdFALSE,
                           portMAX_DELAY);
    }
}

char* WiFiManager::getMacAddress(bool removeColons) {
    static char macStr[18];
    uint8_t mac[6];
    
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    
    if (removeColons) {
        snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    
    return macStr;
}

bool WiFiManager::isConnected() {
    wifi_ap_record_t apInfo;
    return (esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK);
}
