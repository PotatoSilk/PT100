#include "process.h"

extern STR_KEYSTATE g_strKeyState;
UN_INT xdata g_aunChTemperare[3] = {0};
STR_MAXTEMPERATURE xdata g_strMaxTemperature = {'E', 0};
STR_MAXTEMPERATURE xdata g_strTripMaxTemp = {'E', 0};				//跳闸最高温度
uchar code aucDefaultSettings[6] = {80, 100, 130, 150, 1, 120};		//系统默认设置

/**
 * [CalculateTemperature 温度计算函数]
 * 将ADC计算结果放入数组g_aunChTemperare[]中，并将最高温度放入g_strMaxTemperature中
 * @param ucCount     [测量次数取平均值]
 * @param pstrSysState [系统状态结构体]
 */
void CalculateTemperature(uchar ucCount, STR_SYSTEMSTATE *pstrSysState)
{
	g_aunChTemperare[0].iX = GetAvgResult(ADCChA, ucCount);
	g_aunChTemperare[1].iX = GetAvgResult(ADCChB, ucCount);
	g_aunChTemperare[2].iX = GetAvgResult(ADCChC, ucCount);

	/*分别检测三通道是否在量程范围内。
	若超出量程，则将ucChErrorReg中相应bit置1，同时检测结果赋予最小值不予显示*/
	if (g_aunChTemperare[0].iX > MAXTEMP || g_aunChTemperare[0].iX < MINTEMP)
	{
		pstrSysState->ucChErrorReg |= 0x01;
		g_aunChTemperare[0].iX = -500;
	}
	else
	{
		pstrSysState->ucChErrorReg &= ~0x01;
	}

	if (g_aunChTemperare[1].iX > MAXTEMP || g_aunChTemperare[1].iX < MINTEMP)
	{
		pstrSysState->ucChErrorReg |= 0x02;
		g_aunChTemperare[1].iX = -500;
	}
	else
	{
		pstrSysState->ucChErrorReg &= ~0x02;
	}

	if (g_aunChTemperare[2].iX > MAXTEMP || g_aunChTemperare[2].iX < MINTEMP)
	{
		pstrSysState->ucChErrorReg |= 0x04;
		g_aunChTemperare[2].iX = -500;
	}
	else
	{
		pstrSysState->ucChErrorReg &= ~0x04;
	}

	/*比较并得出ABC通道中*/
	if((pstrSysState->ucChErrorReg & 0x01) == 0 && g_aunChTemperare[0].iX >= g_aunChTemperare[1].iX)
	{
		g_strMaxTemperature.ucChannel = 'a';
		g_strMaxTemperature.unMAXTemperature.iX = g_aunChTemperare[0].iX;
	}

	if ((pstrSysState->ucChErrorReg & 0x02) == 0 && g_aunChTemperare[1].iX > g_aunChTemperare[0].iX)
	{
		g_strMaxTemperature.ucChannel = 'b';
		g_strMaxTemperature.unMAXTemperature.iX = g_aunChTemperare[1].iX;
	}

	if ((pstrSysState->ucChErrorReg & 0x04) == 0 && g_aunChTemperare[2].iX > g_aunChTemperare[0].iX
		&& g_aunChTemperare[2].iX > g_aunChTemperare[1].iX)
	{
		g_strMaxTemperature.ucChannel = 'c';
		g_strMaxTemperature.unMAXTemperature.iX = g_aunChTemperare[2].iX;
	}

	if (pstrSysState->ucChErrorReg == 0x07)
	{
		g_strMaxTemperature.ucChannel = 'E';
		g_strMaxTemperature.unMAXTemperature.iX = -500;	//-500超出数码管显示范围，最终显示效果E ----
	}
}

/**
 * [UpdateDisplayCycle 温度循环扫描程序]
 * pstrDisplayState->ucDisplayChannelCount用于每隔4s切换显示通道
 */
void UpdateDisplayCycle(STR_DISPLAYSTATE *pstrDisplayState)
{
	if (SCAN_ROUND_MODE == pstrDisplayState->eDisplayMode)	//正常扫描模式
	{
		if (11 < ++pstrDisplayState->ucDisplayChannelCount)
		{
			pstrDisplayState->ucDisplayChannelCount = 0;
		}
	}
}

/**
 * [UpdateDisplayData 更新显示数据]
 * 由按键处理函数调用
 */
void UpdateDisplayData(STR_DISPLAYSTATE *pstrDisplayState)
{
	switch (pstrDisplayState->eDisplayMode)
	{
		case SCAN_ROUND_MODE:
		if (pstrDisplayState->ucDisplayChannelCount < 4)
		{
			pstrDisplayState->ucDisplayChannel = 'A';
			pstrDisplayState->iDisplayNum = g_aunChTemperare[0].iX;
		}
		else if (pstrDisplayState->ucDisplayChannelCount < 8)
		{
			pstrDisplayState->ucDisplayChannel = 'B';
			pstrDisplayState->iDisplayNum = g_aunChTemperare[1].iX;
		}
		else
		{
			pstrDisplayState->ucDisplayChannel = 'C';
			pstrDisplayState->iDisplayNum = g_aunChTemperare[2].iX;
		}
		break;

		case QUERY_MODE:
		switch (g_strKeyState.ucKeyQueryCount)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			pstrDisplayState->ucDisplayChannel = '0' + g_strKeyState.ucKeyQueryCount;
			pstrDisplayState->iDisplayNum = g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] * 10;
			break;

			case 5:
			pstrDisplayState->ucDisplayChannel = 'P';
			pstrDisplayState->iDisplayNum = g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] * 10;
			break;

			case 6:
			pstrDisplayState->ucDisplayChannel = 'F';
			pstrDisplayState->iDisplayNum = g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] * 2;
			break;

			case 7:
			pstrDisplayState->ucDisplayChannel = g_strTripMaxTemp.ucChannel;
			pstrDisplayState->iDisplayNum = g_strTripMaxTemp.unMAXTemperature.iX;
			default:break;
		}
		break;

		case SETTING_MODE:
		switch (g_strKeyState.ucKeyQueryCount)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			pstrDisplayState->ucDisplayChannel = '4' + g_strKeyState.ucKeyQueryCount;
			pstrDisplayState->iDisplayNum = g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] * 10;
			break;

			case 5:
			pstrDisplayState->ucDisplayChannel = 'p';
			pstrDisplayState->iDisplayNum = g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] * 10;
			break;

			case 6:
			pstrDisplayState->ucDisplayChannel = 'f';
			pstrDisplayState->iDisplayNum = g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] * 2;
			break;

			default: break;
		}
		break;

		case SCAN_MAX_MODE:
		pstrDisplayState->ucDisplayChannel = g_strMaxTemperature.ucChannel;
		pstrDisplayState->iDisplayNum = g_strMaxTemperature.unMAXTemperature.iX;
		break;

		default: break;
	}
	pstrDisplayState->ucRefreshDisplayFlag = 1;
}

/**
 * [KeyReturnProcess 按键处理函数]
 * .ucKeyQueryCount == 1：关风机温度;	.ucKeyQueryCount == 2：开风机温度
 * .ucKeyQueryCount == 3：高温报警温度;	.ucKeyQueryCount == 4：跳闸温度
 * .ucKeyQueryCount == 5：485通讯地址	.ucKeyQueryCount == 6：风机定时周期
 */
void KeyReturnProcess(STR_DISPLAYSTATE *pstrDisplayState, STR_SYSTEMSTATE *pstrSystemState)
{
	if (g_strKeyState.eKeyReturnVal)
	{
		switch (g_strKeyState.eKeyReturnVal)
		{
			case KeyQuerySet:	//KeyQuery
			switch (pstrDisplayState->eDisplayMode)
			{
				case SCAN_ROUND_MODE:
				case SCAN_MAX_MODE:
				pstrDisplayState->eDisplayMode = QUERY_MODE;
				g_strKeyState.ucKeyQueryCount = 1;
				break;
				
				case SETTING_MODE:
				if (g_strKeyState.ucKeyQueryCount < sizeof(g_aucSettings) / sizeof(*g_aucSettings))
				{
					//逐项设置
					g_strKeyState.ucKeyQueryCount++;
				}	
				else	//写入设置到EEPROM中
				{
					pstrDisplayState->eDisplayMode = SCAN_ROUND_MODE;
					g_strKeyState.ucKeyQueryCount = 1;
					WriteEERPOMData(g_aucSettings, IAP_ADDRESS);
					RTCReturnToZero();
				}
				break;

				case QUERY_MODE:
				if (g_strKeyState.ucKeyQueryCount < sizeof(g_aucSettings) / sizeof(*g_aucSettings) \
					+ sizeof(g_strTripMaxTemp) / sizeof(g_strTripMaxTemp))
				{
					//逐项设置
					g_strKeyState.ucKeyQueryCount++;
				}	
				else
				{
					pstrDisplayState->eDisplayMode = SCAN_ROUND_MODE;
					g_strKeyState.ucKeyQueryCount = 1;
				}
				break;

				default: break;
			}
			UpdateDisplayData(pstrDisplayState);
			break;

			case KeyQuerySetLP1:	//keySet
				switch (pstrDisplayState->eDisplayMode)
				{
					case SCAN_ROUND_MODE:
					case SCAN_MAX_MODE:
						pstrDisplayState->eDisplayMode = SETTING_MODE;
						g_strKeyState.ucKeyQueryCount = 1;
						break;

					case QUERY_MODE:
						pstrDisplayState->eDisplayMode = SCAN_ROUND_MODE;
						g_strKeyState.ucKeyQueryCount = 1;
						break;

					case SETTING_MODE:
						//BuzzerLaunch();
						g_strKeyState.ucKeyQueryCount = 1;
						pstrDisplayState->eDisplayMode = SCAN_ROUND_MODE;
						ReadEERPOMData(g_aucSettings, IAP_ADDRESS);
						break;
				}
				UpdateDisplayData(pstrDisplayState);
				break;

			case KeyDown:
				switch(pstrDisplayState->eDisplayMode)
				{
					case SCAN_ROUND_MODE:
					case SCAN_MAX_MODE:
					case QUERY_MODE:
						if (YES == pstrSystemState->eForceMute)
						{
							pstrSystemState->eForceMute = NO;
						}
						else
						{
							pstrSystemState->eForceMute = YES;
						}
						break;

					default:
						break;
				}
				break;

			case KeyDownLP1:	//keyDown Hold First Trigger
				switch(pstrDisplayState->eDisplayMode)
				{
					case SETTING_MODE:		//reset default settings
						pstrDisplayState->eDisplayMode = SCAN_ROUND_MODE;
						memcpy(g_aucSettings, aucDefaultSettings, sizeof(aucDefaultSettings));
						WriteEERPOMData(g_aucSettings, IAP_ADDRESS);
						RTCReturnToZero();
						UpdateDisplayData(pstrDisplayState);
						break;

					default:
						break;
				}
				break;

			case KeyMaxFan:
				switch (pstrDisplayState->eDisplayMode)
				{
					case SCAN_ROUND_MODE:
						pstrDisplayState->eDisplayMode = SCAN_MAX_MODE;
						UpdateDisplayData(pstrDisplayState);
						break;

					case SCAN_MAX_MODE:
						pstrDisplayState->eDisplayMode = SCAN_ROUND_MODE;
						UpdateDisplayData(pstrDisplayState);
						break;

					case SETTING_MODE:
						switch (g_strKeyState.ucKeyQueryCount)
						{
							case 1:		//关风机温度
							case 6:		//定时风机
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] > 0)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]--;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							case 2:
							case 3:
							case 4:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] \
									> g_aucSettings[g_strKeyState.ucKeyQueryCount - 2] + 5)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]--;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							case 5:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] > 1)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]--;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							default: break;
						}
						UpdateDisplayData(pstrDisplayState);
						break;

					default:
						break;
				}
				break;

			case KeyMaxFanLP1:
				switch (pstrDisplayState->eDisplayMode)
				{
					case SCAN_ROUND_MODE:
					case SCAN_MAX_MODE:
					case QUERY_MODE:
						if(NO == pstrSystemState->eForceTurnOnFan)
						{
							pstrSystemState->eForceTurnOnFan = YES;
							LEDForceTurnOnFan = 0;
							RelayFan = 0;
						}
						else
						{
							pstrSystemState->eForceTurnOnFan = NO;
							LEDForceTurnOnFan = 1;
							RelayFan = 1;
						}
						//BuzzerLaunch();
						break;

					default:
						break;
				}
				//no break

			case KeyMaxFanLP2:
				if (pstrDisplayState->eDisplayMode == SETTING_MODE)
				{
					switch (g_strKeyState.ucKeyQueryCount)
					{
						case 1:
							if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] > 1)
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] -= 2;
							}
							else if (1 == g_aucSettings[g_strKeyState.ucKeyQueryCount - 1])
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]--;
							}
							else
							{
								//BuzzerLaunch();
							}
							break;

						case 2:
						case 3:
						case 4:
							if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] \
								> g_aucSettings[g_strKeyState.ucKeyQueryCount - 2] + 6)
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] -= 2;
							}
							else if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] \
								== g_aucSettings[g_strKeyState.ucKeyQueryCount - 2] + 6)
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]--;
							}
							else
							{
								//BuzzerLaunch();
							}
							break;

						case 5:
							if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] > 5)
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] -= 5;
							}
							else if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] > 1)
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]--;
							}
							else
							{
								//BuzzerLaunch();
							}
							break;

						case 6:
							if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] > 4)
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] -= 5;
							}
							else if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] > 0)
							{
								g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]--;
							}
							else
							{
								//BuzzerLaunch();
							}
							break;

						default:
							break;
					}
					UpdateDisplayData(pstrDisplayState);
				}
				break;

			case KeyUp:
				switch (pstrDisplayState->eDisplayMode)
				{
					case SETTING_MODE:
						switch (g_strKeyState.ucKeyQueryCount)
						{
							case 1:
							case 2:
							case 3:
							case 4:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] \
									< 240 + 5 * (g_strKeyState.ucKeyQueryCount - 1))
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]++;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							case 5:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] < 255)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]++;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							case 6:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] < 240)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]++;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							default:
								break;

						}
						UpdateDisplayData(pstrDisplayState);
						break;

					default:
						break;
				}
				break;

			case KeyUpLP1:	//keyUp Hold First Trigger
				if (pstrDisplayState->eDisplayMode == QUERY_MODE)
				{
					if (NO == pstrSystemState->eAlarmTest)
					{
						pstrSystemState->eAlarmTest = YES;
					}
					else
					{
						pstrSystemState->eAlarmTest = NO;
					}
				}
				//no break
			
			case KeyUpLP2:	//keyUp Hold
				switch (pstrDisplayState->eDisplayMode)
				{
					case SETTING_MODE:
						switch (g_strKeyState.ucKeyQueryCount)
						{
							case 1:
							case 2:
							case 3:
							case 4:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] < \
									239 + 5 * (g_strKeyState.ucKeyQueryCount - 1))
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] += 2;
								}
								else if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] \
									== 239 + 5 * (g_strKeyState.ucKeyQueryCount - 1))
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]++;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							case 5:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] < 251)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] += 5;
								}
								else if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] < 255)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]++;
								}
								else
								{
									//BuzzerLaunch();
								}
								break;

							case 6:
								if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] < 236)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] += 5;
								}
								else if (g_aucSettings[g_strKeyState.ucKeyQueryCount - 1] < 240)
								{
									g_aucSettings[g_strKeyState.ucKeyQueryCount - 1]++;
								}
								else
								{
									//BuzzerLaunch()
								}
								break;

							default:
								break;
						}
						UpdateDisplayData(pstrDisplayState);
						break;

					default:
						break;
				}
				break;

			default:
				break;
		}
		g_strKeyState.eKeyReturnVal = KeyNone;
		pstrDisplayState->ucRefreshDisplayFlag = 1;	//请求刷新数码管寄存器
	}
}