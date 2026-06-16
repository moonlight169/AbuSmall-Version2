//This file don't have Encoder
#ifndef Motor_h
#define Motor_h

#include "Arduino.h"
class Motor
{
private:
    int _pinA;
    int _pinB;
    int _speed = 0;
    int _maxRPM = 0;

public:
    Motor(int pinA, int pinB, int maxRPM);
    void setSpeed(int speed);
    int getSpeed();
    void run(int speed);
    void runRPM(int rpm);
    void run();
    int getRPM();
    ~Motor();
};
#endif