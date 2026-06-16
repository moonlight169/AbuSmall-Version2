#include "Arduino.h"
#include "Motor_encoder.h"
#include "config.h"
#include "Holding.h"
#include "Kinematics.h"
#include <ESP32Encoder.h>
#include "PID.h"
#include <PS4Controller.h>

//a4:cf:12:87:7c:cc

ESP32Encoder EncoderFL;
ESP32Encoder EncoderFR;
ESP32Encoder EncoderRL;
ESP32Encoder EncoderRR;

Motor_encoder MotorFL(MotorPinFLM2_A, MotorPinFLM2_B, M2_MAX_RPM, &EncoderFL, COUNT_PER_REV_FL);
Motor_encoder MotorFR(MotorPinFRM2_A, MotorPinFRM2_B, M2_MAX_RPM, &EncoderFR, COUNT_PER_REV_FR);
Motor_encoder MotorRL(MotorPinRLM2_A, MotorPinRLM2_B, M2_MAX_RPM, &EncoderRL, COUNT_PER_REV_RL);
Motor_encoder MotorRR(MotorPinRRM2_A, MotorPinRRM2_B, M2_MAX_RPM, &EncoderRR, COUNT_PER_REV_RR);

Kinematics kinematics(Kinematics::MECANUM, M2_MAX_RPM, WHEEL_DIAMETER_M2, FR_WHEELS_DISTANCE_M2, LR_WHEELS_DISTANCE_M2);

#define COMMAND_RATE 50
unsigned long prev_control_time = 0;

float g_req_linear_vel_x = 0;
float g_req_linear_vel_y = 0;
float g_req_angular_vel_z = 0;

float f_walkspeed = 1.0;
float n_walkspeed = 0.5;

float f_turnspeed = 1.5;
float n_turnspeed = 1.0;

float f_slidespeed = 0.8;
float n_slidespeed = 0.5;
//------------------------------------
float walkspeed = f_walkspeed;
float turnspeed = f_turnspeed;
float slidespeed = f_slidespeed;

int speed_mode = 0; 

bool last_options_state = false; 

const int LStickX_Calib = 40;
const int LStickY_Calib = 20;
const int RStickX_Calib = 20;

Holding Holding;

void moveBase() {
  Kinematics::rpm req_rpm = kinematics.getRPM(g_req_linear_vel_x, g_req_linear_vel_y, g_req_angular_vel_z);

  MotorFL.runRPM(req_rpm.motor1);
  MotorFR.runRPM(req_rpm.motor2);
  MotorRL.runRPM(req_rpm.motor3);
  MotorRR.runRPM(req_rpm.motor4);
}

void update_control() {
  if (!PS4.isConnected()) {
    g_req_linear_vel_x = 0; 
    g_req_linear_vel_y = 0; 
    g_req_angular_vel_z = 0;
    MotorFL.run(0); MotorFR.run(0); MotorRL.run(0); MotorRR.run(0);
    return;
  }

  bool current_options_state = PS4.Options();
  if (current_options_state && !last_options_state) {
    speed_mode = (speed_mode + 1) % 2;
    if (speed_mode == 0) {
      walkspeed = f_walkspeed;
      turnspeed = f_turnspeed;
      slidespeed = f_slidespeed;
    } else {
      walkspeed = n_walkspeed;
      turnspeed = n_turnspeed;
      slidespeed = n_slidespeed;
    }
  }
  last_options_state = current_options_state;

  float walk_speed = walkspeed;
  float turn_speed = turnspeed;
  float slide_speed = slidespeed;

  float d_x = 0;
  float d_y = 0;
  float d_z = 0;
  float a_x = 0;
  float a_y = 0;
  float a_z = 0;

  // --- ควบคุมการเดินหน้า / ถอยหลัง (D-Pad บน/ล่าง และแนวเฉียง) ---
  if (PS4.Up() || PS4.UpRight() || PS4.UpLeft()) {
    d_x = walk_speed;
  } else if (PS4.Down() || PS4.DownRight() || PS4.DownLeft()) {
    d_x = -walk_speed;
  }
  
  // --- ควบคุมการสไลด์ซ้าย / ขวา (D-Pad ซ้าย/ขวา และแนวเฉียง) ---
  if (PS4.Left() || PS4.UpLeft() || PS4.DownLeft()) {
    d_y = slide_speed;
  } else if (PS4.Right() || PS4.UpRight() || PS4.DownRight()) {
    d_y = -slide_speed;
  }

  // --- ควบคุมการหมุนตัว (L1 / R1) ---
  if (PS4.L1()) {
    d_z = turn_speed;  // L1 = หมุนตัวทวนเข็มนาฬิกา
  } else if (PS4.R1()) {
    d_z = -turn_speed; // R1 = หมุนตัวตามเข็มนาฬิกา
  }

  int L_X = PS4.LStickX();
  int L_Y = PS4.LStickY();
  int R_X = PS4.RStickX();

  if (abs(L_Y) > LStickY_Calib) {
    a_x = ((float)L_Y / 127.0) * walk_speed;
  }
  if (abs(R_X) > RStickX_Calib) {

    a_y = ((float)R_X / 127.0) * -slide_speed;
  }
  if (abs(L_X) > LStickX_Calib) {
    a_z = ((float)L_X / 127.0) * -turn_speed;
  }

  g_req_linear_vel_x = a_x + d_x;
  g_req_linear_vel_y = a_y + d_y;
  g_req_angular_vel_z = a_z + d_z;

  g_req_linear_vel_x = constrain(g_req_linear_vel_x, -walk_speed, walk_speed);
  g_req_linear_vel_y = constrain(g_req_linear_vel_y, -slide_speed, slide_speed);
  g_req_angular_vel_z = constrain(g_req_angular_vel_z, -turn_speed, turn_speed);
}

void setup() {
  Serial.begin(115200);
  setCpuFrequencyMhz(240);
  PS4.begin("a4:cf:12:87:7c:cc");
  Wire.begin(SDA_PIN, SCL_PIN);
  Holding.init();
  EncoderFL.attachSingleEdge(EncoderPinFL_A, EncoderPinFL_B);
  EncoderFR.attachSingleEdge(EncoderPinFR_A, EncoderPinFR_B);
  EncoderRL.attachSingleEdge(EncoderPinRL_A, EncoderPinRL_B);
  EncoderRR.attachSingleEdge(EncoderPinRR_A, EncoderPinRR_B);
}

void loop() {
  update_control();

  unsigned long now = millis();
  if ((now - prev_control_time) >= (1000 / COMMAND_RATE)) {
    moveBase();
    prev_control_time = now;

    // --- เพิ่มส่วนนี้เพื่อ Debug ค่า Encoder ---
    Serial.print("FL: "); Serial.print(EncoderFL.getCount());
    Serial.print(" | FR: "); Serial.print(EncoderFR.getCount());
    Serial.print(" | RL: "); Serial.print(EncoderRL.getCount());
    Serial.print(" | RR: "); Serial.println(EncoderRR.getCount());
    // ---------------------------------------
  }
}