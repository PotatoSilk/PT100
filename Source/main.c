/*
* Elvis. Liu
* C51
* V1.0 Build 1121 
* 2017.11
*/

#include <intrins.h>
// #include <stdio.h> 串口测试使用
#include "struct.h"
#include "process.h"
#include "RTC.h"
#include "HAL_DisplayScan.h"
#include "HAL_RS485.h"
#include "HAL_EEPROM.h"
#include "HAL_keyscan.h"
#include "HAL_TLC2543.h"
#include "HAL_GPIO.h"

static volatile unsigned int g_uiSysTick10ms = 0;
uchar xdata g_aucSettings[6] = {80, 100, 130, 150, 1, 120};
STR_KEYSTATE g_strKeyState = {KeyNone, 1}; 		//eKeyReturnValue, ucKeyPressCount
extern STR_MAXTEMPERATURE xdata g_strTripMaxTemp;

/**
 * [SysTickTimer0Init 系统定时器初始化函数]
 */
void SysTickTimer0Init(void)		//10000微秒@11.0592MHz
{
	AUXR &= 0x7F;	//定时器时钟12T模式
	TMOD &= 0xF0;	//设置定时器模式
	TMOD |= 0x01;	//设置定时器模式
	TL0  = 0x00;	//设置定时初值
	TH0  = 0xDC;	//设置定时初值
	TF0  = 0;		//清除TF0标志
	ET0  = 1;
	EA   = 1;
	TR0  = 1;		//定时器0开始计时
}

int main(void)
{
	uint uiTiggerTick = 0, uiUARTTick = 0;
	STR_DISPLAYSTATE xdata strDisplayState;
	STR_SYSTEMSTATE xdata strSystemState;
	strDisplayState.ucDisplayChannel      = 'A';
	strDisplayState.ucDisplayChannelCount = 0;
	strDisplayState.iDisplayNum           = 0;
	strDisplayState.eDisplayMode          = SCAN_ROUND_MODE;
	strDisplayState.ucRefreshDisplayFlag  = 0;
	strSystemState.eForceTurnOnFan        = NO;
	strSystemState.eTimingTurnOnFan       = NO;
	strSystemState.eForceMute             = NO;
	strSystemState.eAlarmTest             = NO;
	strSystemState.ucChErrorReg           = 0;
	
	SysTickTimer0Init();
	SegScanTimer1Init();
	RS485UART1Init();
	ReadEERPOMData(g_aucSettings, IAP_ADDRESS);
	ReadTripTemperature(IAP_ADDRESS + 0x200);
	while(1)
	{
		KeyReturnProcess(&strDisplayState, &strSystemState);
		RS485Driver();
		if (g_uiSysTick10ms - uiTiggerTick > 35)
		{
			CalculateTemperature(20, &strSystemState);
			uiTiggerTick = g_uiSysTick10ms;
			if (g_uiSysTick10ms - uiUARTTick > 70)
			{
				uiUARTTick = g_uiSysTick10ms;
				UpdateDisplayCycle(&strDisplayState);
				// printf("\r\nSysTick is: %u\r\n", g_uiSysTick10ms);
				// RTC测试函数
				/*printf("The Time is: %u:%u:%u\r\n", (uint)g_strRTC.ucHour,
				 (uint)g_strRTC.ucMinute, (uint)g_strRTC.ucSecond);*/
			}
			UpdateDisplayData(&strDisplayState);
		}
		FanTiming(&strSystemState);
		CheckSysState(&strSystemState);
		if (strDisplayState.ucRefreshDisplayFlag)
		{
			strDisplayState.ucRefreshDisplayFlag = 0;
			DisplayTemperature(strDisplayState.ucDisplayChannel, strDisplayState.iDisplayNum);
		}
	}
}

/**
 * [Timer0SysTickISR 系统时间中断函数]
 * 每10ms触发一次
 * 用于更新系统时间，执行按键扫描
 */
void Timer0SysTickISR() interrupt 1
{
	uchar ucKeyReturnValue = 0;
	TL0  = 0x00;	//重新装载
	TH0  = 0xDC;
	g_uiSysTick10ms++;
	ucKeyReturnValue = KeyScan();
	if (ucKeyReturnValue)
	{
		g_strKeyState.eKeyReturnVal = ucKeyReturnValue;
	}
	UartRxMonitor(2);  //串口接收监控
	UpdateRTC();
}
