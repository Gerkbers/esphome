#pragma once

#include "esphome/components/cover/cover.h"

namespace esphome {
namespace curtains_cover {

using namespace esphome::cover;


enum MessageType : uint16_t {
  COMMAND = 0x0006,
  POSITION_STATUS = 0x0307,
};

enum MessageSubtype : uint32_t {
  SET_FINAL_STATE = 0x00050104,
  SET_POSITION = 0x00080202,
  RECEIVED_POSITION = 0x00080302,
};

enum FinalState : uint8_t {
  OPEN = 0x00,
  STOP = 0x01,
  CLOSE = 0x02,
};

struct MessageHeader {
  uint16_t seq;
  MessageType messageType;
  MessageSubtype messageSubtype;
  uint16_t len;

  MessageHeader() = default;
  MessageHeader(MessageType messageType, MessageSubtype messageSubtype, uint16_t len) {
    this->seq = 0x55AA;
    this->messageType = messageType;
    this->messageSubtype = messageSubtype;
    this->len = len;
  }

  void byteswap() {
    this->seq = convert_big_endian(this->seq);
    this->messageType = convert_big_endian(this->messageType);
    this->messageSubtype = convert_big_endian(this->messageSubtype);
    this->len = convert_big_endian(this->len);
  }
} __attribute__((packed));

template<typename T> std::vector<uint8_t> serialize(T obj) {
  obj.byteswap();

  std::vector<uint8_t> out(sizeof(T));
  memcpy(out.data(), &obj, sizeof(T));

  return out;
}

struct FinalStatePayload {
  FinalState state;

  FinalStatePayload() = default;

  FinalStatePayload(FinalState state) {
    this->state = state;
  }

  void byteswap() {
    return;
  }
} __attribute__((packed));

struct PositionPayload {
  uint32_t position;

  PositionPayload() = default;
  PositionPayload(uint32_t position) {
    this->position = position;
  }

  void byteswap() {
    this->position = convert_big_endian(this->position);
  }
} __attribute__((packed));

}
}
