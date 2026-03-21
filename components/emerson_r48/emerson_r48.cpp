#include "emerson_r48.h"
#include "esphome/core/application.h"
#include "esphome/core/base_automation.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emerson_r48 {

static const char *const TAG = "emerson_r48";

static const float EMR48_OUTPUT_VOLTAGE_MIN = 41.0;
static const float EMR48_OUTPUT_VOLTAGE_MAX = 58.5;

static const float EMR48_OUTPUT_CURRENT_RATED_VALUE = 62.5;
static const float EMR48_OUTPUT_CURRENT_RATED_PERCENTAGE_MIN = 10;
static const float EMR48_OUTPUT_CURRENT_RATED_PERCENTAGE_MAX = 121;
static const float EMR48_OUTPUT_CURRENT_RATED_PERCENTAGE = 121;
static const float EMR48_OUTPUT_CURRENT_MIN = 5.5; // 10%, rounded up to nearest 0.5A
static const float EMR48_OUTPUT_CURRENT_MAX = EMR48_OUTPUT_CURRENT_RATED_VALUE;

static const uint32_t CAN_ID_REQUEST = 0x06000783;
static const uint32_t CAN_ID_DATA = 0x60f8003; // 0x0707F803;
static const uint32_t CAN_ID_DATA2 = 0x60f8007;
static const uint32_t CAN_ID_SET = 0x0607FF83; // set voltage and max current
static const uint32_t CAN_ID_SET2 = 0x0677FF83; // set voltage and max current
static const uint32_t CAN_ID_SET_CTL = 0x06080783; // set control
static const uint32_t CAN_ID_SYNC = 0x0707FF83;
static const uint32_t CAN_ID_SYNC2 = 0x0717FF83;
static const uint32_t CAN_ID_GIMME5 = 0x06080783;

static const uint8_t EMR48_DATA_OUTPUT_V = 0x01;
static const uint8_t EMR48_DATA_OUTPUT_A = 0x02;
static const uint8_t EMR48_DATA_OUTPUT_AL = 0x03;
static const uint8_t EMR48_DATA_OUTPUT_T = 0x04;
static const uint8_t EMR48_DATA_OUTPUT_IV = 0x05;


void EmersonR48Component::log_can_data_(const char *prefix, const std::vector<uint8_t> &data) {
  char buffer[25];  // max 8 bytes * 3 chars + null
  size_t pos = 0;
  for (size_t i = 0; i < data.size() && pos < sizeof(buffer) - 3; ++i) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02x ", data[i]);
  }
  ESP_LOGD(TAG, "%s: %s", prefix, buffer);
}

EmersonR48Component::EmersonR48Component(canbus::Canbus *canbus) { this->canbus = canbus; }

void EmersonR48Component::sendSync(){
  std::vector<uint8_t> data = {0x04, 0xF0, 0x01, 0x5A, 00, 00, 00, 00};
  this->canbus->send_data(CAN_ID_SYNC, true, data);
}
void EmersonR48Component::sendSync2(){
  std::vector<uint8_t> data = {0x04, 0xF0, 0x5A, 00, 00, 00, 00, 00};
  this->canbus->send_data(CAN_ID_SYNC2, true, data);
}

void EmersonR48Component::gimme5(){
  std::vector<uint8_t> data = {0x20, 0xF0, 00, 0x80, 00, 00, 00, 00};
  this->canbus->send_data(CAN_ID_GIMME5, true, data);
}


void EmersonR48Component::setup() {
  Automation<std::vector<uint8_t>, uint32_t, bool> *automation;
  LambdaAction<std::vector<uint8_t>, uint32_t, bool> *lambdaaction;
  canbus::CanbusTrigger *canbus_canbustrigger;

  // catch all received messages
  canbus_canbustrigger = new canbus::CanbusTrigger(this->canbus, 0, 0, true);
  // canbus_canbustrigger->set_component_source("canbus");  // Commented for ESPHome 2025.x compatibility
  App.register_component(canbus_canbustrigger);
  automation = new Automation<std::vector<uint8_t>, uint32_t, bool>(canbus_canbustrigger);
  auto cb = [this](std::vector<uint8_t> x, uint32_t can_id, bool remote_transmission_request) -> void {
    this->on_frame(can_id, remote_transmission_request, x);
};
  lambdaaction = new LambdaAction<std::vector<uint8_t>, uint32_t, bool>(cb);
  automation->add_actions({lambdaaction});

  this->sendSync();
  this->gimme5();
}

void EmersonR48Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Emerson R48:");
  ESP_LOGCONFIG(TAG, "  Update interval: %u ms", this->update_interval_);
  LOG_SENSOR("  ", "Input Voltage", this->input_voltage_sensor_);
  LOG_SENSOR("  ", "Input Frequency", this->input_frequency_sensor_);
  LOG_SENSOR("  ", "Input Current", this->input_current_sensor_);
  LOG_SENSOR("  ", "Input Power", this->input_power_sensor_);
  LOG_SENSOR("  ", "Input Temp", this->input_temp_sensor_);
  LOG_SENSOR("  ", "Efficiency", this->efficiency_sensor_);
  LOG_SENSOR("  ", "Output Voltage", this->output_voltage_sensor_);
  LOG_SENSOR("  ", "Output Current", this->output_current_sensor_);
  LOG_SENSOR("  ", "Max Output Current", this->max_output_current_sensor_);
  LOG_SENSOR("  ", "Output Power", this->output_power_sensor_);
  LOG_SENSOR("  ", "Output Temp", this->output_temp_sensor_);
}

void EmersonR48Component::update() {
  static uint8_t cnt = 0;
  cnt++;

  if (cnt == 1) {
    ESP_LOGD(TAG, "Requesting output voltage message");
    std::vector<uint8_t> data = {0x01, 0xF0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    this->canbus->send_data(CAN_ID_REQUEST, true, data);
  }
  if (cnt == 2) {
    ESP_LOGD(TAG, "Requesting output current message");
    std::vector<uint8_t> data = {0x01, 0xF0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00};
    this->canbus->send_data(CAN_ID_REQUEST, true, data);
  }
  if (cnt == 3) {
    ESP_LOGD(TAG, "Requesting output current limit message");
    std::vector<uint8_t> data = {0x01, 0xF0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00};
    this->canbus->send_data(CAN_ID_REQUEST, true, data);
  }
  if (cnt == 4) {
    ESP_LOGD(TAG, "Requesting temperature (C) message");
    std::vector<uint8_t> data = {0x01, 0xF0, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
    this->canbus->send_data(CAN_ID_REQUEST, true, data);
  }
  if (cnt == 5) {
    ESP_LOGD(TAG, "Requesting supply voltage message");
    std::vector<uint8_t> data = {0x01, 0xF0, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00};
    this->canbus->send_data(CAN_ID_REQUEST, true, data);
  }

  if (cnt == 6) {
    cnt = 0; 
    // send control every 10 seconds
    this->set_control(this->get_control_byte());

    if (millis() - this->lastCtlSent_ > 10000) {

      this->lastCtlSent_ = millis();
    }
  }


  // no new value for 5* intervall -> set sensors to NAN)
  if (millis() - lastUpdate_ > this->update_interval_ * 10 && cnt == 0) {
    this->publish_sensor_state_(this->input_power_sensor_, NAN);
    this->publish_sensor_state_(this->input_voltage_sensor_, NAN);
    this->publish_sensor_state_(this->input_current_sensor_, NAN);
    this->publish_sensor_state_(this->input_temp_sensor_, NAN);
    this->publish_sensor_state_(this->input_frequency_sensor_, NAN);
    this->publish_sensor_state_(this->output_power_sensor_, NAN);
    this->publish_sensor_state_(this->output_current_sensor_, NAN);
    this->publish_sensor_state_(this->output_voltage_sensor_, NAN);
    this->publish_sensor_state_(this->output_temp_sensor_, NAN);
    this->publish_sensor_state_(this->efficiency_sensor_, NAN);
    this->publish_number_state_(this->max_output_current_number_, NAN);

    this->sendSync();
    this->gimme5();
  }
}

// Function to convert float to byte array
void float_to_bytearray(float value, uint8_t *bytes) {
    uint32_t temp;
    memcpy(&temp, &value, sizeof(temp));
    bytes[0] = (temp >> 24) & 0xFF; // Most significant byte
    bytes[1] = (temp >> 16) & 0xFF;
    bytes[2] = (temp >> 8) & 0xFF;
    bytes[3] = temp & 0xFF; // Least significant byte
}


// https://github.com/PurpleAlien/R48_Rectifier/blob/main/rectifier.py
// # Set the output voltage to the new value. 
// # The 'fixed' parameter 
// #  - if True makes the change permanent ('offline command')
// #  - if False the change is temporary (30 seconds per command received, 'online command', repeat at 15 second intervals).
// # Voltage between 41.0 and 58.5V - fan will go high below 48V!
// def set_voltage(channel, voltage, fixed=False):
//    if OUTPUT_VOLTAGE_MIN <= voltage <= OUTPUT_VOLTAGE_MAX:
//        b = float_to_bytearray(voltage)
//        p = 0x21 if not fixed else 0x24
//        data = [0x03, 0xF0, 0x00, p, *b]
//        send_can_message(channel, data)
//    else:
//        print(f"Voltage should be between {OUTPUT_VOLTAGE_MIN}V and {OUTPUT_VOLTAGE_MAX}V")

void EmersonR48Component::set_output_voltage(float value, bool offline) {
  int32_t raw = 0;
  if (value >= EMR48_OUTPUT_VOLTAGE_MIN && value <= EMR48_OUTPUT_VOLTAGE_MAX) {
    memcpy(&raw, &value, sizeof(raw));
    uint8_t p = offline ? 0x24 : 0x21;
    std::vector<uint8_t> data = {
        0x03, 0xF0, 0x0, p, (uint8_t) (raw >> 24), (uint8_t) (raw >> 16), (uint8_t) (raw >> 8), (uint8_t) raw};
    this->canbus->send_data(CAN_ID_SET, true, data);
    log_can_data_("sent can_message.data", data);

  } else {
    ESP_LOGD(TAG, "set output voltage is out of range: %f", value);
  }

}

//# The output current is set as a value
//# Possible values for 'current': 5.5A - 62.5A
//# The 'fixed' parameter
//#  - if True makes the change permanent ('offline command')
//#  - if False the change is temporary (30 seconds per command received, 'online command', repeat at 15 second intervals).
//def set_current_value(channel, current, fixed=False): 
//    if OUTPUT_CURRENT_MIN <= current <= OUTPUT_CURRENT_MAX:
//        # 62.5A is the nominal current of Emerson/Vertiv R48-3000e and corresponds to 121%
//        percentage = (current/OUTPUT_CURRENT_RATED_VALUE)*OUTPUT_CURRENT_RATED_PERCENTAGE
//        set_current_percentage(channel , percentage, fixed)
//    else:
//       print(f"Current should be between {OUTPUT_CURRENT_MIN}A and {OUTPUT_CURRENT_MAX}A")

//# The output current is set in percent to the rated value of the rectifier written in the manual
//# Possible values for 'current': 10% - 121% (rated current in the datasheet = 121%)
//# The 'fixed' parameter
//#  - if True makes the change permanent ('offline command')
//#  - if False the change is temporary (30 seconds per command received, 'online command', repeat at 15 second intervals).
//def set_current_percentage(channel, current, fixed=False):
//    if OUTPUT_CURRENT_RATED_PERCENTAGE_MIN <= current <= OUTPUT_CURRENT_RATED_PERCENTAGE_MAX:
//        limit = current / 100
//        b = float_to_bytearray(limit)
//        p = 0x22 if not fixed else 0x19
//        data = [0x03, 0xF0, 0x00, p, *b]
//        send_can_message(channel, data)
//    else:
//        print(f"Current should be between {OUTPUT_CURRENT_RATED_PERCENTAGE_MIN}% and {OUTPUT_CURRENT_RATED_PERCENTAGE_MAX}%")

// Function to set current percentage
void EmersonR48Component::set_max_output_current(float value, bool offline) {

    if (value >= EMR48_OUTPUT_CURRENT_RATED_PERCENTAGE_MIN && value <= EMR48_OUTPUT_CURRENT_RATED_PERCENTAGE_MAX) {
        float limit = value / 100.0f;
        uint8_t byte_array[4];
        float_to_bytearray(limit, byte_array);
        
        uint8_t p = offline ? 0x19 : 0x22;
        std::vector<uint8_t> data = { 0x03, 0xF0, 0x00, p, byte_array[0], byte_array[1], byte_array[2], byte_array[3] };
        
        this->canbus->send_data(CAN_ID_SET, true, data);
        log_can_data_("max_output_current: sent can_message.data", data);

    } else {
        ESP_LOGD(TAG, "Current should be between 10 and 121\n");
    }
}

void EmersonR48Component::set_max_input_current(float value) {

    uint8_t byte_array[4];
    float_to_bytearray(value, byte_array);
    
    std::vector<uint8_t> data = { 0x03, 0xF0, 0x00, 0x1A, byte_array[0], byte_array[1], byte_array[2], byte_array[3] };
    
    this->canbus->send_data(CAN_ID_SET, true, data);
    log_can_data_("max_input_current: sent can_message.data", data);
}

void EmersonR48Component::set_offline_values() {
  if (output_voltage_number_) {
    set_output_voltage(output_voltage_number_->state, true);
  };
  if (max_output_current_number_) {
    set_max_output_current(max_output_current_number_->state, true);
  }
}

uint8_t EmersonR48Component::get_control_byte() const {
  return (dcOff_ << 7) | (fanFull_ << 4) | (flashLed_ << 3) | (acOff_ << 2) | 1;
}

void EmersonR48Component::set_control(uint8_t msgv) {
  std::vector<uint8_t> data = { 0x00, 0xF0, msgv, 0x80, 0, 0, 0, 0 };
  this->canbus->send_data(CAN_ID_SET_CTL, true, data);
  log_can_data_("sent control can_message.data", data);
}

void EmersonR48Component::on_frame(uint32_t can_id, bool rtr, std::vector<uint8_t> &data) {
  log_can_data_("received can_message.data", data);

  if (data.size() < 8) {
    ESP_LOGW(TAG, "Ignoring short CAN frame (%d bytes) from 0x%08X", data.size(), can_id);
    return;
  }

  if (can_id == CAN_ID_DATA) {
    uint32_t value = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7];
    float conv_value = 0;
    memcpy(&conv_value, &value, sizeof(conv_value));
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
        this->publish_sensor_state_(this->input_voltage_sensor_, conv_value);
        ESP_LOGV(TAG, "Input voltage: %f", conv_value);

        this->lastUpdate_ = millis();
        break;

      default:
        break;
    }
  }
}

void EmersonR48Component::publish_sensor_state_(sensor::Sensor *sensor, float value) {
  if (sensor) {
    sensor->publish_state(value);
  }
}

void EmersonR48Component::publish_number_state_(number::Number *number, float value) {
  if (number) {
    number->publish_state(value);
  }
}

}  // namespace emerson_r48
}  // namespace esphome
