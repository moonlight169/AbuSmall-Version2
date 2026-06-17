#include <PS4Controller.h>
#include <nvs_flash.h> // ไลบรารีสำหรับจัดการหน่วยความจำหลัก
#include "config.h"

void setup() {
  Serial.begin(115200);
  
  // --- คำสั่งล้างสมอง (เคลียร์ความจำ Bluetooth เก่าที่ค้างในบอร์ด) ---
  nvs_flash_erase();
  nvs_flash_init();
  // --------------------------------------------------------

  // ใส่ MAC ให้ตรงกับที่ตั้งในจอย
  PS4.begin(MAC); 
  
  Serial.println("Memory cleared! Ready and waiting for PS4...");
}

void loop() {
  if (PS4.isConnected()) {
    Serial.println("🟢 Connected! (รอดแล้วเว้ย!)");
    delay(1000);
  } else {
    Serial.println("🔴 Not Connected...");
    delay(1000);
  }
}