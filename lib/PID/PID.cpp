#include "Arduino.h"
#include "PID.h"

PID::PID(float min_val, float max_val, float kp, float ki, float kd) : min_val_(min_val),
                                                                       max_val_(max_val),
                                                                       kp_(kp),
                                                                       ki_(ki),
                                                                       kd_(kd)
{
}

double PID::compute(float setpoint, float measured_value)
{
    double error;
    double pid;

    // setpoint is constrained between min and max to prevent pid from having too much error
    error = setpoint - measured_value;
    this->integral_ += error;
    this->derivative_ = error - this->prev_error_;

    if (setpoint == 0)
    {
        this->integral_ = 0;
        this->derivative_ = 0;
    }

    pid = (kp_ * error) + (ki_ * integral_) + (kd_ * derivative_);
    this->prev_error_ = error;

    return constrain(pid, min_val_, max_val_);
}

void PID::updateConstants(float kp, float ki, float kd)
{
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    this->integral_ = 0;
    this->derivative_ = 0;
    this->prev_error_ = 0;
}