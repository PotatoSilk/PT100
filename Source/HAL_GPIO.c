#include "HAL_GPIO.h"

extern uchar xdata g_aucSettings[6];
extern STR_RTC g_strRTC;
extern STR_MAXTEMPERATURE xdata g_strMaxTemperature;
extern STR_MAXTEMPERATURE xdata g_strTripMaxTemp;

/**
 * [FanTimerON 风机开启循环周期]
 */
void FanTiming(STR_SYSTEMSTATE *pstrSystemState)
{
	/**
	 * g_aucSettings[5]内存储的时间单位为0.2小时（12分钟）
	 * 例g_aucSettings[5] == 123，123 = 120 + 3
	 * 即 120 / 5 = 24小时；3 * 12 = 36分钟
	 * 即定时周期为24小时36分钟（24.6小时）
	 */
	
	//达到风扇开启触发时间
	if (g_aucSettings[5] != 0 && g_strRTC.ucHour >= g_aucSettings[5] / 5 && \
		g_strRTC.ucMinute >= (g_aucSettings[5] % 5) * 12)
	{
		RTCReturnToZero();
		if (1 == RelayFan)
		{
			RelayFan = 0;
			pstrSystemState->eTimingTurnOnFan = YES;
		}
	}

	//触发开启后10分钟关闭
	if (0 == g_strRTC.ucHour && 10 == g_strRTC.ucMinute && 1 >= g_strRTC.ucSecond && \
		YES == pstrSystemState->eTimingTurnOnFan)
	{
		pstrSystemState->eTimingTurnOnFan = NO;
		if (NO == pstrSystemState->eForceTurnOnFan)
		{
			RelayFan = 1;
		}
	}
}

/**
 * [CheckSysState 继电器控制函数]
 * @param pstrSystemState [系统状态结构体]
 */
void CheckSysState(STR_SYSTEMSTATE *pstrSystemState)
{
	static xdata ENUM_SYSFLAG s_eSysReg = FanOFF;
	if (pstrSystemState->eAlarmTest)
	{
		s_eSysReg        = SysTest;
		RelayTrip        = 0;
		RelayOverHeating = 0;
		RelaySensorError = 0;
		RelayFan         = 0;
	}
	else if (pstrSystemState->eForceMute)
	{
		s_eSysReg        = ForceMute;
		RelayTrip        = 1;
		RelayOverHeating = 1;
		RelaySensorError = 1;
	}
	else
	{
		if ((pstrSystemState->ucChErrorReg & 0x07) != 0)
		{
			RelaySensorError = 0;
		}
		else
		{
			RelaySensorError = 1;
		}

		if (g_strMaxTemperature.unMAXTemperature.iX > g_aucSettings[3] * 10)
		{
			if (s_eSysReg != OHProtect)
			{
				s_eSysReg        = OHProtect;
				RelayTrip        = 0;
				RelayOverHeating = 0;
				RelayFan         = 0;
				WriteTripTemperature(IAP_ADDRESS + 0x200);
			}
		}
		else if (g_strMaxTemperature.unMAXTemperature.iX > g_aucSettings[2] * 10)
		{
			if (s_eSysReg != OverHeat)
			{
				s_eSysReg        = OverHeat;
				RelayTrip        = 1;
				RelayOverHeating = 0;
				RelayFan         = 0;
			}
			
		}
		else if (g_strMaxTemperature.unMAXTemperature.iX > g_aucSettings[1] * 10) 
		{
			if (s_eSysReg != FanON)
			{
				s_eSysReg        = FanON;
				RelayTrip        = 1;
				RelayOverHeating = 1;
				RelayFan         = 0;
			}
		}
		else if (g_strMaxTemperature.unMAXTemperature.iX < g_aucSettings[0] * 10)
		{
			if (s_eSysReg != FanOFF)
			{
				s_eSysReg        = FanOFF;
				RelayTrip        = 1;
				RelayOverHeating = 1;
				RelayFan         = 1;
			}
		}
	}
}
