#include "HAL_TLC2543.h"

/**
 * [GetADCResult 获取ADC转换结果]
 * @param  ucChannel [转换通道]
 * @return           [该通道的温度值]
 */
int GetADCResult(uchar ucChannel)
{
	uchar ucLoopj = 0;
	int iResult   = 0;
	
	TLC2543_EOC   = 1;
	TLC2543_CS    = 0;
	TLC2543_CLK   = 0;

	for (ucLoopj = 0; ucLoopj < 12; ucLoopj++)
	{
		if (TLC2543_SDO)
		{
			iResult |= 0x01;
		}
		TLC2543_SDI = (bit)(ucChannel & 0x80);
		TLC2543_CLK = 1;
		TLC2543_CLK = 0;
		ucChannel <<= 1;
		iResult   <<= 1;
	}
	TLC2543_CS = 1;
	iResult  >>= 1;
	while(!TLC2543_EOC);
	return iResult;
}

/**
 * [GetAvgResult 获取平均温度]
 * @param  ucChannel  [转换通道，可取ADCChA, ADCChB, ADCChC]
 * @param  ucAvgCount [检测次数]
 * @return            [返回到平均温度]
 */
int GetAvgResult(uchar ucChannel, uchar ucAvgCount)
{
	uchar ucLoopi;
	float fSum = 0;
	GetADCResult(ucChannel);
	GetADCResult(ucChannel);	//两次空函数，降低切换通道过程中的抖动
	for (ucLoopi = 0; ucLoopi < ucAvgCount; ucLoopi++)
	{
		fSum += GetADCResult(ucChannel);
	}
	fSum /= ucAvgCount;
	// fSum = 5 * fSum / 4095 / ucAvgCount * 1000;	/*电压*/
	fSum = (fSum - 486) / 0.99;		/*build 0722计算公式*/
	// fSum = 3e-8 * fSum * fSum * fSum - 9e-5 * fSum * fSum + 1.103 * fSum - 536.31;
	return (int)fSum;
}
