#ifndef SRSUECONTROLPACKET_HPP
#define SRSUECONTROLPACKET_HPP
#include "imsi.h"
#include <stdint.h>
#define SRSUE_ADD_PHYADDR 0x01
#define SRSUE_ADD_ADDPORT 0x02
#define SRSUE_ADD_VIRADDR 0x03

struct srsueControlPacket {
    IMSI imsi;
    uint8_t event;
    uint8_t data[4];
};

static void srsueControlPacketParse(srsueControlPacket& packet, uint8_t* buffer) {
    int offset = 0;
    memcpy(&packet.imsi, buffer + offset, sizeof(IMSI));
    offset += 15;
    packet.event = buffer[offset];
    offset ++;
    memcpy(&packet.data[0], buffer + offset, sizeof(packet.data));
}

static int srsueControlPacketToBuffer(srsueControlPacket& packet, uint8_t* buffer) {
    int offset = 0;
    memcpy(buffer, &packet.imsi, sizeof(IMSI));
    offset += 15;
    buffer[offset] = packet.event;
    offset ++;
    memcpy(buffer + offset, &packet.data[0], sizeof(packet.data));
    offset += 4;
    return offset;
}

#endif
