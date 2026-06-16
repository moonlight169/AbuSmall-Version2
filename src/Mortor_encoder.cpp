//This file have Encoder
#include "Motor_encoder.h"
#include <ESP32Encoder.h>

Motor_encoder::Motor_encoder(int pinA, int pinB, int maxRPM, ESP32Encoder *en, int counts_per_rev)
{
    this->_pinA = pinA;
    this->_pinB = pinB;
    this->_maxRPM = maxRPM;
    this->_en = en;
    this->_counts_per_rev = counts_per_rev;
    
    // ค่าเริ่มต้นเพื่อป้องกัน Error ใน getRPM ครั้งแรก
    this->prev_update_time_ = millis();
    this->prev_encoder_ticks_ = 0;
    this->_rpm = 0;

    pinMode(_pinA, OUTPUT);
    pinMode(_pinB, OUTPUT);
}

Motor_encoder::~Motor_encoder()
{
}

void Motor_encoder::setSpeed(int speed)
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
int Motor_encoder::getSpeed()
{
    return this->_speed;
}
int Motor_encoder::getRPM()
{
    long encoder_ticks = this->_en->getCount();

    unsigned long current_time = millis();
    unsigned long dt = current_time - this->prev_update_time_;

    double dtm = (double)dt / 60000;
    // double delta_ticks = encoder_ticks;
    double delta_ticks = encoder_ticks - this->prev_encoder_ticks_;
    this->prev_update_time_ = current_time;
    this->prev_encoder_ticks_ = encoder_ticks;
    // return map(this->_speed, -255, 255, -this->_maxRPM, this->_maxRPM);
    this->_rpm = (delta_ticks / this->_counts_per_rev) / dtm;
    return _rpm;
}

void Motor_encoder::runRPM(int rpm)
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
void Motor_encoder::run(int speed)
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
void Motor_encoder::run()
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