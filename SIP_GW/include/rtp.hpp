#ifndef RTP_H
#define RTP_H
#include "common.h"
using namespace std;
struct RTPHead {
  uint8_t pre_head;
  uint8_t pt;
  uint16_t seq_num;
  uint32_t timestamp;
  uint32_t ssrc;
  //uint32_t csrc;
};
struct RTPPacket {
  RTPHead head;
  uint8_t extHead[4];
  uint8_t extLen;
  uint32_t len;
  uint8_t* buffer;
  uint8_t getPT() {
    return head.pt & 0x7f;
  }
  void setPT(uint8_t PT) {
    head.pt = (head.pt & 0x80) | PT;
  }
  void setMarker(uint8_t marker) {
	if(marker == 0)
		head.pt = head.pt&0x7f;
	else
		head.pt = head.pt|0x80;
  }
};
static uint32_t pharse_GSM(uint8_t* buffer, uint32_t len, RTPPacket* packet) {
  memcpy(packet, buffer, sizeof(RTPHead));
  packet->extLen = 0;
  packet->buffer = buffer + sizeof(RTPHead);
  packet->len = len - sizeof(RTPHead);
  return packet->len;
}
static uint32_t pharse_raw(uint8_t* buffer, uint32_t len, RTPPacket* packet) {
  memcpy(packet, buffer, sizeof(RTPHead));
  packet->extLen = 0;
  packet->buffer = buffer + sizeof(RTPHead);
  packet->len = len - sizeof(RTPHead);
  return packet->len;
}

static uint32_t pharse_AMR(uint8_t* buffer, uint32_t len, RTPPacket* packet) {
	memcpy(packet, buffer, sizeof(RTPHead));
	memcpy(&packet->extHead[0], buffer + sizeof(RTPHead), 1);
	packet->extLen = 1;
	packet->buffer = buffer + sizeof(RTPHead) - 1;
	packet->len = len - sizeof(RTPHead) - 1;
	/*packet->extLen = 0;
	packet->buffer = buffer + sizeof(RTPHead);
	packet->len = len - sizeof(RTPHead);*/
	return packet->len;
}

static uint32_t packet_GSM(RTPPacket* packet) {
	packet->extLen = 0;
}

static uint32_t packet_AMR(RTPPacket* packet) {
	packet->extHead[0] = 0x80;
	packet->extHead[1] = 0x44;
	//packet->extLen = 0;
	packet->extLen = 2;
}

static uint32_t rtp2buffer(uint8_t* buffer, RTPPacket* packet) {
    uint32_t offset = 0;
    memcpy(buffer + offset, &packet->head, sizeof(RTPHead));
    offset += sizeof(RTPHead);
    memcpy(buffer + offset, packet->extHead, packet->extLen);
    offset += packet->extLen;
    memcpy(buffer + offset, packet->buffer, packet->len);
    offset += packet->len;
    return offset;
}
#endif
