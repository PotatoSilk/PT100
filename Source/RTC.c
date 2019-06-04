#include "RTC.h"

STR_RTC g_strRTC = {0, 0, 0, 0};

void UpdateRTC(void)
{
	g_strRTC.uc10ms++;

#ifdef RTC_TEST_MODE
	if(g_strRTC.uc10ms > 9)
#else
	if(g_strRTC.uc10ms > 99)
#endif
	{
		g_strRTC.uc10ms = 0;
		g_strRTC.ucSecond++;
	}
	if(g_strRTC.ucSecond > 59)
	{
		g_strRTC.ucSecond = 0;
		g_strRTC.ucMinute++;
	}
	if(g_strRTC.ucMinute > 59)
	{
		g_strRTC.ucMinute = 0;
		g_strRTC.ucHour++;
	}
}

void RTCReturnToZero(void)
{
	g_strRTC.ucSecond = 0;
	g_strRTC.ucMinute = 0;
	g_strRTC.ucHour   = 0;
}
