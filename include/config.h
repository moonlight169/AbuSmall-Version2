//master small
#define rx_pin 21
#define tx_pin 22

#define MAC "c0:cd:d6:8f:44:34"
#define MAX_RPM 200

#define MotorPinFLM1_A 27
#define MotorPinFLM1_B 13

#define MotorPinFRM1_A 4
#define MotorPinFRM1_B 16

#define MotorPinRLM1_A 18
#define MotorPinRLM1_B 17

#define MotorPinRRM1_A 23
#define MotorPinRRM1_B 19

#define MotorPinFL_A 27
#define MotorPinFL_B 13
#define MotorPinFR_A 4
#define MotorPinFR_B 16
#define MotorPinRL_A 18
#define MotorPinRL_B 17
#define MotorPinRR_A 23
#define MotorPinRR_B 19

#define RelayM1_PIN1 27
#define RelayM1_PIN2 13
#define RelayM1_PIN3 25
#define RelayM1_PIN4 26
#define RelayM1_PIN5 25
#define RelayM1_PIN6 26
#define RelayM1_PIN7 23

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