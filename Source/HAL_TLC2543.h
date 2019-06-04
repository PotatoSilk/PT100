#include <STC89C5xRC.H>

#ifndef HAT_TLC2543_H
#define HAT_TLC2543_H

#define uchar unsigned char
#define uint unsigned int

#define ADCChA 0x10
#define ADCChB 0x20
#define ADCChC 0x30

sbit TLC2543_SDO = P1^0;
sbit TLC2543_SDI = P1^1;
sbit TLC2543_CS  = P1^2;
sbit TLC2543_CLK = P1^3;
sbit TLC2543_EOC = P1^6;

int GetADCResult(uchar ucChannel);
int GetAvgResult(uchar ucChannel, uchar ucAvgCount);

#endif
