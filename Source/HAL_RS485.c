/**
 * 虚拟寄存器地址0x00 ~ 0x08
 * 0x00 ~ 0x02 对应A、B、C的温度，由g_aunChTemperare[]提供
 * 0x03 ~ 0x06 对应4个报警温度，由aucDefaultSettings[0 - 3]提供
 * 0x07为485通讯地址，由aucDefaultSettings[4]提供
 * 0x08为风机定时周期，单位为0.1小时；例如240表示24.0小时，由aucDefaultSettings[5]提供
 * aucDefaultSettings[5]的值为RS485写入值的一半；例如，写入240，则将该寄存器的值更新为120
 */
#include "HAL_RS485.h"

sbit RS485_DIR        = P3^7;				//RS485方向选择引脚
bit g_bCommandArrived = 0;					//flag; 接收到上位机下发的命令
extern uchar g_ucBufCount;					//该变量在UART.c中
extern UN_INT xdata g_aunChTemperare[3];	//该变量在process.c中

void RS485UART1Init(void)
{
	RS485_DIR = 0;
	InitUART1();
}

/**
 * [RS485Delay10us 延时函数]
 * @param t [延时时间(t*10)us]
 */
void RS485Delay10us(uchar t)  //软件延时函数，延时时间(t*10)us
{
	do{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
	}while (--t);
}

/**
 * [RS485Read 串口数据读取函数]
 * @param  pucBuf [数据接收指针]
 * @param  ucLength [读取数据长度]
 * @return     [description]
 */
uchar RS485Read(uchar *pucBuf, uchar ucLength) //返回值为实际读取到的数据长度
{
	uchar ucReadi;
	
	if (ucLength > g_ucBufCount) //读取长度大于接收到的数据长度时，
	{
		ucLength = g_ucBufCount; //读取长度设置为实际接收到的数据长度
	}
	for (ucReadi = 0; ucReadi < ucLength; ucReadi++) //拷贝接收到的数据
	{
		*pucBuf = g_aUART1RxBuf[ucReadi];
		pucBuf++;
	}
	g_ucBufCount = 0;  //清零接收计数器	
	return ucLength;  //返回实际读取长度
}

/**
 * [RS485Write 485串口数据写入函数]
 * @param pucBuf [待发送数据指针]
 * @param ucLength     [数据长度]
 */
void RS485Write(uchar *pucBuf, uchar ucLength)
{
	RS485_DIR = 1;  //RS485设置为发送
	while (ucLength--)   //发送数据
	{
		SendData(*pucBuf++);
	}
	RS485Delay10us(5);  //等待最后的停止位完成，延时时间由波特率决定
	RS485_DIR = 0;  //RS485设置为接收
}

/**
 * [UartRxMonitor 串口接收监控函数]
 * @param ucx10ms [description]
 */
void UartRxMonitor(uchar ucx10ms)  //串口接收监控函数，由定时器0等间隔调用
{
	static uchar s_ucPreBufCount = 0;
	static uchar s_ucRxTimeOut = 0;
	if (g_ucBufCount > 0)  //接收计数器大于零时，监控总线空闲时间
	{
		if (s_ucPreBufCount != g_ucBufCount)  //接收计数器改变，即刚接收到数据时，清零空闲计时
		{
			s_ucPreBufCount = g_ucBufCount;
			s_ucRxTimeOut = 0;
		}
		else
		{
			if (s_ucRxTimeOut < 3)  //接收计数器未改变，即总线空闲时，累积空闲时间
			{
				s_ucRxTimeOut += ucx10ms;
				if (s_ucRxTimeOut >= 3)  //空闲时间超过30ms即认为一帧命令接收完毕
				{
					g_bCommandArrived = 1; //设置命令到达标志
				}
			}
		}
	}
	else
	{
		s_ucPreBufCount = 0;
	}
}

/**
 * [RS485Driver 串口驱动函数]
 * 检测接收到的命令并执行相应动作
 */
void RS485Driver(void)
{
	uchar RS485i, RS485j;
	uchar ucRegCount, ucBufLength;
	uchar ucBase, ucCount;
	uchar aucBufCopy[30];
	uint  uiCRC16;
	uchar ucCRCH, ucCRCL;
	if (g_bCommandArrived) //有命令到达时，读取处理该命令
	{
		g_bCommandArrived = 0;
		ucBufLength = RS485Read(aucBufCopy, sizeof(aucBufCopy)); //将接收到的命令读取到缓冲区中
		if (aucBufCopy[0] == g_aucSettings[4])  //核对地址以决定是否响应命令，默认地址为0x01
		{
			uiCRC16 = CalculateCRC16(aucBufCopy, ucBufLength - 2); //计算CRC校验值
			ucCRCH  = uiCRC16 >> 8;
			ucCRCL  = uiCRC16 & 0xFF;
			//判断CRC校验是否正确
			if ((aucBufCopy[ucBufLength - 2] == ucCRCH) && (aucBufCopy[ucBufLength - 1] == ucCRCL)) 
			{
				switch (aucBufCopy[1]) //按功能码执行操作
				{
					case 0x03:  //读取一个或连续的寄存器
					//寄存器地址支持0x0000～0x0008，0-2为温度16位寄存器，3-8为设置寄存器
						if ((aucBufCopy[2] == 0x00) && (aucBufCopy[3] < sizeof(g_aucSettings) + 3) \
							&& (aucBufCopy[3] + aucBufCopy[5] <= sizeof(g_aucSettings) + 3))
						{
							RS485i        = aucBufCopy[3];		//提取寄存器地址
							ucBase        = aucBufCopy[3];		//寄存器基地址
							ucRegCount    = aucBufCopy[5];		//提取待读取的寄存器数量
							ucCount       = 0;					//当前处理的数据个数
							aucBufCopy[2] = ucRegCount * 2;		//读取数据的字节数，16位
							ucBufLength   = 3;
							while (ucRegCount--)
							{
								ucCount++;
								if (ucBase + ucCount < 4)
								{
									aucBufCopy[ucBufLength++] = g_aunChTemperare[RS485i].ucS[0];
									aucBufCopy[ucBufLength++] = g_aunChTemperare[RS485i++].ucS[1];
								}
								else if (9 == ucBase + ucCount)
								{
									aucBufCopy[ucBufLength++] = ((uint)g_aucSettings[5] * 2) / 256;
									aucBufCopy[ucBufLength++] = ((uint)g_aucSettings[5] * 2) % 256;
									RS485i++;
								}
								else
								{
									aucBufCopy[ucBufLength++] = 0x00;      //寄存器高字节补0
									aucBufCopy[ucBufLength++] = g_aucSettings[RS485i++ - 3];
								}
							}
						}
						else  //寄存器地址不被支持时，返回错误码
						{
							aucBufCopy[1] = 0x83;  //功能码最高位置1
							aucBufCopy[2] = 0x02;  //错误代码02：非法数据地址
							ucBufLength   = 3;
						}
						break;
						
					case 0x06:  //写入单个寄存器

						//地址检查，写入地址高位必须为0，低位范围：0x03 - 0x08
						if ((aucBufCopy[2] == 0x00) && (aucBufCopy[3] > 2) && (aucBufCopy[3] \
							< sizeof(g_aunChTemperare) / sizeof(uint) + sizeof(g_aucSettings)))
						{
							//写入数值有效值0-255；风机定时周期除外，有效值0 - 480
							if ((0 == aucBufCopy[4]) || (8 == aucBufCopy[3]))	
							{
								//提取寄存器地址
								RS485i = aucBufCopy[3] - sizeof(g_aunChTemperare) / sizeof(uint);
								if (RS485i < 4)							//0-3 温度阈值
								{
									//5 * 数据编号 <= 写入温度阈值 <= 240 + 5 * 数据编号
									if (aucBufCopy[5] >= 5 * RS485i \
										&& aucBufCopy[5] <= 240 + 5 * RS485i)
									{
										g_aucSettings[RS485i] = aucBufCopy[5];   //保存寄存器数据
										RS485j = RS485i;
										while (RS485j > 0)
										{	//当与低一级的阈值温度间隔小于5时
											//强制将低一级的温度阈值调整至 该阈值-5
											if (g_aucSettings[RS485j] \
												< g_aucSettings[RS485j - 1] + 5)
											{
												g_aucSettings[RS485j - 1] \
												= g_aucSettings[RS485j] - 5;
											}
											RS485j--;
										}
										RS485j = RS485i;
										while (RS485j < 3)
										{
											//当与高一级的阈值温度间隔小于5时
											//强制将高一级的温度阈值调整至 该阈值+5
											if (g_aucSettings[RS485j] \
												> g_aucSettings[RS485j + 1] - 5)
											{
												g_aucSettings[RS485j + 1] \
												= g_aucSettings[RS485j] + 5;
											}
											RS485j++;
										}
										WriteEERPOMData(g_aucSettings, IAP_ADDRESS);
									}
									else
									{
										aucBufCopy[1] = 0x86;  //功能码最高位置1
										aucBufCopy[2] = 0x03;  //错误代码03：非法数据值
										ucBufLength = 3;
									}
								}
								else if (4 == RS485i)	//4 通讯地址
								{
									g_aucSettings[RS485i] = aucBufCopy[5];   //保存寄存器数据
									WriteEERPOMData(g_aucSettings, IAP_ADDRESS);
								}
								else if (5 == RS485i)	//5 风机定时周期
								{
									if ((aucBufCopy[4] * 256 + aucBufCopy[5]) <= 480)
									{
										g_aucSettings[RS485i] = \
										(aucBufCopy[4] * 256 + aucBufCopy[5]) / 2;
										WriteEERPOMData(g_aucSettings, IAP_ADDRESS);
										RTCReturnToZero();
									}
									else
									{
										aucBufCopy[1] = 0x86;  //功能码最高位置1
										aucBufCopy[2] = 0x03;  //错误代码03：非法数据值
										ucBufLength = 3;
									}
								}
								ucBufLength -= 2; //长度-2以重新计算CRC并返回原帧
							}
							else
							{
								aucBufCopy[1] = 0x86;  //功能码最高位置1
								aucBufCopy[2] = 0x03;  //错误代码03：非法数据值
								ucBufLength = 3;
							}
						}
						else  //寄存器地址不被支持时，返回错误码
						{
							aucBufCopy[1] = 0x86;  //功能码最高位置1
							aucBufCopy[2] = 0x02;  //错误代码02：非法数据地址
							ucBufLength = 3;
						}
						break;
						
					default:  //其它不支持的功能码
						aucBufCopy[1] |= 0x80;  //功能码最高位置1
						aucBufCopy[2] = 0x01;   //错误代码01：无效功能
						ucBufLength = 3;
						break;
				}
				uiCRC16 = CalculateCRC16(aucBufCopy, ucBufLength); //计算CRC校验值
				aucBufCopy[ucBufLength++] = uiCRC16 >> 8;    //CRC高字节
				aucBufCopy[ucBufLength++] = uiCRC16 & 0xFF;  //CRC低字节
				RS485Write(aucBufCopy, ucBufLength);      //发送响应帧
			}
		}
	}
}
