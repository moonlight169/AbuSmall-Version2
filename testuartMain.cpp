#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  
  Serial2.begin(115200, SERIAL_8N1, 21, 22); 
  
  Serial.println("--- Raw Transmitter Ready ---");
}

void loop() {
  Serial2.write('A'); 
  
  Serial.println("Sent -> A");
  delay(2000);
}