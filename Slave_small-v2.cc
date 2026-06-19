#include <Arduino.h>
#include "config.h" // ดึงการตั้งค่าพินทั้งหมดจากไฟล์ของคุณมาใช้ตรงๆ

volatile char boxState = 'U';
volatile char armState = 'V';

// --- ตั้งค่า PWM สำหรับ ESP32 ---
const int freq = 5000;      // ความถี่ 5kHz
const int resolution = 8;   // 0-255

// แชนแนลสำหรับ BOX (มอเตอร์ 1)
const int pwmBoxA = 4;  
const int pwmBoxB = 5;

// แชนแนลสำหรับ ARM (มอเตอร์ 2)
const int pwmArmA = 6;
const int pwmArmB = 7;

const int speed_box_fast = 180;
const int speed_box_slow = 36; 

const int speed_arm_fast = 150;
const int speed_arm_slow = 30;

void setup(){
    Serial.begin(115200);
    
    Serial2.begin(115200, SERIAL_8N1, 21, 22);

    ledcSetup(pwmBoxA, freq, resolution);
    ledcSetup(pwmBoxB, freq, resolution);
    ledcAttachPin(MT_BOX_A, pwmBoxA);
    ledcAttachPin(MT_BOX_B, pwmBoxB);

    ledcSetup(pwmArmA, freq, resolution);
    ledcSetup(pwmArmB, freq, resolution);
    ledcAttachPin(MT_ARM_A, pwmArmA);
    ledcAttachPin(MT_ARM_B, pwmArmB);

    pinMode(LIMIT_SW_1_PIN_FRONT, INPUT_PULLUP);
    pinMode(LIMIT_SW_2_PIN_BACK, INPUT_PULLUP);
    
    pinMode(LIMIT_SW_3_PIN_BACK, INPUT_PULLUP);
    pinMode(LIMIT_SW_4_PIN_FRONT, INPUT_PULLUP);
    
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);

    digitalWrite(RELAY_1, HIGH);
    digitalWrite(RELAY_2, HIGH);
    digitalWrite(RELAY_3, HIGH);
    digitalWrite(RELAY_4, HIGH);

    Serial.println("=====================================");
    Serial.println("Slave UART (Separate Speeds) Ready...");
    Serial.println("=====================================");
}

void stopAllMotors() {
    ledcWrite(pwmBoxA, 0);
    ledcWrite(pwmBoxB, 0);
    ledcWrite(pwmArmA, 0);
    ledcWrite(pwmArmB, 0);
}

void loop() {
    while (Serial2.available()) {
        char command = Serial2.read();

        Serial.print("-> Received: '");
        Serial.print(command);
        Serial.println("'");

        if (command == 'A') {
            digitalWrite(RELAY_2, !digitalRead(RELAY_2));
        } else if (command == 'B') {
            digitalWrite(RELAY_3, !digitalRead(RELAY_3));
        } else if (command == 'C') {
            digitalWrite(RELAY_1, LOW); 
        } else if (command == 'c') {
            digitalWrite(RELAY_1, HIGH); 
        } else if (command == 'D') {
            digitalWrite(RELAY_4, !digitalRead(RELAY_4));
        } 
        else if (command == 'E' || command == 'e' || command == 'F' || command == 'f' || command == 'U') {
            boxState = command;
        }
        else if (command == 'G' || command == 'g' || command == 'H' || command == 'h' || command == 'V') {
            armState = command;
        }
        else if (command == 'S') { 
            boxState = 'U';
            armState = 'V';
        }
    }

    bool hit_box_front = (digitalRead(LIMIT_SW_1_PIN_FRONT) == LOW);
    bool hit_box_back  = (digitalRead(LIMIT_SW_2_PIN_BACK) == LOW);
    bool hit_arm_back  = (digitalRead(LIMIT_SW_3_PIN_BACK) == LOW);
    bool hit_arm_front = (digitalRead(LIMIT_SW_4_PIN_FRONT) == LOW);

    int box_speed_A = 0;
    int box_speed_B = 0;
    int arm_speed_A = 0;
    int arm_speed_B = 0;

    // =========================================
    // ประมวลผลมอเตอร์ Box
    // =========================================
    if (boxState == 'E' || boxState == 'e') {
        if (!hit_box_back) {
            box_speed_A = (boxState == 'E') ? speed_box_fast : speed_box_slow;
        }
    } 
    else if (boxState == 'F' || boxState == 'f') {
        if (!hit_box_front) {
            box_speed_B = (boxState == 'F') ? speed_box_fast : speed_box_slow;
        }
    }

    // =========================================
    // ประมวลผลมอเตอร์ Arm (ทำงานอิสระจาก Box)
    // =========================================
    if (armState == 'G' || armState == 'g') {
        if (!hit_arm_back) {
            arm_speed_A = (armState == 'G') ? speed_arm_fast : speed_arm_slow;
        }
    } 
    else if (armState == 'H' || armState == 'h') {
        if (!hit_arm_front) {
            arm_speed_B = (armState == 'H') ? speed_arm_fast : speed_arm_slow;
        }
    }

    ledcWrite(pwmBoxA, box_speed_A);
    ledcWrite(pwmBoxB, box_speed_B);
    ledcWrite(pwmArmA, arm_speed_A);
    ledcWrite(pwmArmB, arm_speed_B);
}