#include <intrins.h>
#include <string.h>
#include "struct.h"
#include "HAL_DisplayScan.h"
#include "HAL_RS485.h"
#include "HAL_EEPROM.h"
#include "HAL_keyscan.h"
#include "HAL_TLC2543.h"
#include "HAL_GPIO.h"

#ifndef PROCESS_H
#define PROCESS_H

#define MAXTEMP 2600
#define MINTEMP -200

void KeyReturnProcess(STR_DISPLAYSTATE *pstrDisplayState, STR_SYSTEMSTATE *pstrSystemState);
void UpdateDisplayCycle(STR_DISPLAYSTATE *pstrDisplayState);
void UpdateDisplayData(STR_DISPLAYSTATE *pstrDisplayState);
void CalculateTemperature(uchar ucCount, STR_SYSTEMSTATE *pstrSysState);

#endif
