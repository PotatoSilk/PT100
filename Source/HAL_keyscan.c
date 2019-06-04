#include "HAL_keyscan.h"

/**
 * [KeyScan 按键扫描程序]
 * @return  [返回按下的按键值]
 */
uchar KeyScan(void)
{
	static uchar usKeyPrevCheck = 0;
	static uchar usKeyLongCheck = 0;
	static uchar usKeyLongPress = 0;
	static uchar usKeyState     = 0;
	
	uchar usKeyReturn           = 0;
	uchar usKeyPress            = 0;

	P3        |= 0x70;
	S3 = 1;
	
	if (0 == S1)
	{
		usKeyPress = 1;
	}
	else if (0 == S2)
	{
		usKeyPress = 2;
	}
	else if (0 == S3)
	{
		usKeyPress = 3;
	}
	else if (0 == S4)
	{
		usKeyPress = 4;
	}
	
	switch (usKeyState)
	{
		case 0:	//第一轮检测
			if (0 != usKeyPress)
			{
				usKeyPrevCheck = usKeyPress;
				usKeyState = 1;
			}
			break;

		case 1:	//第二轮检测
			if (usKeyPress == usKeyPrevCheck)	//确定按键被按下
			{
				usKeyState = 2;	
			}
			else	//判定为抖动，状态字恢复初始值
			{
				usKeyState = 0;
			}
			break;

		case 2:	//第三轮检测
			if (usKeyPress == usKeyPrevCheck)
			{
				usKeyLongCheck++;
				if (usKeyLongCheck > 97)	//达到长按时间100 * 10ms
				{
					usKeyLongCheck = 0;
					usKeyState = 3;
					usKeyReturn = 0x10 | usKeyPrevCheck;
					return usKeyReturn;
				}
			}
			else	//未达到长按时间，算短按
			{
				usKeyState = 0;
				usKeyLongCheck = 0;
				usKeyReturn = usKeyPrevCheck;
				return usKeyReturn;
			}
			break;

		case 3:
			if (usKeyPress == usKeyPrevCheck) 
			{
				if (0 == ++usKeyLongPress % 20)
				{
					usKeyReturn = 0x20 | usKeyPrevCheck;
					return usKeyReturn;
				}
			}
			else
			{
				usKeyState = 0;
				usKeyLongPress = 0;
			}
			break;
	}

	return 0;
}
