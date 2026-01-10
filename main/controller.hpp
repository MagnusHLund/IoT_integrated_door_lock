#pragma once

#include "buzzer.hpp"
#include "lock.hpp"
#include "matterManager.hpp"

class Controller {
public:
    Controller(Buzzer& buzzer, Lock& lock, MatterManager& matterManager);

    static Controller* getInstance();
    static void staticCallbackUpdateLockState();

private:
    Buzzer& buzzer_;
    Lock& lock_;
    MatterManager& matterManager_;

    static Controller* instance_;

    void updateLockState();
    void lockDoor();
    void unlockDoor();
};
