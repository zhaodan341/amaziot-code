#include "bsp.h"			/* 底层硬件驱动 */


/*
*********************************************************************************************************
*	函 数 名: SendToHTTP
*	功能说明: AM21E向平台发送数据
*	形    参：_pDevIDstr ：平台设置ID
*					：_pAPIkey 	 ：设备APIKEY
*					：_pData 	   ：要发送的数据
*	返 回 值: 无
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
	DataBuf[strlen(DataBuf)] = 0x1A;						//最后一位添加结束符
	

	
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


	bsp_StartAutoTimer(1, 60000);/* 启动软件定时器0，设置为自动模式,周期1000ms */
	while(1)
	{
			if (bsp_CheckTimer(1))
			{
				CheckTimer++;
				if(CheckTimer >= 5)		/* 等大约5分钟，如果还没收到oneNET平台响应表示出现故障 */
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
										SystemReset();									/* 遇到这种问题，目前解决办法最好是重启 */
										break;
								}
								else if (strstr(_pREAD,"failure"))
								{
										AT_SendToCmdData("AT+HTTPTERM","OK",5000);
									//	SystemReset();								/* 遇到这种问题，目前解决办法最好是重启 */
										break;
								}
				}
			}
	}
	return ret_value;
}


