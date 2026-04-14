#pragma once

#include "Lock.h"
#include "MqttManager.h"

class LockController {
public:
    LockController(Lock& lock, MqttManager& mqttManager);

private:
    Lock& _lock;
    MqttManager& _mqttManager;
};
