#include "Arduino.h"
#include "Motor.h"
#include "config.h"
#include "Holding.h"
#include "Kinematics.h"
#include <ps5Controller.h> // เปลี่ยนเป็นไลบรารีของ PS5
#include "esp_bt.h"

Motor MotorFL(MotorPinFLM1_A, MotorPinFLM1_B, MAX_RPM);
Motor MotorFR(MotorPinFRM1_A, MotorPinFRM1_B, MAX_RPM);
Motor MotorRL(MotorPinRLM1_A, MotorPinRLM1_B, MAX_RPM);
Motor MotorRR(MotorPinRRM1_A, MotorPinRRM1_B, MAX_RPM);

Kinematics kinematics(Kinematics::MECANUM, MAX_RPM, WHEEL_DIAMETER, FR_WHEELS_DISTANCE, LR_WHEELS_DISTANCE);

#define COMMAND_RATE 50
unsigned long prev_control_time = 0;

float Nerf = 0.9;

float g_req_linear_vel_x = 0;
float g_req_linear_vel_y = 0;
float g_req_angular_vel_z = 0;

float f_walkspeed = 1.2 * Nerf;
float n_walkspeed = 0.5 * (Nerf - 0.1);
float s_walkspeed = 0.2; 

float f_turnspeed = 3.2 * Nerf;
float n_turnspeed = 1.0 * Nerf;
float s_turnspeed = 0.7; 

float f_slidespeed = 1.0 * Nerf;
float n_slidespeed = 0.5 * Nerf;
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
float comp_fast_right_x = 0.0;  
float comp_fast_right_z = 0.0;  
float comp_fast_left_x  = 0.0;   
float comp_fast_left_z  = 0.0;  

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
  if (!ps5.isConnected()) { // เปลี่ยนเป็น ps5
    stopAllMotors();
    return;
  }
  
  bool is_L2_pressed = ps5.L2Value() > TRIGGER_THRESHOLD; // เปลี่ยนเป็น ps5
  bool is_R2_pressed = ps5.R2Value() > TRIGGER_THRESHOLD; // เปลี่ยนเป็น ps5

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

  if (ps5.Up() || ps5.UpRight() || ps5.UpLeft()) d_x = walkspeed; // เปลี่ยนเป็น ps5
  else if (ps5.Down() || ps5.DownRight() || ps5.DownLeft()) d_x = -walkspeed; // เปลี่ยนเป็น ps5

  if (ps5.Left() || ps5.UpLeft() || ps5.DownLeft()) d_y = -slidespeed; // เปลี่ยนเป็น ps5
  else if (ps5.Right() || ps5.UpRight() || ps5.DownRight()) d_y = slidespeed; // เปลี่ยนเป็น ps5

  if (ps5.L1()) d_z = turnspeed; // เปลี่ยนเป็น ps5
  else if (ps5.R1()) d_z = -turnspeed; // เปลี่ยนเป็น ps5

  g_req_linear_vel_x = d_x;
  g_req_linear_vel_y = d_y;
  g_req_angular_vel_z = d_z;
}

void digital_control(){
  bool triangle_pressed = ps5.Triangle(); // เปลี่ยนเป็น ps5
  if (triangle_pressed && !last_triangle_state) {
    Serial2.write('A'); 
  }
  last_triangle_state = triangle_pressed;

  bool square_pressed = ps5.Square(); // เปลี่ยนเป็น ps5
  if (square_pressed && !last_square_state) {
    Serial2.write('B'); 
  }
  last_square_state = square_pressed;

  bool x_pressed = ps5.Cross(); // เปลี่ยนเป็น ps5
  if (x_pressed && !last_x_state) {
    Serial2.write('C'); 
  }
  last_x_state = x_pressed;  

  bool circle_pressed = ps5.Circle(); // เปลี่ยนเป็น ps5
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
  int R_Y = ps5.RStickY(); // เปลี่ยนเป็น ps5
  int L_Y = ps5.LStickY(); // เปลี่ยนเป็น ps5
  
  int limit_speed = 255; 
  if (current_mode == SLOW_MODE || current_mode == NORMAL_MODE) {
    limit_speed = 120; 
  }
  int current_box_pwm = getAnalogPWM(R_Y, RStickY_Calib, limit_speed);

  if (current_box_pwm != last_box_pwm) {
    Serial2.print("X"); 
    Serial2.println(current_box_pwm);
    last_box_pwm = current_box_pwm;
  }

  char current_arm_state = 'V'; 
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
  setCpuFrequencyMhz(240);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 21, 22);

  ps5.begin(MAC_PS5); // เปลี่ยนเป็น ps5

  esp_bredr_tx_power_set(ESP_PWR_LVL_P9, ESP_PWR_LVL_P9); 

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
  
  Serial.println("Bluetooth TX Power unlocked to MAX (+9dBm)!");
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