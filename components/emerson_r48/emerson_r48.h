#pragma once

#include "esphome/core/component.h"
#include "esphome/components/canbus/canbus.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace emerson_r48 {

// CAN IDs (extended frames)
static const uint32_t CAN_ID_CONTROL = 0x0607FF83;  // For sending control/set commands
static const uint32_t CAN_ID_READ = 0x06000783;     // For sending read requests

// Response/Data IDs (what the device sends back)
static const uint32_t CAN_ID_RESPONSE_1 = 0x0707F803;  // Primary response
static const uint32_t CAN_ID_RESPONSE_2 = 0x060F8003;  // Secondary response  
static const uint32_t CAN_ID_RESPONSE_3 = 0x060F8007;  // Tertiary response


// Request types
static const uint8_t REQUEST_READ_ALL = 0x80;

// Timing constants
static const uint32_t CONTROL_INTERVAL = 10000;  // Send control every 10s
static const uint32_t RESPONSE_TIMEOUT = 30000;  // 30s timeout for responses

// Power and current limits
static const float MAX_POWER_W = 3000.0f;
static const float RATED_CURRENT_A = 62.5f;
static const float MAX_CURRENT_PERCENT = 121.0f;

class EmersonR48Component : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Configuration
  void set_canbus(canbus::Canbus *canbus) { this->canbus_ = canbus; }
  void set_update_interval(uint32_t interval) { this->update_interval_ = interval; }
  void set_offline_voltage(float voltage) { this->offline_voltage_ = voltage; }
  void set_offline_current_percent(float percent) { this->offline_current_percent_ = percent; }

  // Sensors
  void set_output_voltage_sensor(sensor::Sensor *sensor) { this->output_voltage_sensor_ = sensor; }
  void set_output_current_sensor(sensor::Sensor *sensor) { this->output_current_sensor_ = sensor; }
  void set_max_output_current_sensor(sensor::Sensor *sensor) { this->max_output_current_sensor_ = sensor; }
  void set_output_temp_sensor(sensor::Sensor *sensor) { this->output_temp_sensor_ = sensor; }
  void set_input_voltage_sensor(sensor::Sensor *sensor) { this->input_voltage_sensor_ = sensor; }
  void set_output_power_sensor(sensor::Sensor *sensor) { this->output_power_sensor_ = sensor; }
  void set_max_current_amperes_sensor(sensor::Sensor *sensor) { this->max_current_amperes_sensor_ = sensor; }
  void set_power_headroom_sensor(sensor::Sensor *sensor) { this->power_headroom_sensor_ = sensor; }

  // Control methods (called by number/switch/button components)
  void set_output_voltage(float voltage);
  void set_max_output_current(float current_percent);
  void set_power_on(bool on);
  void set_restart_overvoltage(bool enable);
  void set_offline_values();

  // CAN frame handler (public so it can be called from lambda)
  void on_frame_(uint32_t can_id, const std::vector<uint8_t> &data);

  // State for switches (public so switch component can access)
  bool acOff_{false};
  bool dcOff_{false};
  bool fanFull_{false};
  bool flashLed_{false};
  
  // Method for switch component
  void set_control(uint8_t value) {
    // This would send a control message with the switch states
    ESP_LOGD("emerson_r48", "Set control: 0x%02X", value);
  }

 protected:
  canbus::Canbus *canbus_{nullptr};
  
  // Sensors
  sensor::Sensor *output_voltage_sensor_{nullptr};
  sensor::Sensor *output_current_sensor_{nullptr};
  sensor::Sensor *max_output_current_sensor_{nullptr};
  sensor::Sensor *output_temp_sensor_{nullptr};
  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *output_power_sensor_{nullptr};
  sensor::Sensor *max_current_amperes_sensor_{nullptr};
  sensor::Sensor *power_headroom_sensor_{nullptr};

  // Configuration
  uint32_t update_interval_{5000};
  float offline_voltage_{53.1f};
  float offline_current_percent_{50.0f};

  // Target values (what we want to set)
  float target_output_voltage_{53.1f};
  float target_max_output_current_percent_{50.0f};
  bool target_power_on_{true};
  bool target_restart_overvoltage_{false};
  
  // Current values (what device reports)
  float current_output_voltage_{NAN};
  float current_output_current_{NAN};
  float current_max_output_current_percent_{NAN};
  float current_output_temp_{NAN};
  float current_input_voltage_{NAN};
  
  // Timing
  uint32_t last_request_time_{0};
  uint32_t last_control_time_{0};
  uint32_t last_response_time_{0};
  bool first_control_sent_{false};
  
  // Internal methods
  void set_safe_defaults_();
  void send_request_();
  void send_control_message_();
  void update_computed_sensors_();
  void publish_nan_if_timeout_();
  bool validate_power_limit_(float voltage, float current_percent);
};

// Type alias for backward compatibility
using EmersonR48 = EmersonR48Component;

}  // namespace emerson_r48
}  // namespace esphome
