#ifndef __AMR_H__
#define __AMR_H__

//AMR includes
extern "C" {
	#include "typedef.h"
	#include "enc_if.h"
	#include "dec_if.h"
}
//AMR
#ifndef IF2
#define AMRWB_MAGIC_NUMBER "#!AMR-WB\n"
#endif

#define L_FRAME8k 160
const UWord8 block_size[16]= {18, 24, 33, 37, 41, 47, 51, 59, 61, 6, 6, 0, 0, 0, 1, 1};


class AmrCode {
public:
AmrCode(){
	coding_mode =8;
	mode = 8;
	allow_dtx = 0;
	st1 = E_IF_init();
	st2 = D_IF_init();
}

~AmrCode(){
	E_IF_exit(st1);
	D_IF_exit(st2);
}

void EncodeAndDecode(Word16 *signal16k_out){
	serial_size = E_IF_encode(st1, coding_mode, signal16k_out, serial, allow_dtx);
    #ifdef IF2
	mode = (Word16)(serial[0] >> 4);
    #else
	mode = (Word16)((serial[0] >> 3) & 0x0F);
    #endif
	D_IF_decode( st2, serial, signal16k_out, _good_frame);
}

private:
Word32 serial_size, frame;
Word16 signal16k_in[L_FRAME16k];                 /* Buffer for speech @ 16kHz             */
Word16 signal8k[L_FRAME8k];
Word16 coding_mode, allow_dtx, mode_file, mode;
UWord8 serial[NB_SERIAL_MAX];
//Word16 synth[L_FRAME16k];
void *st1, *st2;
};

#endif
