#include "SimpleLink.h"

SimpleLink::SimpleLink()
{
     _serial = &Serial2; // ใช้ Serial2 ของ ESP32
}

void SimpleLink::begin(int baud, int rxPin, int txPin)
{
     _serial->begin(baud, SERIAL_8N1, rxPin, txPin);
}

void SimpleLink::send(DataPacket data)
{
     // ส่ง Header (0xAA) เพื่อให้ตัวรับรู้ว่าเป็นจุดเริ่มต้นข้อมูล
     _serial->write(0xAA);
     _serial->write((uint8_t *)&data, sizeof(DataPacket));
}

bool SimpleLink::receive(DataPacket &data)
{
     if (_serial->available() >= sizeof(DataPacket) + 1)
     {
          if (_serial->read() == 0xAA)
          { // เช็ค Header
               _serial->readBytes((uint8_t *)&data, sizeof(DataPacket));
               return true;
          }
     }
     return false;
}