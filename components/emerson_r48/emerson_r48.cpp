// Add this to the end of emerson_r48.cpp to complete publish_nan_if_timeout_()

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
    this->max_output_current_sensor_->publish_state(NAN);
  }
  if (this->output_temp_sensor_ != nullptr) {
    this->output_temp_sensor_->publish_state(NAN);
  }
  if (this->input_voltage_sensor_ != nullptr) {
    this->input_voltage_sensor_->publish_state(NAN);
  }
  if (this->output_power_sensor_ != nullptr) {
    this->output_power_sensor_->publish_state(NAN);
  }
  if (this->max_current_amperes_sensor_ != nullptr) {
    this->max_current_amperes_sensor_->publish_state(NAN);
  }
  if (this->power_headroom_sensor_ != nullptr) {
    this->power_headroom_sensor_->publish_state(NAN);
  }
}

// Add missing control methods referenced in header
void EmersonR48::set_output_voltage(float voltage) {
  ESP_LOGD(TAG, "Setting output voltage to %.2fV", voltage);
  this->target_output_voltage_ = voltage;
  // Will be sent in next control message cycle
}

void EmersonR48::set_max_output_current(float current_percent) {
  ESP_LOGD(TAG, "Setting max output current to %.0f%%", current_percent);
  this->target_max_output_current_percent_ = current_percent;
  // Will be sent in next control message cycle
}

void EmersonR48::set_power_on(bool on) {
  ESP_LOGD(TAG, "Setting power %s", on ? "ON" : "OFF");
  this->target_power_on_ = on;
  // Will be sent in next control message cycle
}

void EmersonR48::set_restart_overvoltage(bool enable) {
  ESP_LOGD(TAG, "Setting restart overvoltage %s", enable ? "ENABLED" : "DISABLED");
  this->target_restart_overvoltage_ = enable;
  // Will be sent in next control message cycle
}

void EmersonR48::set_offline_values() {
  ESP_LOGD(TAG, "Manually setting offline values");
  this->set_safe_defaults_();
}

bool EmersonR48::validate_power_limit_(float voltage, float current_percent) {
  // Calculate actual current in amperes
  float current_a = (current_percent / MAX_CURRENT_PERCENT) * RATED_CURRENT_A;
  
  // Calculate power
  float power = voltage * current_a;
  
  // Check if power exceeds limit
  if (power > MAX_POWER_W) {
    ESP_LOGW(TAG, "Power limit exceeded: %.0fW > %.0fW (V=%.2f, I=%.2fA)", 
             power, MAX_POWER_W, voltage, current_a);
    
    // Adjust current to stay within power limit
    float max_safe_current_a = MAX_POWER_W / voltage;
    float max_safe_percent = (max_safe_current_a / RATED_CURRENT_A) * MAX_CURRENT_PERCENT;
    
    // Update the target to safe value
    this->target_max_output_current_percent_ = max_safe_percent;
    
    ESP_LOGW(TAG, "Adjusted current limit to %.0f%% (%.2fA)", 
             max_safe_percent, max_safe_current_a);
    
    return false;
  }
  
  return true;
}

}  // namespace emerson_r48
}  // namespace esphome
