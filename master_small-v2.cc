#include "Arduino.h"
#include "Motor.h"
#include "config.h"
#include "Holding.h"
#include "Kinematics.h"
#include <PS4Controller.h>

Motor MotorFL(MotorPinFLM1_A, MotorPinFLM1_B, MAX_RPM);
Motor MotorFR(MotorPinFRM1_A, MotorPinFRM1_B, MAX_RPM);
Motor MotorRL(MotorPinRLM1_A, MotorPinRLM1_B, MAX_RPM);
Motor MotorRR(MotorPinRRM1_A, MotorPinRRM1_B, MAX_RPM);

Kinematics kinematics(Kinematics::MECANUM, MAX_RPM, WHEEL_DIAMETER, FR_WHEELS_DISTANCE, LR_WHEELS_DISTANCE);

#define COMMAND_RATE 50
unsigned long prev_control_time = 0;

float g_req_linear_vel_x = 0;
float g_req_linear_vel_y = 0;
float g_req_angular_vel_z = 0;

float f_walkspeed = 1.2+0.3;
float n_walkspeed = 0.5;

float f_turnspeed = 3.0+0.2;
float n_turnspeed = 1.0;

float f_slidespeed = 1.0;
float n_slidespeed = 0.5;
//------------------------------------
float walkspeed = n_walkspeed;
float turnspeed = n_turnspeed;
float slidespeed = n_slidespeed;

bool last_options_state = false; 
bool last_circle_state = false;
bool last_square_state = false;
bool last_triangle_state = false;
bool last_x_state = false;
bool last_share_state = false;
bool last_l2_state = false;

char last_lift_state = 'S'; 

const int LStickX_Calib = 100;
const int LStickY_Calib = 100;
const int RStickX_Calib = 100;
const int RStickY_Calib = 100;

// =========================================================
// 🎯 โซนตั้งค่า: ปรับจูนอาการเบี้ยวตอนสไลด์ (แยกโหมด ช้า/เร็ว)
// นำไปวางไว้ด้านบนของฟังก์ชัน moveBase หรือตรงกลุ่มตัวแปร Global
// =========================================================

// --- 🔴 โหมดปกติ / เร็ว (ไม่กด R2) ---
float comp_fast_right_x = -0.1;  // สไลด์ขวา: แก้เบี้ยวหน้า-หลัง (บวก=เดินหน้า, ลบ=ถอยหลัง)
float comp_fast_right_z = 0.13;  // สไลด์ขวา: แก้บิดหมุน (บวก=หมุนซ้าย, ลบ=หมุนขวา)
float comp_fast_left_x  = 0.3;  // สไลด์ซ้าย: แก้เบี้ยวหน้า-หลัง
float comp_fast_left_z  = 0.13;  // สไลด์ซ้าย: แก้บิดหมุน

// --- 🐢 โหมดช้า (กด R2) ---
float comp_slow_right_x = 0.0;  // สไลด์ขวา: แก้เบี้ยวหน้า-หลัง 
float comp_slow_right_z = 0.0;  // สไลด์ขวา: แก้บิดหมุน
float comp_slow_left_x  = 0.0;  // สไลด์ซ้าย: แก้เบี้ยวหน้า-หลัง
float comp_slow_left_z  = 0.0;  // สไลด์ซ้าย: แก้บิดหมุน

// =========================================================


void moveBase() {
  float forward_backward_speed = g_req_linear_vel_x; // เดินหน้า(+), ถอยหลัง(-)
  float slide_speed = g_req_linear_vel_y;            // สไลด์ขวา(+), สไลด์ซ้าย(-)
  float spin_speed = g_req_angular_vel_z;            // หมุนซ้าย(+), หมุนขวา(-)

  if (slide_speed != 0 && forward_backward_speed == 0 && spin_speed == 0) {
    
    // เช็คโหมดความเร็ว: ถ้ายอดความเร็วสไลด์เท่ากับ n_slidespeed แสดงว่าเป็นโหมดช้า
    bool is_slow_mode = (abs(slide_speed) == n_slidespeed);

    if (slide_speed > 0) { 
      // ==========================================
      // ➡️ โหมด: กำลังสไลด์ขวา
      // ==========================================
      if (is_slow_mode) {
        forward_backward_speed = comp_slow_right_x; 
        spin_speed = comp_slow_right_z;             
      } else {
        forward_backward_speed = comp_fast_right_x;  
        spin_speed = comp_fast_right_z;  
      }
    } 
    else if (slide_speed < 0) { 
      // ==========================================
      // ⬅️ โหมด: กำลังสไลด์ซ้าย
      // ==========================================
      if (is_slow_mode) {
        forward_backward_speed = comp_slow_left_x; 
        spin_speed = comp_slow_left_z;             
      } else {
        forward_backward_speed = comp_fast_left_x; 
        spin_speed = comp_fast_left_z;  
      }
    }
  }

  Kinematics::rpm req_rpm = kinematics.getRPM(forward_backward_speed, slide_speed, spin_speed);

  MotorFL.runRPM(req_rpm.motor1);
  MotorFR.runRPM(req_rpm.motor2);
  MotorRL.runRPM(req_rpm.motor3);
  MotorRR.runRPM(req_rpm.motor4);
}

void stopAllMotors() {
  g_req_linear_vel_x = 0;
  g_req_linear_vel_y = 0;
  g_req_angular_vel_z = 0;
  MotorFL.run(0); MotorFR.run(0); MotorRL.run(0); MotorRR.run(0);
  
  if (last_lift_state != 'S') {
    Serial2.write('S');

    last_lift_state = 'S';
  }
}

void update_control() {
  if (!PS4.isConnected()) {
    stopAllMotors();
    return;
  }
  
  if (PS4.R2()) {
    walkspeed = n_walkspeed;
    turnspeed = n_turnspeed;
    slidespeed = n_slidespeed;
  } else {
    walkspeed = f_walkspeed;
    turnspeed = f_turnspeed;
    slidespeed = f_slidespeed;
  }

  float d_x = 0;
  float d_y = 0;
  float d_z = 0;

  // ควบคุมการเดินหน้า / ถอยหลัง
  if (PS4.Up() || PS4.UpRight() || PS4.UpLeft()) d_x = walkspeed;
  else if (PS4.Down() || PS4.DownRight() || PS4.DownLeft()) d_x = -walkspeed;

  // ควบคุมการสไลด์ซ้าย / ขวา
  if (PS4.Left() || PS4.UpLeft() || PS4.DownLeft()) d_y = -slidespeed;
  else if (PS4.Right() || PS4.UpRight() || PS4.DownRight()) d_y = slidespeed;

  // ควบคุมการหมุนตัว
  if (PS4.L1()) d_z = turnspeed;
  else if (PS4.R1()) d_z = -turnspeed;

  // อัปเดตค่าเข้า Global Variable
  g_req_linear_vel_x = d_x;
  g_req_linear_vel_y = d_y;
  g_req_angular_vel_z = d_z;
}

void digital_control(){
  bool triangle_pressed = PS4.Triangle();
  if (triangle_pressed && !last_triangle_state) {
    Serial2.write('A'); 
  }
  last_triangle_state = triangle_pressed;

  bool square_pressed = PS4.Square();
  if (square_pressed && !last_square_state) {
    Serial2.write('B'); 
  }
  last_square_state = square_pressed;

  bool x_pressed = PS4.Cross();
  if (x_pressed && !last_x_state) {
    Serial2.write('C');
  } else if (!x_pressed && last_x_state) {
    Serial2.write('c');
  }
  last_x_state = x_pressed;

  bool l2_pressed = PS4.L2();
  if (l2_pressed && !last_l2_state) {
    Serial2.write('D'); 
  }
  last_l2_state = l2_pressed;

  // if (PS4.L2()) {
  //   Serial2.write('D'); 
  // }

  
//   bool share_pressed = PS4.Share();
//   if (share_pressed && !last_share_state) {
//     digitalWrite(RelayM1_PIN1, !digitalRead(RelayM1_PIN1));
//   }
//   last_share_state = share_pressed;

//   bool options_pressed = PS4.Options();
//   if (options_pressed && !last_options_state) {
//     digitalWrite(RelayM1_PIN4, !digitalRead(RelayM1_PIN4));
//   }
//   last_options_state = options_pressed;

//   // sent to slave


//   bool circle_pressed = PS4.Circle();
//   if (circle_pressed && !last_circle_state) {
//     Wire.beginTransmission(Address_Small);
//     Wire.write('B');
//     Wire.endTransmission();
//     }
//     last_circle_state = circle_pressed;  
}

void lift_control() {
  int R_Y = PS4.RStickY();
  int L_Y = PS4.LStickY();
  
  char current_state = last_lift_state; 

  if (abs(R_Y) > RStickY_Calib) {
    if (R_Y > 0) {
      current_state = (walkspeed == f_walkspeed) ? 'E' : 'e';
    } else {
      current_state = (walkspeed == f_walkspeed) ? 'F' : 'f';
    }
  } 

  else if (abs(L_Y) > LStickY_Calib) {
    if (L_Y > 0) {
      current_state = (walkspeed == f_walkspeed) ? 'G' : 'g';
    } else {
      current_state = (walkspeed == f_walkspeed) ? 'H' : 'h';
    }
  } 
  else {
    if (last_lift_state == 'E' || last_lift_state == 'e' || 
        last_lift_state == 'F' || last_lift_state == 'f' ||
        last_lift_state == 'G' || last_lift_state == 'g' || 
        last_lift_state == 'H' || last_lift_state == 'h' ) {
      current_state = 'S'; 
    }
  }

  if (current_state != last_lift_state) {
    Serial2.write(current_state);

    last_lift_state = current_state;
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 21, 22);

  // setCpuFrequencyMhz(240);
  PS4.begin(MAC);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
}

void loop() {
  update_control();

  unsigned long now = millis();
  if ((now - prev_control_time) >= (1000 / COMMAND_RATE)) {
    moveBase();
    digital_control();
    lift_control();

    prev_control_time = now;
  }
}