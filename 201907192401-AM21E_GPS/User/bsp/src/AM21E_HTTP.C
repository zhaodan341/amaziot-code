#include "bsp.h"			/* �ײ�Ӳ������ */


/*
*********************************************************************************************************
*	�� �� ��: SendToHTTP
*	����˵��: AM21E��ƽ̨��������
*	��    �Σ�_pDevIDstr ��ƽ̨����ID
*					��_pAPIkey 	 ���豸APIKEY
*					��_pData 	   ��Ҫ���͵�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/

uint8_t SendToHTTP(char *_pDevIDstr,char *_pAPIkey,char *_pData)
{
	
	static	char Buf[256];
	static 	char DataBuf[1024];
	static	char _pREAD[64];
	uint8_t ucData;
	uint8_t pos = 0;
	
	uint8_t ret_value = 0;
	uint8_t CheckTimer = 0;
	
	sprintf(DataBuf,"%s\r",_pData);
	DataBuf[strlen(DataBuf)] = 0x1A;						//���һλ���ӽ�����
	

	
	AT_SendToCmdData("AT+HTTPTERM","OK",5000);
	AT_SendToCmdData("AT+HTTPINIT","OK",5000);
	AT_SendToCmdData("AT+HTTPPARA = \"CID\",\"1\"","OK",5000);
	
	sprintf(Buf,"AT+HTTPPARA = \"URL\",\"http://api.heclouds.com/devices/%s/datapoints?type=5\"",_pDevIDstr);
	AT_SendToCmdData(Buf,"OK",5000);
	memset(Buf,0,strlen(Buf));
	
	sprintf(Buf,"AT+HTTPPARA = \"API_KEY\",\"%s\"",_pAPIkey);
	AT_SendToCmdData(Buf,"OK",5000);
	memset(Buf,0,strlen(Buf));
	
	AT_SendToCmdData("AT+HTTPDATA",">",5000);
	AT_SendToCmdData(DataBuf,"OK",5000);
	AT_SendToCmdData("AT+HTTPACTION=1","OK",5000);


	bsp_StartAutoTimer(1, 60000);/* ����������ʱ��0������Ϊ�Զ�ģʽ,����1000ms */
	while(1)
	{
			if (bsp_CheckTimer(1))
			{
				CheckTimer++;
				if(CheckTimer >= 5)		/* �ȴ�Լ5���ӣ������û�յ�oneNETƽ̨��Ӧ��ʾ���ֹ��� */
				{
					AT_SendToCmdData("AT+HTTPTERM","OK",5000);
					break;	
				}
				
			}		
			if(comGetChar(COM_GPRS, &ucData))
			{
				GPRS_PrintRxData(ucData);		
				_pREAD[pos++] = ucData;
				if(ucData == 0x0D)
				{
							if(strstr(_pREAD,"1"))
							 {
										ret_value = 1;
										break;
							 }else if (strstr(_pREAD,"wait"))
								{
										SystemReset();									/* �����������⣬Ŀǰ����취��������� */
										break;
								}
								else if (strstr(_pREAD,"failure"))
								{
										AT_SendToCmdData("AT+HTTPTERM","OK",5000);
									//	SystemReset();								/* �����������⣬Ŀǰ����취��������� */
										break;
								}
				}
			}
	}
	return ret_value;
}

