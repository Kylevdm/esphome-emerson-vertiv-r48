#include "emerson_r48.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emerson_r48 {

static const char *const TAG = "emerson_r48";

void EmersonR48::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Emerson R48...");
  
  // Set safe offline defaults on boot
  this->set_safe_defaults_();
  
  // Initialize timing
  this->last_request_time_ = millis();
  this->last_control_time_ = millis();
  this->last_response_time_ = millis();
}

void EmersonR48::loop() {
  uint32_t now = millis();
  
  // Send control message every 10 seconds
  if (now - this->last_control_time_ >= CONTROL_INTERVAL) {
    this->send_control_message_();
    this->last_control_time_ = now;
  }
  
  // Send request message at configured interval
  if (now - this->last_request_time_ >= this->update_interval_) {
    this->send_request_();
    this->last_request_time_ = now;
  }
  
  // Check for timeout and publish NaN if needed
  if (now - this->last_response_time_ >= RESPONSE_TIMEOUT) {
    this->publish_nan_if_timeout_();
  }
}

void EmersonR48::dump_config() {
  ESP_LOGCONFIG(TAG, "Emerson R48:");
  ESP_LOGCONFIG(TAG, "  Update Interval: %u ms", this->update_interval_);
  ESP_LOGCONFIG(TAG, "  Offline Voltage: %.2f V", this->offline_voltage_);
  ESP_LOGCONFIG(TAG, "  Offline Current: %.0f %%", this->offline_current_percent_);
  ESP_LOGCONFIG(TAG, "  Max Power Limit: %.0f W", MAX_POWER_W);
}

void EmersonR48::set_safe_defaults_() {
  ESP_LOGI(TAG, "Setting safe offline defaults: %.2fV, %.0f%%", 
           this->offline_voltage_, this->offline_current_percent_);
  
  // Set target values to offline defaults
  this->target_output_voltage_ = this->offline_voltage_;
  this->target_max_output_current_percent_ = this->offline_current_percent_;
  this->target_power_on_ = true;
  
  // Send immediately to establish offline values
  this->send_control_message_();
  this->first_control_sent_ = true;
}

void EmersonR48::send_request_() {
  if (this->canbus_ == nullptr) {
    ESP_LOGW(TAG, "CAN bus not configured");
    return;
  }
  
  // Send "read all" request - single frame to get all sensor data
  std::vector<uint8_t> data = {
    0x01,                    // Command type
    0xF0,                    // Read command
    0x00,                    // Reserved
    REQUEST_READ_ALL,        // Read all parameters (0x80)
    0x00, 0x00, 0x00, 0x00   // Padding
  };
  
  this->canbus_->send_data(CAN_ID_CONTROL, false, data);
  ESP_LOGD(TAG, "Sent read all request");
}

void EmersonR48::send_control_message_() {
  if (this->canbus_ == nullptr) {
    ESP_LOGW(TAG, "CAN bus not configured");
    return;
  }
  
  // Validate power limit before sending
  if (!this->validate_power_limit_(this->target_output_voltage_, 
                                    this->target_max_output_current_percent_)) {
    // Already logged in validate function, value was adjusted
  }
  
  // Convert voltage to protocol format (voltage * 1024 / 1000)
  uint16_t voltage_raw = static_cast<uint16_t>((this->target_output_voltage_ * 1024.0f) / 100.0f);
  
  // Convert current percent to protocol format (percent * 20)
  uint16_t current_raw = static_cast<uint16_t>(this->target_max_output_current_percent_ * 20.0f);
  
  // Build control message
  std::vector<uint8_t> data = {
    static_cast<uint8_t>(voltage_raw & 0xFF),        // Voltage low byte
    static_cast<uint8_t>((voltage_raw >> 8) & 0xFF), // Voltage high byte
    static_cast<uint8_t>(current_raw & 0xFF),        // Current low byte
    static_cast<uint8_t>((current_raw >> 8) & 0xFF), // Current high byte
    this->target_power_on_ ? 0x01 : 0x00,            // Power on/off
    this->target_restart_overvoltage_ ? 0x01 : 0x00, // Restart overvoltage
    0x00,                                             // Reserved
    0x00                                              // Reserved
  };
  
  this->canbus_->send_data(CAN_ID_CONTROL, false, data);
  
  ESP_LOGD(TAG, "Sent control: V=%.2fV (0x%04X), I=%.0f%% (0x%04X), Power=%s", 
           this->target_output_voltage_, voltage_raw,
           this->target_max_output_current_percent_, current_raw,
           this->target_power_on_ ? "ON" : "OFF");
}

void EmersonR48::on_frame_(uint32_t can_id, const std::vector<uint8_t> &data) {
  // Update last response time
  this->last_response_time_ = millis();
  
  // Check if this is a data frame we care about
  if (can_id != CAN_ID_DATA && can_id != CAN_ID_DATA2) {
    return;
  }
  
  if (data.size() < 8) {
    ESP_LOGW(TAG, "Received frame with insufficient data: %zu bytes", data.size());
    return;
  }
  
  ESP_LOGD(TAG, "Received CAN frame ID: 0x%08X", can_id);
  
  // Parse based on CAN ID
  if (can_id == CAN_ID_DATA) {
    // Primary data frame
    // Bytes 0-1: Output voltage (V * 1024 / 100)
    uint16_t voltage_raw = data[0] | (data[1] << 8);
    this->current_output_voltage_ = (voltage_raw * 100.0f) / 1024.0f;
    
    // Bytes 2-3: Output current (A * 20)
    uint16_t current_raw = data[2] | (data[3] << 8);
    this->current_output_current_ = current_raw / 20.0f;
    
    // Bytes 4-5: Max output current limit (% * 20)
    uint16_t max_current_raw = data[4] | (data[5] << 8);
    this->current_max_output_current_percent_ = max_current_raw / 20.0f;
    
    // Publish to sensors
    if (this->output_voltage_sensor_ != nullptr) {
      this->output_voltage_sensor_->publish_state(this->current_output_voltage_);
    }
    if (this->output_current_sensor_ != nullptr) {
      this->output_current_sensor_->publish_state(this->current_output_current_);
    }
    if (this->max_output_current_sensor_ != nullptr) {
      this->max_output_current_sensor_->publish_state(this->current_max_output_current_percent_);
    }
    
    ESP_LOGD(TAG, "Parsed data: V=%.2fV, I=%.2fA, MaxI=%.0f%%", 
             this->current_output_voltage_, 
             this->current_output_current_,
             this->current_max_output_current_percent_);
    
  } else if (can_id == CAN_ID_DATA2) {
    // Secondary data frame
    // Bytes 0-1: Temperature (°C * 10)
    uint16_t temp_raw = data[0] | (data[1] << 8);
    this->current_output_temp_ = temp_raw / 10.0f;
    
    // Bytes 2-3: Input voltage (V * 10)
    uint16_t input_voltage_raw = data[2] | (data[3] << 8);
    this->current_input_voltage_ = input_voltage_raw / 10.0f;
    
    // Publish to sensors
    if (this->output_temp_sensor_ != nullptr) {
      this->output_temp_sensor_->publish_state(this->current_output_temp_);
    }
    if (this->input_voltage_sensor_ != nullptr) {
      this->input_voltage_sensor_->publish_state(this->current_input_voltage_);
    }
    
    ESP_LOGD(TAG, "Parsed data2: Temp=%.1f°C, InputV=%.1fV", 
             this->current_output_temp_,
             this->current_input_voltage_);
  }
  
  // Update computed sensors after receiving new data
  this->update_computed_sensors_();
}

void EmersonR48::update_computed_sensors_() {
  // Calculate and publish output power (V * I)
  if (!std::isnan(this->current_output_voltage_) && 
      !std::isnan(this->current_output_current_)) {
    float output_power = this->current_output_voltage_ * this->current_output_current_;
    if (this->output_power_sensor_ != nullptr) {
      this->output_power_sensor_->publish_state(output_power);
    }
    
    // Calculate and publish power headroom (3000W - current power)
    float headroom = MAX_POWER_W - output_power;
    if (this->power_headroom_sensor_ != nullptr) {
      this->power_headroom_sensor_->publish_state(headroom);
    }
  }
  
  // Calculate and publish max current in amperes
  if (!std::isnan(this->current_max_output_current_percent_)) {
    float max_current_a = (this->current_max_output_current_percent_ / MAX_CURRENT_PERCENT) * RATED_CURRENT_A;
    if (this->max_current_amperes_sensor_ != nullptr) {
      this->max_current_amperes_sensor_->publish_state(max_current_a);
    }
  }
}

void EmersonR48::publish_nan_if_timeout_() {
  static bool timeout_logged = false;
  
  if (!timeout_logged) {
    ESP_LOGW(TAG, "Response timeout - publishing NaN to sensors");
    timeout_logged = true;
  }
  
  // Publish NaN to all sensors to indicate no data
  if (this->output_voltage_sensor_ != nullptr) {
    this->output_voltage_sensor_->publish_state(NAN);
  }
  if (this->output_current_sensor_ != nullptr) {
    this->output_current_sensor_->publish_state(NAN);
  }
  if (this->max_output_current_sensor_ != nullptr) {
    this->max_output_current_sensor_->publish_state(
