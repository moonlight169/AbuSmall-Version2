//This file have Encoder
#ifndef MOTOR_ENCODER_H
#define MOTOR_ENCODER_H

#include <Arduino.h>
#include <ESP32Encoder.h>

class Motor_encoder {
private:
    int _pinA, _pinB, _maxRPM, _speed;
    ESP32Encoder *_en; 
    int _counts_per_rev;   
    float _rpm;      
    unsigned long prev_update_time_; 
    long prev_encoder_ticks_;  

public:
    Motor_encoder(int pinA, int pinB, int maxRPM, ESP32Encoder *en, int counts_per_rev);
    ~Motor_encoder();

    void runRPM(int rpm);
    void run(int speed);
    void run();
    void setSpeed(int speed);
    int getSpeed();
    int getRPM();
};

#endif