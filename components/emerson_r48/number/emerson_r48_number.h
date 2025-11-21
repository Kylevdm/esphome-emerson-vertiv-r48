#pragma once
#include "../emerson_r48.h"
#include "esphome/core/component.h"
#include "esphome/components/number/number.h"

namespace esphome {
namespace emerson_r48 {

class EmersonR48Number : public number::Number, public Component {
 public:
  void set_parent(EmersonR48Component *parent) { 
    this->parent_ = parent;
  }
  
  void set_type(int8_t type) {
    this->type_ = type;
  }

 protected:
  EmersonR48Component *parent_;
  int8_t type_;  // 0 = voltage, 1 = current

  void control(float value) override;
};

}  // namespace emerson_r48
}  // namespace esphome
