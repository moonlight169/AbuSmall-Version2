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
  bool current_connection_state = ps5.isConnected();
  if (current_connection_state != last_connection_state) {
    if (current_connection_state == true) {
      Serial.println("✅ PS5 Controller Connected Successfully!");
    } else {
      Serial.println("❌ PS5 Controller Disconnected.");
    }
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