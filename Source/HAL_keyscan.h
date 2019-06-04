#include <STC89C5xRC.H>
#include "struct.h"

#ifndef HAL_KEYSCAN_INT_H
#define HAL_KEYSCAN_INT_H

#define uchar unsigned char
#define uint unsigned int

sbit S1 = P3^5;
sbit S2 = P3^6;
sbit S3 = P1^7;
sbit S4 = P3^4;

uchar KeyScan(void);

#endif
