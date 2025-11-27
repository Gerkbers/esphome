//
// Created by Konstantin Bersenev on 12.11.2025.
//

#pragma once

#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/rotary_encoder/rotary_encoder.h"
#include "PwmCoverTimeTransition.h"

namespace esphome {
namespace pwm_cover {

class PwmCoverMovementController {
 public:
  void set_pwm_output(output::FloatOutput *pwm_output) { pwm_output_ = pwm_output; }
  void set_encoder_sensor(rotary_encoder::RotaryEncoderSensor *encoder_sensor) { encoder_sensor_ = encoder_sensor; }
  void set_default_transition_length(uint32_t transition_length) { acceleration_transition_length_ = transition_length; }
  void set_max_encoder_value(float max_encoder_value) { max_encoder_value_ = max_encoder_value; }
  void set_estimated_breaking_pwm_level(float stop_opening_level) { estimated_breaking_pwm_level_ = stop_opening_level; }
  void set_min_speed_coef(float min_speed_coef) { min_speed_coef_ = min_speed_coef; }

  void start_movement();
  bool control_correct_movement(bool is_reverse_direction);
  void stop_movement();
  void calibrate_max_value();
  void init_position(float position) const;
  float get_position() const;

 protected:
  void set_pwm_level(float level);
  float calculate_deceleration_pwm_level(bool is_reverse_direction) const;
  void apply_acceleration_speed();
  void apply_speed_by_position(bool is_reverse_direction, uint32_t d_last_movement_control);

  std::unique_ptr<PwmCoverTimeTransition> acceleration_transition_{nullptr};
  uint32_t acceleration_transition_length_;

  output::FloatOutput *pwm_output_;
  float current_pwm_level_ = 0;
  float estimated_breaking_pwm_level_ = 0.5f;
  float min_speed_coef_ = 0.35f;

  rotary_encoder::RotaryEncoderSensor *encoder_sensor_{nullptr};
  float max_encoder_value_ = 0;
  float prev_encoder_value = 0;
  uint32_t prev_encoder_value_time_ = 0;

  uint32_t last_moving_time = 0;
  bool min_moving_speed_before_stop_detected = false;
};

}  // namespace pwm_cover
}  // namespace esphome
