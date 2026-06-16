#include <Ros2Duino.h>

Ros2Duino esp32board;

const char * WIFI_SSID = "YOUR_WIFI_SSID";
const char * WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char * ROS2_IP = "192.168.1.10";

const uint8_t LED_PIN = 2;
uint32_t last_pub_ms = 0;

void onBoolFromROS(bool value, uint16_t topic_id)
{
  (void)topic_id;
  esp32board.digitalWrite(LED_PIN, value ? HIGH : LOW);
}

void setup()
{
  Serial.begin(115200);

  esp32board.identity(1, "esp32_basic");
  esp32board.pinMode(LED_PIN, OUTPUT);

  if (!esp32board.useWiFi(WIFI_SSID, WIFI_PASS)) {
    Serial.println("WiFi failed");
  }

  if (!esp32board.connectROS(ROS2_IP, 15000, 15001)) {
    Serial.println("connectROS failed");
  }

  esp32board.onBool(onBoolFromROS);
  esp32board.begin();
}

void loop()
{
  esp32board.spinOnce();

  if (millis() - last_pub_ms >= 100) {
    last_pub_ms = millis();

    float data[] = {
      static_cast<float>(millis() * 0.001f),
      static_cast<float>(esp32board.connected() ? 1.0f : 0.0f),
      static_cast<float>(esp32board.timeSynced() ? 1.0f : 0.0f)
    };

    esp32board.publishFloat32Array(data);
  }
}
