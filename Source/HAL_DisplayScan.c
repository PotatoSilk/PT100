#include "HAL_DisplayScan.h"

/**
 * SEGTableNum为共阴数字管0-9的段码
 * SEGTableABC前4为字母ABCPFE的段码，后5位数带小数点
 * SEGTableNumDot为带小数点的数字0-9段码
 * SEGTableChar分别为：(空), -, r, o
 * SEGW为共阴数字管位选码
 */
uchar code SEGTableNum[]    = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
uchar code SEGTableABC[]    = {0x77, 0x7c, 0x39, 0x73, 0x71, 0x79, 0xf7, 0xfc, 0xb9, 0xf3, 0xf1};
uchar code SEGTableNumDot[] = {0xbf, 0x86, 0xdb, 0xcf, 0xe6, 0xed, 0xfd, 0x87, 0xff, 0xef};
uchar code SEGTableChar[]   = {0x00, 0x40, 0x50, 0x5c};
uchar code SEGTableBit[]    = {0xef, 0xf7, 0xfb, 0xfd, 0xfe};

uchar volatile g_aucDisplayScanBuf[5] = {0, 0x0f, 0x09, 0x09, 0x39};

/**
 * [SegScanTimer1Init 定时器1初始化函数]
 */
void SegScanTimer1Init(void)		//1000微秒@11.0592MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TMOD |= 0x10;		//设置定时器模式
	TL1  = 0x65;		//设置定时初值
	TH1  = 0xFC;		//设置定时初值
	TF1  = 0;		//清除TF1标志
	EA   = 1;
	ET1  = 1;
	TR1  = 1;		//定时器1开始计时
}

/**
 * [DisplayPrepare 显示数组载入函数]
 * @param ucAddress [显示位置，取值范围0-4]
 * @param ucData    [显示字符，0-9为对应数字，10-14为ABCPF，16-19为(空), -, r, o; +0x80后显示小数点]
 */
void DisplayPrepare(uchar ucAddress, uchar ucData)
{
	if (ucData < 10)
	{
		g_aucDisplayScanBuf[ucAddress] = SEGTableNum[ucData];
	}
	else if (ucData < 10 + 6)
	{
		g_aucDisplayScanBuf[ucAddress] = SEGTableABC[ucData - 10];
	}
	else if (ucData > 15 && ucData < 20)
	{
		g_aucDisplayScanBuf[ucAddress] = SEGTableChar[ucData - 16];
	}
	else if (ucData >= 0x80 && ucData < 0x80 + 10)
	{
		g_aucDisplayScanBuf[ucAddress] = SEGTableNumDot[ucData - 0x80];
	}
	else if (ucData >= 0x80 + 10 && ucData < 0x80 + 15)
	{
		g_aucDisplayScanBuf[ucAddress] = SEGTableABC[ucData - 0x84];
	}
	else
	{
		g_aucDisplayScanBuf[ucAddress] = SEGTableChar[0];
	}
}

/**
 * [DisplayTemperature 为温度显示函数]
 * @param ucChannel [温度显示通道，可取'A', 'B', 'C']
 * @param iNumber  [待显示温度，范围在MINTEMP ~ MAXTEMP之间，该参数是显示温度的10倍]
 * 例如：输入参数('A', 2012)，数码管显示"A 201.2"
 */
void DisplayTemperature(uchar ucChannel, int iNumber)
{
	uchar ucNumberSize = 0, ucDisplayi = 0;
	uchar aucDisplayTemp[4] = {0};
	bit bOutOfRange = 0;

/**
* 这里用于求出温度数据的长度
* 并将温度各位数分离放入待扫描的数组中
* 例如：传输值Number为212，则aucDisplayTemp[]里存放：0, 2, 1, 2
* 传输值Number为-212，则aucDisplayTemp[]里存放：10, 2, 1, 2	; 10代表负号
*/
	if (iNumber >= 0 && iNumber <= MAXDISPTEMP)	//正数
	{
		while(0 != iNumber)
		{
			aucDisplayTemp[ucDisplayi++] = iNumber % 10;
			iNumber /= 10;
			ucNumberSize++;
		}
	}
	else if (iNumber < 0 && iNumber >= MINDISPTEMP)	//负数
	{
		iNumber = -iNumber;
		while(0 != iNumber)
		{
			aucDisplayTemp[ucDisplayi++] = iNumber % 10;
			iNumber /= 10;
			ucNumberSize++;
		}
		
		//这里用于确定负号显示的位置。负数至少两位，因此负号仅可能放入第3或第4位	 
		if (ucNumberSize > 2)	
		{
			aucDisplayTemp[3] = 17;
		}
		else
		{
			aucDisplayTemp[2] = 17;
		}
	}
	else
	{
		bOutOfRange = 1;
		for (ucDisplayi = 0; ucDisplayi < 4; ucDisplayi++)
		{
			aucDisplayTemp[ucDisplayi] = 0x11;
		}
	}
	
/**
 * [显示通道]
 */
	switch (ucChannel)
	{
		case '1':	//显示1
		DisplayPrepare(0, 1);
		break;

		case '2':
		DisplayPrepare(0, 2);
		break;

		case '3':
		DisplayPrepare(0, 3);
		break;

		case '4':
		DisplayPrepare(0, 4);
		break;

		case '5':	//显示1.
		DisplayPrepare(0, 1 + 0x80);
		break;

		case '6':
		DisplayPrepare(0, 2 + 0x80);
		break;

		case '7':
		DisplayPrepare(0, 3 + 0x80);
		break;

		case '8':
		DisplayPrepare(0, 4 + 0x80);
		break;
		
		case 'A':	//显示A
		DisplayPrepare(0, 10);
		break;

		case 'B':
		DisplayPrepare(0, 11);
		break;

		case 'C':
		DisplayPrepare(0, 12);
		break;

		case 'P':
		DisplayPrepare(0, 13);
		break;

		case 'F':
		DisplayPrepare(0, 14);
		break;

		case 'E':
		DisplayPrepare(0, 15);
		break;

		case 'a':	//显示A.
		DisplayPrepare(0, 10 + 0x80);
		break;

		case 'b':
		DisplayPrepare(0, 11 + 0x80);
		break;

		case 'c':
		DisplayPrepare(0, 12 + 0x80);
		break;

		case 'p':
		DisplayPrepare(0, 13 + 0x80);
		break;

		case 'f':
		DisplayPrepare(0, 14 + 0x80);
		break;
	}

/**[显示数值]
 * 倒数第二位显示小数点
 * 同时清除无意义的0
 */
	for (ucDisplayi = 0; ucDisplayi < 4; ucDisplayi++)
	{
		if (ucDisplayi == 1)	//第三位显示小数点
		{
			if (bOutOfRange)
			{
				DisplayPrepare(ucDisplayi + 1, aucDisplayTemp[ucDisplayi]);
			}
			else
			{
				DisplayPrepare(ucDisplayi + 1, aucDisplayTemp[ucDisplayi] + 0x80);
			}
		}
		else if (ucDisplayi + 1 > ucNumberSize)
		{
			if (0 == aucDisplayTemp[ucDisplayi])	//不显示数值前的0
			{
				DisplayPrepare(ucDisplayi + 1, 16);
			}
			else
			{
				DisplayPrepare(ucDisplayi + 1, aucDisplayTemp[ucDisplayi]);
			}
		}
		else
		{
			DisplayPrepare(ucDisplayi + 1, aucDisplayTemp[ucDisplayi]);
		}
	}
}

/**
 * [Timer1DisplayScan 定时1中断函数]
 * 扫描数码管 1ms @ 11059200
 */
void Timer1DisplayScan() interrupt 3
{
	static char s_ucDisplaypoint = 0;
	TL1 = 0x65;		//设置定时初值
	TH1 = 0xFC;		//设置定时初值
	
	P2 |= 0x1f;
	P0 = g_aucDisplayScanBuf[s_ucDisplaypoint];
	P2 &= SEGTableBit[s_ucDisplaypoint];

	if (++s_ucDisplaypoint == 5)
	{
		s_ucDisplaypoint = 0;
	}
}
