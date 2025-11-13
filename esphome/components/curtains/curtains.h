#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/components/cover/cover.h"

#include "curtains_protocol.h"

namespace esphome {
namespace curtains_cover {

using namespace esphome::cover;

class CurtainsCover : public cover::Cover, public uart::UARTDevice, public Component {
 public:
  void loop() override;
  cover::CoverTraits get_traits() override;
  void control(const cover::CoverCall &call) override;
 private:
  uint8_t dataRX[15] = {};

  uint8_t positionPrefix[13] = {0x55, 0xAA, 0x03, 0x07, 0x00, 0x08, 0x03, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00};

  uint8_t currentIndex = 0;
  template<typename T> void sendMessage(MessageSubtype subtype, T req);

  static uint8_t getChecksum(const uint8_t * message, uint8_t size);

  void send_final_state_command(FinalState state);

  void send_position(uint32_t position);
};

}
}
