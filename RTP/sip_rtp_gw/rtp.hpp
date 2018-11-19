#ifndef RTP_H
#define RTP_H
#include "common.h"
struct RTPHead {
  uint8_t pre_head;
  uint8_t pt;
  uint16_t seq_num;
  uint32_t timestamp;
  uint32_t ssrc;
  uint32_t csrc;
};
struct RTPPacket {
  RTPHead head;
  uint32_t len;
  uint8_t* buffer;
  uint8_t getPT() {
    return head.pt & 0x7f;
  }
  void setPT(uint8_t PT) {
    head.pt = (head.pt & 0x80) | PT;
  }
};
static uint32_t buffer2rtp(uint8_t* buffer, uint32_t len, RTPPacket* packet) {
  memcpy(packet, buffer, sizeof(RTPHead));
  packet->buffer = buffer + sizeof(RTPHead);
  packet->len = len - sizeof(RTPHead);
  return packet->len;
}
static uint32_t rtp2buffer(uint8_t buffer, RTPPacket* packet) {
  memcpy(buffer, packet, sizeof(RTPHead));
  memcpy(buffer + sizeof(RTPHead), packet->buffer, packet->len);
  return packet->len + sizeof(RTPHead);
}
#endif
