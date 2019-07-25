#include "bsp.h"			/* 底层硬件驱动 */

/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"GPS test"
#define EXAMPLE_DATE	"2019-07-16"
#define DEMO_VER			"1.0"

AM21E_T t_ATReadCMD;

uint8_t	am_CSQ[8];		//信号值
uint8_t	am_CIMI[12];  //CIMI信息

uint8_t	am_LAT[64];		//纬度信息
uint8_t	am_LON[64];		//经度信息

uint8_t	am_SPEED[12];				//纬度信息
uint8_t	am_ALTITUDE[12];		//经度信息

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void DispGPSStatus(void);
void AM21E_init(void);
uint8_t AT_SendToCmdData(char *_pCMDstr,char *_pBACKstr,uint16_t _usTimeOut);

/* 初始化AM21相关变量 */
void bsp_InitGprsToAM21E(void);

/* 得到GPS数据 */
void AT_GetToGPSData(char *_pATstr);

/* 系统重启函数 */
__asm void SystemReset(void);


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	char DataBuf[512];
	char LocationBuf[512];
	uint8_t ucData;
	
	/* 硬件初始化 */
	bsp_Init();		
	/* 打印例程信息到串口1 */
	PrintfLogo();	
	/* 初始化AM21E相关变量 */
	bsp_InitGprsToAM21E();
	AM21E_init();
	
	bsp_StartTimer(0, 10000); /* 启动软件定时器0，设置为自动模式,周期1000ms */
	
	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();																			/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		if(comGetChar(COM1, &ucData))										/* 将串口转发到GPRS串口 */
		{
			comSendBuf(COM_GPRS, &ucData, 1);	
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */
		}
		
			if (bsp_CheckTimer(0))
			{			
				#if ENABLEGPS
				
				AT_GetToGPSData("AT+AMGNSSRD?");
				DispGPSStatus();														/* 打印GPS解析结果 */		
				sprintf(LocationBuf,",;%s,%s;%s,%s", t_ATReadCMD.strLAT,t_ATReadCMD.LAT	
																					 , t_ATReadCMD.strLON,t_ATReadCMD.LON
							 );			
				#endif
				
				#if ENABLEHTTP
				
					/* POST 数据流 */
					sprintf(DataBuf,",;%s,%d;%s,%d;%s",t_ATReadCMD.str_vTemp,t_ATReadCMD._vTemp,				//温度
																					   t_ATReadCMD.str_vPm25,t_ATReadCMD._vPm25,				//PM2.5
																							LocationBuf																			//GPS信息
								 );
					SendToHTTP(ONENETDEVID,ONENETDEVAPI,DataBuf);
	
					/* GET 数据流 */
				#endif
				
				#if ENABLETCP
				
					/* POST 数据流 */
					sprintf(LocationBuf,"{\"datastreams\":[{\"id\":\"location\",\"datapoints\":[{\"value\":{\"lon\":%s,\"lat\":%s}}]}]}\r\n",t_ATReadCMD.LON,t_ATReadCMD.LAT);	
					sprintf(DataBuf,"POST /devices/%s/datapoints HTTP/1.1\r\napi-key: %s\r\nHost: api.heclouds.com\r\nContent-Length: %d\r\n\r\n%s"	,ONENETDEVID,ONENETDEVAPI,strlen(LocationBuf),LocationBuf);						
					SendToTCP(SERVERIP,SERVERPROT,DataBuf);
					
					/* GET 数据流 */
				#endif

			 /* 启动软件定时器0，设置为自动模式,周期1000ms */
				bsp_StartTimer(0, 10000);								
			}
		}
}


/*
*********************************************************************************************************
*	函 数 名: AM21E_init
*	功能说明: AM21E开机初始化
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AM21E_init(void)
{
	uint8_t i;
	
	/* 在此处添加控制开机引脚 */
	
	
	/* AT 发送 等待 OK 响应 */
	if(GPRS_PowerOn())
	{		
		#if ENABLEGPS		
		
				if(AT_SendToCmdData("AT+AMGNSSC=1","OK",2000))					
				{	
							/*GPS芯片第一次用EOP需要返回Download EPO ok and save success! 则为成功，再次上电就会返回 OK */
							/* 获取不到可以重启模组，也可以不使用EPO，对模组工作没有太大影响 */
							if(AT_SendToCmdData("AT+AMGNSSEPO=1","Download EPO ok",5000))
							{					
									if(AT_SendToCmdData("AT+AMGEPOAID","OK",5000))
									{
										return;
									}else if (AT_SendToCmdData("AT+AMGEPOAID","+CME ERROR",5000))
									{
										return;			//失败了也推出函数
									}						
							}else if (AT_SendToCmdData("AT+AMGNSSEPO=1","OK",5000))	//一般EPO两个小时有效，模组设置好EPO后会直接返回 OK
							{
										return;
							}													
				}
		goto exit;	
		#else
		return;
		#endif
	}else{
		goto exit;	
	}
	/* 响应失败后加入失败代码 */
exit:	
	while(1)
	{
	    printf(" AM21E Response failure   \n ");
			printf(" AM21E system Reswet ...   \r\n ");
			SystemReset();
		
	}
}




/*
*********************************************************************************************************
*	函 数 名: AT_GetToGPSData
*	功能说明: 得到GPS数据
*	形    参：_pATtr		 : AT 指令
*	返 回 值: 无
*********************************************************************************************************
*/
void AT_GetToGPSData(char *_pATstr)
{
	GPRS_SendAT(_pATstr);						   //发送 AT 指令
	bsp_StartTimer(GPRS_TMR_ID,10000); //固定时间10000ms，给足模组反馈信息的时间
	while(1)
	{	
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			gps_pro();
			return;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispGPSStatus
*	功能说明: 打印GPS数据包解码结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispGPSStatus(void)
{
	char buf[128];
	
	char _ddmmDuf[64];
	char _ddDuf[64];
	
	double   	MM = 0.0;	
	double   	DD = 0.0;		
		
	/* 纬度换算 */
	sprintf(_ddmmDuf, "%02d.%05d",g_tGPS.WeiDu_Fen/100000,g_tGPS.WeiDu_Fen%100000);
	MM = atof(_ddmmDuf);										//得到分的浮点数
	MM = MM / 60.0;
	DD = MM + (double)g_tGPS.WeiDu_Du;
	sprintf(_ddDuf,"%lf",DD);							 //换算后的纬度，单位 °
	/* 将临时缓存值写入要发送的数组 */
	
	memcpy(t_ATReadCMD.LAT,_ddDuf,strlen(_ddDuf));
	
	/* 清楚临时缓存 */
	memset(_ddmmDuf,0,strlen(_ddmmDuf));
	memset(_ddDuf,0,strlen(_ddDuf));
	/* 精度换算 */
	sprintf(_ddmmDuf, "%02d.%05d",g_tGPS.JingDu_Fen/100000,g_tGPS.JingDu_Fen%100000);
	MM = atof(_ddmmDuf);									//得到分的浮点数
	MM = MM / 60.0;
	DD = MM + (double)g_tGPS.JingDu_Du;
	sprintf(_ddDuf,"%lf",DD);							//换算后的经度，单位 °
	/* 将临时缓存值写入要发送的数组 */
	
	memcpy(t_ATReadCMD.LON,_ddDuf,strlen(_ddDuf));
	
	/* 清楚临时缓存 */
	memset(_ddmmDuf,0,strlen(_ddmmDuf));
	memset(_ddDuf,0,strlen(_ddDuf));

	/* 速度 */
	sprintf(buf,"%5d.%d", g_tGPS.SpeedKM / 10, g_tGPS.SpeedKM % 10);

	memcpy(t_ATReadCMD.Speed,buf,strlen(buf));

	/* 海拔 */
	sprintf(buf, "%5d.%d", g_tGPS.Altitude / 10, g_tGPS.Altitude % 10);
	
	memcpy(t_ATReadCMD.Altitude,buf,strlen(buf));
	
}


/*
	AT+CREG?  查询当前网络状态

	AT+CSQ 查询信号质量命令

	AT+CIMI 查询SIM 卡的IMSI 号。

*/
/*
*********************************************************************************************************
*	函 数 名: bsp_InitGprsToAM21E
*	功能说明: 初始化AM21E相关变量
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitGprsToAM21E(void)
{	
	t_ATReadCMD.CSQ = am_CSQ;
	t_ATReadCMD.CIMI = am_CIMI;
	t_ATReadCMD.CEREG = 0;
	
	t_ATReadCMD.LAT = am_LAT;
	t_ATReadCMD.LON = am_LON;
	
	t_ATReadCMD.Speed = am_SPEED;
	t_ATReadCMD.Altitude = am_ALTITUDE;
	
	t_ATReadCMD._vPm25 = 20;
	t_ATReadCMD._vTemp = 88;
	
	
	t_ATReadCMD.strLAT = "lat";
	t_ATReadCMD.strLON = "lon";
	
	t_ATReadCMD.str_vTemp = "temperature";
	t_ATReadCMD.str_vPm25 = "pm2.5";
	
	t_ATReadCMD.strSpeed    = "SPEED";
	t_ATReadCMD.strAltitude = "Altitude";
}
/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("\n\r");
	printf("*************************************************************\r\n");
	printf("* Routine name    : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* Routine version : %s\r\n", DEMO_VER);		  /* 打印例程版本 */
	printf("* publish date    : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST固件库版本，这3个定义宏在stm32f10x.h文件中 */
	printf("* Hardware version: V%d.%d.%d (STM32F10x_StdPeriph_Driver)\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	printf("*************************************************************\r\n");
}




