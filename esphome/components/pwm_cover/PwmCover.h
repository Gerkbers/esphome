#pragma once

#include "PwmCoverMovementController.h"
#include "PwmCoverTimeTransition.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/rotary_encoder/rotary_encoder.h"
#include "esphome/components/cover/cover.h"

namespace esphome {
namespace pwm_cover {

class PwmCover : public cover::Cover, public Component {
  public:
    PwmCover(PwmCoverMovementController *controller);

    void loop() override;
    void control(const cover::CoverCall &call) override;
    cover::CoverTraits get_traits() override;
    void setup() override;

    void set_opening_pin(GPIOPin *opening_pin) { opening_pin_ = opening_pin; }
    void set_closing_pin(GPIOPin *closing_pin) { closing_pin_ = closing_pin; }
    void set_open_endstop(binary_sensor::BinarySensor *open_endstop) { open_endstop_ = open_endstop; }
    void set_close_endstop(binary_sensor::BinarySensor *close_endstop) { close_endstop_ = close_endstop; }

   protected:
    bool is_open_() const { return this->open_endstop_->state; }
    bool is_closed_() const { return this->close_endstop_->state; }
    void stop_immediate() const;
    void update_position_after_stop();

    PwmCoverMovementController *movement_controller_;

    GPIOPin *opening_pin_;
    GPIOPin *closing_pin_;

    binary_sensor::BinarySensor *open_endstop_;
    binary_sensor::BinarySensor *close_endstop_;
};

}
}
