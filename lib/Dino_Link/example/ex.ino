// MASTER
#include "SimpleLink.h"
SimpleLink link;

void setup()
{
     link.begin(115200, 21, 22); // RX=21, TX=22
}

void loop()
{
     SimpleLink::DataPacket myData = {500.5, 480.2, 1};
     link.send(myData);
     delay(20); // ส่งด้วยความถี่ 50Hz
}

// SLAVE

#include "SimpleLink.h"
SimpleLink link;

void setup()
{
     Serial.begin(115200);
     link.begin(115200, 21, 22);
}

void loop()
{
     SimpleLink::DataPacket incomingData;
     if (link.receive(incomingData))
     {
          Serial.printf("Received Setpoint: %.2f\n", incomingData.setpoint);
     }
}