#include "emerson_r48.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emerson_r48 {

static const char *const TAG = "emerson_r48";

// FIXED: CAN IDs reverted to original values that match PSU responses
static const uint32_t CAN_ID_DATA = 0x60f8003;
static const uint32_t CAN_ID_DATA2 = 0x60f8007;

static const int8_t EMR48_DATA_OUTPUT_V = 0x0;
static const int8_t EMR48_DATA_OUTPUT_A = 0x1;
static const int8_t EMR48_DATA_OUTPUT_T = 0x2;
static const int8_t EMR48_DATA_OUTPUT_AL = 0x3;
static const int8_t EMR48_DATA_OUTPUT_IV = 0x4;

void EmersonR48Component::setup() {
  this->high_freq_.start();
  // Uncommented for ESPHome 2025.x compatibility
  // this->canbus->set_component_source("canbus");
}

void EmersonR48Component::on_frame(uint32_t can_id, bool rtr, std::vector<uint8_t> &data) {
  // Log received CAN data
  ESP_LOGD(TAG, "received can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", data[0], data[1], data[2],
           data[3], data[4], data[5], data[6], data[7]);

  // FIXED: Update lastUpdate_ for ANY valid CAN_ID_DATA response
  // This prevents NaN timeouts when PSU is responding
  if (can_id == CAN_ID_DATA) {
    this->lastUpdate_ = millis();
    
    uint32_t value = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7];
    float conv_value = 0;
    memcpy(&conv_value, &value, sizeof(conv_value));

    switch (data[3]) {
      case EMR48_DATA_OUTPUT_V:
        ESP_LOGV(TAG, "Output voltage: %f", conv_value);
        this->publish_sensor_state_(this->output_voltage_sensor_, conv_value);
        break;

      case EMR48_DATA_OUTPUT_A:
        ESP_LOGV(TAG, "Output current: %f", conv_value);
        this->publish_sensor_state_(this->output_current_sensor_, conv_value);
        break;

      case EMR48_DATA_OUTPUT_AL:
        ESP_LOGV(TAG, "Output current limit: %f", conv_value);
        conv_value = conv_value * 100.0;
        this->publish_number_state_(this->max_output_current_number_, conv_value);
        this->publish_sensor_state_(this->max_output_current_sensor_, conv_value);
        break;

      case EMR48_DATA_OUTPUT_T:
        ESP_LOGV(TAG, "Temperature: %f", conv_value);
        this->publish_sensor_state_(this->output_temp_sensor_, conv_value);
        break;

      case EMR48_DATA_OUTPUT_IV:
        ESP_LOGV(TAG, "Input voltage: %f", conv_value);
        this->publish_sensor_state_(this->input_voltage_sensor_, conv_value);
        // REMOVED: this->lastUpdate_ = millis(); - Now handled at top of CAN_ID_DATA block
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
        ESP_LOGV(TAG, "Output voltage (DATA2): %f", conv_value);
        this->publish_sensor_state_(this->output_voltage_sensor_, conv_value);
        break;

      case EMR48_DATA_OUTPUT_A:
        ESP_LOGV(TAG, "Output current (DATA2): %f", conv_value);
        this->publish_sensor_state_(this->output_current_sensor_, conv_value);
        break;
    }
  }
}

void EmersonR48Component::update() {
  int cnt = 0;

  if (millis() - lastUpdate_ > this->update_interval_ * 10 && cnt == 0) {
    ESP_LOGW(TAG, "No PSU response for %dms, publishing NaN", this->update_interval_ * 10);
    
    this->publish_sensor_state_(this->output_voltage_sensor_, NAN);
    this->publish_sensor_state_(this->output_current_sensor_, NAN);
    this->publish_sensor_state_(this->output_temp_sensor_, NAN);
    this->publish_sensor_state_(this->input_voltage_sensor_, NAN);
    this->publish_sensor_state_(this->max_output_current_sensor_, NAN);
    this->publish_number_state_(this->max_output_current_number_, NAN);
    
    cnt = 1;
  }

  std::vector<uint8_t> data = {0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  ESP_LOGD(TAG, "sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", data[0], data[1], data[2], data[3],
           data[4], data[5], data[6], data[7]);
  this->send_data(data);
}

void EmersonR48Component::send_data(std::vector<uint8_t> &data) { this->canbus->send_data(0x0607FF83, false, data); }

void EmersonR48Component::set_output_voltage(float value, bool offline) {
  uint32_t can_value = 0;
  memcpy(&can_value, &value, sizeof(can_value));

  std::vector<uint8_t> data;
  if (offline == false) {
    data = {0x01, 0xf0, 0x00, 0x00, static_cast<uint8_t>((can_value >> 24) & 0xFF),
            static_cast<uint8_t>((can_value >> 16) & 0xFF), static_cast<uint8_t>((can_value >> 8) & 0xFF),
            static_cast<uint8_t>(can_value & 0xFF)};
  } else {
    data = {0x02, 0xf0, 0x00, 0x00, static_cast<uint8_t>((can_value >> 24) & 0xFF),
            static_cast<uint8_t>((can_value >> 16) & 0xFF), static_cast<uint8_t>((can_value >> 8) & 0xFF),
            static_cast<uint8_t>(can_value & 0xFF)};
  }

  ESP_LOGD(TAG, "max_output_current: sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", data[0],
           data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  this->send_data(data);
}

void EmersonR48Component::set_max_output_current(float value, bool offline) {
  value = value / 100.0;
  uint32_t can_value = 0;
  memcpy(&can_value, &value, sizeof(can_value));

  std::vector<uint8_t> data;
  if (offline == false) {
    data = {0x01, 0xf0, 0x00, 0x03, static_cast<uint8_t>((can_value >> 24) & 0xFF),
            static_cast<uint8_t>((can_value >> 16) & 0xFF), static_cast<uint8_t>((can_value >> 8) & 0xFF),
            static_cast<uint8_t>(can_value & 0xFF)};
  } else {
    data = {0x02, 0xf0, 0x00, 0x03, static_cast<uint8_t>((can_value >> 24) & 0xFF),
            static_cast<uint8_t>((can_value >> 16) & 0xFF), static_cast<uint8_t>((can_value >> 8) & 0xFF),
            static_cast<uint8_t>(can_value & 0xFF)};
  }

  ESP_LOGD(TAG, "max_output_current: sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", data[0],
           data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  this->send_data(data);
}

void EmersonR48Component::set_max_input_current(float value) {
  uint32_t can_value = 0;
  memcpy(&can_value, &value, sizeof(can_value));

  std::vector<uint8_t> data = {0x01, 0xf0, 0x00, 0x04, static_cast<uint8_t>((can_value >> 24) & 0xFF),
                               static_cast<uint8_t>((can_value >> 16) & 0xFF),
                               static_cast<uint8_t>((can_value >> 8) & 0xFF), static_cast<uint8_t>(can_value & 0xFF)};

  ESP_LOGD(TAG, "max_input_current: sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", data[0], data[1],
           data[2], data[3], data[4], data[5], data[6], data[7]);
  this->send_data(data);
}

void EmersonR48Component::sw_control(int8_t sw, bool state) {
  float val = state ? 1.0 : 0.0;
  uint32_t can_value = 0;
  memcpy(&can_value, &val, sizeof(can_value));

  std::vector<uint8_t> data = {0x01, 0xf0, 0x00, static_cast<uint8_t>(sw), static_cast<uint8_t>((can_value >> 24) & 0xFF),
                               static_cast<uint8_t>((can_value >> 16) & 0xFF),
                               static_cast<uint8_t>((can_value >> 8) & 0xFF), static_cast<uint8_t>(can_value & 0xFF)};

  ESP_LOGD(TAG, "sw_control: sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", data[0], data[1],
           data[2], data[3], data[4], data[5], data[6], data[7]);
  this->send_data(data);
}

// NEW: Walk-in control function
void EmersonR48Component::walk_in_control(bool enable, float time_seconds) {
  uint32_t can_value = 0;
  
  if (enable) {
    // Enable walk-in with specified time
    memcpy(&can_value, &time_seconds, sizeof(can_value));
  } else {
    // Disable walk-in (send 0.0)
    float zero = 0.0f;
    memcpy(&can_value, &zero, sizeof(can_value));
  }

  std::vector<uint8_t> data = {0x01, 0xf0, 0x00, 0x09, static_cast<uint8_t>((can_value >> 24) & 0xFF),
                               static_cast<uint8_t>((can_value >> 16) & 0xFF),
                               static_cast<uint8_t>((can_value >> 8) & 0xFF), static_cast<uint8_t>(can_value & 0xFF)};

  ESP_LOGD(TAG, "walk_in_control: sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", data[0], data[1],
           data[2], data[3], data[4], data[5], data[6], data[7]);
  this->send_data(data);
}

// NEW: Restart after overvoltage control function
void EmersonR48Component::restart_overvoltage_control(bool enable) {
  float val = enable ? 1.0f : 0.0f;
  uint32_t can_value = 0;
  memcpy(&can_value, &val, sizeof(can_value));

  std::vector<uint8_t> data = {0x01, 0xf0, 0x00, 0x0A, static_cast<uint8_t>((can_value >> 24) & 0xFF),
                               static_cast<uint8_t>((can_value >> 16) & 0xFF),
                               static_cast<uint8_t>((can_value >> 8) & 0xFF), static_cast<uint8_t>(can_value & 0xFF)};

  ESP_LOGD(TAG, "restart_overvoltage_control: sent can_message.data: %02x %02x %02x %02x %02x %02x %02x %02x", 
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  this->send_data(data);
}

void EmersonR48Component::publish_sensor_state_(sensor::Sensor *sensor, float value) {
  if (sensor != nullptr) {
    sensor->publish_state(value);
  }
}

void EmersonR48Component::publish_number_state_(number::Number *number, float value) {
  if (number != nullptr) {
    number->publish_state(value);
  }
}

}  // namespace emerson_r48
}  // namespace esphome
