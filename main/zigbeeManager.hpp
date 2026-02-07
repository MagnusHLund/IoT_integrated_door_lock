#pragma once

#include "esp_log.h"
#include <functional>

class ZigbeeManager {
private:
    bool connected;
    
    // Callback function pointer for command handling
    std::function<void(char*)> commandCallback;
    
    static ZigbeeManager* instance_;

public:
    ZigbeeManager();
    ~ZigbeeManager();
    
    void start();
    void awaitConnectivity();
    bool isConnected() const;
    void setConnected(bool state);
    void publishLockState(const char* state);
    void setCommandCallback(std::function<void(char*)> callback);
    void processCommand(const char* command);
    static ZigbeeManager* getInstance();
    
    // Zigbee constants
    static constexpr uint8_t ENDPOINT = 10;
    static constexpr uint16_t ON_OFF_CLUSTER = 0x0006;
    static constexpr uint16_t ON_OFF_ATTR = 0x0000;
};
