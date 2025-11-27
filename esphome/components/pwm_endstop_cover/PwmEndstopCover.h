#pragma once

#include "PwmEndstopCoverTimeTransition.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/rotary_encoder/rotary_encoder.h"
#include "esphome/components/cover/cover.h"

namespace esphome {
namespace pwm_endstop_cover {

class PwmEndstopCover : public cover::Cover, public Component {
  public:
    PwmEndstopCover();

    void loop() override;
    void control(const cover::CoverCall &call) override;
    cover::CoverTraits get_traits() override;
    void setup() override;

    void set_opening_pin(GPIOPin *opening_pin) { opening_pin_ = opening_pin; }
    void set_closing_pin(GPIOPin *closing_pin) { closing_pin_ = closing_pin; }
    void set_open_endstop(binary_sensor::BinarySensor *open_endstop) { open_endstop_ = open_endstop; }
    void set_close_endstop(binary_sensor::BinarySensor *close_endstop) { close_endstop_ = close_endstop; }
    void set_acceleration_transition_length(uint32_t transition_length) { acceleration_transition_length_ = transition_length; }
    void set_deceleration_transition_length(uint32_t transition_length) { deceleration_transition_length_ = transition_length; }
    void set_closing_time(uint32_t closing_time) { closing_time_ = closing_time; }
    void set_opening_time(uint32_t opening_time) { opening_time_ = opening_time; }
    void set_pwm_output(output::FloatOutput *pwm_output) { pwm_output_ = pwm_output; }

   protected:
    void set_pwm_level(float level);
    bool is_open_() const { return this->open_endstop_->state; }
    bool is_closed_() const { return this->close_endstop_->state; }
    void startAnimation();
    void stopAnimation();
    void stop_immediate();
    void update_position_after_stop();
    void calculate_position(float new_pos);
    bool is_target_time_reached();

    GPIOPin *opening_pin_;
    GPIOPin *closing_pin_;

    std::unique_ptr<PwmEndstopCoverTimeTransition> time_transition_{nullptr};
    uint32_t acceleration_transition_length_;
    uint32_t deceleration_transition_length_;
    uint32_t closing_time_;
    uint32_t opening_time_;
    bool stop_animation_running_ = false;

    uint32_t target_time_ = 0;

    binary_sensor::BinarySensor *open_endstop_;
    binary_sensor::BinarySensor *close_endstop_;

    output::FloatOutput *pwm_output_;
    float current_pwm_level_ = 0;
};

}
}
