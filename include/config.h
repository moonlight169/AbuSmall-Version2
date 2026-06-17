//master small
#define rx_pin 21
#define tx_pin 22

#define MAC "c0:cd:d6:8d:0a:64"
#define MAX_RPM 200

// #define MotorPinFLM1_A 23
// #define MotorPinFLM1_B 19

// #define MotorPinFRM1_A 18
// #define MotorPinFRM1_B 17

// #define MotorPinRLM1_A 4
// #define MotorPinRLM1_B 16

// #define MotorPinRRM1_A 27
// #define MotorPinRRM1_B 13

#define MotorPinFLM1_A 27
#define MotorPinFLM1_B 13

#define MotorPinFRM1_A 16
#define MotorPinFRM1_B 4

#define MotorPinRLM1_A 18
#define MotorPinRLM1_B 17

#define MotorPinRRM1_A 19
#define MotorPinRRM1_B 23

#define RELAY_1 32
#define RELAY_2 33
#define RELAY_3 25
#define RELAY_4 26

#define limitSWFPin 27
#define limitSWBPin 13
#define MotorPinLift_M1_A 32
#define MotorPinLift_M1_B 33

// #define WHEEL_DIAMETER 0.153
#define WHEEL_DIAMETER 0.1524

#define FR_WHEELS_DISTANCE 0.440
#define LR_WHEELS_DISTANCE 0.560
// #define PWM_BITS 8
#define PWM_BITS 10

#define PWM_MAX pow(2, PWM_BITS) - 1
#define PWM_MIN -PWM_MAX