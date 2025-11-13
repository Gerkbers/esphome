//
// Created by Konstantin Bersenev on 12.11.2025.
//

#include "PwmCoverMovementController.h"

namespace esphome {
namespace pwm_cover {

void PwmCoverMovementController::set_pwm_level(float level) {
  this->pwm_output_->set_level(level);
  this->current_pwm_level_ = level;
}

void PwmCoverMovementController::start_movement() {
  this->acceleration_transition_ = make_unique<PwmCoverTimeTransition>();
  this->acceleration_transition_->setup(0.0f, 1.0f, this->acceleration_transition_length_);
  const auto now = millis();

  this->prev_encoder_value = this->encoder_sensor_->state;
  this->prev_encoder_value_time_ = now;
  this->last_moving_time = now;
}

bool PwmCoverMovementController::control_correct_movement(bool is_reverse_direction) {
  const auto now = millis();
  auto d_last_movement_control = now - this->prev_encoder_value_time_;
  if (d_last_movement_control < 6)
    return true;

  if (this->acceleration_transition_ != nullptr) {
    this->apply_acceleration_speed();
  } else {
    this->apply_speed_by_position(is_reverse_direction, d_last_movement_control);
  }

  if (this->prev_encoder_value != this->encoder_sensor_->state) {
    last_moving_time = now;
  }

  this->prev_encoder_value = this->encoder_sensor_->state;
  this->prev_encoder_value_time_ = now;

  return now - last_moving_time < 1500;
}

void PwmCoverMovementController::stop_movement() {
  ESP_LOGD("Debug pwm", "Level on stop %f", this->current_pwm_level_);
  set_pwm_level(0);
  acceleration_transition_ = nullptr;
  min_moving_speed_before_stop_detected = false;
}

void PwmCoverMovementController::calibrate_max_value() { this->max_encoder_value_ = this->encoder_sensor_->state; }

void PwmCoverMovementController::init_position(float position) const {
  this->encoder_sensor_->set_value(position * this->max_encoder_value_);
}

float PwmCoverMovementController::get_position() const {
  return clamp(this->encoder_sensor_->state / this->max_encoder_value_, 0.0f, 1.0f);
}

void PwmCoverMovementController::apply_acceleration_speed() {
  const auto level = this->acceleration_transition_->apply();
  this->set_pwm_level(level);

  if (acceleration_transition_->is_finished()) {
    acceleration_transition_ = nullptr;
  }
}

void PwmCoverMovementController::apply_speed_by_position(bool is_reverse_direction, uint32_t d_last_movement_control) {
  const auto speed = std::abs(this->prev_encoder_value - this->encoder_sensor_->state) / d_last_movement_control;

  if (speed < 0.35f) {
    min_moving_speed_before_stop_detected = true;
  }
  if (!min_moving_speed_before_stop_detected) {
    this->set_pwm_level(this->calculate_deceleration_pwm_level(is_reverse_direction));
  }
}

float PwmCoverMovementController::calculate_deceleration_pwm_level(const bool is_reverse_direction) const {
  float common_progress;
  if (is_reverse_direction) {
    common_progress = 1 - this->encoder_sensor_->state / this->max_encoder_value_;
  } else {
    common_progress = this->encoder_sensor_->state / this->max_encoder_value_;
  }

  if (common_progress < 0.8f)
    return 1.0f;
  const float deceleration_progress = clamp((common_progress - 0.8f) / 0.17f, 0.0f, 1.0f);

  return std::lerp(1.0f, this->estimated_breaking_pwm_level_, deceleration_progress * deceleration_progress);
}

}  // namespace pwm_cover
}  // namespace esphome
