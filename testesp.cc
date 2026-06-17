#include <Arduino.h>

// กำหนดให้ LED_BUILTIN มีค่าเท่ากับพิน 2 (สำหรับ ESP32)
#define LED_BUILTIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Hello World - LED ON");
  delay(1000);
  
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Hello World - LED OFF");
  delay(1000);
}