//
// Created by Konstantin Bersenev on 09.11.2025.
//

#pragma once

#include <cstdint>
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/components/cover/cover.h"

namespace esphome {
namespace pwm_endstop_cover {

class PwmEndstopCoverTimeTransition {
public:
  void setup(float start_level, float end_level, const uint32_t length) {
    this->start_time_ = millis();
    this->length_ = length;
    this->start_level_ = start_level;
    this->end_level_ = end_level;
  }

  float apply() const {
    const float p = this->get_progress_();

    float v;
    if (end_level_ > start_level_) {
      v = smoothed_progress_acceleration(p);
    } else {
      v = smoothed_progress_deceleration(p);
    }
    return std::lerp(start_level_, end_level_, v);
  }

  bool is_finished() {

    ESP_LOGE("progress", "progress %f", this->get_progress_());
    return this->get_progress_() >= 1.0f;
  }
protected:
  static float smoothed_progress_acceleration(float p) {
    return 1 - (1 - p) * (1 - p);
  }

  static float smoothed_progress_deceleration(float p) {
    return p * p * p;
  }

  float get_progress_() const {
    uint32_t now = millis();
    if (now < this->start_time_)
      return 0.0f;
    if (now >= this->start_time_ + this->length_) {
      return 1.0f;
    }

    return clamp((now - this->start_time_) / float(this->length_), 0.0f, 1.0f);
  }

  uint32_t start_time_ = 0;
  uint32_t length_ = 0;
  float start_level_ = 0;
  float end_level_ = 0;
};

}  // namespace pwm_endstop_cover
}
