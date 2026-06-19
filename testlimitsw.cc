#include <Arduino.h>
#include "config.h"
// #define LIMIT_SW_1_PIN 35
// #define LIMIT_SW_2_PIN 34
// #define LIMIT_SW_3_PIN 39
// #define LIMIT_SW_4_PIN 36

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- 4 Limit Switches Test ---");

  // ใช้โหมด INPUT ธรรมดา (เพราะขาพวกนี้ไม่มี Pull-up ภายใน)
  // **ต้องมั่นใจว่าต่อ Pull-up ภายนอกแล้วนะครับ**
  pinMode(LIMIT_SW_1_PIN, INPUT);
  pinMode(LIMIT_SW_2_PIN, INPUT);
  pinMode(LIMIT_SW_3_PIN, INPUT);
  pinMode(LIMIT_SW_4_PIN, INPUT);
}

void loop() {
  bool sw1 = digitalRead(LIMIT_SW_1_PIN);
  bool sw2 = digitalRead(LIMIT_SW_2_PIN);
  bool sw3 = digitalRead(LIMIT_SW_3_PIN);
  bool sw4 = digitalRead(LIMIT_SW_4_PIN);

  // สมมติว่าต่อแบบ Pull-up (กดแล้วเป็น LOW)
  Serial.print("SW1(35): "); Serial.print(sw1 == LOW ? "[X] " : "[ ] ");
  Serial.print(" | SW2(34): "); Serial.print(sw2 == LOW ? "[X] " : "[ ] ");
  Serial.print(" | SW3(39): "); Serial.print(sw3 == LOW ? "[X] " : "[ ] ");
  Serial.print(" | SW4(36): "); Serial.println(sw4 == LOW ? "[X] " : "[ ] ");

  delay(200);
}