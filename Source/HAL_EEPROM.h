#include <STC89C5xRC.H>
#include <intrins.h>
#include "struct.h"

#ifndef HAL_EEPROM_H
#define HAL_EEPROM_H

/*Declare SFR associated with the IAP */
sfr IAP_DATA    =   0xE2;           //Flash data register
sfr IAP_ADDRH   =   0xE3;           //Flash address HIGH
sfr IAP_ADDRL   =   0xE4;           //Flash address LOW
sfr IAP_CMD     =   0xE5;           //Flash command register
sfr IAP_TRIG    =   0xE6;           //Flash command trigger
sfr IAP_CONTR   =   0xE7;           //Flash control register

#define uchar unsigned char
#define uint unsigned int

#define CMD_IDLE    0               //空闲模式
#define CMD_READ    1               //IAP字节读命令
#define CMD_PROGRAM 2               //IAP字节编程命令
#define CMD_ERASE   3               //IAP扇区擦除命令

//#define ENABLE_IAP 0x80           //if SYSCLK<40MHz
#define ENABLE_IAP   0x81           //if SYSCLK<20MHz
//#define ENABLE_IAP x82            //if SYSCLK<10MHz
//#define ENABLE_IAP 0x83           //if SYSCLK<5MHz

//测试地址
#define IAP_ADDRESS 0x02000

void IapIdle(void);
uchar IapReadByte(uint addr);
void IapProgramByte(uint addr, uchar dat);
void IapEraseSector(uint addr);
void EEPROMDelay(uchar n);
void ReadEERPOMData(uchar aucDest[], uint uiSrcAddress);
void WriteEERPOMData(uchar aucSrc[], uint uiDestAddress);
void ReadTripTemperature(uint uiSrcAddress);
void WriteTripTemperature(uint uiDestAddress);

#endif
