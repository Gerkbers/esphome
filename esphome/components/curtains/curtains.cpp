
#include "esphome.h"
#include "esphome/core/defines.h"
#include "curtains.h"

using namespace std;

namespace esphome {
namespace curtains_cover {

using namespace esphome::cover;

void CurtainsCover::loop() {
  if (currentIndex == 0 && esphome::uart::UARTDevice::available() >= 6) {
    // Read first 6 byte
    dataRX[currentIndex] = esphome::uart::UARTDevice::read();

    if (dataRX[currentIndex] != 0x55) {
      ESP_LOGD("Curtains position", "No 55");
      return;
    }
    currentIndex++;

    dataRX[currentIndex] = esphome::uart::UARTDevice::read();
    if (dataRX[currentIndex] != 0xAA) {
      ESP_LOGD("Curtains position", "No AA");
      currentIndex = 0;
      return;
    }
    currentIndex++;

    dataRX[currentIndex] = esphome::uart::UARTDevice::read();
    if (dataRX[currentIndex] != 0x03) {
      ESP_LOGD("Curtains position", "No 03");
      currentIndex = 0;
      return;
    }

    currentIndex++;

    esphome::uart::UARTDevice::read_array(dataRX + currentIndex, 3);

    currentIndex += 2;
  }

  // Continue reading
  if (currentIndex == 5) {
    uint8_t dataSize = dataRX[currentIndex];

    if (esphome::uart::UARTDevice::available() >= dataSize + 1) {
      currentIndex++;

      esphome::uart::UARTDevice::read_array(dataRX + currentIndex, dataSize + 1);

      uint8_t allMessageSize = currentIndex + dataSize;

      // 6 byte впереди + динамическое количество
      uint8_t check = getChecksum(dataRX, allMessageSize);

      if (check != dataRX[allMessageSize]) {
        currentIndex = 0;
        ESP_LOGD("No all message size", "No check sum");
        return;
      }

      if (allMessageSize == 14 &&
          std::equal(std::begin(positionPrefix), std::end(positionPrefix), std::begin(dataRX))) {
        auto pos = (float) (100 - dataRX[13]) / 100.0f;

        if (pos != this->position) {
          this->position = pos;
          this->current_operation = COVER_OPERATION_IDLE;
          this->publish_state();
        }
      } else {
        ESP_LOGD("No all message size", "No command");
      }

      currentIndex = 0;
    }
  }
}

template<typename T> void CurtainsCover::sendMessage(
    MessageSubtype subtype,
    T req
) {
  MessageHeader header(
      COMMAND,
      subtype,
      sizeof(req)
  );

  auto out = serialize(header);
  auto reqv = serialize(req);
  out.insert(out.end(), reqv.begin(), reqv.end());

  uint8_t checksum = getChecksum(&out[0], out.size());
  out.push_back(checksum);

  this->write_array(out);
}

void CurtainsCover::send_final_state_command(FinalState state) {
  FinalStatePayload payload(state);

  this->sendMessage(SET_FINAL_STATE, payload);
}

void CurtainsCover::send_position(uint32_t position) {
    PositionPayload payload(position);

    this->sendMessage(SET_POSITION, payload);
}

void CurtainsCover::control(const cover::CoverCall &call) {
    if (call.get_stop()) {
      this->send_final_state_command(STOP);
      return;
    }

    if (call.get_position().has_value()) {
      const auto pos = call.get_position().value();
      if (pos > this->position) {
        this->current_operation = COVER_OPERATION_OPENING;
        this->publish_state();
      } else if (pos < this->position) {
        this->current_operation = COVER_OPERATION_CLOSING;
        this->publish_state();
      }
      if (pos == 1.0f) {
        this->send_final_state_command(OPEN);
        return;
      }
      if (pos == 0.0f) {
        this->send_final_state_command(CLOSE);
        return;
      }

      uint32_t positionPercent = static_cast<uint32_t>((1.0f - pos) * 100.0f);
      this->send_position(positionPercent);
    }
}

cover::CoverTraits CurtainsCover::get_traits() {
  auto traits = CoverTraits();
  traits.set_supports_stop(true);
  traits.set_supports_position(true);
  traits.set_is_assumed_state(false);
  return traits;
}

uint8_t CurtainsCover::getChecksum(const uint8_t *message, uint8_t size) {
  uint8_t position = size;
  int crc = 0;
  for (int i = 0; i < position; i++) {
    crc += message[i];
  }

  crc %= 256;
  return crc;
}

}  // namespace curtains_cover
}  // namespace esphome
