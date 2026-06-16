# ros2duino Arduino/ESP32 Library — Session 2

Created for **KTC_DINO_ROBOT วิทยาลัยเทคนิคกาฬสินธุ์**  
Author: **พิฆเนศ แสงสะเดาะ (PIKNATE D.I.Y.)**

This library is the MCU side of the `ros2duino` project. It is designed to make Arduino/ESP32 code simple while the ROS2 C++ bridge handles the heavier backend work.

## Design goal

Frontend Arduino code should feel like this:

```cpp
Ros2Duino esp32wheel;

void setup() {
  esp32wheel.identity(1, "esp32wheel");
  esp32wheel.useWiFi("ssid", "password");
  esp32wheel.connectROS("192.168.1.10", 15000);
  esp32wheel.pinMode(2, OUTPUT);
  esp32wheel.begin();
}

void loop() {
  esp32wheel.spinOnce();
  esp32wheel.digitalWrite(2, HIGH);

  float data[] = {120.0, 118.0, 0.03};
  esp32wheel.publishFloat32Array(data);
}
```

## Current features

- ESP32 WiFi UDP transport
- Arduino/ESP32 Serial transport
- device identity by `device_id`
- HELLO / ACK handshake
- heartbeat
- basic ROS2 time sync
- publish float32 array
- publish int32 array
- publish bytes
- publish bool
- publish string
- publish twist
- receive data from ROS2 with callbacks
- Arduino wrapper functions:
  - `pinMode()`
  - `digitalWrite()`
  - `digitalRead()`
  - `analogRead()`
  - `analogWrite()`

## ROS2 topic mapping

For `device_id = 1`, the Session 1 ROS2 bridge creates these topics:

### MCU to ROS2

```text
/ros2duino/device_1/from_mcu/float32_array
/ros2duino/device_1/from_mcu/int32_array
/ros2duino/device_1/from_mcu/bytes
/ros2duino/device_1/from_mcu/string
/ros2duino/device_1/from_mcu/bool
/ros2duino/device_1/from_mcu/twist
```

### ROS2 to MCU

```text
/ros2duino/device_1/to_mcu/float32_array
/ros2duino/device_1/to_mcu/int32_array
/ros2duino/device_1/to_mcu/bytes
/ros2duino/device_1/to_mcu/string
/ros2duino/device_1/to_mcu/bool
/ros2duino/device_1/to_mcu/twist
```

## ESP32 WiFi example

```cpp
#include <Ros2Duino.h>

Ros2Duino esp32board;

void setup() {
  Serial.begin(115200);
  esp32board.identity(1, "esp32_basic");
  esp32board.useWiFi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
  esp32board.connectROS("192.168.1.10", 15000, 15001);
  esp32board.begin();
}

void loop() {
  esp32board.spinOnce();

  float data[] = {1.0, 2.0, 3.0};
  esp32board.publishFloat32Array(data);

  delay(100);
}
```

## Arduino Serial example

```cpp
#include <Ros2Duino.h>

Ros2Duino arduinoBoard;

void setup() {
  arduinoBoard.identity(2, "arduino_serial_basic");
  arduinoBoard.useSerial(Serial, 115200);
  arduinoBoard.begin();
}

void loop() {
  arduinoBoard.spinOnce();

  int32_t data[] = {1, 2, 3};
  arduinoBoard.publishInt32Array(data);

  delay(100);
}
```

## ROS2 bridge run commands

UDP:

```bash
ros2 run ros2duino_bridge ros2duino_bridge_node --ros-args \
  -p transport.mode:=udp \
  -p udp.bind_ip:=0.0.0.0 \
  -p udp.port:=15000
```

Serial:

```bash
ros2 run ros2duino_bridge ros2duino_bridge_node --ros-args \
  -p transport.mode:=serial \
  -p serial.port:=/dev/ttyUSB0 \
  -p serial.baud:=115200
```

Both:

```bash
ros2 run ros2duino_bridge ros2duino_bridge_node --ros-args \
  -p transport.mode:=both \
  -p serial.port:=/dev/ttyUSB0 \
  -p serial.baud:=115200
```

## Test from ROS2

Send LED command to MCU through bool callback:

```bash
ros2 topic pub /ros2duino/device_1/to_mcu/bool std_msgs/msg/Bool "{data: true}"
```

Read MCU float array:

```bash
ros2 topic echo /ros2duino/device_1/from_mcu/float32_array
```

Send Twist command:

```bash
ros2 topic pub /ros2duino/device_1/to_mcu/twist geometry_msgs/msg/Twist \
"{linear: {x: 0.2}, angular: {z: 0.5}}"
```

## Important note for Session 2

This session intentionally keeps the MCU side simple and stable. It does not try to deserialize every ROS2 message type directly on Arduino because many MCU boards have limited RAM. Full generic ROS2 message support should be added later through a message-codec/generator layer.

The practical industrial pattern is:

```text
MCU friendly data: arrays / bool / string / twist
ROS2 bridge side: maps these to ROS2 topics
Future phase: add generated codecs for std_msgs, sensor_msgs, geometry_msgs, nav_msgs, custom_msgs
```

## Recommended publish rate

Keep MCU publish rate at or below 200 Hz. For most robot use cases:

```text
motor control internal loop: 100–1000 Hz on MCU only
ROS2 telemetry publish: 30–100 Hz
cmd_vel receive: 30–100 Hz
heartbeat/time sync: 1–5 Hz
```
