//This file don't have Encoder
#include "Motor.h"
Motor::Motor(int pinA, int pinB, int maxRPM)
{
    this->_pinA = pinA;
    this->_pinB = pinB;
    this->_maxRPM = maxRPM;
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
}

Motor::~Motor()
{
}

void Motor::setSpeed(int speed)
{
    if (speed > 100)
    {
        this->_speed = 100;
    }
    else if (speed < -100)
    {
        this->_speed = -100;
    }
    else
    {
        this->_speed = speed;
    }
}

int Motor::getSpeed()
{
    return this->_speed;
}

int Motor::getRPM()
{
    // No encoder feedback available, return 0
    return 0;
}

void Motor::runRPM(int rpm)
{
    this->_speed = map(rpm, -this->_maxRPM, this->_maxRPM, -255, 255);
    if (this->_speed > 0)
    {
        analogWrite(this->_pinA, abs(this->_speed));
        analogWrite(this->_pinB, 0);
    }
    else if (this->_speed < 0)
    {
        analogWrite(this->_pinA, 0);
        analogWrite(this->_pinB, abs(this->_speed));
    }
    else
    {
        analogWrite(this->_pinA, 0);
        analogWrite(this->_pinB, 0);
    }
}
void Motor::run(int speed)
{
    if (speed > 255)
    {
        this->_speed = 255;
    }
    else if (speed < -255)
    {
        this->_speed = -255;
    }
    else
    {
        this->_speed = speed;
    }

    if (this->_speed > 0)
    {
        analogWrite(this->_pinA, abs(this->_speed));
        analogWrite(this->_pinB, 0);
    }
    else if (this->_speed < 0)
    {
        analogWrite(this->_pinA, 0);
        analogWrite(this->_pinB, abs(this->_speed));
    }
    else
    {
        analogWrite(this->_pinA, 0);
        analogWrite(this->_pinB, 0);
    }
}
void Motor::run()
{
    if (this->_speed > 0)
    {
        analogWrite(this->_pinA, abs(this->_speed));
        analogWrite(this->_pinB, 0);
    }
    else if (this->_speed < 0)
    {
        analogWrite(this->_pinA, 0);
        analogWrite(this->_pinB, abs(this->_speed));
    }
    else
    {
        analogWrite(this->_pinA, 0);
        analogWrite(this->_pinB, 0);
    }
}