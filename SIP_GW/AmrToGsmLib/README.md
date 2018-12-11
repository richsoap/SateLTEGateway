Usage
===
包括
---
* 四个头文件(位于include)：amr.h gsm.h fir.h coder.h
* 两个静态链接库（位于lib）： libAMR.a libgsm.a
* 一个实例程序：example

使用方法
---
* GSM和AMR互转
    - 需要include coder.h
    - 两个静态链接库均需要
    - coder.h中有GsmToAmrCoder和AmrToGsmCoder两个类，每路通话必须实例化两个类，通话之间不能共享。

GsmToAmrCoder成员函数
```
void GsmToAmrCoder::GsmToAmr(unsigned char gsm, unsigned char amr);
```

AmrToGsmCoder成员函数
```
void AmrToGsmCoder::AmrToGsm(unsigned char* amr, unsigned char* gsm);
```

用法示例
```
#include "coder.h"

unsigned char amrCoded[61];//AMR longgest coded=61byte
unsigned char gsmCoded[33];//Gms coded length=33byte(260 bits actually)

GsmToAmrCoder gsmCoder;
AmrToGsmCoder amrCoder;

//将GSM编码转换为AMR编码
gsmCoder.GsmToAmr(gsmCoded, AmrCoded);
//将AMR编码转换为GSM编码
amrCoder.AmrToGsm(AmrCoded, gsmCoded);
```


说明
---
1. GSM使用16bit量化8KHz采样的PCM作为输入；
AMR使用16bit量化16KHz采样的PCM作为输入；
两者互转时需要转换采样率，根据DSP的理论，需要加入滤波器滤除4Khz以上的频段
2. GSM实际输出为260bits，33byte=264bit。GSM编解码函数要求向右对齐，即前4bit无用
