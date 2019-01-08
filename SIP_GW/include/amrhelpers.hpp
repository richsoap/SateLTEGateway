#ifndef AMRHELPERS_HPP
#define AMRHELPERS_HPP
#include <unistd.h>
#include <string>
static int AMRNoOctetAlign(uint8_t* tar, uint8_t* src,int len) {
	tar[0] = 0xf0 | (src[0] >> 4);
	tar[1] = (src[0] << 4) & 0xc0;
	int offset = 1;
	for(;offset < len;offset ++) {
		tar[offset] = tar[offset] | (src[offset] >> 2);
		tar[offset + 1] = (src[offset] << 6) & 0xc0;
	}
	if(tar[len] == 0)
		return len;
	else
		return len + 1;
}
static int AMROctetAlign(uint8_t* tar, uint8_t* src, int len) {
	tar[0] = ((src[0] << 4) | ((src[1] >> 4) & 0x0f)) & 0xfc;	
	int offset = 1;
	for(;offset < len - 1;offset ++) {
		tar[offset] = (src[offset] << 2) | (src[offset + 1] >> 6);
	}
	tar[offset] = src[offset] << 2;
	return offset + 1;
}
#endif
