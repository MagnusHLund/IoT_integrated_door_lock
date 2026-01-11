#pragma once

#include "buzzer.hpp"
#include "lock.hpp"
#include "matterManager.hpp"

class LockController {
public:
    LockController(Buzzer& buzzer, Lock& lock, MatterManager& matterManager);

    static LockController* getInstance();
    static void staticCallbackUpdateLockState(const char* newState);

private:
    Buzzer& buzzer_;
    Lock& lock_;
    MatterManager& matterManager_;

    static LockController* instance_;

    void updateLockState(const char* newState);
    void lockDoor();
    void unlockDoor();
};
