#include "esphome/core/log.h"
#include "emerson_switch.h"

namespace esphome {
namespace emerson_r48 {

static const char *TAG = "emerson_switch.switch";

static const int8_t SET_AC_FUNCTION = 0x0;
static const int8_t SET_DC_FUNCTION = 0x1;
static const int8_t SET_FAN_FUNCTION = 0x2;
static const int8_t SET_LED_FUNCTION = 0x3;



void EmersonR48Switch::setup() {

}

void EmersonR48Switch::write_state(bool state) {
    ESP_LOGD(TAG, "-> new switch state: %d", state);

    switch (this->functionCode_) {
        case SET_AC_FUNCTION:  parent_->acOff_ = state; break;
        case SET_DC_FUNCTION:  parent_->dcOff_ = state; break;
        case SET_FAN_FUNCTION: parent_->fanFull_ = state; break;
        case SET_LED_FUNCTION: parent_->flashLed_ = state; break;
        default: return;
    }
    parent_->set_control(parent_->get_control_byte());
    this->publish_state(state);
}

void EmersonR48Switch::dump_config(){
    ESP_LOGCONFIG(TAG, "Emerson custom switch");
}

} //namespace emerson_r48
} //namespace esphome