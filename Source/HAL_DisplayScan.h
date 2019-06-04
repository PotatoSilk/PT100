#include <STC89C5xRC.H>

#ifndef HAL_DISPLAYSCAN_INT
#define HAL_DISPLAYSCAN_INT

#define uchar unsigned char
#define uint unsigned int

#define MAXDISPTEMP 2600
#define MINDISPTEMP -200

void SegScanTimer1Init(void);
void DisplayTemperature(uchar Channel, int Number);
void DisplayPrepare(uchar ucAddress, uchar ucData);

#endif