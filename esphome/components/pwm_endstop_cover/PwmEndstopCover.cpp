//
// Created by Konstantin Bersenev on 08.11.2025.
//

#include "PwmEndstopCover.h"

namespace esphome {
namespace pwm_endstop_cover {

PwmEndstopCover::PwmEndstopCover() {}

void PwmEndstopCover::loop() {
  if (this->current_operation == cover::COVER_OPERATION_OPENING
    && this->is_open_()
    && !stop_animation_running_
    && current_pwm_level_ >= 0.5f
  ) {
    stopAnimation();
  } else if (this->current_operation == cover::COVER_OPERATION_CLOSING
    && this->is_closed_()
    && !stop_animation_running_
    && current_pwm_level_ >= 0.9f
  ) {
    stopAnimation();
  } else if (is_target_time_reached() && !stop_animation_running_) {
    stopAnimation();
  } else if (this->current_operation != cover::COVER_OPERATION_IDLE) {
    if (time_transition_ != nullptr) {
      const auto level = this->time_transition_->apply();
      this->set_pwm_level(level);
      ESP_LOGE("progress", "level %f", level);

      if (time_transition_->is_finished()) {
        const auto level2 = this->time_transition_->apply();

        ESP_LOGE("progress", "level2 %f", level2);
        this->set_pwm_level(level2);
        time_transition_ = nullptr;
        if (current_pwm_level_ < 0.05f) {
          this->stop_immediate();
          this->update_position_after_stop();
        }
      }
    }
  }
}

void PwmEndstopCover::control(const cover::CoverCall &call) {
  if (call.get_stop()) {
    time_transition_ = nullptr;
    stop_immediate();
    update_position_after_stop();
    return;
  }

  if (call.get_position().has_value()) {
    const auto pos = call.get_position().value();

    if (pos == this->position) {
      return;
    }
    if (pos == 1.0f) {
      this->current_operation = cover::COVER_OPERATION_OPENING;
      this->publish_state();
      opening_pin_->digital_write(true);
      closing_pin_->digital_write(false);
      startAnimation();
      return;
    }
    if (pos == 0.0f) {
      this->current_operation = cover::COVER_OPERATION_CLOSING;
      this->publish_state();
      opening_pin_->digital_write(false);
      closing_pin_->digital_write(true);
      startAnimation();
      return;
    }
    calculate_position(pos);
  }
}

cover::CoverTraits PwmEndstopCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_supports_stop(true);
  traits.set_is_assumed_state(false);
  traits.set_supports_position(true);
  return traits;
}

void PwmEndstopCover::setup() {
  if (this->is_open_()) {
    this->position = cover::COVER_OPEN;
    this->publish_state();
  } else if (this->is_closed_()) {
    this->position = cover::COVER_CLOSED;
    this->publish_state();
  } else {
    this->position = 0.5f;
    this->publish_state();
  }
}

void PwmEndstopCover::set_pwm_level(float level) {
  this->pwm_output_->set_level(level);
  this->current_pwm_level_ = level;
}

void PwmEndstopCover::startAnimation() {
  this->time_transition_ = make_unique<PwmEndstopCoverTimeTransition>();
  this->time_transition_->setup(0.0f, 1.0f, this->acceleration_transition_length_);
}

void PwmEndstopCover::stopAnimation() {
  this->time_transition_ = nullptr;
  this->time_transition_ = make_unique<PwmEndstopCoverTimeTransition>();
  this->time_transition_->setup(this->current_pwm_level_, 0.0f, this->deceleration_transition_length_);
  this->stop_animation_running_ = true;
}

void PwmEndstopCover::stop_immediate() {
  this->stop_animation_running_ = false;
  this->opening_pin_->digital_write(false);
  this->closing_pin_->digital_write(false);
  this->pwm_output_->set_level(0.0f);
}

void PwmEndstopCover::update_position_after_stop() {
  this->current_operation = cover::COVER_OPERATION_IDLE;
  if (target_time_ != 0) {
    target_time_ = 0;
  } else if (this->is_open_()) {
    this->position = cover::COVER_OPEN;
  } else if (this->is_closed_()) {
    this->position = cover::COVER_CLOSED;
  } else {
    this->position = 0.5f;
  }
  this->publish_state();
}
void PwmEndstopCover::calculate_position(float new_pos) {
  if (new_pos > this->position) {
    auto d_time = (new_pos - this->position) / 1.0f * opening_time_;
    if (d_time < 2000) d_time = 2000;
    target_time_ = millis() + d_time;
    this->current_operation = cover::COVER_OPERATION_OPENING;
    this->position = new_pos;
    this->publish_state();
    opening_pin_->digital_write(true);
    closing_pin_->digital_write(false);
  } else {
    auto d_time = (this->position - new_pos) / 1.0f * closing_time_;
    if (this->is_open_()) d_time += 5000;
    if (d_time < 2000) d_time = 2000;
    target_time_ = millis() + d_time;
    this->current_operation = cover::COVER_OPERATION_CLOSING;
    this->position = new_pos;
    this->publish_state();
    opening_pin_->digital_write(false);
    closing_pin_->digital_write(true);
  }
  startAnimation();
}
bool PwmEndstopCover::is_target_time_reached() {
  if (target_time_ == 0) return false;

  return millis() >= target_time_;
}

}  // namespace pwm_endstop_cover
}  // namespace esphome
