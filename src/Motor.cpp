#include "Motor.h"
#include <Arduino.h>

static int channelCounter = 0;

Motor::Motor(int pinA, int pinB, int maxRPM)
{
    this->_pinA = pinA;
    this->_pinB = pinB;
    this->_maxRPM = maxRPM;
    this->_speed = 0;

    this->_chA = channelCounter++;
    this->_chB = channelCounter++;
    
    ledcSetup(this->_chA, 5000, 8);
    ledcSetup(this->_chB, 5000, 8);

    ledcAttachPin(this->_pinA, this->_chA);
    ledcAttachPin(this->_pinB, this->_chB);
}

Motor::~Motor()
{
}

void Motor::setSpeed(int speed)
{
    if (speed > 100) this->_speed = 100;
    else if (speed < -100) this->_speed = -100;
    else this->_speed = speed;
}

int Motor::getSpeed()
{
    return this->_speed;
}

int Motor::getRPM()
{
    return 0;
}

void Motor::runRPM(int rpm)
{
    this->_speed = map(rpm, -this->_maxRPM, this->_maxRPM, -255, 255);
    
    if (this->_speed > 0)
    {
        ledcWrite(this->_chA, abs(this->_speed));
        ledcWrite(this->_chB, 0);
    }
    else if (this->_speed < 0)
    {
        ledcWrite(this->_chA, 0);
        ledcWrite(this->_chB, abs(this->_speed));
    }
    else
    {
        ledcWrite(this->_chA, 0);
        ledcWrite(this->_chB, 0);
    }
}

void Motor::run(int speed)
{
    if (speed > 255) this->_speed = 255;
    else if (speed < -255) this->_speed = -255;
    else this->_speed = speed;

    this->run();
}

void Motor::run()
{
    if (this->_speed > 0)
    {
        ledcWrite(this->_chA, abs(this->_speed));
        ledcWrite(this->_chB, 0); 
    }
    else if (this->_speed < 0)
    {
        ledcWrite(this->_chA, 0);
        ledcWrite(this->_chB, abs(this->_speed));
    }
    else
    {
        ledcWrite(this->_chA, 0);
        ledcWrite(this->_chB, 0);
    }
}