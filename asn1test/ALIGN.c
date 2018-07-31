#include "ALIGN.h"
#include <asn_internal.h>
#include <asn_bit_data.h>
#include <list>
void asn_put_water(asn_bit_outp_t *po) {
	int i;
	uint8_t asn_water[] = {0x3F,0xFF,0xFF,0xFF,0xFF,0xFC};
	for(i = 0;i < 6;i ++)
		asn_put_few_bits(po, asn_water[i], 8);
}

//difine of water
//   | head|  body    | tail
//xxx0 0111 FF FF FF FF 1111 1110

//return length of head
//no water, return -1
//
int get_water_head(uint8_t* location) {
	int index;
	uint8_t tail;
	int tail_length;
	for(index = 0;index < 4;index ++) {
		if(location[index] != 0xFF) {
			return -1;
		}
	}
	if(location[4] == 0xFF) {
		tail = location[5];
		tail_length = 8;
	}
	else {
		tail = location[4];
		tail_length = 0;
	}
	for(index = 0;index < 8;index ++) {
		if(tail & 0x80 == 0x80) {
			tail_length ++;
			tail = tail << 1;
		}
		else {
			break;
		}
	}
	tail_length += 2;
	return 16 - tail_length;
}
typedef struct phase {
	int start; // start location
	int start_offset; // where to start
	int end; // location of the last Byte of this phase
	int end_offset; // how many bits in the last Byte
	phase_t * next;
}phase_t;

int asn_merge_phases(uint8_t* buffer, phase_t * list) {
	phase_t* freenode;
	int count, tar, src;
	uint8_t cachei, head_off, tail_off;
	count = tar = src = 0;
	while(list) {
		cache = 0;
		head_off = BIT_ZERO(list->start_offset);
		tail_off = BIT_FF(list->end_offset);
	}
}

int asn_squeeze_water(void* buffer, int length) {
	uint8_t* tmp;
	int i,head_length;
	phase_t* phase_list,phase_node;

	phase_list = calloc(1,sizeof(phase_t));
	phase_node = phase_list;
	tmp = (uint8_t*) buffer;
	tmp->end = length - 1;
	tmp->end_offset = 8;
	if(length < 6)
		return length;
	for(i =  0;i < length - 6;i ++) {
		head_length = get_water_head(tmp + i);
		if(head_length != -1) {
			phase_node->end = i - 1;
			phase_node->end_offset = 8 - head_length;
			phase_node->next = calloc(1, sizeof(phase_t));
			phase_node = phase_node->next;
			phase_node->start = (48-head_length)/8 + i;
			phase_node->start_offset = (48 - head_length) % 8;
			i = phase_node->start;
		}
	}
	if(phase_list != phase_node)
		phase_node->start = -1;
	return asn_merge_phases(tmp, phase_list);
}
