#ifndef HOLDING_H
#define HOLDING_H

#include "Arduino.h"
#include <pcf8574.h>

PCF8574 Extended_I2C(0x20);

class Holding
{
public:
     const int PIN_State_Holding_Relay_Creeper_1 = 0;
     const int PIN_State_Holding_Relay_Creeper_2 = 1;
     const int PIN_State_Holding_Relay_Slider = 2;
     const int PIN_State_Holding_Relay_Lift = 3;
     const int PIN_State_Holding_Relay_SHOOT = 6;
     const int PIN_State_Holding_Relay_Motor = 5;
     const int param_vTaskDelay = 350;

     int State_Holding_Relay_Creeper_1 = 0;
     int State_Holding_Relay_Creeper_2 = 0;
     int State_Holding_Relay_Slider = 0;
     int State_Holding_Relay_Lift = 0;
     int State_Holding_Relay_SHOOT = 0;
     int State_Holding_Relay_Motor = 0;
     void toggleState(int &state)
     {
          state = state == 0 ? 1 : 0;
     }

     void updateState(int pin, int state)
     {
          digitalWrite(Extended_I2C, pin, state);
     }

     void init()
     {
          pinMode(Extended_I2C, PIN_State_Holding_Relay_Creeper_1, OUTPUT);
          pinMode(Extended_I2C, PIN_State_Holding_Relay_Creeper_2, OUTPUT);
          pinMode(Extended_I2C, PIN_State_Holding_Relay_Slider, OUTPUT);
          pinMode(Extended_I2C, PIN_State_Holding_Relay_Lift, OUTPUT);
          pinMode(Extended_I2C, PIN_State_Holding_Relay_SHOOT, OUTPUT);
          pinMode(Extended_I2C, PIN_State_Holding_Relay_Motor, OUTPUT);
          digitalWrite(Extended_I2C, PIN_State_Holding_Relay_Motor, LOW);
          //  Serial.println("Holding Setup Success");
     }

     void toggleState_Holding_Relay_Creeper_1()
     {
          vTaskDelay(param_vTaskDelay);
          // Serial.println("State_Holding_Relay_Creeper_1");
          toggleState(State_Holding_Relay_Creeper_1);
          updateState(PIN_State_Holding_Relay_Creeper_1, State_Holding_Relay_Creeper_1);
     }

     void toggleState_Holding_Relay_Creeper_2()
     {
          vTaskDelay(param_vTaskDelay);
          // Serial.println("State_Holding_Relay_Creeper_2");
          toggleState(State_Holding_Relay_Creeper_2);
          updateState(PIN_State_Holding_Relay_Creeper_2, State_Holding_Relay_Creeper_2);
     }

     void toggleState_Holding_Relay_Dual_Creeper()
     {
          vTaskDelay(param_vTaskDelay);
          toggleState(State_Holding_Relay_Creeper_1);
          toggleState(State_Holding_Relay_Creeper_2);
          updateState(PIN_State_Holding_Relay_Creeper_1, State_Holding_Relay_Creeper_1);
          updateState(PIN_State_Holding_Relay_Creeper_2, State_Holding_Relay_Creeper_2);
     }

     void toggleState_Holding_Relay_Slider()
     {
          vTaskDelay(param_vTaskDelay);
          // Serial.println("State_Holding_Relay_Slider");
          toggleState(State_Holding_Relay_Slider);
          updateState(PIN_State_Holding_Relay_Slider, State_Holding_Relay_Slider);
     }

     void toggleState_Holding_Relay_Lift()
     {
          vTaskDelay(param_vTaskDelay);
          // Serial.println("State_Holding_Relay_Lift");
          toggleState(State_Holding_Relay_Lift);
          updateState(PIN_State_Holding_Relay_Lift, State_Holding_Relay_Lift);
     }
     void toggleState_Holding_Relay_SHOOT()
     {
          vTaskDelay(param_vTaskDelay);
          digitalWrite(Extended_I2C, PIN_State_Holding_Relay_SHOOT, LOW);
          vTaskDelay(700);
          digitalWrite(Extended_I2C, PIN_State_Holding_Relay_SHOOT, HIGH);
     }
     void toggleState_Holding_Relay_Motor()
     {
          vTaskDelay(param_vTaskDelay);
          // Serial.println("State_Holding_Relay_Motor");
          toggleState(State_Holding_Relay_Motor);
          updateState(PIN_State_Holding_Relay_Motor, State_Holding_Relay_Motor);
     }
};

#endif