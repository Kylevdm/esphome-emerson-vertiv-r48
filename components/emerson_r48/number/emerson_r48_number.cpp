#include "emerson_r48_number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emerson_r48 {

static const char *const TAG = "emerson_r48.number";

void EmersonR48Number::control(float value) {
  switch (this->type_) {
    case 0:  // Voltage
      ESP_LOGD(TAG, "Setting voltage to %.2f V", value);
      this->parent_->set_output_voltage(value);
      this->publish_state(value);
      break;
      
    case 1:  // Current percent
      ESP_LOGD(TAG, "Setting max current to %.0f %%", value);
      this->parent_->set_max_output_current(value);
      this->publish_state(value);
      break;

    default:
      ESP_LOGW(TAG, "Unknown number type: %d", this->type_);
      break;
  }
}

}  // namespace emerson_r48
}  // namespace esphome
