#include <STC89C5xRC.H>

#ifndef RTC_H
#define RTC_H

#define uchar unsigned char
#define uint unsigned int

typedef struct _STR_RTC
{
	uchar uc10ms;
	uchar ucHour;
	uchar ucMinute;
	uchar ucSecond;
} STR_RTC;

extern STR_RTC g_strRTC;

void UpdateRTC(void);
void RTCReturnToZero(void);

#endif
