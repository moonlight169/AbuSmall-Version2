#include <Arduino.h>
#include "config.h"

void setup() {
    Serial.begin(115200);

    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);

    digitalWrite(RELAY_1, LOW);
    digitalWrite(RELAY_2, LOW);
    digitalWrite(RELAY_3, LOW);
    digitalWrite(RELAY_4, LOW);

}

void loop() {
  
}