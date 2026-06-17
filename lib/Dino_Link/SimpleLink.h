#ifndef SIMPLE_LINK_H
#define SIMPLE_LINK_H

#include <Arduino.h>

class SimpleLink
{
public:
     // กำหนดโครงสร้างข้อมูลที่ต้องการส่ง (ปรับแต่งได้ตามใจชอบ)
     struct DataPacket
     {
          float setpoint;
          float currentPos;
          int status;
     };

     SimpleLink();
     void begin(int baud = 115200, int rxPin = 21, int txPin = 22);
     void send(DataPacket data);
     bool receive(DataPacket &data);

private:
     HardwareSerial *_serial;
};

#endif