//master small
#define rx_pin 21
#define tx_pin 22

#define MAC "88:57:21:8b:61:8c"
#define MAC_PS5 "E8:47:3A:A1:F3:FE"
#define MAX_RPM 200

// ช่องสำหรับ ESP-NOW (ใช้ร่วมกันทั้งเครื่องส่งและเครื่องรับ)
#ifndef ESP_NOW_CHANNEL
#define ESP_NOW_CHANNEL 1
#endif

// Receiver MAC for ESP-NOW (6 bytes)
static const uint8_t RECEIVER_MAC[6] = {0x88, 0x57, 0x21, 0x8B, 0x61, 0x8C};

// PS4 MAC address string (for PS4Controller library)
#ifndef PS4_MAC
#define PS4_MAC "1A:2B:3C:4D:5E:6F"
#endif

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

#define MT_BOX_A 18
#define MT_BOX_B 17
#define MT_ARM_A 19
#define MT_ARM_B 23

//BOX
#define LIMIT_SW_1_PIN_FRONT 35
#define LIMIT_SW_2_PIN_BACK 34

//ARM
#define LIMIT_SW_3_PIN_BACK 39
#define LIMIT_SW_4_PIN_FRONT 36

#define RELAY_1 32
#define RELAY_2 33
#define RELAY_3 25
#define RELAY_4 26

// #define WHEEL_DIAMETER 0.153
#define WHEEL_DIAMETER 0.1524

#define FR_WHEELS_DISTANCE 0.440
#define LR_WHEELS_DISTANCE 0.560
// #define PWM_BITS 8
#define PWM_BITS 10

#define PWM_MAX pow(2, PWM_BITS) - 1
#define PWM_MIN -PWM_MAX