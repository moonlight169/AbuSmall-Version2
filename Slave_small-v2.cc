#include <Arduino.h>
#include "config.h"

void setup() {
    Serial.begin(115200);

    Serial2.begin(115200, SERIAL_8N1, 21, 22); 
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);

    digitalWrite(RELAY_1, HIGH);
    digitalWrite(RELAY_2, HIGH);
    digitalWrite(RELAY_3, HIGH);
    digitalWrite(RELAY_4, HIGH);
}

void loop() {
    if (Serial2.available() > 0) {

        char receivedChar = Serial2.read();

        if (receivedChar == 'A') {
            digitalWrite(RELAY_2, !digitalRead(RELAY_2));
        } else if (receivedChar == 'B') {
            digitalWrite(RELAY_3, !digitalRead(RELAY_3));
            
        }  else if (receivedChar == 'C') {
            digitalWrite(RELAY_1, LOW); 
        } else if (receivedChar == 'c') {
            digitalWrite(RELAY_1, HIGH); 

        } else if (receivedChar == 'D') {
            digitalWrite(RELAY_4, !digitalRead(RELAY_4));
        }
    }
}