#pragma once

#include "buzzer.hpp"
#include "lock.hpp"
#include "mqttManager.hpp"

class LockController {
public:
    LockController(Buzzer& buzzer, Lock& lock, MqttManager& mqttManager);

    static LockController* getInstance();
    static void staticCallbackUpdateLockState(char* topic, uint8_t* data, unsigned int size);

private:
    Buzzer& buzzer_;
    Lock& lock_;
    MqttManager& mqttManager_;

    static LockController* instance_;

    void updateLockState(const char* newState);
    void lockDoor();
    void unlockDoor();
};
