#include <Arduino.h>
#include <Ros2Duino.h>

// สร้าง Object ชื่อ esp32hello
Ros2Duino esp32hello; 

// ฟังก์ชัน Callback: จะทำงานเมื่อมีข้อความ String ส่งมาจาก ROS 2
void stringCallback(const char* data, uint16_t length) {
  // สร้าง String จาก const char* 
  String msg = "";
  for(uint16_t i = 0; i < length; i++) {
    msg += data[i];
  }
  
  // ตรวจสอบข้อความและสั่งงาน LED โดยไม่ใช้ Serial.print
  if(length > 0 && data[0] == 'o') {
    digitalWrite(2, HIGH);
  } else if (length > 0 && data[0] == 'c') {
    digitalWrite(2, LOW);
  }
}

void setup() {
  // เอา Serial.begin(115200); ออก เพราะไลบรารีจะจัดการให้เอง
  pinMode(2, OUTPUT);

  // 1. ตั้งค่า Identity (ID: 1, Name: master_esp32)
  esp32hello.identity(1, "master_esp32"); 

  // 2. เลือกการเชื่อมต่อแบบ Serial (สาย USB)
  esp32hello.useSerial(Serial, 115200); 

  // 3. ผูกฟังก์ชัน Callback สำหรับรอรับ String
  esp32hello.onString(stringCallback); 

  // 4. เริ่มต้นระบบ
  esp32hello.begin(); 
}

void loop() {
  // 5. เรียกใช้เพื่อเช็กข้อมูลขาเข้าตลอดเวลา
  esp32hello.spinOnce(); 
}