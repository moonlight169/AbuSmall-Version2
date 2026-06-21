#include <Arduino.h>
#include "config.h" 

int current_box_pwm = 0;
char armState = 'V';

const int speed_arm_fast = 150;
const int speed_arm_slow = 45;
const int freq = 5000;      
const int resolution = 8;   

const int pwmBoxA = 4; const int pwmBoxB = 5;
const int pwmArmA = 6; const int pwmArmB = 7;

void setup(){
    setCpuFrequencyMhz(240); // ดัน CPU รับขีดสุด
    Serial.begin(115200);
    
    // ตั้งค่า Serial2 ให้ตรงกับบอร์ด Main (2 Mbps + Buffer)
    Serial2.setRxBufferSize(2048);
    Serial2.setTxBufferSize(2048);
    Serial2.begin(2000000, SERIAL_8N1, 21, 22);
    // เพิ่ม timeout ให้ parseInt() มีเวลารับตัวเลขจากบอร์ด Main
    Serial2.setTimeout(200);

    ledcSetup(pwmBoxA, freq, resolution); ledcSetup(pwmBoxB, freq, resolution);
    ledcAttachPin(MT_BOX_A, pwmBoxA); ledcAttachPin(MT_BOX_B, pwmBoxB);

    ledcSetup(pwmArmA, freq, resolution); ledcSetup(pwmArmB, freq, resolution);
    ledcAttachPin(MT_ARM_A, pwmArmA); ledcAttachPin(MT_ARM_B, pwmArmB);

    pinMode(LIMIT_SW_1_PIN_FRONT, INPUT_PULLUP); pinMode(LIMIT_SW_2_PIN_BACK, INPUT_PULLUP);
    pinMode(LIMIT_SW_3_PIN_BACK, INPUT_PULLUP); pinMode(LIMIT_SW_4_PIN_FRONT, INPUT_PULLUP);
    
    pinMode(RELAY_1, OUTPUT); pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT); pinMode(RELAY_4, OUTPUT);

    digitalWrite(RELAY_1, HIGH); digitalWrite(RELAY_2, HIGH);
    digitalWrite(RELAY_3, HIGH); digitalWrite(RELAY_4, HIGH);

    Serial.println("=====================================");
    Serial.println("Slave UART MAX SPEED Ready");
    Serial.println("=====================================");
}

void loop() {
    // 1. อ่านข้อมูล UART ทรงพลัง
    while (Serial2.available()) {
        char command = Serial2.peek(); 

        if (command == 'A' || command == 'B' || command == 'C' || command == 'D' || command == 'd') {
            command = Serial2.read(); 
            if (command == 'A') digitalWrite(RELAY_2, !digitalRead(RELAY_2));
            else if (command == 'B') digitalWrite(RELAY_3, !digitalRead(RELAY_3));
            else if (command == 'D') digitalWrite(RELAY_1, LOW); 
            else if (command == 'd') digitalWrite(RELAY_1, HIGH); 
            else if (command == 'C') digitalWrite(RELAY_4, !digitalRead(RELAY_4));
        } 
        else if (command == 'S') { 
            Serial2.read();
            current_box_pwm = 0; armState = 'V';
        }
        else if (command == 'X') {
            Serial2.read(); 
            current_box_pwm = Serial2.parseInt(); 
            if (Serial2.peek() == '\n') Serial2.read(); 
        }
        else if (command == 'G' || command == 'g' || command == 'H' || command == 'h' || command == 'V') {
            armState = Serial2.read(); 
        }
        else {
            Serial2.read(); // ล้างขยะ
        }
    }

    // 2. ตรวจสอบ Limit Switch และสั่งมอเตอร์
    bool hit_box_front = (digitalRead(LIMIT_SW_1_PIN_FRONT) == LOW);
    bool hit_box_back  = (digitalRead(LIMIT_SW_2_PIN_BACK) == LOW);
    bool hit_arm_back  = (digitalRead(LIMIT_SW_3_PIN_BACK) == LOW);
    bool hit_arm_front = (digitalRead(LIMIT_SW_4_PIN_FRONT) == LOW);

    int box_speed_A = 0; int box_speed_B = 0;
    int arm_speed_A = 0; int arm_speed_B = 0;

    // มอเตอร์ Box
    if (current_box_pwm < 0 && !hit_box_back) box_speed_A = abs(current_box_pwm); 
    else if (current_box_pwm > 0 && !hit_box_front) box_speed_B = current_box_pwm;

    // มอเตอร์ Arm
    if ((armState == 'G' || armState == 'g') && !hit_arm_back) {
        arm_speed_A = (armState == 'G') ? speed_arm_fast : speed_arm_slow;
    } 
    else if ((armState == 'H' || armState == 'h') && !hit_arm_front) {
        arm_speed_B = (armState == 'H') ? speed_arm_fast : speed_arm_slow;
    }

    ledcWrite(pwmBoxA, box_speed_A); ledcWrite(pwmBoxB, box_speed_B);
    ledcWrite(pwmArmA, arm_speed_A); ledcWrite(pwmArmB, arm_speed_B);
}