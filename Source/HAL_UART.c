/**
 * UART for STC89/90
 */
#include "HAL_UART.h"

// sbit RS485_DIR = P3^7;				//RS485方向选择引脚

bit g_bBusy;
uchar xdata g_aUART1RxBuf[30] = {0};
uchar g_ucBufCount = 0;

/**
 * [InitUART1 为串口1初始化函数]
 * 这里用于配置传输位数和波特率，定时器等
 */
void InitUART1(void)
{

#if (PARITYBIT == NONE_PARITY)
	SCON = 0x50;                //8位可变波特率
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
	SCON = 0xda;                //9位可变波特率,校验位初始为1
#elif (PARITYBIT == SPACE_PARITY)
	SCON = 0xd2;                //9位可变波特率,校验位初始为0
#endif

	TL2 = RCAP2L = (65536 - (FOSC / 32 / BAUD));   //设置波特率重装值
	TH2 = RCAP2H = (65536 - (FOSC / 32 / BAUD)) >> 8;
	T2CON = 0x34;           //Timer2 start run
	ES   = 1;                     //使能串口1中断
	EA   = 1;
}

/**
 * [putchar 重构函数]
 * 该函数由printf()调用，旨在解决printf()与串口中断冲突的问题
 */
/*char putchar(char c)
{
	SendData(c);
	return c;
}*/

/**
 * [SendData 用于发送单个字符，由putchar()调用]
 * @param dat [该参数由printf()传递]
 */
void SendData(uchar dat)
{
	while (g_bBusy);               //等待前面的数据发送完成
	ACC = dat;                  //获取校验位P (PSW.0)
	if (P)                      //根据P来设置校验位
	{
#if (PARITYBIT == ODD_PARITY)
		TB8 = 0;                //设置校验位为0`
#elif (PARITYBIT == EVEN_PARITY)
		TB8 = 1;                //设置校验位为1
#endif
	}
	else
	{
#if (PARITYBIT == ODD_PARITY)
		TB8 = 1;                //设置校验位为1
#elif (PARITYBIT == EVEN_PARITY)
		TB8 = 0;                //设置校验位为0
#endif
	}
	g_bBusy = 1;
	SBUF = ACC;                 //写数据到UART数据寄存器
}

/**
 * [Uart1 为串口中断函数]
 */
void UART1INT() interrupt 4 using 1
{
	if (RI)
	{
		RI = 0;				//清除RI位
		if(g_ucBufCount < sizeof(g_aUART1RxBuf))
		{
			g_aUART1RxBuf[g_ucBufCount++] = SBUF;		//g_UART1RxBuff接受串口数据
		}
	}
	if (TI)
	{
		TI = 0;				//清除TI位
		g_bBusy = 0;		//清忙标志
	}
}
