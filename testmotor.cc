#include <Arduino.h>
#include <PS4Controller.h>
#include "config.h"
#include "Motor.h"

Motor MotorBOX(MT_BOX_A, MT_BOX_B, MAX_RPM);

const int testSpeed = 100; // ความเร็วสำหรับทดสอบ

void setup() {
  Serial.begin(115200);

  PS4.begin(MAC);
  Serial.println("รอการเชื่อมต่อจอย PS4...");
}

void loop() {
  if (!PS4.isConnected()) {
    MotorBOX.run(0);
    return; 
  }

  if (PS4.Up()) {
    MotorBOX.run(testSpeed);
  } 
  else if (PS4.Down()) {
    MotorBOX.run(-testSpeed);
  } 
  else {
    MotorBOX.run(0);
  }

  delay(20);
}