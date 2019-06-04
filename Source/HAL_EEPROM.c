/**
 * IAP for STC89/90
 */
#include "HAL_EEPROM.h"

extern STR_MAXTEMPERATURE xdata g_strMaxTemperature;
extern STR_MAXTEMPERATURE xdata g_strTripMaxTemp;
/*----------------------------
关闭IAP
----------------------------*/
void IapIdle(void)
{
	IAP_CONTR = 0;                  //关闭IAP功能
	IAP_CMD = 0;                    //清除命令寄存器
	IAP_TRIG = 0;                   //清除触发寄存器
	IAP_ADDRH = 0x80;               //将地址设置到非IAP区域
	IAP_ADDRL = 0;
}

/*----------------------------
从ISP/IAP/EEPROM区域读取一字节
----------------------------*/
uchar IapReadByte(uint addr)
{
	uchar dat;                       //数据缓冲区

	IAP_CONTR = ENABLE_IAP;         //使能IAP
	IAP_CMD = CMD_READ;             //设置IAP命令
	IAP_ADDRL = addr;               //设置IAP低地址
	IAP_ADDRH = addr >> 8;          //设置IAP高地址
	IAP_TRIG = 0x46;                //写触发命令(0x5a)
	IAP_TRIG = 0xb9;                //写触发命令(0xa5)
	_nop_();                        //等待ISP/IAP/EEPROM操作完成
	dat = IAP_DATA;                 //读ISP/IAP/EEPROM数据
	IapIdle();                      //关闭IAP功能

	return dat;                     //返回
}

/*----------------------------
写一字节数据到ISP/IAP/EEPROM区域
----------------------------*/
void IapProgramByte(uint addr, uchar dat)
{
	IAP_CONTR = ENABLE_IAP;         //使能IAP
	IAP_CMD = CMD_PROGRAM;          //设置IAP命令
	IAP_ADDRL = addr;               //设置IAP低地址
	IAP_ADDRH = addr >> 8;          //设置IAP高地址
	IAP_DATA = dat;                 //写ISP/IAP/EEPROM数据
	IAP_TRIG = 0x46;                //写触发命令(0x5a)
	IAP_TRIG = 0xb9;                //写触发命令(0xa5)
	_nop_();                        //等待ISP/IAP/EEPROM操作完成
	IapIdle();
}

/*----------------------------
扇区擦除
----------------------------*/
void IapEraseSector(uint addr)
{
	IAP_CONTR = ENABLE_IAP;         //使能IAP
	IAP_CMD = CMD_ERASE;            //设置IAP命令
	IAP_ADDRL = addr;               //设置IAP低地址
	IAP_ADDRH = addr >> 8;          //设置IAP高地址
	IAP_TRIG = 0x46;                //写触发命令(0x5a)
	IAP_TRIG = 0xb9;                //写触发命令(0xa5)
	_nop_();                        //等待ISP/IAP/EEPROM操作完成
	IapIdle();
}

void EEPROMDelay(uchar n)
{
	uint x;
	while (n--)
	{
		x = 0;
		while (++x);
	}
}

/**
 * [ReadEERPOMData 内部EEPROM读取函数]
 * [IAP模拟EEPROM每单元8bit，存入16/32/64位数据均需共用体]
 * @param aucDest [接收数组]
 * @param uiSrcAddress [读取地址]
 */
void ReadEERPOMData(uchar aucDest[], uint uiSrcAddress)
{
	uchar ucReadi;
	EEPROMDelay(1);
	for(ucReadi = 0; ucReadi < 6; ucReadi++)
	{
		aucDest[ucReadi] = IapReadByte(uiSrcAddress + ucReadi);
	}
}

/**
 * [SaveSettings 保存设置到EEPROM中]
 * @param aucSrc [读取数组]
 * @param uiDestAddress [保存地址，必须是0x200的整数倍]
 */
void WriteEERPOMData(uchar aucSrc[], uint uiDestAddress)
{
	uchar ucSavei;
	IapEraseSector(uiDestAddress);
	for(ucSavei = 0; ucSavei < 6; ucSavei++)
	{
		IapProgramByte(uiDestAddress + ucSavei, aucSrc[ucSavei]);
	}
}

/**
 * [WriteTripTemperature 跳闸温度保存函数]
 * @param uiDestAddress [写入的EEPROM地址，必须是0x200的整数倍]
 */
void WriteTripTemperature(uint uiDestAddress)
{
	IapEraseSector(uiDestAddress);
	IapProgramByte(uiDestAddress + 0, g_strMaxTemperature.ucChannel);
	IapProgramByte(uiDestAddress + 1, g_strMaxTemperature.unMAXTemperature.ucS[0]);
	IapProgramByte(uiDestAddress + 2, g_strMaxTemperature.unMAXTemperature.ucS[1]);
}

/**
 * [ReadTripTemperature 跳闸温度读取函数]
 * @param uiSrcAddress [保存的EEPROM地址，必须是0x200的整数倍]
 */
void ReadTripTemperature(uint uiSrcAddress)
{
	g_strTripMaxTemp.ucChannel = IapReadByte(uiSrcAddress + 0);
	g_strTripMaxTemp.unMAXTemperature.ucS[0] = IapReadByte(uiSrcAddress + 1);
	g_strTripMaxTemp.unMAXTemperature.ucS[1] = IapReadByte(uiSrcAddress + 2);
	if(g_strTripMaxTemp.ucChannel != 'a' && g_strTripMaxTemp.ucChannel != 'b' \
		&& g_strTripMaxTemp.ucChannel != 'c')
	{
		g_strTripMaxTemp.ucChannel = 'E';
	}
}
