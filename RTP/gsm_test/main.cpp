#include "gsm.h"
#include <stdio.h>
#include <string.h>

using namespace std;
int main(int argc, char** argv) {
    gsm handle;
    gsm_frame buf;
    gsm_frame buf2;
    gsm_signal sample[160];
    handle = gsm_create();
    for(int i = 0;i < 160;i ++) {
        sample[i] = i;
        sample[i] = sample[i] & 0xfff8;
        printf("0x%x ", sample[i]);
    }
    printf("\n--------------------------\n");
    gsm_encode(handle, sample, buf);
    for(int i = 0;i < 33;i ++)
        printf("0x%x ", buf[i]);
    printf("\n--------------------------\n");
   if(gsm_decode(handle, buf, sample) < 0) {
        printf("Decode error\n");
        return -1;
    }
    for(int i = 0;i < 160;i ++)
        printf("0x%x ", sample[i]);
    printf("\n-------------------------\n");
    gsm_encode(handle, sample, buf2);
    for(int i = 0;i < 33;i ++)
        printf("0x%x ", buf2[i]);
    printf("\n--------------------------\n");
    for(int i = 0;i < 33;i ++)
        printf("%d ", buf[i]-buf2[i]);
   gsm_destroy(handle);
    return 0;
}
