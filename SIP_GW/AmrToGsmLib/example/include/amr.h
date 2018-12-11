#ifndef __AMR_H__
#define __AMR_H__

//AMR includes
extern "C" {
//enc_if.h
int E_IF_encode(void *st, short mode, short *speech,unsigned char *serial, short dtx);
void *E_IF_init(void);
void E_IF_exit(void *state);
//dec_if.h
#define _good_frame  0
void D_IF_decode(void *st, unsigned char *bits, short *synth, int bfi);
void * D_IF_init(void);
void D_IF_exit(void *state);
}

//AMR
#ifndef IF2
#define AMRWB_MAGIC_NUMBER "#!AMR-WB\n"
#endif

#define L_FRAME8k 160
const unsigned char block_size[16]= {18, 24, 33, 37, 41, 47, 51, 59, 61, 6, 6, 0, 0, 0, 1, 1};



//EnCoder
class AmrEnCoder {
public:
AmrEnCoder(){
	st1 = E_IF_init();
	coding_mode = 8;
	allow_dtx = 1;
}
~AmrEnCoder(){
	E_IF_exit(st1);
}
int EnCode(short* input,unsigned char *output){
	return E_IF_encode(st1, coding_mode, input, output, allow_dtx);
}
private:
void *st1;
short coding_mode,allow_dtx;
};


//DeCoder
class AmrDeCoder {
public:
AmrDeCoder(){
	st2 = D_IF_init();
}
~AmrDeCoder(){
	D_IF_exit(st2);
}
void DeCode(unsigned char *input, short* output){
	short mode;
	const int FrameLength16k=320;
	#ifdef IF2
	mode = (short)(input[0] >> 4);
    #else
	mode = (short)((input[0] >> 3) & 0x0F);
    #endif
	if(mode != 9) {
		D_IF_decode( st2, input, output, _good_frame);
	}
	else{
		for(int i = 0; i < FrameLength16k; i++) {
			output[i] = 0;
		}
	}
}
private:
void *st2;
};

#endif
