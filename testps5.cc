#include <Arduino.h>
#include <ps5Controller.h>
#include "config.h"

// ตัวแปรเก็บสถานะการเชื่อมต่อก่อนหน้า
bool last_connection_state = false; 

void setup() {
  Serial.begin(115200);
  
  // *** อย่าลืมเปลี่ยน MAC Address ให้ตรงกับที่คุณเซ็ตไว้ในจอย PS5 ***
  // หรือถ้าดึงจาก config.h ของคุณมาใช้ ก็ใส่ ps5.begin(MAC); ได้เลย
  ps5.begin(MAC_PS5); 
  
  Serial.println("Bluetooth Started! Waiting for PS5 Controller to connect...");
}

void loop() {
  // เช็คสถานะปัจจุบัน
  bool current_connection_state = ps5.isConnected();

  // ตรวจสอบว่ามีการเปลี่ยนแปลงสถานะหรือไม่ (จะได้ไม่ Print ซ้ำรัวๆ)
  if (current_connection_state != last_connection_state) {
    if (current_connection_state == true) {
      Serial.println("✅ PS5 Controller Connected Successfully!");
      
      // ตัวอย่างการสั่งสั่นเบาๆ เพื่อยืนยันว่าต่อติดแล้ว
      ps5.setRumble(50, 50); // มอเตอร์ซ้าย, ขวา (0-255)
      delay(300);
      ps5.setRumble(0, 0);
      
    } else {
      Serial.println("❌ PS5 Controller Disconnected.");
    }
    
    // อัปเดตสถานะล่าสุด
    last_connection_state = current_connection_state;
  }

  // หากเชื่อมต่ออยู่ ลอง Print ค่าแบตเตอรี่เล่นๆ ได้ (เช็คทุกๆ 5 วินาที)
  static unsigned long last_print_time = 0;
  if (current_connection_state && (millis() - last_print_time > 5000)) {
    Serial.printf("🔋 Battery Level: %d%%\n", ps5.Battery());
    last_print_time = millis();
  }

  delay(50); // หน่วงเวลาเล็กน้อยให้ CPU ได้พัก
}