/*
*********************************************************************************************************
*
*	模块名称 : GPRS模块驱动程序
*	文件名称 : bsp_SIM800.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef __BSP_GPRS_H
#define __BSP_GPRS_H

#define COM_GPRS	COM2			/* 选择串口 */
//#define COM_GPRS	COM6		/* 选择串口 */

/* 定义下面这句话, 将把收到的字符发送到调试串口1 */
#define GPRS_TO_COM1_EN

/* 
  本模块部分函数用到了软件定时器最后1个ID。 因此主程序调用本模块的函数时，请注意回避定时器 TMR_COUNT - 1。
  bsp_StartTimer(3, _usTimeOut);
  
  TMR_COUNT 在 bsp_timer.h 文件定义
*/
#define GPRS_TMR_ID	(TMR_COUNT - 1)

#define AT_CR		'\r'
#define AT_LF		'\n'

/* AT+CREG? 命令应答中的网络状态定义 	当前网络注册状态  GPRS_GetNetStatus() */
enum
{
	CREG_NO_REG = 0,  	/* 0：没有注册，ME现在并没有在搜寻要注册的新的运营商 */
	CREG_LOCAL_OK = 1,	/* 1：注册了本地网络 */
	CREG_SEARCH = 2,	  /* 2：没有注册，但MS正在搜寻要注册的新的运营商 */
	CREG_REJECT = 3,	  /* 3：注册被拒绝 */
	CREG_UNKNOW = 4,	  /* 4：未知原因 */
	CREG_REMOTE_OK = 5, /* 5：注册了漫游网络 */
};


typedef struct
{
	char Manfacture[12];			/* 厂商 SIMCOM_Ltd */
	char Model[12];		  			/* 型号 GPRS */
	char Revision[15 + 1];		/* 固件版本 R13.08 */
	//char IMEI[15 + 1];			/* IMEI 码 需要通过AT+GSN获得 */
}GPRS_INFO_T;

/* 供外部调用的函数声明 */
void bsp_InitGPRS(void);
void GPRS_Reset(void);

uint8_t GPRS_PowerOn(void);
void GPRS_PowerOff(void);
void GPRS_SendAT(char *_Cmd);

uint8_t AT_SendToCmdData(char *_pCMDstr,char *_pBACKstr,uint16_t _usTimeOut);

void GPRS_PrintRxData(uint8_t _ch);
uint8_t GPRS_WaitResponse(char *_pAckStr, uint16_t _usTimeOut);
uint16_t GPRS_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut);

uint8_t GPRS_GetHardInfo(GPRS_INFO_T *_pInfo);
uint8_t GPRS_GetNetStatus(void);




#endif



