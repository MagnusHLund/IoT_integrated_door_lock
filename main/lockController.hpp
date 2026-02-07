#pragma once

#include "lock.hpp"
#include "zigbeeManager.hpp"

class LockController {
public:
    LockController(Lock& lock, ZigbeeManager& zigbeeManager);

    static LockController* getInstance();
    static void staticCallbackUpdateLockState(char* command);

private:
    Lock& lock_;
    ZigbeeManager& zigbeeManager_;

    static LockController* instance_;

    void updateLockState(const char* newState);
    void lockDoor();
    void unlockDoor();
};
