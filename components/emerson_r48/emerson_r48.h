#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/canbus/canbus.h"

namespace esphome {
namespace emerson_r48 {

class EmersonR48Switch;  // forward declaration

class EmersonR48Component : public PollingComponent {
  friend class EmersonR48Switch;

 public:
  EmersonR48Component(canbus::Canbus *canbus);
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_output_voltage(float value, bool offline = false);
  void set_max_output_current(float value, bool offline = false);
  void set_max_input_current(float value);
  void set_offline_values();

  void set_input_voltage_sensor(sensor::Sensor *input_voltage_sensor) { input_voltage_sensor_ = input_voltage_sensor; }
  void set_input_frequency_sensor(sensor::Sensor *input_frequency_sensor) {
    input_frequency_sensor_ = input_frequency_sensor;
  }
  void set_input_current_sensor(sensor::Sensor *input_current_sensor) { input_current_sensor_ = input_current_sensor; }
  void set_input_power_sensor(sensor::Sensor *input_power_sensor) { input_power_sensor_ = input_power_sensor; }
  void set_input_temp_sensor(sensor::Sensor *input_temp_sensor) { input_temp_sensor_ = input_temp_sensor; }
  void set_efficiency_sensor(sensor::Sensor *efficiency_sensor) { efficiency_sensor_ = efficiency_sensor; }
  void set_output_voltage_sensor(sensor::Sensor *output_voltage_sensor) {
    output_voltage_sensor_ = output_voltage_sensor;
  }
  void set_output_current_sensor(sensor::Sensor *output_current_sensor) {
    output_current_sensor_ = output_current_sensor;
  }
  void set_max_output_current_sensor(sensor::Sensor *max_output_current_sensor) {
     max_output_current_sensor_ = max_output_current_sensor;
  }

  void set_output_power_sensor(sensor::Sensor *output_power_sensor) { output_power_sensor_ = output_power_sensor; }
  void set_output_temp_sensor(sensor::Sensor *output_temp_sensor) { output_temp_sensor_ = output_temp_sensor; }

  void set_output_voltage_number(number::Number *output_voltage_number) {
    output_voltage_number_ = output_voltage_number;
  }
  void set_max_output_current_number(number::Number *max_output_current_number) {
    max_output_current_number_ = max_output_current_number;
  }
  void set_max_input_current_number(number::Number *max_input_current_number) {
    max_input_current_number_ = max_input_current_number;
  }

  void set_control(uint8_t msgv);
  uint8_t get_control_byte() const;

  void sendSync();
  void sendSync2();
  void gimme5();

 protected:
  canbus::Canbus *canbus;
  uint32_t lastUpdate_;
  uint32_t lastCtlSent_;

  bool dcOff_ = false;
  bool fanFull_ = false;
  bool flashLed_ = false;
  bool acOff_ = false;

  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *input_frequency_sensor_{nullptr};
  sensor::Sensor *input_current_sensor_{nullptr};
  sensor::Sensor *input_power_sensor_{nullptr};
  sensor::Sensor *input_temp_sensor_{nullptr};
  sensor::Sensor *efficiency_sensor_{nullptr};
  sensor::Sensor *output_voltage_sensor_{nullptr};
  sensor::Sensor *output_current_sensor_{nullptr};
  sensor::Sensor *max_output_current_sensor_{nullptr};
  sensor::Sensor *output_power_sensor_{nullptr};
  sensor::Sensor *output_temp_sensor_{nullptr};

  number::Number *output_voltage_number_{nullptr};
  number::Number *max_output_current_number_{nullptr};
  number::Number *max_input_current_number_{nullptr};

  void on_frame(uint32_t can_id, bool rtr, std::vector<uint8_t> &data);

  static void log_can_data_(const char *prefix, const std::vector<uint8_t> &data);
  void publish_sensor_state_(sensor::Sensor *sensor, float value);
  void publish_number_state_(number::Number *number, float value);
};

}  // namespace emerson_r48
}  // namespace esphome
