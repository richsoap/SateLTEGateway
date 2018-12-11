#ifndef __FIR_H__
#define __FIR_H__

#define FIRHEADLENGTH 12
#define FrameLength16 320
#define FrameLength8 160

//Filters
//const float Num[]={0.00823430253091267,0.0174128367082878,-0.0235086927063444,-0.0903908289012040,0.00336587787922763,0.304425007545655,0.486839892946463,0.304425007545655,0.00336587787922763,-0.0903908289012040,-0.0235086927063444,0.0174128367082878,0.00823430253091267};
//Equip 1
//const float Num[]={0.0491016022984981,0.0542835151332936,-0.0668058602130903,-0.0722412632863941,0.0518164913275538,0.316121560900997,0.431775533174077,0.316121560900997,0.0518164913275538,-0.0722412632863941,-0.0668058602130903,0.0542835151332936,0.0491016022984981};
//Equip 2
//const float Num[]={-6.08815992126238e-06,0.0909890579145591,3.24931958667768e-05,-0.100297228266116,-5.71261792280654e-07,0.316236666134962,0.499948332451694,0.316236666134962,-5.71261792280654e-07,-0.100297228266116,3.24931958667768e-05,0.0909890579145591,-6.08815992126238e-06};
//Equip 3
const static float Num[]={0.00823430253091267,0.0174128367082878,-0.0235086927063444,-0.0903908289012040,0.00336587787922763,0.304425007545655,0.486839892946463,0.304425007545655,0.00336587787922763,-0.0903908289012040,-0.0235086927063444,0.0174128367082878,0.00823430253091267};


class ChangeBps{
public:
	void ChangeBpsTo8(short *input,short* output){
		short temp[FrameLength16+FIRHEADLENGTH];
		short middle[FrameLength16];
		memcpy(&temp[FIRHEADLENGTH], input, FrameLength16*sizeof(short)  );
		for(int i = FIRHEADLENGTH ;i < FrameLength16+FIRHEADLENGTH; i++){
			temp[i]/=2;
		}
		memcpy(temp, head8,FIRHEADLENGTH*sizeof(short));

		UseFir(temp, middle);
		memcpy(head8, temp + FrameLength16, FIRHEADLENGTH*sizeof(short));
		for(int i = 0; i < FrameLength8; i++) {
			output[i] = middle[2*i];
		}
	}

	void ChangeBpsTo16(short *input,short* output){
		short temp[FrameLength16+12];
		short middle[FrameLength16+12];

		for(int i = 0; i < FrameLength8; i++) {
			temp[2*i+12] = input[i];
			temp[2*i+1+12] = 0;
		}
		memcpy(temp, head16,FIRHEADLENGTH*sizeof(short));

		UseFir(temp, middle);
		memcpy(head16, temp + FrameLength16, FIRHEADLENGTH*sizeof(short));
		for(int i = 0; i < FrameLength16; i++)
			output[i]= middle[i+FIRHEADLENGTH];
	}
private:
	short* head8[FIRHEADLENGTH];
	short* head16[FIRHEADLENGTH];


	void UseFir(const short* input, short* output)
	{
		float middle[FrameLength16+FIRHEADLENGTH];
		int frameNum = FrameLength16+FIRHEADLENGTH;
		memset(middle, 0, sizeof(middle));
		int k;
		int j;
		for (k = 0; k < 13; k++) {
			for (j = 12; j  < frameNum + 12; j++) {
				middle[j-12] += Num[k] * input[j - k];
			}
		}
		for (k = 0; k < frameNum; k++) {
			output[k]=static_cast<short>(middle[k]);

		}
	}
};

#endif
