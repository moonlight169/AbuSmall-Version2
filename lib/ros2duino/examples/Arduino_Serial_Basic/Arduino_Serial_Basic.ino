#include <Ros2Duino.h>

Ros2Duino arduinoBoard;

const uint8_t LED_PIN = 13;
uint32_t last_pub_ms = 0;

void onBoolFromROS(bool value, uint16_t topic_id)
{
  (void)topic_id;
  arduinoBoard.digitalWrite(LED_PIN, value ? HIGH : LOW);
}

void setup()
{
  arduinoBoard.identity(2, "arduino_serial_basic");
  arduinoBoard.pinMode(LED_PIN, OUTPUT);

  arduinoBoard.useSerial(Serial, 115200);
  arduinoBoard.onBool(onBoolFromROS);
  arduinoBoard.begin();
}

void loop()
{
  arduinoBoard.spinOnce();

  if (millis() - last_pub_ms >= 100) {
    last_pub_ms = millis();

    int32_t values[] = {
      static_cast<int32_t>(millis()),
      static_cast<int32_t>(arduinoBoard.connected()),
      static_cast<int32_t>(arduinoBoard.timeSynced())
    };

    arduinoBoard.publishInt32Array(values);
  }
}
