#include<stdio.h>
#include<stdint.h>
#define BIT_ZERO(n) 0xFF<<n
#define BIT_FF(n) ~(0xFF<<n)
int main() {
	uint8_t test = 0x04;
	int i;
	for(i = 0;i < 8 && test & 0x80;i ++) {
		test = test << 1;
	}
	printf("%d",i);
	return 0;
}
