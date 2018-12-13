#ifndef __CODER_H__
#define __CODER_H__
#include "amr.h"
#include "gsm.h"
#include "fir.h"
#include <fstream>
#include <stdio.h>

class AmrToGsmCoder {
public:
AmrToGsmCoder(){
	middle8 = new short[FrameLength8];
	middle16 = new short[FrameLength16];
}
~AmrToGsmCoder(){
	delete [] middle8;
	delete [] middle16;
}
void AmrToGsm(unsigned char* input, unsigned char* output){
	amrCoder.DeCode(input, middle16);
	changer.ChangeBpsTo8(middle16, middle8);
	gsmCoder.EnCode(middle8, output);
}

private:
AmrDeCoder amrCoder;
GsmEnCoder gsmCoder;
ChangeBps changer;
short *middle8,*middle16;
};

class GsmToAmrCoder {
public:
GsmToAmrCoder(){
	middle8 = new short[FrameLength8];
	middle16 = new short[FrameLength16];

	outputFile.open("middle.pcm");
	if(!outputFile){
		printf("Open File error\n");
	}
}
~GsmToAmrCoder(){
	delete [] middle8;
	delete [] middle16;
}
int GsmToAmr(unsigned char* input, unsigned char* output){
	gsmCoder.DeCode(input, middle8);
	changer.ChangeBpsTo16(middle8, middle16);
	outputFile.write((unsigned char*)middle16, FrameLength16*sizeof(short));
	printf("output\n");
	return amrCoder.EnCode(middle16, output);
}
private:
AmrEnCoder amrCoder;
GsmDeCoder gsmCoder;
ChangeBps changer;
short *middle8;
short *middle16;

std::ofstream outputFile;
};

#endif
