#include <Ros2Duino.h>

Ros2Duino esp32wheel;

const char * WIFI_SSID = "YOUR_WIFI_SSID";
const char * WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char * ROS2_IP = "192.168.1.10";

const uint8_t LED_PIN = 2;
uint32_t last_pub_ms = 0;
float target_linear_x = 0.0f;
float target_angular_z = 0.0f;

void onTwistCommand(const Ros2DuinoTwist & twist, uint16_t topic_id)
{
  (void)topic_id;
  target_linear_x = static_cast<float>(twist.linear_x);
  target_angular_z = static_cast<float>(twist.angular_z);
}

void setup()
{
  Serial.begin(115200);

  esp32wheel.identity(1, "esp32wheel");
  esp32wheel.pinMode(LED_PIN, OUTPUT);

  esp32wheel.useWiFi(WIFI_SSID, WIFI_PASS);
  esp32wheel.connectROS(ROS2_IP, 15000, 15001);
  esp32wheel.onTwist(onTwistCommand);
  esp32wheel.begin();
}

void loop()
{
  esp32wheel.spinOnce();

  // Example: user code remains simple.
  esp32wheel.digitalWrite(LED_PIN, esp32wheel.connected() ? HIGH : LOW);

  if (millis() - last_pub_ms >= 20) {  // 50 Hz
    last_pub_ms = millis();

    // Example payload: left_rpm, right_rpm, target_linear_x, target_angular_z, ros_time_sync_flag
    float wheel_state[] = {
      target_linear_x * 100.0f - target_angular_z * 20.0f,
      target_linear_x * 100.0f + target_angular_z * 20.0f,
      target_linear_x,
      target_angular_z,
      esp32wheel.timeSynced() ? 1.0f : 0.0f
    };

    esp32wheel.publishFloat32Array(wheel_state);
  }
}
