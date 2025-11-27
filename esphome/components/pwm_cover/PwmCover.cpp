//
// Created by Konstantin Bersenev on 08.11.2025.
//

#include "PwmCover.h"

namespace esphome {
namespace pwm_cover {

PwmCover::PwmCover(PwmCoverMovementController *controller) : movement_controller_(controller) {}

void PwmCover::loop() {
  if (this->current_operation == cover::COVER_OPERATION_OPENING && this->is_open_()) {
    this->stop_immediate();
    this->movement_controller_->calibrate_max_value();
    update_position_after_stop();
  } else if (this->current_operation == cover::COVER_OPERATION_CLOSING && this->is_closed_()) {
    this->stop_immediate();
    this->movement_controller_->init_position(0.0f);
    update_position_after_stop();
  } else if (this->current_operation != cover::COVER_OPERATION_IDLE) {
    const bool reverse = this->current_operation == cover::COVER_OPERATION_CLOSING;

    bool is_correct_movement = movement_controller_->control_correct_movement(reverse);
    if (!is_correct_movement) {
      stop_immediate();
      update_position_after_stop();
    }
  }
}

void PwmCover::control(const cover::CoverCall &call) {
  if (call.get_stop()) {
    stop_immediate();
    update_position_after_stop();
    return;
  }

  if (call.get_position().has_value()) {
    const auto pos = call.get_position().value();

    if (pos == this->position) {
      return;
    }
    if (pos == 1.0f && this->current_operation != cover::COVER_OPERATION_OPENING) {
      this->current_operation = cover::COVER_OPERATION_OPENING;
      this->publish_state();
      opening_pin_->digital_write(true);
      closing_pin_->digital_write(false);
      movement_controller_->start_movement();
      return;
    }
    if (pos == 0.0f && this->current_operation != cover::COVER_OPERATION_CLOSING) {
      this->current_operation = cover::COVER_OPERATION_CLOSING;
      this->publish_state();
      opening_pin_->digital_write(false);
      closing_pin_->digital_write(true);
      movement_controller_->start_movement();
    }
  }
}

cover::CoverTraits PwmCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_supports_stop(true);
  traits.set_is_assumed_state(false);
  traits.set_supports_position(true);
  return traits;
}

void PwmCover::setup() {
  if (this->is_open_()) {
    this->movement_controller_->init_position(1.0f);
    this->position = cover::COVER_OPEN;
    this->publish_state();
  } else if (this->is_closed_()) {
    this->movement_controller_->init_position(0.0f);
    this->position = cover::COVER_CLOSED;
    this->publish_state();
  } else {
    this->movement_controller_->init_position(0.5f);
    this->position = 0.5f;
    this->publish_state();
  }
}

void PwmCover::stop_immediate() const {
  this->opening_pin_->digital_write(false);
  this->closing_pin_->digital_write(false);
  this->movement_controller_->stop_movement();
}

void PwmCover::update_position_after_stop() {
  this->current_operation = cover::COVER_OPERATION_IDLE;
  this->position = this->movement_controller_->get_position();
  this->publish_state();
}

}  // namespace pwm_cover
}  // namespace esphome
