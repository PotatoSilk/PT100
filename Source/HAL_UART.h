#include <STC89C5xRC.H>
#include <intrins.h>

#ifndef HAL_UART_H
#define HAL_UART_H

#define uchar unsigned char
#define uint unsigned int

#define FOSC 11059200L          //系统频率
#define BAUD 9600             //串口波特率

#define NONE_PARITY     0       //无校验
#define ODD_PARITY      1       //奇校验
#define EVEN_PARITY     2       //偶校验
#define MARK_PARITY     3       //标记校验
#define SPACE_PARITY    4       //空白校验
#define PARITYBIT NONE_PARITY   //定义校验位

extern bit bBusy;
extern uchar g_UART1RxBuff;
extern uchar xdata g_aUART1RxBuf[30];

void SendData(uchar dat);
void InitUART1(void);
void IOInit(void);

#endif
