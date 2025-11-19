#include "emerson_r48.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome {
namespace emerson_r48 {

static const char *const TAG = "emerson_r48";

// CAN IDs
static const uint32_t CAN_ID_DATA = 0x60f8003;
static const uint32_t CAN_ID_DATA2 = 0x60f8007;

// Data types
static const uint8_t EMR48_DATA_OUTPUT_V = 0x0;
static const uint8_t EMR48_DATA_OUTPUT_A = 0x1;
static const uint8_t EMR48_DATA_OUTPUT_T = 0x2;
static const uint8_t EMR48_DATA_OUTPUT_AL = 0x3;
static const uint8_t EMR48_DATA_OUTPUT_IV = 0x4;

// Command types
static const uint8_t EMR48_CMD_SET_VOLTAGE = 0x0;
static const uint8_t EMR48_CMD_SET_CURRENT = 0x3;
static const uint8_t EMR48_CMD_SET_INPUT_CURRENT = 0x4;
static const uint8_t EMR48_CMD_AC_SW = 0x5;
static const uint8_t EMR48_CMD_DC_SW = 0x6;
static const uint8_t EMR48_CMD_FAN_SW = 0x7;
static const uint8_t EMR48_CMD_LED_SW = 0x8;
static const uint8_t EMR48_CMD_WALK_IN = 0x9;
static const uint8_t EMR48_CMD_RESTART_OVERVOLTAGE = 0xA;

void EmersonR48Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Emerson R48...");  
  // Initialize lastUpdate_ to current time to prevent immediate NaN
  this->lastUpdate_ = millis();
}

void EmersonR48Component::loop() {
  // Periodic update request
  uint32_t now = millis();
  if (now - this->last_send_ > this->update_interval_) {
    this->request_data();
    this->last_send_ = now;
  }
}

void EmersonR48Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Emerson R48:");
  ESP_LOGCONFIG(TAG, "  Update Interval: %ums", this->update_interval_);
}

void EmersonR48Component::on_frame(uint32_t can_id, bool rtr, std::vector<uint8_t> &data) {
  if (data.size() < 8) {
    ESP_LOGW(TAG, "Received CAN message with invalid size: %d", data.size());
    return;
  }

  ESP_LOGD(TAG, "received can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

  // FIXED: Update lastUpdate_ for ANY valid CAN_ID_DATA response
  // This prevents NaN timeouts when PSU is responding but not sending all data types
  if (can_id == CAN_ID_DATA) {
    uint32_t value = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7];
    float conv_value = 0;
    memcpy(&conv_value, &value, sizeof(conv_value));
    this->lastUpdate_ = millis();
    switch (data[3]) {
      case EMR48_DATA_OUTPUT_V:
        this->publish_sensor_state_(this->output_voltage_sensor_, conv_value);
        ESP_LOGV(TAG, "Output voltage: %f", conv_value);
        break;

      case EMR48_DATA_OUTPUT_A:
        this->publish_sensor_state_(this->output_current_sensor_, conv_value);
        ESP_LOGV(TAG, "Output current: %f", conv_value);
        break;

      case EMR48_DATA_OUTPUT_AL:
        conv_value = conv_value * 100.0;
        this->publish_number_state_(this->max_output_current_number_, conv_value);
        this->publish_sensor_state_(this->max_output_current_sensor_, conv_value);
        ESP_LOGV(TAG, "Output current limit: %f", conv_value);
        break;

      case EMR48_DATA_OUTPUT_T:
        this->publish_sensor_state_(this->output_temp_sensor_, conv_value);
        ESP_LOGV(TAG, "Temperature: %f", conv_value);
        break;

      case EMR48_DATA_OUTPUT_IV:
        case EMR48_DATA_OUTPUT_IV:
        ESP_LOGV(TAG, "Input voltage: %f", conv_value);
        this->publish_sensor_state_(this->input_voltage_sensor_, conv_value);
        break;

      default:
        ESP_LOGV(TAG, "Unknown data type: %02x", data[3]);
        break;
    }
  }

  // FIXED: Also update lastUpdate_ for CAN_ID_DATA2 responses
  if (can_id == CAN_ID_DATA2) {
    this->lastUpdate_ = millis();
    
    uint32_t value = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7];
    float conv_value = 0;
    memcpy(&conv_value, &value, sizeof(conv_value));

    switch (data[3]) {
      case EMR48_DATA_OUTPUT_V:
        this->publish_sensor_state_(this->output_voltage_sensor_, conv_value);
        ESP_LOGV(TAG, "Output voltage (DATA2): %f", conv_value);
        break;

      case EMR48_DATA_OUTPUT_A:
        this->publish_sensor_state_(this->output_current_sensor_, conv_value);
        ESP_LOGV(TAG, "Output current (DATA2): %f", conv_value);
        break;

      default:
        ESP_LOGV(TAG, "Unknown data type (DATA2): %02x", data[3]);
        break;
    }
  }
}

void EmersonR48Component::update() {
  int cnt = 0;

  // Check for communication timeout
  if (millis() - lastUpdate_ > this->update_interval_ * 10 && cnt == 0) {
    ESP_LOGW(TAG, "No response from PSU for %d ms, publishing NaN", this->update_interval_ * 10);
    
    this->publish_sensor_state_(this->output_voltage_sensor_, NAN);
    this->publish_sensor_state_(this->output_current_sensor_, NAN);
    this->publish_sensor_state_(this->output_temp_sensor_, NAN);
    this->publish_sensor_state_(this->input_voltage_sensor_, NAN);
    this->publish_sensor_state_(this->max_output_current_sensor_, NAN);
    this->publish_number_state_(this->max_output_current_number_, NAN);
    
    cnt = 1;
  }
}

void EmersonR48Component::request_data() {
  std::vector<uint8_t> data = {0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  ESP_LOGD(TAG, "Requesting data from PSU");
  this->canbus->send_data(0x0607FF83, false, data);
}

// Voltage control
void EmersonR48Component::set_output_voltage(float voltage) {
  uint32_t value;
  memcpy(&value, &voltage, sizeof(value));
  
  std::vector<uint8_t> data = {
    0x01, 0xf0, 0x00, EMR48_CMD_SET_VOLTAGE,
    static_cast<uint8_t>((value >> 24) & 0xFF),
    static_cast<uint8_t>((value >> 16) & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>(value & 0xFF)
  };
  
  ESP_LOGD(TAG, "Setting output voltage to %.2fV", voltage);
  ESP_LOGD(TAG, "sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  
  this->canbus->send_data(0x0607FF83, false, data);
}

// Current control
void EmersonR48Component::set_max_output_current(float current) {
  float percentage = current / 100.0;
  uint32_t value;
  memcpy(&value, &percentage, sizeof(value));
  
  std::vector<uint8_t> data = {
    0x01, 0xf0, 0x00, EMR48_CMD_SET_CURRENT,
    static_cast<uint8_t>((value >> 24) & 0xFF),
    static_cast<uint8_t>((value >> 16) & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>(value & 0xFF)
  };
  
  ESP_LOGD(TAG, "Setting max output current to %.2f%% (%.2fA)", current, current);
  ESP_LOGD(TAG, "sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  
  this->canbus->send_data(0x0607FF83, false, data);
}

void EmersonR48Component::set_max_input_current(float current) {
  uint32_t value;
  memcpy(&value, &current, sizeof(value));
  
  std::vector<uint8_t> data = {
    0x01, 0xf0, 0x00, EMR48_CMD_SET_INPUT_CURRENT,
    static_cast<uint8_t>((value >> 24) & 0xFF),
    static_cast<uint8_t>((value >> 16) & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>(value & 0xFF)
  };
  
  ESP_LOGD(TAG, "Setting max input current to %.2fA", current);
  ESP_LOGD(TAG, "sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  
  this->canbus->send_data(0x0607FF83, false, data);
}

// Switch controls
void EmersonR48Component::ac_switch(bool state) {
  float value = state ? 1.0f : 0.0f;
  uint32_t int_value;
  memcpy(&int_value, &value, sizeof(int_value));
  
  std::vector<uint8_t> data = {
    0x01, 0xf0, 0x00, EMR48_CMD_AC_SW,
    static_cast<uint8_t>((int_value >> 24) & 0xFF),
    static_cast<uint8_t>((int_value >> 16) & 0xFF),
    static_cast<uint8_t>((int_value >> 8) & 0xFF),
    static_cast<uint8_t>(int_value & 0xFF)
  };
  
  ESP_LOGD(TAG, "Setting AC switch to %s", state ? "ON" : "OFF");
  this->canbus->send_data(0x0607FF83, false, data);
}

void EmersonR48Component::dc_switch(bool state) {
  float value = state ? 1.0f : 0.0f;
  uint32_t int_value;
  memcpy(&int_value, &value, sizeof(int_value));
  
  std::vector<uint8_t> data = {
    0x01, 0xf0, 0x00, EMR48_CMD_DC_SW,
    static_cast<uint8_t>((int_value >> 24) & 0xFF),
    static_cast<uint8_t>((int_value >> 16) & 0xFF),
    static_cast<uint8_t>((int_value >> 8) & 0xFF),
    static_cast<uint8_t>(int_value & 0xFF)
  };
  
  ESP_LOGD(TAG, "Setting DC switch to %s", state ? "ON" : "OFF");
  this->canbus->send_data(0x0607FF83, false, data);
}

void EmersonR48Component::fan_switch(bool state) {
  float value = state ? 1.0f : 0.0f;
  uint32_t int_value;
  memcpy(&int_value, &value, sizeof(int_value));
  
  std::vector<uint8_t> data = {
    0x01, 0xf0, 0x00, EMR48_CMD_FAN_SW,
    static_cast<uint8_t>((int_value >> 24) & 0xFF),
    static_cast<uint8_t>((int_value >> 16) & 0xFF),
    static_cast<uint8_t>((int_value >> 8) & 0xFF),
    static_cast<uint8_t>(int_value & 0xFF)
  };
  
  ESP_LOGD(TAG, "Setting FAN switch to %s", state ? "MAX" : "AUTO");
  this->canbus->send_data(0x0607FF83, false, data);
}

void EmersonR48Component::led_switch(bool state) {
  float value = state ? 1.0f : 0.0f;
  uint32_t int_value;
  memcpy(&int_value, &value, sizeof(int_value));
  
  std::vector<uint8_t> data = {
    0x01, 0xf0, 0x00, EMR48_CMD_LED_SW,
    static_cast<uint8_t>((int_value >> 24) & 0xFF),
    static_cast<uint8_t>((int_value >> 16) & 0xFF),
    static_cast<uint8_t>((int_value >> 8) & 0xFF),
    static_cast<uint8_t>(int_value & 0xFF)
  };
  
  ESP_LOGD(TAG, "Setting LED switch to %s", state ? "ON" : "OFF");
  this->canbus
