#pragma once

#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #define ROS2DUINO_HAS_WIFI 1
#else
  #define ROS2DUINO_HAS_WIFI 0
#endif

#ifndef ROS2DUINO_MAX_PAYLOAD
#define ROS2DUINO_MAX_PAYLOAD 512
#endif

#ifndef ROS2DUINO_SERIAL_RX_BUFFER
#define ROS2DUINO_SERIAL_RX_BUFFER 768
#endif

namespace ros2duino
{

static const uint32_t MAGIC = 0x30443252UL;  // 'R2D0' little endian
static const uint8_t PROTOCOL_VERSION = 1;
static const size_t HEADER_SIZE = 32;

struct PacketType
{
  enum : uint8_t
  {
    HELLO = 1,
    TIME_SYNC_REQ = 2,
    TIME_SYNC_RESP = 3,
    HEARTBEAT = 4,
    DATA = 10,
    ACK = 11,
    ERROR_PACKET = 250
  };
};

struct ValueType
{
  enum : uint8_t
  {
    NONE = 0,
    FLOAT32_ARRAY = 1,
    INT32_ARRAY = 2,
    UINT8_ARRAY = 3,
    BOOL_VALUE = 4,
    STRING_VALUE = 5,
    TWIST = 10,
    IMU_MINIMAL = 11,
    RAW_CDR = 100
  };
};

#pragma pack(push, 1)
struct Header
{
  uint32_t magic;
  uint8_t version;
  uint8_t packet_type;
  uint8_t value_type;
  uint8_t flags;
  uint32_t seq;
  int64_t stamp_ns;
  uint16_t device_id;
  uint16_t topic_id;
  uint32_t payload_size;
  uint32_t crc32;
};
#pragma pack(pop)

struct TwistData
{
  double linear_x;
  double linear_y;
  double linear_z;
  double angular_x;
  double angular_y;
  double angular_z;
};

struct Status
{
  bool transport_ready;
  bool ros_ack;
  bool time_synced;
  uint32_t tx_count;
  uint32_t rx_count;
  uint32_t crc_error_count;
  uint32_t decode_error_count;
  int64_t time_offset_ns;
  uint32_t last_rx_ms;
};

class Ros2Duino
{
public:
  typedef void (*Float32ArrayCallback)(const float * data, uint16_t count, uint16_t topic_id);
  typedef void (*Int32ArrayCallback)(const int32_t * data, uint16_t count, uint16_t topic_id);
  typedef void (*Uint8ArrayCallback)(const uint8_t * data, uint16_t count, uint16_t topic_id);
  typedef void (*BoolCallback)(bool value, uint16_t topic_id);
  typedef void (*StringCallback)(const char * text, uint16_t topic_id);
  typedef void (*TwistCallback)(const TwistData & twist, uint16_t topic_id);

  Ros2Duino();

  void identity(uint16_t device_id, const char * device_name);

  // Serial transport. Works on Arduino and ESP32.
  void useSerial(Stream & stream);
  void useSerial(HardwareSerial & serial, uint32_t baud);

#if ROS2DUINO_HAS_WIFI
  // ESP32 WiFi/UDP transport.
  bool useWiFi(const char * ssid, const char * password, uint32_t timeout_ms = 15000);
  bool connectROS(const char * ros_ip, uint16_t ros_port = 15000, uint16_t local_port = 15001);
#endif

  void begin();
  void spinOnce();

  bool connected() const;
  bool timeSynced() const;
  int64_t rosNowNs() const;
  const Status & status() const;

  // Arduino-friendly wrappers.
  void pinMode(uint8_t pin, uint8_t mode);
  void digitalWrite(uint8_t pin, uint8_t value);
  int digitalRead(uint8_t pin);
  int analogRead(uint8_t pin);
  void analogWrite(uint8_t pin, int value);

  // Publish helpers. Default topic_id follows the ROS2 bridge Session 1 mapping.
  bool publishFloat32Array(const float * data, uint16_t count, uint16_t topic_id = 1);
  bool publishInt32Array(const int32_t * data, uint16_t count, uint16_t topic_id = 2);
  bool publishBytes(const uint8_t * data, uint16_t count, uint16_t topic_id = 3);
  bool publishBool(bool value, uint16_t topic_id = 5);
  bool publishString(const char * text, uint16_t topic_id = 4);
  bool publishTwist(double linear_x, double angular_z, uint16_t topic_id = 6);
  bool publishTwist(const TwistData & twist, uint16_t topic_id = 6);

  // Convenience template for C arrays.
  template <size_t N>
  bool publishFloat32Array(const float (&data)[N], uint16_t topic_id = 1)
  {
    return publishFloat32Array(data, static_cast<uint16_t>(N), topic_id);
  }

  template <size_t N>
  bool publishInt32Array(const int32_t (&data)[N], uint16_t topic_id = 2)
  {
    return publishInt32Array(data, static_cast<uint16_t>(N), topic_id);
  }

  template <size_t N>
  bool publishBytes(const uint8_t (&data)[N], uint16_t topic_id = 3)
  {
    return publishBytes(data, static_cast<uint16_t>(N), topic_id);
  }

  // Subscribers for messages coming from ROS2 /to_mcu/* topics.
  void onFloat32Array(Float32ArrayCallback cb);
  void onInt32Array(Int32ArrayCallback cb);
  void onBytes(Uint8ArrayCallback cb);
  void onBool(BoolCallback cb);
  void onString(StringCallback cb);
  void onTwist(TwistCallback cb);

  // Manual control packets.
  bool sendHello();
  bool requestTimeSync();
  bool sendHeartbeat();
  bool sendError(const char * text);

private:
  enum TransportMode : uint8_t
  {
    TRANSPORT_NONE = 0,
    TRANSPORT_SERIAL = 1,
    TRANSPORT_UDP = 2
  };

  struct PacketView
  {
    Header header;
    const uint8_t * payload;
  };

  uint16_t device_id_;
  char device_name_[32];
  TransportMode transport_;
  Stream * serial_;

#if ROS2DUINO_HAS_WIFI
  WiFiUDP udp_;
  IPAddress ros_ip_;
  uint16_t ros_port_;
  uint16_t local_port_;
#endif

  uint32_t seq_;
  Status status_;
  uint32_t last_hello_ms_;
  uint32_t last_heartbeat_ms_;
  uint32_t last_time_sync_ms_;

  uint8_t rx_buffer_[ROS2DUINO_SERIAL_RX_BUFFER];
  size_t rx_size_;

  Float32ArrayCallback float_cb_;
  Int32ArrayCallback int_cb_;
  Uint8ArrayCallback bytes_cb_;
  BoolCallback bool_cb_;
  StringCallback string_cb_;
  TwistCallback twist_cb_;

  int64_t localNowNs() const;
  uint32_t crc32(const uint8_t * data, size_t length) const;
  bool sendPacket(uint8_t packet_type, uint8_t value_type, uint16_t topic_id, const uint8_t * payload, uint32_t payload_size);
  bool writeRawPacket(const uint8_t * data, size_t length);
  void pollSerial();
  void pollUdp();
  bool parsePacketBytes(const uint8_t * data, size_t length, PacketView * out);
  void handlePacket(const PacketView & packet);

  bool encodePodArray(const void * data, uint16_t count, size_t item_size, uint8_t * out, uint32_t * out_size);
  bool readCountPayload(const uint8_t * payload, uint32_t payload_size, uint16_t * count, const uint8_t ** values, size_t item_size);
};

}  // namespace ros2duino

using Ros2Duino = ros2duino::Ros2Duino;
using Ros2DuinoTwist = ros2duino::TwistData;
