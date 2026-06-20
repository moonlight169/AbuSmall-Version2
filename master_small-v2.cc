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

float f_walkspeed = 1.5 * 0.9;
float n_walkspeed = 0.5 * 0.9;
float s_walkspeed = 0.2; 

float f_turnspeed = 3.2 * 0.9;
float n_turnspeed = 1.0 * 0.9;
float s_turnspeed = 0.7; 

float f_slidespeed = 1.0 * 0.9;
float n_slidespeed = 0.5 * 0.9;
float s_slidespeed = 0.5; 

//------------------------------------
float walkspeed = n_walkspeed;
float turnspeed = n_turnspeed;
float slidespeed = n_slidespeed;

// =========================================================
enum SpeedMode { FAST_MODE, NORMAL_MODE, SLOW_MODE };
SpeedMode current_mode = FAST_MODE;

const int TRIGGER_THRESHOLD = 30; 

bool last_options_state = false; 
bool last_circle_state = false;
bool last_square_state = false;
bool last_triangle_state = false;
bool last_x_state = false;
bool last_share_state = false;
bool last_l2_state = false;

const int LStickX_Calib = 100;
const int LStickY_Calib = 100;
const int RStickX_Calib = 100;
const int RStickY_Calib = 20;

int last_box_pwm = 0;
char last_arm_state = 'V';

// =========================================================
float comp_fast_right_x = -0.1;  
float comp_fast_right_z = 0.13;  
float comp_fast_left_x  = 0.3;   
float comp_fast_left_z  = 0.13;  

float comp_slow_right_x = 0.0;   
float comp_slow_right_z = 0.0;   
float comp_slow_left_x  = 0.0;   
float comp_slow_left_z  = 0.0;   
// =========================================================

void moveBase() {
  float forward_backward_speed = g_req_linear_vel_x; 
  float slide_speed = g_req_linear_vel_y;            
  float spin_speed = g_req_angular_vel_z;            

  if (slide_speed != 0 && forward_backward_speed == 0 && spin_speed == 0) {
    bool is_slow_mode = (current_mode != FAST_MODE);

    if (slide_speed > 0) { 
      if (is_slow_mode) {
        forward_backward_speed = comp_slow_right_x; 
        spin_speed = comp_slow_right_z;            
      } else {
        forward_backward_speed = comp_fast_right_x;  
        spin_speed = comp_fast_right_z;  
      }
    } 
    else if (slide_speed < 0) { 
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

  if (last_box_pwm != 0 || last_arm_state != 'V') {
    Serial2.print('S');
    last_box_pwm = 0;
    last_arm_state = 'V';
  }
}

void update_control() {
  if (!PS4.isConnected()) {
    stopAllMotors();
    return;
  }
  
  bool is_L2_pressed = PS4.L2Value() > TRIGGER_THRESHOLD;
  bool is_R2_pressed = PS4.R2Value() > TRIGGER_THRESHOLD;

  if (is_L2_pressed && is_R2_pressed) {
    current_mode = SLOW_MODE;
  } else if (is_L2_pressed) {
    current_mode = SLOW_MODE;   
  } else if (is_R2_pressed) {
    current_mode = NORMAL_MODE; 
  } else {
    current_mode = FAST_MODE;   
  }

  if (current_mode == SLOW_MODE) {
    walkspeed = s_walkspeed; turnspeed = s_turnspeed; slidespeed = s_slidespeed;
  } else if (current_mode == NORMAL_MODE) {
    walkspeed = n_walkspeed; turnspeed = n_turnspeed; slidespeed = n_slidespeed;
  } else {
    walkspeed = f_walkspeed; turnspeed = f_turnspeed; slidespeed = f_slidespeed;
  }

  float d_x = 0;
  float d_y = 0;
  float d_z = 0;

  if (PS4.Up() || PS4.UpRight() || PS4.UpLeft()) d_x = walkspeed;
  else if (PS4.Down() || PS4.DownRight() || PS4.DownLeft()) d_x = -walkspeed;

  if (PS4.Left() || PS4.UpLeft() || PS4.DownLeft()) d_y = -slidespeed;
  else if (PS4.Right() || PS4.UpRight() || PS4.DownRight()) d_y = slidespeed;

  if (PS4.L1()) d_z = turnspeed;
  else if (PS4.R1()) d_z = -turnspeed;

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
  }
  last_x_state = x_pressed;  

  bool circle_pressed = PS4.Circle();
  if (circle_pressed && !last_circle_state) {
    Serial2.write('D');
  } else if (!circle_pressed && last_circle_state) {
    Serial2.write('d');
  }
  last_circle_state = circle_pressed;
}

int getAnalogPWM(int stick_val, int deadzone, int max_speed) {
  if (abs(stick_val) <= deadzone) return 0;
  if (stick_val > 0) {
    return map(stick_val, deadzone, 127, 0, max_speed);
  } else {
    return map(stick_val, -deadzone, -128, 0, -max_speed);
  }
}

void lift_control() {
  int R_Y = PS4.RStickY();
  int L_Y = PS4.LStickY();
  
  // ==========================================
  // 1. ควบคุม BOX (R_Y) แบบ อนาล็อก
  // ==========================================
  int limit_speed = 255; 
  if (current_mode == SLOW_MODE || current_mode == NORMAL_MODE) {
    limit_speed = 120; // โหมดช้า
  }
  int current_box_pwm = getAnalogPWM(R_Y, RStickY_Calib, limit_speed);

  if (current_box_pwm != last_box_pwm) {
    Serial2.print("X"); 
    Serial2.println(current_box_pwm);
    last_box_pwm = current_box_pwm;
  }

  // ==========================================
  // 2. ควบคุม ARM (L_Y) แบบส่งตัวอักษรเดิม
  // ==========================================
  char current_arm_state = 'V'; // ค่า Default
  if (abs(L_Y) > LStickY_Calib) {
    if (L_Y > 0) {
      current_arm_state = (current_mode == FAST_MODE) ? 'G' : 'g';
    } else {
      current_arm_state = (current_mode == FAST_MODE) ? 'H' : 'h';
    }
  } 

  if (current_arm_state != last_arm_state) {
    Serial2.write(current_arm_state);
    last_arm_state = current_arm_state;
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 21, 22);
  PS4.begin(MAC);
}

void loop() {
  unsigned long now = millis();

  if ((now - prev_control_time) >= (1000 / COMMAND_RATE)) {
    update_control();
    moveBase();
    digital_control();
    lift_control();
    prev_control_time = now;
  }
}