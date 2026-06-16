#include <PS4Controller.h>
#include <nvs_flash.h>

unsigned long lastTime = 0;
volatile bool dataReceived = false;

// ฟังก์ชัน Callback จะทำงานทันทีที่มีข้อมูลส่งมาจากจอย
void onReceive() {
  dataReceived = true;
}

void setup() {
  Serial.begin(115200);
  nvs_flash_erase();
  nvs_flash_init();

  // กำหนดฟังก์ชันที่จะให้ทำงานเมื่อได้รับข้อมูล
  PS4.attach(onReceive); 
  
  PS4.begin("08:a6:f7:10:a8:5c"); 
  Serial.println("Waiting for PS4 Controller...");
}

void loop() {
  if (PS4.isConnected()) {
    if (dataReceived) {
      dataReceived = false; // Reset flag
      
      unsigned long currentTime = millis();
      unsigned long latency = currentTime - lastTime;
      lastTime = currentTime;

      // กรองค่า latency ที่ผิดปกติ (เช่น ตอนเชื่อมต่อครั้งแรก)
      if (latency > 2 && latency < 500) {
        Serial.print("Latency: ");
        Serial.print(latency);
        Serial.println(" ms");
      }
    }
  } else {
    // ถ้าไม่เชื่อมต่อ ให้พิมพ์บอกทุกๆ 1 วินาที
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
      Serial.println("🔴 Not Connected...");
      lastUpdate = millis();
    }
  }
}