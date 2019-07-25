#include "bsp.h"

/*
*********************************************************************************************************
*	函 数 名: GPRS_PrintRxData
*	功能说明: 打印STM32从GPRS收到的数据到COM1串口，主要用于跟踪调试
*	形    参: _ch : 收到的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void GPRS_PrintRxData(uint8_t _ch)
{
	#ifdef GPRS_TO_COM1_EN
		comSendChar(COM1, _ch);		/* 将接收到数据打印到调试串口1 */
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: GPRS_PowerOn
*	功能说明: 模块上电. 函数内部先判断是否已经开机，如果已开机则直接返回1
*	形    参: 无
*	返 回 值: 1 表示上电成功  0: 表示异常
*********************************************************************************************************
*/
uint8_t GPRS_PowerOn(void)
{
	uint8_t ret_value = 0;
	uint8_t i;
	
	/* 判断是否开机 */
	
	for (i = 0; i < 20; i++)										/* 开机等一会 */
	{
		if (GPRS_WaitResponse("+CTZV:", 5000))		/* 成功开机后并注册到网络，模组会自动返回注册信息 */
		{
			ret_value = 1;
			break;		
		}
	}
	
	for (i = 0; i < 5; i++)
	{
		GPRS_SendAT("AT");
		if (GPRS_WaitResponse("OK", 1000))
		{
			ret_value = 1;
			break;		
		}
	}
	for (i = 0; i < 5; i++)
	{
		GPRS_SendAT("AT+CSQ");
		if (GPRS_WaitResponse(_rATCMDCSQ, 1000))
			{
				ret_value = 1;
				break;		
			}
	}
	
	for (i = 0; i < 10; i++)
	{
		GPRS_SendAT("AT+CEREG?");
		if (GPRS_WaitResponse(_rATCMDCEREG, 5000))
				{
					ret_value = 1;
					break;		
				}else{
					ret_value = 0;
				}
	}
	for (i = 0; i < 5; i++)
	{
		GPRS_SendAT("AT+CGACT=1,1");
		if (GPRS_WaitResponse("OK", 1000))
				{
					ret_value = 1;
					break;		
				}else{
					ret_value = 0;
				}
	}
	comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */
	return ret_value;
}


/*
*********************************************************************************************************
*	函 数 名: GPRS_PowerOff
*	功能说明: 控制GPRS模块关机
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void GPRS_PowerOff(void)
{
	/* 硬件关机 */

}

/*
*********************************************************************************************************
*	函 数 名: GPRS_WaitResponse
*	功能说明: 等待GPRS返回指定的应答字符串. 比如等待 OK
*	形    参: _pAckStr : 应答的字符串， 长度不得超过255
*			 _usTimeOut : 命令执行超时，0表示一直等待. >０表示超时时间，单位1ms
*	返 回 值: 1 表示成功  0 表示失败
*********************************************************************************************************
*/
uint8_t GPRS_WaitResponse(char *_pAckStr, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint8_t ucRxBuf[256];
	uint16_t pos = 0;
	uint32_t len;
	uint8_t ret;

	len = strlen(_pAckStr);
	if (len > 255)
	{
		return 0;
	}

	/* _usTimeOut == 0 表示无限等待 */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器3，作为超时控制 */
	}
	while (1)
	{
		bsp_Idle();				/* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

		if (_usTimeOut > 0)
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				ret = 0;	/* 超时 */
				break;
			}
		}

		if (comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */

			if (ucData == '\n')
			{
				if (pos > 0)	/* 第2次收到回车换行 */
				{
					if (memcmp(ucRxBuf, _pAckStr,  len) == 0)
					{
						ret = 1;	/* 收到指定的应答数据，返回成功 */
						break;
					}
					else
					{
						pos = 0;
					}
				}
				else
				{
					pos = 0;
				}
			}
			else
			{
				if (pos < sizeof(ucRxBuf))
				{
					/* 只保存可见字符 */
					if (ucData >= ' ')
					{
						ucRxBuf[pos++] = ucData;
					}
				}
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: GPRS_ReadResponse
*	功能说明: 读取GPRS返回应答字符串。该函数根据字符间超时判断结束。 本函数需要紧跟AT命令发送函数。
*	形    参: _pBuf : 存放模块返回的完整字符串
*			  _usBufSize : 缓冲区最大长度
*			 _usTimeOut : 命令执行超时，0表示一直等待. >0 表示超时时间，单位1ms
*	返 回 值: 0 表示错误（超时）  > 0 表示应答的数据长度
*********************************************************************************************************
*/
uint16_t GPRS_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t ret;
	uint8_t status = 0;		/* 接收状态 */

	/* _usTimeOut == 0 表示无限等待 */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	}
	while (1)
	{
		bsp_Idle();					/* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

		if (status == 2)		/* 正在接收有效应答阶段，通过字符间超时判断数据接收完毕 */
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				_pBuf[pos]	 = 0;	/* 结尾加0， 便于函数调用者识别字符串结束 */
				ret = pos;				/* 成功。 返回数据长度 */
				break;
			}
		}
		else
		{
			if (_usTimeOut > 0)
			{
				if (bsp_CheckTimer(GPRS_TMR_ID))
				{
					ret = 0;	/* 超时 */
					break;
				}
			}
		}
		
		if (comGetChar(COM_GPRS, &ucData))
		{			
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */

			switch (status)
			{
				case 0:									  /* 首字符 */
					if (ucData == AT_CR)		/* 如果首字符是回车，表示 AT命令不会显 */
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
						status = 2;	 /* 认为收到模块应答结果 */
					}
					else					 /* 首字符是 A 表示 AT命令回显 */
					{
						status = 1;	 /* 这是主机发送的AT命令字符串，不保存应答数据，直到遇到 CR字符 */
					}
					break;
					
				case 1:			/* AT命令回显阶段, 不保存数据. 继续等待 */
					if (ucData == AT_CR)
					{
						status = 2;
					}
					break;
					
				case 2:			/* 开始接收模块应答结果 */
										/* 只要收到模块的应答字符，则采用字符间超时判断结束，此时命令总超时不起作用 */
					bsp_StartTimer(GPRS_TMR_ID, 500);
					if (pos < _usBufSize - 1)
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
					}
					break;
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: GPRS_SendAT
*	功能说明: 向GSM模块发送AT命令。 本函数自动在AT字符串口增加<CR>字符
*	形    参: _Str : AT命令字符串，不包括末尾的回车<CR>. 以字符0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void GPRS_SendAT(char *_Cmd)
{
	comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */	
	
	comSendBuf(COM_GPRS, (uint8_t *)_Cmd, strlen(_Cmd));
	comSendBuf(COM_GPRS, "\r", 1);
}

/*
*********************************************************************************************************
*	函 数 名: AT_SendToCmdData
*	功能说明: 发送控制指令数据到模组
*	形    参：_pCMDstr		 : AT 指令
*					: _pBACKstr    ：查询返回指令
*					: _usTimeOut	 : 溢出时间
*	返 回 值: 1 表示成功  0 表示失败
*********************************************************************************************************
*/
uint8_t AT_SendToCmdData(char *_pCMDstr,char *_pBACKstr,uint16_t _usTimeOut)
{
	uint8_t ret;
	uint8_t i;
	GPRS_SendAT(_pCMDstr);
	for (i = 0; i < 5; i++)													//系统默认连续检测 5 次
	{
		if (GPRS_WaitResponse(_pBACKstr, _usTimeOut))
				{
					ret = 1;
					break;		
				}else{
					ret = 0;
				}
	}
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: GPRS_GetHardInfo
*	功能说明: 读取模块的硬件信息. 分析 ATI 命令应答结果。
*	形    参: 存放结果的结构体指针
*	返 回 值: 1 表示成功， 0 表示失败
*********************************************************************************************************
*/
uint8_t GPRS_GetHardInfo(GPRS_INFO_T *_pInfo)
{	
	char buf[64];
	uint16_t len;
	
	GPRS_SendAT("ATI");		/* 发送 ATI 命令 */	
	len = GPRS_ReadResponse(buf, sizeof(buf), 300);	/* 超时 300ms */
	if (len == 0)
	{
		return 0;		
	}
	
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: GPRS_GetNetStatus
*	功能说明: 查询当前网络状态
*	形    参: 无
*	返 回 值: 网络状态, CREG_NO_REG, CREG_LOCAL_OK 等。
*********************************************************************************************************
*/
uint8_t GPRS_GetNetStatus(void)
{
	/*
		AT+CREG?
		+CREG: 0,1
		
		OK				
	*/	
	char buf[128];
	uint16_t len, i;
	uint8_t status = 0;
	
	GPRS_SendAT("AT+CREG?");	/* 发送 AT 命令 */
	
	len = GPRS_ReadResponse(buf, sizeof(buf), 200);	/* 超时 200ms */
	if (len == 0)
	{
		return 0;		
	}
	
	for (i = 2; i < len; i++)
	{
		if (buf[i] == ',')
		{
			i++;
			
			status = buf[i] - '0';
			break;
		}
	}
	return status;
}
