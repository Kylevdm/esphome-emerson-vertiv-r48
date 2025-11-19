#pragma once
#include "../emerson_r48.h"
#include "esphome/core/component.h"
#include "esphome/components/button/button.h"

namespace esphome {
namespace emerson_r48 {

class EmersonR48Button : public button::Button, public Component {
 public:
  void set_parent(EmersonR48Component *parent) { this->parent_ = parent; };

 protected:
  EmersonR48Component *parent_;

  void press_action() override;
};

class WalkInButton : public button::Button, public Component {
 public:
  void set_parent(EmersonR48Component *parent) { this->parent_ = parent; };
  void set_enable(bool enable) { this->enable_ = enable; };
  void set_time(float time) { this->time_ = time; };

 protected:
  EmersonR48Component *parent_;
  bool enable_ = false;
  float time_ = 0.0;

  void press_action() override;
};

class RestartOvervoltageButton : public button::Button, public Component {
 public:
  void set_parent(EmersonR48Component *parent) { this->parent_ = parent; };
  void set_enable(bool enable) { this->enable_ = enable; };

 protected:
  EmersonR48Component *parent_;
  bool enable_ = false;

  void press_action() override;
};

}  // namespace emerson_r48
}  // namespace esphome
