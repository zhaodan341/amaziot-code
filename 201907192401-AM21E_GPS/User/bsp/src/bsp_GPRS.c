#include "bsp.h"

/*
*********************************************************************************************************
*	�� �� ��: GPRS_PrintRxData
*	����˵��: ��ӡSTM32��GPRS�յ������ݵ�COM1���ڣ���Ҫ���ڸ��ٵ���
*	��    ��: _ch : �յ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GPRS_PrintRxData(uint8_t _ch)
{
	#ifdef GPRS_TO_COM1_EN
		comSendChar(COM1, _ch);		/* �����յ����ݴ�ӡ�����Դ���1 */
	#endif
}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_PowerOn
*	����˵��: ģ���ϵ�. �����ڲ����ж��Ƿ��Ѿ�����������ѿ�����ֱ�ӷ���1
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ϵ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t GPRS_PowerOn(void)
{
	uint8_t ret_value = 0;
	uint8_t i;
	
	/* �ж��Ƿ񿪻� */
	
	for (i = 0; i < 20; i++)										/* ������һ�� */
	{
		if (GPRS_WaitResponse("+CTZV:", 5000))		/* �ɹ�������ע�ᵽ���磬ģ����Զ�����ע����Ϣ */
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
	comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */
	return ret_value;
}


/*
*********************************************************************************************************
*	�� �� ��: GPRS_PowerOff
*	����˵��: ����GPRSģ��ػ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GPRS_PowerOff(void)
{
	/* Ӳ���ػ� */

}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_WaitResponse
*	����˵��: �ȴ�GPRS����ָ����Ӧ���ַ���. ����ȴ� OK
*	��    ��: _pAckStr : Ӧ����ַ����� ���Ȳ��ó���255
*			 _usTimeOut : ����ִ�г�ʱ��0��ʾһֱ�ȴ�. >����ʾ��ʱʱ�䣬��λ1ms
*	�� �� ֵ: 1 ��ʾ�ɹ�  0 ��ʾʧ��
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

	/* _usTimeOut == 0 ��ʾ���޵ȴ� */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ��3����Ϊ��ʱ���� */
	}
	while (1)
	{
		bsp_Idle();				/* CPU����ִ�еĲ����� �� bsp.c �� bsp.h �ļ� */

		if (_usTimeOut > 0)
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				ret = 0;	/* ��ʱ */
				break;
			}
		}

		if (comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */

			if (ucData == '\n')
			{
				if (pos > 0)	/* ��2���յ��س����� */
				{
					if (memcmp(ucRxBuf, _pAckStr,  len) == 0)
					{
						ret = 1;	/* �յ�ָ����Ӧ�����ݣ����سɹ� */
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
					/* ֻ����ɼ��ַ� */
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
*	�� �� ��: GPRS_ReadResponse
*	����˵��: ��ȡGPRS����Ӧ���ַ������ú��������ַ��䳬ʱ�жϽ����� ��������Ҫ����AT����ͺ�����
*	��    ��: _pBuf : ���ģ�鷵�ص������ַ���
*			  _usBufSize : ��������󳤶�
*			 _usTimeOut : ����ִ�г�ʱ��0��ʾһֱ�ȴ�. >0 ��ʾ��ʱʱ�䣬��λ1ms
*	�� �� ֵ: 0 ��ʾ���󣨳�ʱ��  > 0 ��ʾӦ������ݳ���
*********************************************************************************************************
*/
uint16_t GPRS_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t ret;
	uint8_t status = 0;		/* ����״̬ */

	/* _usTimeOut == 0 ��ʾ���޵ȴ� */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ����Ϊ��ʱ���� */
	}
	while (1)
	{
		bsp_Idle();					/* CPU����ִ�еĲ����� �� bsp.c �� bsp.h �ļ� */

		if (status == 2)		/* ���ڽ�����ЧӦ��׶Σ�ͨ���ַ��䳬ʱ�ж����ݽ������ */
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				_pBuf[pos]	 = 0;	/* ��β��0�� ���ں���������ʶ���ַ������� */
				ret = pos;				/* �ɹ��� �������ݳ��� */
				break;
			}
		}
		else
		{
			if (_usTimeOut > 0)
			{
				if (bsp_CheckTimer(GPRS_TMR_ID))
				{
					ret = 0;	/* ��ʱ */
					break;
				}
			}
		}
		
		if (comGetChar(COM_GPRS, &ucData))
		{			
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */

			switch (status)
			{
				case 0:									  /* ���ַ� */
					if (ucData == AT_CR)		/* ������ַ��ǻس�����ʾ AT������� */
					{
						_pBuf[pos++] = ucData;		/* ������յ������� */
						status = 2;	 /* ��Ϊ�յ�ģ��Ӧ���� */
					}
					else					 /* ���ַ��� A ��ʾ AT������� */
					{
						status = 1;	 /* �����������͵�AT�����ַ�����������Ӧ�����ݣ�ֱ������ CR�ַ� */
					}
					break;
					
				case 1:			/* AT������Խ׶�, ����������. �����ȴ� */
					if (ucData == AT_CR)
					{
						status = 2;
					}
					break;
					
				case 2:			/* ��ʼ����ģ��Ӧ���� */
										/* ֻҪ�յ�ģ���Ӧ���ַ���������ַ��䳬ʱ�жϽ�������ʱ�����ܳ�ʱ�������� */
					bsp_StartTimer(GPRS_TMR_ID, 500);
					if (pos < _usBufSize - 1)
					{
						_pBuf[pos++] = ucData;		/* ������յ������� */
					}
					break;
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_SendAT
*	����˵��: ��GSMģ�鷢��AT��� �������Զ���AT�ַ���������<CR>�ַ�
*	��    ��: _Str : AT�����ַ�����������ĩβ�Ļس�<CR>. ���ַ�0����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GPRS_SendAT(char *_Cmd)
{
	comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */	
	
	comSendBuf(COM_GPRS, (uint8_t *)_Cmd, strlen(_Cmd));
	comSendBuf(COM_GPRS, "\r", 1);
}

/*
*********************************************************************************************************
*	�� �� ��: AT_SendToCmdData
*	����˵��: ���Ϳ���ָ�����ݵ�ģ��
*	��    �Σ�_pCMDstr		 : AT ָ��
*					: _pBACKstr    ����ѯ����ָ��
*					: _usTimeOut	 : ���ʱ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0 ��ʾʧ��
*********************************************************************************************************
*/
uint8_t AT_SendToCmdData(char *_pCMDstr,char *_pBACKstr,uint16_t _usTimeOut)
{
	uint8_t ret;
	uint8_t i;
	GPRS_SendAT(_pCMDstr);
	for (i = 0; i < 5; i++)													//ϵͳĬ��������� 5 ��
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
*	�� �� ��: GPRS_GetHardInfo
*	����˵��: ��ȡģ���Ӳ����Ϣ. ���� ATI ����Ӧ������
*	��    ��: ��Ž���Ľṹ��ָ��
*	�� �� ֵ: 1 ��ʾ�ɹ��� 0 ��ʾʧ��
*********************************************************************************************************
*/
uint8_t GPRS_GetHardInfo(GPRS_INFO_T *_pInfo)
{	
	char buf[64];
	uint16_t len;
	
	GPRS_SendAT("ATI");		/* ���� ATI ���� */	
	len = GPRS_ReadResponse(buf, sizeof(buf), 300);	/* ��ʱ 300ms */
	if (len == 0)
	{
		return 0;		
	}
	
	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_GetNetStatus
*	����˵��: ��ѯ��ǰ����״̬
*	��    ��: ��
*	�� �� ֵ: ����״̬, CREG_NO_REG, CREG_LOCAL_OK �ȡ�
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
	
	GPRS_SendAT("AT+CREG?");	/* ���� AT ���� */
	
	len = GPRS_ReadResponse(buf, sizeof(buf), 200);	/* ��ʱ 200ms */
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
