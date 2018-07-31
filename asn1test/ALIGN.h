#ifndef ALIGN_H
#define ALIGN_H

#include <asn_internal.h>
#include <asn_bit_data.h>
#define BIT_ZERO(n) 0xFF<<n
#define BIT_FF(n) ~(0xFF<<n)
// add water
void asn_put_water(asn_bit_outp_t *po);
// squeeze water from wet buffer
// length is length of buffer in uint8_t
// return length of dry data, -1 means wrong
int asn_squeeze_water(void* buffer, int length);
#endif
