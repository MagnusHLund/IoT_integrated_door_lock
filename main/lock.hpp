#pragma once

class Lock {
public:
    enum class State {
        Locked,
        Unlocked,
        Locking,
        Unlocking
    };
    Lock(int gpioPin);
    void lock();
    void unlock();

private:
    int pin_;
    State state_ = State::Locked;
};
