#include <iostream>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <stdlib.h>

#include "coder.h"
const char configFilePath[]="file.config";

int main(){
	char inputFilePath[50],outputFilePath[50];
	std::ifstream configFile;
	configFile.open(configFilePath);
	if(!configFile) {
		printf("No Configure file!\n");
		exit(0);
	}
	configFile>>inputFilePath;
	configFile>>outputFilePath;
	configFile.close();
	printf("Input File:%s\n",inputFilePath);
	printf("Output File:%s\n",outputFilePath);

	std::ifstream inputFile;
	std::ofstream outputFile;
	inputFile.open(inputFilePath);
	outputFile.open(outputFilePath);
	short input[320];
	unsigned char middleIn[320],middleOut[320];
	short output[320];

	GsmDeCoder gsmCoder;
	AmrEnCoder amrCoder;
	AmrToGsmCoder coder;

	while(inputFile.read((char*)input,sizeof(short)*320)) {
		amrCoder.EnCode(input, middleIn);//PCM to AMR

		coder.AmrToGsm(middleIn, middleOut);//AMR TO GSM

		gsmCoder.DeCode(middleOut, output);//GSM TO PCM
		//useFir(speech, output);
		outputFile.write((const char*)output, sizeof(short) * 160);
	}
	inputFile.close();
	outputFile.close();
	return 0;
}
