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
  string extHead;
  uint32_t len;
  uint8_t* buffer;
  uint8_t getPT() {
    return head.pt & 0x7f;
  }
  void setPT(uint8_t PT) {
    head.pt = (head.pt & 0x80) | PT;
  }
};
static uint32_t pharse_gsm(uint8_t* buffer, uint32_t len, RTPPacket* packet) {
  memcpy(packet, buffer, sizeof(RTPHead));
  packet->buffer = buffer + sizeof(RTPHead);
  packet->len = len - sizeof(RTPHead);
  return packet->len;
}
static uint32_t rtp2buffer(uint8_t* buffer, RTPPacket* packet) {
    uint32_t offset = 0;
    memcpy(buffer + offset, &packet->head, sizeof(RTPHead));
    offset += sizeof(RTPHead);
    memcpy(buffer + offset, packet->extHead.c_str(), packet->extHead.length());
    offset += packet->extHead.length();
    memcpy(buffer + offset, packet->buffer, packet->len);
    offset += packet->len;
    return offset;
}
#endif
