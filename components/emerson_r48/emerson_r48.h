#pragma once

#include "esphome/core/component.h"
#include "esphome/components/canbus/canbus.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace emerson_r48 {

// CAN IDs
static const uint32_t CAN_ID_CONTROL = 0x108040FE;
static const uint32_t CAN_ID_DATA = 0x1081407F;
static const uint32_t CAN_ID_DATA2 = 0x1081D27F;

// R48 Parameters for requests
static const uint8_t PARAM_INPUT_VOLTAGE = 0x00;
static const uint8_t PARAM_INPUT_CURRENT = 0x01;
static const uint8_t PARAM_OUTPUT_VOLTAGE = 0x02;
static const uint8_t PARAM_OUTPUT_CURRENT = 0x03;
static const uint8_t PARAM_OUTPUT_TEMP = 0x04;

// Power limit constant
static const float MAX_POWER_WATTS = 3000.0f;
static const float RATED_CURRENT_AMPS = 62.5f;

class EmersonR48Component : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Configuration
  void set_canbus(canbus::Canbus *canbus) { canbus_ = canbus; }
  void set_update_interval(uint32_t interval) { update_interval_ = interval; }
  void set_offline_voltage(float voltage) { offline_voltage_ = voltage; }
  void set_offline_current_percent(float percent) { offline_current_percent_ = percent; }

  // Sensors
  void set_output_voltage_sensor(sensor::Sensor *sensor) { output_voltage_sensor_ = sensor; }
  void set_output_current_sensor(sensor::Sensor *sensor) { output_current_sensor_ = sensor; }
  void set_max_output_current_sensor(sensor::Sensor *sensor) { max_output_current_sensor_ = sensor; }
  void set_output_temp_sensor(sensor::Sensor *sensor) { output_temp_sensor_ = sensor; }
  void set_input_voltage_sensor(sensor::Sensor *sensor) { input_voltage_sensor_ = sensor; }
  void set_output_power_sensor(sensor::Sensor *sensor) { output_power_sensor_ = sensor; }
  void set_max_current_amperes_sensor(sensor::Sensor *sensor) { max_current_amperes_sensor_ = sensor; }
  void set_power_headroom_sensor(sensor::Sensor *sensor) { power_headroom_sensor_ = sensor; }

  // Control methods
  void set_output_voltage(float voltage);
  void set_max_output_current(float current_percent);
  void set_power_on(bool on);
  void set_restart_overvoltage(bool enable);

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

  // State tracking
  float target_voltage_{53.5f};
  float target_current_percent_{50.0f};
  float current_voltage_{0.0f};
  float current_current_{0.0f};
  bool power_on_{true};
  
  uint32_t last_update_{0};
  uint32_t last_control_message_{0};
  uint32_t last_receive_time_{0};
  
  // Internal methods
  void send_control_message_();
  void send_read_all_request_();
  void process_can_message_(uint32_t can_id, const std::vector<uint8_t> &data);
  void check_timeout_();
  void set_safe_defaults_();
  float calculate_max_safe_current_(float voltage);
  void update_computed_sensors_();
};

}  // namespace emerson_r48
}  // namespace esphome
