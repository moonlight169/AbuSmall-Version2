#include <Arduino.h>
#include "config.h" 

int current_box_pwm = 0;
char armState = 'V';

const int speed_arm_fast = 150;
const int speed_arm_slow = 36;

const int freq = 5000;      // ความถี่ 5kHz
const int resolution = 8;   // 0-255

// แชนแนลสำหรับ BOX (มอเตอร์ 1)
const int pwmBoxA = 4;  
const int pwmBoxB = 5;

// แชนแนลสำหรับ ARM (มอเตอร์ 2)
const int pwmArmA = 6;
const int pwmArmB = 7;

void setup(){
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 21, 22);
    
    Serial2.setTimeout(5); 

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
    Serial.println("Slave UART (Box:Analog / Arm:Digital)");
    Serial.println("=====================================");
}

void loop() {
    // -----------------------------------------
    // 1. ตรวจสอบข้อมูล UART ขาเข้า
    // -----------------------------------------
    while (Serial2.available()) {
        char command = Serial2.peek(); // แอบดูตัวอักษรแรกก่อนดึงออก

        // --- กลุ่มคำสั่ง Relay ดิจิทัลเดิม ---
        if (command == 'A' || command == 'B' || command == 'C' || command == 'D' || command == 'd') {
            command = Serial2.read(); 
            if (command == 'A') digitalWrite(RELAY_2, !digitalRead(RELAY_2));
            else if (command == 'B') digitalWrite(RELAY_3, !digitalRead(RELAY_3));
            else if (command == 'D') digitalWrite(RELAY_1, LOW); 
            else if (command == 'd') digitalWrite(RELAY_1, HIGH); 
            else if (command == 'C') digitalWrite(RELAY_4, !digitalRead(RELAY_4));
        } 
        // --- กลุ่มคำสั่งหยุดฉุกเฉิน ---
        else if (command == 'S') { 
            Serial2.read();
            current_box_pwm = 0;
            armState = 'V';
        }
        // --- กลุ่มคำสั่ง อนาล็อก Box ---
        else if (command == 'X') {
            Serial2.read(); 
            current_box_pwm = Serial2.parseInt(); 
            if (Serial2.peek() == '\n') Serial2.read(); 
        }
        // --- กลุ่มคำสั่ง ดิจิทัล Arm แบบเดิม ---
        else if (command == 'G' || command == 'g' || command == 'H' || command == 'h' || command == 'V') {
            armState = Serial2.read(); // รับอักษรไปใช้ตรงๆ
        }
        else {
            Serial2.read(); // ถ้าเป็นขยะอื่นๆ ดึงทิ้ง
        }
    }

    // -----------------------------------------
    // 2. ประมวลผลและส่งไฟเข้ามอเตอร์
    // -----------------------------------------
    bool hit_box_front = (digitalRead(LIMIT_SW_1_PIN_FRONT) == LOW);
    bool hit_box_back  = (digitalRead(LIMIT_SW_2_PIN_BACK) == LOW);
    bool hit_arm_back  = (digitalRead(LIMIT_SW_3_PIN_BACK) == LOW);
    bool hit_arm_front = (digitalRead(LIMIT_SW_4_PIN_FRONT) == LOW);

    int box_speed_A = 0;
    int box_speed_B = 0;
    int arm_speed_A = 0;
    int arm_speed_B = 0;

    // =========================================
    // ประมวลผลมอเตอร์ Box (อนาล็อก)
    // =========================================
    if (current_box_pwm < 0) { 
        if (!hit_box_back) {
            box_speed_A = abs(current_box_pwm); 
        }
    } 
    else if (current_box_pwm > 0) { 
        if (!hit_box_front) {
            box_speed_B = current_box_pwm;
        }
    }

    // =========================================
    // ประมวลผลมอเตอร์ Arm (ดิจิทัล คงที่)
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