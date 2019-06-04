#include <STC89C5xRC.H>
#include "RTC.h"
#include "struct.h"
#include "HAL_EEPROM.h"

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

sbit RelayFan          = P2^7;
sbit RelayTrip         = P2^6;
sbit RelayOverHeating  = P2^5;
sbit RelaySensorError  = P3^2;
sbit LEDForceTurnOnFan = P3^3;

void FanTiming(STR_SYSTEMSTATE *pstrSystemState);
void CheckSysState(STR_SYSTEMSTATE *pstrSystemState);

#endif
