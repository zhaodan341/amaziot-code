#include "bsp.h"			/* 底层硬件驱动 */

/*
*********************************************************************************************************
*	函 数 名: SendToTCP
*	功能说明: AM21E向平台发送数据 
*	形    参：
*					：_pIPstr		 ：IP地址
*					：_pIPstr    : 端口
*					: _pDevIDstr ：平台设置ID
*					：_pAPIkey 	 ：设备APIKEY
*					：_pData 	   ：要发送的数据
*	返 回 值: 1 ：成功； 0：连接服务器失败；-1：发送失败；-2：发送长度超限
*********************************************************************************************************
*/

int8_t SendToTCP(char *_pIPstr,uint32_t _vPortstr,char *_pData)
{
	char Buf[64];
	int8_t ret = 0;
	char  *_pSendfBuf;		//发送缓存
	int32_t vSendLen;			//发送长度
	
	vSendLen = strlen(_pData);
	
	if(vSendLen >= 2048)	//长度超限，退出
	{
			return -2;
	}
	
		_pSendfBuf = (char *) malloc(vSendLen);																	//分配一段内存
	
		sprintf(_pSendfBuf,"%s\r",_pData);
		_pSendfBuf[vSendLen] = 0x1A;																						//最后一位添加结束符
	
	  sprintf(Buf,"AT+CIPSTART=\"TCP\",\"%s\",%d\r",_pIPstr,_vPortstr);				//将IP 和 端口先格式化																					
		
		/* 查询一下TCP连接状态	*/								
			if (AT_SendToCmdData("AT+CIPSTATUS","STATE:IP INITIAL",100))						//第一次开机首先要初始化TCP的相关IP地址和端口
			{		
					if(AT_SendToCmdData(Buf,"CONNECT OK",5000))												//等待连接成功
					{				
						 ret = 1;			
					}else{			
						free(_pSendfBuf);//释放内存
						return ret = 0;					
					}
					AT_SendToCmdData("AT+CIPSEND",">",1);														 //发送特殊性, 由于" > " 字符后没有换行，所以时间简短一点   
					if(AT_SendToCmdData(_pSendfBuf,"SEND OK",1000))											//发送数据到服务器
					{
						ret = 1;
					}else{
						free(_pSendfBuf);//释放内存
						return ret = -1;
					}		

			}else if(AT_SendToCmdData("AT+CIPSTATUS","STATE:CONNECT OK",100))
			{
			
					AT_SendToCmdData("AT+CIPSEND",">",1);														    
					if(AT_SendToCmdData(_pSendfBuf,"SEND OK",1000))													//发送数据到服务器
					{
						ret = 1;
					}else{
						free(_pSendfBuf);//释放内存
						return ret = -1;
					}			
			}
			else if(AT_SendToCmdData("AT+CIPSTATUS","STATE:CLOSED",100))
			{
					if(AT_SendToCmdData(Buf,"CONNECT OK",5000))												//等待连接成功
					{				
						 ret = 1;			
					}else{			
						AT_SendToCmdData("AT+CIPCLOSE","CLOSE OK",1000);
						free(_pSendfBuf);//释放内存
						return ret = 0;					
					}
					AT_SendToCmdData("AT+CIPSEND",">",1);														 //发送特殊性, 由于" > " 字符后没有换行，所以时间简短一点   
					if(AT_SendToCmdData(_pSendfBuf,"SEND OK",1000))											//发送数据到服务器
					{
						ret = 1;
					}else{
						free(_pSendfBuf);//释放内存
						return ret = -1;
					}	
			}
			free(_pSendfBuf);//释放内存
			return ret;
}


