#include <STC89C5xRC.H>
#include <intrins.h>
#include "CRC16.h"
#include "HAL_UART.h"
#include "HAL_EEPROM.h"
#include "struct.h"
#include "RTC.h"

#ifndef HAL_RS485_H
#define HAL_RS485_H

extern uchar xdata g_aucSettings[6];

void RS485Delay10us(uchar t);
uchar UartRead(uchar *pucBuf, uchar ucLength);
void RS485Write(uchar *pucBuf, uchar ucLength);
void UartRxMonitor(uchar ucx10ms);
void RS485Driver(void);
void RS485UART1Init(void);

#endif
