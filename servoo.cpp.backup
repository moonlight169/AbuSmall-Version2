#include <Arduino.h>
#include <Servo.h>
  
Servo servo1;
Servo servo2;

int servoVal ,servoVal_1, servoVal_2;
  
void setup() 
{
  servo1.attach(5);
  servo2.attach(11);
  servo1.write(180);
  servo2.write(0);
}
  
void loop()
{
  delay(5000);
  servo1.write(180);
  servo2.write(0);
  delay(5000);
  servo1.write(80);
  servo2.write(100);

  // int servoVal_2 = map(servoVal, 0, 1023, 0, 180);
  // servo2.write(servoVal_2);
  // delay(13.33);  
}