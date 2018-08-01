#include "ALIGN.h"
#include <asn_internal.h>
#include <asn_bit_data.h>
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
	printf("tail is :%x\n",tail);
	for(index = 0;index < 8 && tail & 0x80;index ++) 
			tail = tail << 1;
	tail_length += index;
	tail_length += 2;
	return 16 - tail_length;
}
int asn_merge_phases(const uint8_t* buffer, phase_t * list, uint8_t* tarbuffer) {
	phase_t* freenode;
	int tar, src;
	uint8_t cache, head_mask, tail_mask;
	tar = src = 0;
	freenode = list;
	while(list) {
		head_mask = BIT_ZERO(list->start_offset);
		tail_mask = BIT_FF(list->end_offset);
		cache = buffer[list->start] & head_mask;
		for(src = list->start + 1; src < list->end; src ++) {
			tarbuffer[tar ++] = cache << list->start_offset | ((buffer[src] & (~head_mask)) >> (8-list->start_offset));
			cache = buffer[src] & head_mask;
		}
		if(list->end_offset > list->start_offset) {
			tarbuffer[tar ++] = cache << list->start_offset | ((buffer[src] & (~head_mask)) >> (8-list->start_offset));
			tarbuffer[tar ++] = (buffer[src] & tail_mask & head_mask) >> (8-list->start_offset);
		}
		else {
			tarbuffer[tar ++] = cache << list->start_offset | ((buffer[src] & tail_mask) >> (8-list->start_offset));
		}
		list = list->next;
	}
	while(freenode) {
		list = freenode->next;
		printf("%d:%d-%d:%d\n",freenode->start,freenode->start_offset,freenode->end,freenode->end_offset);
		free(freenode);
		freenode = list;
	}
	return tar;
}

int asn_squeeze_water(const void* buffer, int length, void* tarbuffer) {
	uint8_t* tmp;
	int i,head_length;
	phase_t* phase_list;
	phase_t* phase_node;

	phase_list = calloc(1,sizeof(phase_t));
	phase_node = phase_list;
	tmp = (uint8_t*) buffer;
	phase_list->end = length - 1;
	phase_list->end_offset = 8;
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
	phase_node->end = length - 1;
	phase_node->end_offset = 8;
	/*if(phase_list != phase_node)
		phase_node->start = -1;
	*/ // need to cut the tail of uper
	return asn_merge_phases(tmp, phase_list, tarbuffer);
}
