#include <Arduino.h>
#include <Wire.h>
#include "Motor.h"
#include "config.h"

#define Address 0x01

Motor MotorFL(MotorPinS1FL_A, MotorPinS1FL_B, S1_MAX_RPM);
Motor MotorFR(MotorPinS1FR_A, MotorPinS1FR_B, S1_MAX_RPM);
Motor MotorRL(MotorPinS1RL_A, MotorPinS1RL_B, S1_MAX_RPM);
Motor MotorRR(MotorPinS1RR_A, MotorPinS1RR_B, S1_MAX_RPM);

int MotorFL_Mode = 0;
int MotorFR_Mode = 0;
int MotorRL_Mode = 0;
int MotorRR_Mode = 0;

char last_command = '\0';
bool new_data_received = false;

// ----------------------------------------------------
// ฟังก์ชันนี้จะทำงานอัตโนมัติ เมื่อ Master ส่งข้อมูลมาให้
// ----------------------------------------------------
void receiveEvent(int howMany) {
  while (Wire.available()) {
    char command = Wire.read();
    last_command = command;
    new_data_received = true;

    if (command == 'A') {
      MotorFL_Mode = (MotorFL_Mode + 1) % 3;
    } else if (command == 'B') {
      MotorFR_Mode = (MotorFR_Mode + 1) % 3;
    } else if (command == 'C') { 
      MotorRL_Mode = (MotorRL_Mode + 1) % 3;
    } else if (command == 'D') {
      MotorRR_Mode = (MotorRR_Mode + 1) % 3;
    } else if (command == 'S') {
      MotorFL_Mode = 0;
      MotorFR_Mode = 0;
      MotorRL_Mode = 0;
      MotorRR_Mode = 0;
    }
  }
}

// ----------------------------------------------------
// ฟังก์ชันนี้จะทำงานอัตโนมัติ เมื่อ Master ทวงถามข้อมูล
// ----------------------------------------------------
// void requestEvent() {

// }

void setup() {
  Serial.begin(115200);

  Wire.begin(Address); 

  // ผูกฟังก์ชัน (Interrupt) ว่าจะให้ทำอะไรเมื่อ Master ติดต่อมา
  Wire.onReceive(receiveEvent); // เมื่อ Master "ส่ง" ข้อมูลมาให้
  // Wire.onRequest(requestEvent); // เมื่อ Master "ร้องขอ" ข้อมูล

  pinMode(limitSWM2_PIN1, INPUT_PULLUP);
  pinMode(limitSWM2_PIN2, INPUT_PULLUP);
  pinMode(limitSWM2_PIN3, INPUT_PULLUP);
  pinMode(limitSWM2_PIN4, INPUT_PULLUP);
  pinMode(limitSWM2_PIN5, INPUT_PULLUP);
  pinMode(limitSWM2_PIN6, INPUT_PULLUP);
  pinMode(limitSWM2_PIN7, INPUT_PULLUP);
  pinMode(limitSWM2_PIN8, INPUT_PULLUP);
}

void loop() {
  if (MotorFL_Mode == 1) {
    if (digitalRead(limitSWM2_PIN1) == HIGH) { 
        MotorFL.runRPM(S1_MAX_RPM); 
    } else {
        MotorFL.runRPM(0);
    }
  } else if (MotorFL_Mode == 2) {
    if (digitalRead(limitSWM2_PIN2) == HIGH) { 
        MotorFL.runRPM(-S1_MAX_RPM); 
    } else {
        MotorFL.runRPM(0); 
    }
  } else {
    MotorFL.runRPM(0);
  }

  if (MotorFR_Mode == 1) {
    if (digitalRead(limitSWM2_PIN3) == HIGH) { 
        MotorFR.runRPM(S1_MAX_RPM); 
    } else {
        MotorFR.runRPM(0);
    }
  } else if (MotorFR_Mode == 2) {
    if (digitalRead(limitSWM2_PIN4) == HIGH) { 
        MotorFR.runRPM(-S1_MAX_RPM); 
    } else {
        MotorFR.runRPM(0); 
    }
  } else {
    MotorFR.runRPM(0);
  }

  if (MotorRL_Mode == 1) {
    if (digitalRead(limitSWM2_PIN5) == HIGH) { 
        MotorRL.runRPM(S1_MAX_RPM); 
    } else {
        MotorRL.runRPM(0);
    }
  } else if (MotorRL_Mode == 2) {
    if (digitalRead(limitSWM2_PIN6) == HIGH) { 
        MotorRL.runRPM(-S1_MAX_RPM); 
    } else {
        MotorRL.runRPM(0); 
    }
  } else {
    MotorRL.runRPM(0);
  }
  
  if (MotorRR_Mode == 1) {
    if (digitalRead(limitSWM2_PIN7) == HIGH) { 
        MotorRR.runRPM(S1_MAX_RPM); 
    } else {
        MotorRR.runRPM(0);
    }
  } else if (MotorRR_Mode == 2) {
    if (digitalRead(limitSWM2_PIN8) == HIGH) { 
        MotorRR.runRPM(-S1_MAX_RPM); 
    } else {
        MotorRR.runRPM(0); 
    }
  } else {
    MotorRR.runRPM(0);
  }
}