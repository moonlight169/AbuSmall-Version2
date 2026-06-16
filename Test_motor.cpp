#include "Arduino.h"
#include "config.h"       // ไฟล์ config.h ต้องมี MotorPinFLM1_A, MotorPinFLM1_B
#include <PS4Controller.h>

// ไม่ต้องใช้ Motor.h หรือ Kinematics แล้วในสคริปต์ทดสอบนี้
// เราจะคุมขา (Pin) ของไดรเวอร์มอเตอร์ตรงๆ เลยเพื่อทดสอบแรง

// กำหนดขาของมอเตอร์ Lift (ดึงมาจาก config.h)
const int motor_pin_A = MotorPinLift_M1_A;
const int motor_pin_B = MotorPinLift_M1_B;

// ค่า Deadzone ของจอย
const int LStickY_Calib = 20;

void setup() {
  Serial.begin(115200);

  pinMode(motor_pin_A, OUTPUT);
  pinMode(motor_pin_B, OUTPUT);
  
  digitalWrite(motor_pin_A, LOW);
  digitalWrite(motor_pin_B, LOW);

  setCpuFrequencyMhz(160);
  PS4.begin("d4:e9:f4:e2:5e:44");
  
  Serial.println("Ready to test Maximum Power (255)!");
}

void loop() {
  if (!PS4.isConnected()) {
    digitalWrite(motor_pin_A, LOW);
    digitalWrite(motor_pin_B, LOW);
    return;
  }

  int L_Y = PS4.LStickY();

  if (abs(L_Y) > LStickY_Calib) {
    
    if (L_Y > 0) {
      digitalWrite(motor_pin_A, HIGH); 
      digitalWrite(motor_pin_B, LOW);
      Serial.println("Moving UP (FULL POWER)");
      
    } else {
      digitalWrite(motor_pin_A, LOW);
      digitalWrite(motor_pin_B, HIGH);
      Serial.println("Moving DOWN (FULL POWER)");
    }
    
  } else {
    digitalWrite(motor_pin_A, LOW);
    digitalWrite(motor_pin_B, LOW);
  }

  delay(50); // หน่วงเวลาเล็กน้อยให้บอร์ดได้หายใจ
}