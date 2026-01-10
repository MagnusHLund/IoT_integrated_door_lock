#pragma once

class Lock {
public:
    Lock(int gpioPin);
    void lock();
    void unlock();

private:
    int pin_;
};
