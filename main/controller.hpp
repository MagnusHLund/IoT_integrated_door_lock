#pragma once

#include "buzzer.hpp"
#include "lock.hpp"
#include "matterManager.hpp"

class Controller {
public:
    Controller(Buzzer& buzzer, Lock& lock, MatterManager& matterManager);

    static Controller* getInstance();
    static void staticCallbackUpdateLockState(const char* newState);

private:
    Buzzer& buzzer_;
    Lock& lock_;
    MatterManager& matterManager_;

    static Controller* instance_;

    void updateLockState(const char* newState);
    void lockDoor();
    void unlockDoor();
};
