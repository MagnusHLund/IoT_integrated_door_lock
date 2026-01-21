#pragma once

#include "buzzer.hpp"
#include "lock.hpp"

class LockController {
public:
    LockController(Buzzer& buzzer, Lock& lock);

    static LockController* getInstance();
    static void staticCallbackUpdateLockState(const char* newState);

private:
    Buzzer& buzzer_;
    Lock& lock_;

    static LockController* instance_;

    void updateLockState(const char* newState);
    void lockDoor();
    void unlockDoor();
};
