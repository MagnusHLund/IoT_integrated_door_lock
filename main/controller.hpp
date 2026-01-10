#pragma once

#include "buzzer.hpp"
#include "lock.hpp"
#include "matterController.hpp"

class Controller {
public:
    Controller(Buzzer& buzzer, Lock& lock, MatterController& matterController);

    static Controller* getInstance();
    static void staticCallbackUpdateLockState();

private:
    Buzzer& buzzer_;
    Lock& lock_;
    MatterController& matterController_;

    static Controller* instance_;

    void updateLockState();
    void lockDoor();
    void unlockDoor();
};
