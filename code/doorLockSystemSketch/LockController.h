#pragma once
#include "Lock.h"

class LockController {
public:
    LockController(Lock& lock, MqttManager& mqttManager);

private:
    Lock& _lock;
    MqttManager& _mqttManager;
};
