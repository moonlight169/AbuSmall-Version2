#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <PS4Controller.h>
#include "esp_bt.h"
#include "config.h"

// NOTE: `RECEIVER_MAC` and `PS4_MAC` are defined in include/config.h

// โครงสร้างข้อมูล (ต้องเหมือนกันทั้งบอร์ดส่งและบอร์ดรับ)
typedef struct __attribute__((packed)) {
  int8_t LStickY;
  int8_t RStickY;
  uint8_t L2Value;
  uint8_t R2Value;
  bool Up, Down, Left, Right, UpRight, UpLeft, DownRight, DownLeft;
  bool L1, R1, Triangle, Square, Cross, Circle;
  bool isConnected;
} PS4Data;

PS4Data ps4_data;
esp_now_peer_info_t peerInfo;

void setup() {
  setCpuFrequencyMhz(240); // ดัน CPU ให้รับส่งข้อมูลได้ไวที่สุด
  Serial.begin(115200);

  // 1. ตั้งค่าจอย PS4 และปลดล็อก Bluetooth Max Power (+9dBm)
  PS4.begin(PS4_MAC);
  esp_bredr_tx_power_set(ESP_PWR_LVL_P9, ESP_PWR_LVL_P9);
  
  // 2. ตั้งค่า Wi-Fi Station + Long Range Mode
  WiFi.mode(WIFI_STA);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
  
  // กำหนดช่องสำหรับ ESP-NOW ให้คงที่เพื่อ Long Range ที่เสถียร
  esp_err_t chRes = esp_wifi_set_channel(ESP_NOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  if (chRes != ESP_OK) {
    Serial.print("esp_wifi_set_channel failed: "); Serial.println(chRes);
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // 3. ผูกเป้าหมาย (บอร์ด Main) เข้ากับระบบ
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, RECEIVER_MAC, 6);
  peerInfo.channel = ESP_NOW_CHANNEL;  
  peerInfo.encrypt = false;
  esp_err_t addRes = esp_now_add_peer(&peerInfo);
  if (addRes != ESP_OK) {
    Serial.print("esp_now_add_peer failed: ");
    Serial.println(addRes);
  }

  // ลงทะเบียน callback ตรวจสถานะการส่ง (optional, ช่วยดีบัก)
  esp_now_register_send_cb([](const uint8_t *mac_addr, esp_now_send_status_t status){
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  });

  Serial.println("Sender Ready!");
}

void loop() {
  ps4_data.isConnected = PS4.isConnected();

  if (ps4_data.isConnected) {
    // แพ็กข้อมูลจากจอย
    ps4_data.LStickY = PS4.LStickY();
    ps4_data.RStickY = PS4.RStickY();
    ps4_data.L2Value = PS4.L2Value();
    ps4_data.R2Value = PS4.R2Value();
    
    ps4_data.Up = PS4.Up(); ps4_data.Down = PS4.Down();
    ps4_data.Left = PS4.Left(); ps4_data.Right = PS4.Right();
    ps4_data.UpRight = PS4.UpRight(); ps4_data.UpLeft = PS4.UpLeft();
    ps4_data.DownRight = PS4.DownRight(); ps4_data.DownLeft = PS4.DownLeft();
    
    ps4_data.L1 = PS4.L1(); ps4_data.R1 = PS4.R1();
    ps4_data.Triangle = PS4.Triangle(); ps4_data.Square = PS4.Square();
    ps4_data.Cross = PS4.Cross(); ps4_data.Circle = PS4.Circle();
  } else {
    // ล้างค่าหากจอยหลุด
    memset(&ps4_data, 0, sizeof(ps4_data));
  }

  // ส่งข้อมูลออกไปที่ความถี่ 50Hz (ทุกๆ 20ms)
  // อย่าส่งถ้าจอยไม่ต่อเชื่อมเพื่อลดทราฟฟิก
  if (ps4_data.isConnected) {
    esp_err_t sendRes = esp_now_send(RECEIVER_MAC, (uint8_t *) &ps4_data, sizeof(ps4_data));
    if (sendRes != ESP_OK) {
      Serial.print("esp_now_send failed: "); Serial.println(sendRes);
    }
  }
  delay(20);
}