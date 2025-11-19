#include "emerson_r48_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emerson_r48 {


static const char *const TAG = "emerson_r48";

void EmersonR48Button::press_action() { 
    ESP_LOGD(TAG, "-> set_offline_values button pressed");
    this->parent_->set_offline_values(); 
}

void WalkInButton::press_action() {
    ESP_LOGD(TAG, "-> walk_in button pressed: enable=%d, time=%f", this->enable_, this->time_);
    this->parent_->set_walk_in(this->enable_, this->time_);
}

void RestartOvervoltageButton::press_action() {
    ESP_LOGD(TAG, "-> restart_overvoltage button pressed: enable=%d", this->enable_);
    this->parent_->set_restart_overvoltage(this->enable_);
}

}  // namespace emerson_r48
}  // namespace esphome
