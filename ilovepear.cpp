#include "Arduino.h"
#include "Motor.h"
#include "config.h"
#include "Holding.h"
#include "Kinematics.h"
#include <PS4Controller.h>
#include <Wire.h>

// ============================================
// Constants
// ============================================
#define ADDRESS_SLAVE 0x04
#define COMMAND_RATE 50
#define I2C_CLOCK_SPEED 400000
#define PS4_MAC_ADDRESS "08:a6:f7:10:a8:5c"
#define CPU_FREQUENCY_MHZ 240

// Stick calibration thresholds
#define LSTICK_X_THRESHOLD 100
#define LSTICK_Y_THRESHOLD 100
#define RSTICK_X_THRESHOLD 100
#define RSTICK_Y_THRESHOLD 100

// Speed profiles
#define NORMAL_WALK_SPEED 0.5f
#define FAST_WALK_SPEED 1.4f
#define NORMAL_TURN_SPEED 2.0f
#define FAST_TURN_SPEED 3.2f
#define NORMAL_SLIDE_SPEED 1.2f
#define FAST_SLIDE_SPEED 2.0f

// ============================================
// Motor Instances
// ============================================
Motor MotorFL(MotorPinFLM1_A, MotorPinFLM1_B, M_MAX_RPM);
Motor MotorFR(MotorPinFRM1_A, MotorPinFRM1_B, M_MAX_RPM);
Motor MotorRL(MotorPinRLM1_A, MotorPinRLM1_B, M_MAX_RPM);
Motor MotorRR(MotorPinRRM1_A, MotorPinRRM1_B, M_MAX_RPM);

Kinematics kinematics(Kinematics::MECANUM, M_MAX_RPM, WHEEL_DIAMETER, 
                      FR_WHEELS_DISTANCE, LR_WHEELS_DISTANCE);

// ============================================
// Control State Variables
// ============================================
typedef struct {
  float req_linear_vel_x;
  float req_linear_vel_y;
  float req_angular_vel_z;
} MotorVelocity;

typedef struct {
  bool triangle;
  bool circle;
  bool square;
  bool cross;
  bool options;
  bool share;
} ButtonState;

typedef struct {
  float walk;
  float turn;
  float slide;
} SpeedProfile;

MotorVelocity motor_velocity = {0, 0, 0};
ButtonState button_state = {false, false, false, false, false, false};
ButtonState button_state_prev = {false, false, false, false, false, false};

SpeedProfile speed_normal = {NORMAL_WALK_SPEED, NORMAL_TURN_SPEED, NORMAL_SLIDE_SPEED};
SpeedProfile speed_fast = {FAST_WALK_SPEED, FAST_TURN_SPEED, FAST_SLIDE_SPEED};
SpeedProfile *current_speed_profile = &speed_normal;

char lift_state = 'S';
char lift_state_prev = 'S';

unsigned long prev_control_time = 0;

// ============================================
// Helper Functions - I2C Communication
// ============================================
bool sendI2CCommand(uint8_t address, char command) {
  Wire.beginTransmission(address);
  Wire.write(command);
  uint8_t error = Wire.endTransmission();
  
  if (error != 0) {
    Serial.printf("I2C Error: %d sending '%c'\n", error, command);
    return false;
  }
  return true;
}

// ============================================
// Control Functions
// ============================================
void updateControlProfile() {
  // Select speed profile based on R2 button
  current_speed_profile = PS4.R2() ? &speed_normal : &speed_fast;
}

void updateMotorVelocity() {
  float d_x = 0;
  float d_y = 0;
  float d_z = 0;

  // Forward / Backward movement
  if (PS4.Up() || PS4.UpRight() || PS4.UpLeft()) {
    d_x = current_speed_profile->walk;
  } 
  else if (PS4.Down() || PS4.DownRight() || PS4.DownLeft()) {
    d_x = -current_speed_profile->walk;
  }

  // Slide Left / Right
  if (PS4.Left() || PS4.UpLeft() || PS4.DownLeft()) {
    d_y = current_speed_profile->slide;
  } 
  else if (PS4.Right() || PS4.UpRight() || PS4.DownRight()) {
    d_y = -current_speed_profile->slide;
  }

  // Rotation
  if (PS4.L1()) {
    d_z = current_speed_profile->turn;
  } 
  else if (PS4.R1()) {
    d_z = -current_speed_profile->turn;
  }

  motor_velocity.req_linear_vel_x = d_x;
  motor_velocity.req_linear_vel_y = d_y;
  motor_velocity.req_angular_vel_z = d_z;
}

void moveBase() {
  Kinematics::rpm req_rpm = kinematics.getRPM(
    motor_velocity.req_linear_vel_x, 
    motor_velocity.req_linear_vel_y, 
    motor_velocity.req_angular_vel_z
  );

  MotorFL.runRPM(req_rpm.motor1);
  MotorFR.runRPM(req_rpm.motor2);
  MotorRL.runRPM(req_rpm.motor3);
  MotorRR.runRPM(req_rpm.motor4);
}

void stopAllMotors() {
  motor_velocity.req_linear_vel_x = 0;
  motor_velocity.req_linear_vel_y = 0;
  motor_velocity.req_angular_vel_z = 0;
  
  MotorFL.run(0);
  MotorFR.run(0);
  MotorRL.run(0);
  MotorRR.run(0);

  // Stop lift if it was moving
  if (lift_state != 'S') {
    sendI2CCommand(ADDRESS_SLAVE, 'S');
    lift_state = 'S';
  }
}

// ============================================
// Digital Control (Relays & I2C)
// ============================================
void updateButtonState() {
  button_state.triangle = PS4.Triangle();
  button_state.circle = PS4.Circle();
  button_state.square = PS4.Square();
  button_state.cross = PS4.Cross();
  button_state.options = PS4.Options();
  button_state.share = PS4.Share();
}

void handleRelayControl() {
  // Triangle: Toggle RelayM1_PIN3
  if (button_state.triangle && !button_state_prev.triangle) {
    digitalWrite(RelayM1_PIN3, !digitalRead(RelayM1_PIN3));
  }

  // Cross: Toggle RelayM1_PIN2
  digitalWrite(RelayM1_PIN2, button_state.cross ? LOW : HIGH);

  // Share: Toggle RelayM1_PIN1
  if (button_state.share && !button_state_prev.share) {
    digitalWrite(RelayM1_PIN1, !digitalRead(RelayM1_PIN1));
  }

  // Options: Toggle RelayM1_PIN4
  if (button_state.options && !button_state_prev.options) {
    digitalWrite(RelayM1_PIN4, !digitalRead(RelayM1_PIN4));
  }
}

void handleSlaveI2CControl() {
  // Square: Send 'A' to slave
  if (button_state.square && !button_state_prev.square) {
    sendI2CCommand(ADDRESS_SLAVE, 'A');
  }

  // Circle: Send 'B' to slave
  if (button_state.circle && !button_state_prev.circle) {
    sendI2CCommand(ADDRESS_SLAVE, 'B');
  }
}

void handleLiftControl() {
  int r_y = PS4.RStickY();
  int l_y = PS4.LStickY();
  
  char current_lift_state = lift_state;

  // Right stick Y-axis control
  if (abs(r_y) > RSTICK_Y_THRESHOLD) {
    if (r_y > 0) {
      current_lift_state = (current_speed_profile == &speed_fast) ? 'D' : 'd';
    } else {
      current_lift_state = (current_speed_profile == &speed_fast) ? 'U' : 'u';
    }
  }
  // Left stick Y-axis control
  else if (abs(l_y) > LSTICK_Y_THRESHOLD) {
    if (l_y > 0) {
      current_lift_state = (current_speed_profile == &speed_fast) ? 'O' : 'o';
    } else {
      current_lift_state = (current_speed_profile == &speed_fast) ? 'C' : 'c';
    }
  }
  // No input - stop if moving
  else {
    if (lift_state == 'U' || lift_state == 'u' || 
        lift_state == 'D' || lift_state == 'd') {
      current_lift_state = 'S';
    }
  }

  // Send command if state changed
  if (current_lift_state != lift_state) {
    if (sendI2CCommand(ADDRESS_SLAVE, current_lift_state)) {
      lift_state = current_lift_state;
    }
  }
}

// ============================================
// Main Control Update
// ============================================
void updateControl() {
  if (!PS4.isConnected()) {
    stopAllMotors();
    return;
  }

  updateControlProfile();
  updateMotorVelocity();
  updateButtonState();
  
  handleRelayControl();
  handleSlaveI2CControl();
  handleLiftControl();

  // Update previous button state for next cycle
  button_state_prev = button_state;
}

// ============================================
// Setup & Loop
// ============================================
void setup() {
  Serial.begin(115200);
  delay(500); // Stabilization delay
  
  setCpuFrequencyMhz(CPU_FREQUENCY_MHZ);
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(I2C_CLOCK_SPEED);
  
  // Initialize PS4 Controller
  PS4.begin(PS4_MAC_ADDRESS);
  
  // Initialize relay pins
  pinMode(RelayM1_PIN1, OUTPUT);
  pinMode(RelayM1_PIN2, OUTPUT);
  pinMode(RelayM1_PIN3, OUTPUT);
  pinMode(RelayM1_PIN4, OUTPUT);
  
  // Set initial relay states (HIGH = inactive)
  digitalWrite(RelayM1_PIN1, HIGH);
  digitalWrite(RelayM1_PIN2, HIGH);
  digitalWrite(RelayM1_PIN3, HIGH);
  digitalWrite(RelayM1_PIN4, HIGH);
  
  Serial.println("System initialized successfully");
}

void loop() {
  updateControl();

  unsigned long now = millis();
  if ((now - prev_control_time) >= (1000 / COMMAND_RATE)) {
    moveBase();
    prev_control_time = now;
  }
}
