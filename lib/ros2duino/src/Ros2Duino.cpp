#include "Ros2Duino.h"

namespace ros2duino
{

Ros2Duino::Ros2Duino()
: device_id_(1),
  transport_(TRANSPORT_NONE),
  serial_(nullptr),
#if ROS2DUINO_HAS_WIFI
  ros_port_(15000),
  local_port_(15001),
#endif
  seq_(1),
  last_hello_ms_(0),
  last_heartbeat_ms_(0),
  last_time_sync_ms_(0),
  rx_size_(0),
  float_cb_(nullptr),
  int_cb_(nullptr),
  bytes_cb_(nullptr),
  bool_cb_(nullptr),
  string_cb_(nullptr),
  twist_cb_(nullptr)
{
  strncpy(device_name_, "ros2duino", sizeof(device_name_) - 1);
  device_name_[sizeof(device_name_) - 1] = '\0';
  memset(&status_, 0, sizeof(status_));
}

void Ros2Duino::identity(uint16_t device_id, const char * device_name)
{
  device_id_ = device_id == 0 ? 1 : device_id;
  if (device_name && device_name[0] != '\0') {
    strncpy(device_name_, device_name, sizeof(device_name_) - 1);
    device_name_[sizeof(device_name_) - 1] = '\0';
  }
}

void Ros2Duino::useSerial(Stream & stream)
{
  serial_ = &stream;
  transport_ = TRANSPORT_SERIAL;
  status_.transport_ready = true;
}

void Ros2Duino::useSerial(HardwareSerial & serial, uint32_t baud)
{
  serial.begin(baud);
  useSerial(static_cast<Stream &>(serial));
}

#if ROS2DUINO_HAS_WIFI
bool Ros2Duino::useWiFi(const char * ssid, const char * password, uint32_t timeout_ms)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout_ms) {
    delay(20);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool Ros2Duino::connectROS(const char * ros_ip, uint16_t ros_port, uint16_t local_port)
{
  ros_port_ = ros_port;
  local_port_ = local_port;
  if (!ros_ip_.fromString(ros_ip)) {
    return false;
  }
  if (!udp_.begin(local_port_)) {
    return false;
  }
  transport_ = TRANSPORT_UDP;
  status_.transport_ready = true;
  return true;
}
#endif

void Ros2Duino::begin()
{
  rx_size_ = 0;
  status_.ros_ack = false;
  status_.time_synced = false;
  last_hello_ms_ = 0;
  last_heartbeat_ms_ = 0;
  last_time_sync_ms_ = 0;
  sendHello();
  requestTimeSync();
}

void Ros2Duino::spinOnce()
{
  if (transport_ == TRANSPORT_SERIAL) {
    pollSerial();
  } else if (transport_ == TRANSPORT_UDP) {
    pollUdp();
  }

  const uint32_t now = millis();

  if (!status_.ros_ack && (now - last_hello_ms_ > 1000)) {
    sendHello();
  }

  if (now - last_heartbeat_ms_ > 1000) {
    sendHeartbeat();
  }

  if (!status_.time_synced || (now - last_time_sync_ms_ > 5000)) {
    requestTimeSync();
  }
}

bool Ros2Duino::connected() const
{
  return status_.transport_ready && status_.ros_ack;
}

bool Ros2Duino::timeSynced() const
{
  return status_.time_synced;
}

int64_t Ros2Duino::rosNowNs() const
{
  return localNowNs() + status_.time_offset_ns;
}

const Status & Ros2Duino::status() const
{
  return status_;
}

void Ros2Duino::pinMode(uint8_t pin, uint8_t mode)
{
  ::pinMode(pin, mode);
}

void Ros2Duino::digitalWrite(uint8_t pin, uint8_t value)
{
  ::digitalWrite(pin, value);
}

int Ros2Duino::digitalRead(uint8_t pin)
{
  return ::digitalRead(pin);
}

int Ros2Duino::analogRead(uint8_t pin)
{
  return ::analogRead(pin);
}

void Ros2Duino::analogWrite(uint8_t pin, int value)
{
#if defined(ESP32)
  ::analogWrite(pin, value);
#else
  ::analogWrite(pin, value);
#endif
}

bool Ros2Duino::publishFloat32Array(const float * data, uint16_t count, uint16_t topic_id)
{
  uint8_t payload[ROS2DUINO_MAX_PAYLOAD];
  uint32_t size = 0;
  if (!encodePodArray(data, count, sizeof(float), payload, &size)) return false;
  return sendPacket(PacketType::DATA, ValueType::FLOAT32_ARRAY, topic_id, payload, size);
}

bool Ros2Duino::publishInt32Array(const int32_t * data, uint16_t count, uint16_t topic_id)
{
  uint8_t payload[ROS2DUINO_MAX_PAYLOAD];
  uint32_t size = 0;
  if (!encodePodArray(data, count, sizeof(int32_t), payload, &size)) return false;
  return sendPacket(PacketType::DATA, ValueType::INT32_ARRAY, topic_id, payload, size);
}

bool Ros2Duino::publishBytes(const uint8_t * data, uint16_t count, uint16_t topic_id)
{
  uint8_t payload[ROS2DUINO_MAX_PAYLOAD];
  uint32_t size = 0;
  if (!encodePodArray(data, count, sizeof(uint8_t), payload, &size)) return false;
  return sendPacket(PacketType::DATA, ValueType::UINT8_ARRAY, topic_id, payload, size);
}

bool Ros2Duino::publishBool(bool value, uint16_t topic_id)
{
  const uint8_t payload[1] = { static_cast<uint8_t>(value ? 1 : 0) };
  return sendPacket(PacketType::DATA, ValueType::BOOL_VALUE, topic_id, payload, 1);
}

bool Ros2Duino::publishString(const char * text, uint16_t topic_id)
{
  if (!text) text = "";
  const uint16_t len = static_cast<uint16_t>((strlen(text) < static_cast<size_t>(ROS2DUINO_MAX_PAYLOAD - 2)) ? strlen(text) : static_cast<size_t>(ROS2DUINO_MAX_PAYLOAD - 2));
  uint8_t payload[ROS2DUINO_MAX_PAYLOAD];
  memcpy(payload, &len, sizeof(uint16_t));
  if (len > 0) memcpy(payload + sizeof(uint16_t), text, len);
  return sendPacket(PacketType::DATA, ValueType::STRING_VALUE, topic_id, payload, sizeof(uint16_t) + len);
}

bool Ros2Duino::publishTwist(double linear_x, double angular_z, uint16_t topic_id)
{
  TwistData twist;
  twist.linear_x = linear_x;
  twist.linear_y = 0.0;
  twist.linear_z = 0.0;
  twist.angular_x = 0.0;
  twist.angular_y = 0.0;
  twist.angular_z = angular_z;
  return publishTwist(twist, topic_id);
}

bool Ros2Duino::publishTwist(const TwistData & twist, uint16_t topic_id)
{
  const double values[6] = {
    twist.linear_x, twist.linear_y, twist.linear_z,
    twist.angular_x, twist.angular_y, twist.angular_z
  };
  uint8_t payload[ROS2DUINO_MAX_PAYLOAD];
  uint32_t size = 0;
  if (!encodePodArray(values, 6, sizeof(double), payload, &size)) return false;
  return sendPacket(PacketType::DATA, ValueType::TWIST, topic_id, payload, size);
}

void Ros2Duino::onFloat32Array(Float32ArrayCallback cb) { float_cb_ = cb; }
void Ros2Duino::onInt32Array(Int32ArrayCallback cb) { int_cb_ = cb; }
void Ros2Duino::onBytes(Uint8ArrayCallback cb) { bytes_cb_ = cb; }
void Ros2Duino::onBool(BoolCallback cb) { bool_cb_ = cb; }
void Ros2Duino::onString(StringCallback cb) { string_cb_ = cb; }
void Ros2Duino::onTwist(TwistCallback cb) { twist_cb_ = cb; }

bool Ros2Duino::sendHello()
{
  last_hello_ms_ = millis();
  const uint16_t len = static_cast<uint16_t>((strlen(device_name_) < static_cast<size_t>(ROS2DUINO_MAX_PAYLOAD - 2)) ? strlen(device_name_) : static_cast<size_t>(ROS2DUINO_MAX_PAYLOAD - 2));
  uint8_t payload[ROS2DUINO_MAX_PAYLOAD];
  memcpy(payload, &len, sizeof(uint16_t));
  if (len > 0) memcpy(payload + sizeof(uint16_t), device_name_, len);
  return sendPacket(PacketType::HELLO, ValueType::STRING_VALUE, 0, payload, sizeof(uint16_t) + len);
}

bool Ros2Duino::requestTimeSync()
{
  last_time_sync_ms_ = millis();
  return sendPacket(PacketType::TIME_SYNC_REQ, ValueType::NONE, 0, nullptr, 0);
}

bool Ros2Duino::sendHeartbeat()
{
  last_heartbeat_ms_ = millis();
  return sendPacket(PacketType::HEARTBEAT, ValueType::NONE, 0, nullptr, 0);
}

bool Ros2Duino::sendError(const char * text)
{
  if (!text) text = "";
  const uint16_t len = static_cast<uint16_t>((strlen(text) < static_cast<size_t>(ROS2DUINO_MAX_PAYLOAD - 2)) ? strlen(text) : static_cast<size_t>(ROS2DUINO_MAX_PAYLOAD - 2));
  uint8_t payload[ROS2DUINO_MAX_PAYLOAD];
  memcpy(payload, &len, sizeof(uint16_t));
  if (len > 0) memcpy(payload + sizeof(uint16_t), text, len);
  return sendPacket(PacketType::ERROR_PACKET, ValueType::STRING_VALUE, 0, payload, sizeof(uint16_t) + len);
}

int64_t Ros2Duino::localNowNs() const
{
  return static_cast<int64_t>(micros()) * 1000LL;
}

uint32_t Ros2Duino::crc32(const uint8_t * data, size_t length) const
{
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      const uint32_t mask = static_cast<uint32_t>(-(static_cast<int32_t>(crc & 1UL)));
      crc = (crc >> 1UL) ^ (0xEDB88320UL & mask);
    }
  }
  return ~crc;
}

bool Ros2Duino::sendPacket(uint8_t packet_type, uint8_t value_type, uint16_t topic_id, const uint8_t * payload, uint32_t payload_size)
{
  if (!status_.transport_ready) return false;
  if (payload_size > ROS2DUINO_MAX_PAYLOAD) return false;

  Header header;
  memset(&header, 0, sizeof(header));
  header.magic = MAGIC;
  header.version = PROTOCOL_VERSION;
  header.packet_type = packet_type;
  header.value_type = value_type;
  header.flags = 0;
  header.seq = seq_++;
  header.stamp_ns = timeSynced() ? rosNowNs() : localNowNs();
  header.device_id = device_id_;
  header.topic_id = topic_id;
  header.payload_size = payload_size;
  header.crc32 = (payload && payload_size > 0) ? crc32(payload, payload_size) : 0;

  uint8_t packet[HEADER_SIZE + ROS2DUINO_MAX_PAYLOAD];
  memcpy(packet, &header, HEADER_SIZE);
  if (payload && payload_size > 0) {
    memcpy(packet + HEADER_SIZE, payload, payload_size);
  }

  const bool ok = writeRawPacket(packet, HEADER_SIZE + payload_size);
  if (ok) status_.tx_count++;
  return ok;
}

bool Ros2Duino::writeRawPacket(const uint8_t * data, size_t length)
{
  if (transport_ == TRANSPORT_SERIAL && serial_) {
    return serial_->write(data, length) == length;
  }

#if ROS2DUINO_HAS_WIFI
  if (transport_ == TRANSPORT_UDP) {
    udp_.beginPacket(ros_ip_, ros_port_);
    udp_.write(data, length);
    return udp_.endPacket() == 1;
  }
#endif

  return false;
}

void Ros2Duino::pollSerial()
{
  if (!serial_) return;

  while (serial_->available() > 0) {
    const int b = serial_->read();
    if (b < 0) break;

    if (rx_size_ < sizeof(rx_buffer_)) {
      rx_buffer_[rx_size_++] = static_cast<uint8_t>(b);
    } else {
      rx_size_ = 0;
      status_.decode_error_count++;
    }
  }

  while (rx_size_ >= HEADER_SIZE) {
    size_t start = 0;
    bool found = false;
    for (; start + 4 <= rx_size_; ++start) {
      uint32_t magic = 0;
      memcpy(&magic, rx_buffer_ + start, sizeof(uint32_t));
      if (magic == MAGIC) {
        found = true;
        break;
      }
    }

    if (!found) {
      rx_size_ = 0;
      return;
    }

    if (start > 0) {
      memmove(rx_buffer_, rx_buffer_ + start, rx_size_ - start);
      rx_size_ -= start;
    }

    Header header;
    memcpy(&header, rx_buffer_, HEADER_SIZE);
    if (header.payload_size > ROS2DUINO_MAX_PAYLOAD) {
      memmove(rx_buffer_, rx_buffer_ + 1, rx_size_ - 1);
      rx_size_ -= 1;
      status_.decode_error_count++;
      continue;
    }

    const size_t packet_size = HEADER_SIZE + header.payload_size;
    if (rx_size_ < packet_size) return;

    PacketView view;
    if (parsePacketBytes(rx_buffer_, packet_size, &view)) {
      handlePacket(view);
    }

    memmove(rx_buffer_, rx_buffer_ + packet_size, rx_size_ - packet_size);
    rx_size_ -= packet_size;
  }
}

void Ros2Duino::pollUdp()
{
#if ROS2DUINO_HAS_WIFI
  if (transport_ != TRANSPORT_UDP) return;
  const int size = udp_.parsePacket();
  if (size <= 0) return;
  if (size > static_cast<int>(HEADER_SIZE + ROS2DUINO_MAX_PAYLOAD)) {
    while (udp_.available()) udp_.read();
    status_.decode_error_count++;
    return;
  }

  uint8_t packet[HEADER_SIZE + ROS2DUINO_MAX_PAYLOAD];
  const int read_len = udp_.read(packet, sizeof(packet));
  PacketView view;
  if (read_len > 0 && parsePacketBytes(packet, static_cast<size_t>(read_len), &view)) {
    handlePacket(view);
  }
#endif
}

bool Ros2Duino::parsePacketBytes(const uint8_t * data, size_t length, PacketView * out)
{
  if (!data || !out || length < HEADER_SIZE) {
    status_.decode_error_count++;
    return false;
  }

  Header header;
  memcpy(&header, data, HEADER_SIZE);

  if (header.magic != MAGIC || header.version != PROTOCOL_VERSION) {
    status_.decode_error_count++;
    return false;
  }

  if (header.payload_size > ROS2DUINO_MAX_PAYLOAD) {
    status_.decode_error_count++;
    return false;
  }

  if (length < HEADER_SIZE + header.payload_size) {
    status_.decode_error_count++;
    return false;
  }

  const uint8_t * payload = data + HEADER_SIZE;
  if (header.payload_size > 0) {
    const uint32_t computed = crc32(payload, header.payload_size);
    if (computed != header.crc32) {
      status_.crc_error_count++;
      return false;
    }
  }

  out->header = header;
  out->payload = payload;
  return true;
}

void Ros2Duino::handlePacket(const PacketView & packet)
{
  status_.rx_count++;
  status_.last_rx_ms = millis();

  if (packet.header.packet_type == PacketType::ACK) {
    status_.ros_ack = true;
    return;
  }

  if (packet.header.packet_type == PacketType::TIME_SYNC_RESP) {
    status_.time_offset_ns = packet.header.stamp_ns - localNowNs();
    status_.time_synced = true;
    status_.ros_ack = true;
    return;
  }

  if (packet.header.packet_type == PacketType::HEARTBEAT) {
    status_.ros_ack = true;
    return;
  }

  if (packet.header.packet_type != PacketType::DATA) return;

  uint16_t count = 0;
  const uint8_t * values = nullptr;

  switch (packet.header.value_type) {
    case ValueType::FLOAT32_ARRAY:
      if (float_cb_ && readCountPayload(packet.payload, packet.header.payload_size, &count, &values, sizeof(float))) {
        float_cb_(reinterpret_cast<const float *>(values), count, packet.header.topic_id);
      }
      break;

    case ValueType::INT32_ARRAY:
      if (int_cb_ && readCountPayload(packet.payload, packet.header.payload_size, &count, &values, sizeof(int32_t))) {
        int_cb_(reinterpret_cast<const int32_t *>(values), count, packet.header.topic_id);
      }
      break;

    case ValueType::UINT8_ARRAY:
      if (bytes_cb_ && readCountPayload(packet.payload, packet.header.payload_size, &count, &values, sizeof(uint8_t))) {
        bytes_cb_(values, count, packet.header.topic_id);
      }
      break;

    case ValueType::BOOL_VALUE:
      if (bool_cb_ && packet.header.payload_size == 1) {
        bool_cb_(packet.payload[0] != 0, packet.header.topic_id);
      }
      break;

    case ValueType::STRING_VALUE:
      if (string_cb_ && readCountPayload(packet.payload, packet.header.payload_size, &count, &values, sizeof(char))) {
        char text[ROS2DUINO_MAX_PAYLOAD];
        const uint16_t len = (count < static_cast<uint16_t>(ROS2DUINO_MAX_PAYLOAD - 1)) ? count : static_cast<uint16_t>(ROS2DUINO_MAX_PAYLOAD - 1);
        memcpy(text, values, len);
        text[len] = '\0';
        string_cb_(text, packet.header.topic_id);
      }
      break;

    case ValueType::TWIST:
      if (twist_cb_ && readCountPayload(packet.payload, packet.header.payload_size, &count, &values, sizeof(double)) && count == 6) {
        const double * v = reinterpret_cast<const double *>(values);
        TwistData twist;
        twist.linear_x = v[0];
        twist.linear_y = v[1];
        twist.linear_z = v[2];
        twist.angular_x = v[3];
        twist.angular_y = v[4];
        twist.angular_z = v[5];
        twist_cb_(twist, packet.header.topic_id);
      }
      break;

    default:
      status_.decode_error_count++;
      break;
  }
}

bool Ros2Duino::encodePodArray(const void * data, uint16_t count, size_t item_size, uint8_t * out, uint32_t * out_size)
{
  if (!out || !out_size) return false;
  const uint32_t needed = sizeof(uint16_t) + static_cast<uint32_t>(count) * static_cast<uint32_t>(item_size);
  if (needed > ROS2DUINO_MAX_PAYLOAD) return false;

  memcpy(out, &count, sizeof(uint16_t));
  if (data && count > 0) {
    memcpy(out + sizeof(uint16_t), data, static_cast<size_t>(count) * item_size);
  }
  *out_size = needed;
  return true;
}

bool Ros2Duino::readCountPayload(const uint8_t * payload, uint32_t payload_size, uint16_t * count, const uint8_t ** values, size_t item_size)
{
  if (!payload || !count || !values || payload_size < sizeof(uint16_t)) {
    status_.decode_error_count++;
    return false;
  }

  uint16_t local_count = 0;
  memcpy(&local_count, payload, sizeof(uint16_t));
  const uint32_t expected = sizeof(uint16_t) + static_cast<uint32_t>(local_count) * static_cast<uint32_t>(item_size);
  if (payload_size != expected) {
    status_.decode_error_count++;
    return false;
  }

  *count = local_count;
  *values = payload + sizeof(uint16_t);
  return true;
}

}  // namespace ros2duino
