#include <Arduino.h>

const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 21, 22); 
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("--- Raw Receiver Ready (with Debug) ---");
}

void loop() {
  // เช็กว่ามีข้อมูลค้างอยู่ในสายหรือไม่
  if (Serial2.available() > 0) {
    
    // อ่านข้อมูลมา 1 ตัวอักษร
    char receivedChar = Serial2.read();
    
    // ----------------------------------------------------
    // 🔍 ส่วนของ DEBUG: ปริ้นท์ทุกอย่างที่รับเข้ามาให้เราเห็น
    // ----------------------------------------------------
    Serial.print("[DEBUG] Received: '");
    Serial.print(receivedChar);
    Serial.print("' | ASCII Value: ");
    Serial.println((int)receivedChar); // แปลงเป็นตัวเลขเพื่อดูรหัสที่ซ่อนอยู่
    // ----------------------------------------------------

    // ตรวจสอบเงื่อนไขเปิดไฟ
    if (receivedChar == 'A') {
      Serial.println(">>> MATCH! Turning LED ON");
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      Serial.println(">>> LED OFF");
      Serial.println("-------------------------");
    }
  }
}