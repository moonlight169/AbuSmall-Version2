//master small
#define MotorPinFLM1_A 4
#define MotorPinFLM1_B 16

#define MotorPinFRM1_A 18
#define MotorPinFRM1_B 17

#define MotorPinRLM1_A 19
#define MotorPinRLM1_B 23

#define MotorPinRRM1_A 32
#define MotorPinRRM1_B 33

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

//master big
#define MotorPinFLM2_A 27
#define MotorPinFLM2_B 13

#define MotorPinFRM2_A 4
#define MotorPinFRM2_B 16

#define MotorPinRLM2_A 17
#define MotorPinRLM2_B 18

#define MotorPinRRM2_A 23
#define MotorPinRRM2_B 19


#define EncoderPinFL_A 26
#define EncoderPinFL_B 25

#define EncoderPinFR_A 32
#define EncoderPinFR_B 33

#define EncoderPinRL_A 34
#define EncoderPinRL_B 35

#define EncoderPinRR_A 36
#define EncoderPinRR_B 39

#define COUNT_PER_REV_FL 6410
#define COUNT_PER_REV_RL 6410

#define COUNT_PER_REV_FR 6410
#define COUNT_PER_REV_RR 6410

#define limitSWM2_PIN1 32
#define limitSWM2_PIN2 33
#define limitSWM2_PIN3 34
#define limitSWM2_PIN4 35
#define limitSWM2_PIN5 36
#define limitSWM2_PIN6 37
#define limitSWM2_PIN7 32
#define limitSWM2_PIN8 33

//slave1
#define MotorPinS1FL_A 17
#define MotorPinS1FL_B 18

#define MotorPinS1FR_A 16
#define MotorPinS1FR_B 4

#define MotorPinS1RL_A 33
#define MotorPinS1RL_B 32

#define MotorPinS1RR_A 23
#define MotorPinS1RR_B 19

//slave2
#define MotorPinS2FL_A 17
#define MotorPinS2FL_B 18

#define MotorPinS2FR_A 16
#define MotorPinS2FR_B 4

#define MotorPinS2RL_A 33
#define MotorPinS2RL_B 32

#define MotorPinS2RR_A 23
#define MotorPinS2RR_B 19

//slave3
#define MotorPinS3FL_A 17
#define MotorPinS3FL_B 18

#define MotorPinS3FR_A 16
#define MotorPinS3FR_B 4

#define MotorPinS3RL_A 33
#define MotorPinS3RL_B 32

#define MotorPinS3RR_A 23
#define MotorPinS3RR_B 19

#define M_MAX_RPM 255
#define M_LIFT_MAX_RPM 255

#define M2_MAX_RPM 170
#define S1_MAX_RPM 200
#define S2_MAX_RPM 200
#define S3_MAX_RPM 200

// #define K_P 1.5  // P constant
// #define K_I 0.25 // I constant
// #define K_D 0.4  // D constant

#define K_P 0.8  // ลด P constant สำหรับระบบที่ไม่มี encoder feedback
#define K_I 0.3  // ลด I constant
#define K_D 0.02 // ลด D constant

// #define WHEEL_DIAMETER 0.153
#define WHEEL_DIAMETER 0.1524

#define FR_WHEELS_DISTANCE 0.440
#define LR_WHEELS_DISTANCE 0.560
// #define PWM_BITS 8
#define PWM_BITS 10

#define PWM_MAX pow(2, PWM_BITS) - 1
#define PWM_MIN -PWM_MAX

//master big
#define WHEEL_DIAMETER_M2 0.090
#define FR_WHEELS_DISTANCE_M2 0.550
#define LR_WHEELS_DISTANCE_M2 0.790

#define SDA_PIN 21
#define SCL_PIN 22