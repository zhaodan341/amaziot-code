#include "bsp.h"			/* �ײ�Ӳ������ */

/* ���������������̷������� */
#define EXAMPLE_NAME	"GPS test"
#define EXAMPLE_DATE	"2019-07-16"
#define DEMO_VER			"1.0"

AM21E_T t_ATReadCMD;

uint8_t	am_CSQ[8];		//�ź�ֵ
uint8_t	am_CIMI[12];  //CIMI��Ϣ

uint8_t	am_LAT[64];		//γ����Ϣ
uint8_t	am_LON[64];		//������Ϣ

uint8_t	am_SPEED[12];				//γ����Ϣ
uint8_t	am_ALTITUDE[12];		//������Ϣ

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void DispGPSStatus(void);
void AM21E_init(void);
uint8_t AT_SendToCmdData(char *_pCMDstr,char *_pBACKstr,uint16_t _usTimeOut);

/* ��ʼ��AM21��ر��� */
void bsp_InitGprsToAM21E(void);

/* �õ�GPS���� */
void AT_GetToGPSData(char *_pATstr);

/* ϵͳ�������� */
__asm void SystemReset(void);


/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	char DataBuf[512];
	char LocationBuf[512];
	uint8_t ucData;
	
	/* Ӳ����ʼ�� */
	bsp_Init();		
	/* ��ӡ������Ϣ������1 */
	PrintfLogo();	
	/* ��ʼ��AM21E��ر��� */
	bsp_InitGprsToAM21E();
	AM21E_init();
	
	bsp_StartTimer(0, 10000); /* ���������ʱ��0������Ϊ�Զ�ģʽ,����1000ms */
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();																			/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		if(comGetChar(COM1, &ucData))										/* ������ת����GPRS���� */
		{
			comSendBuf(COM_GPRS, &ucData, 1);	
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */
		}
		
			if (bsp_CheckTimer(0))
			{			
				#if ENABLEGPS
				
				AT_GetToGPSData("AT+AMGNSSRD?");
				DispGPSStatus();														/* ��ӡGPS������� */		
				sprintf(LocationBuf,",;%s,%s;%s,%s", t_ATReadCMD.strLAT,t_ATReadCMD.LAT	
																					 , t_ATReadCMD.strLON,t_ATReadCMD.LON
							 );			
				#endif
				
				#if ENABLEHTTP
				
					/* POST ������ */
					sprintf(DataBuf,",;%s,%d;%s,%d;%s",t_ATReadCMD.str_vTemp,t_ATReadCMD._vTemp,				//�¶�
																					   t_ATReadCMD.str_vPm25,t_ATReadCMD._vPm25,				//PM2.5
																							LocationBuf																			//GPS��Ϣ
								 );
					SendToHTTP(ONENETDEVID,ONENETDEVAPI,DataBuf);
	
					/* GET ������ */
				#endif
				
				#if ENABLETCP
				
					/* POST ������ */
					sprintf(LocationBuf,"{\"datastreams\":[{\"id\":\"location\",\"datapoints\":[{\"value\":{\"lon\":%s,\"lat\":%s}}]}]}\r\n",t_ATReadCMD.LON,t_ATReadCMD.LAT);	
					sprintf(DataBuf,"POST /devices/%s/datapoints HTTP/1.1\r\napi-key: %s\r\nHost: api.heclouds.com\r\nContent-Length: %d\r\n\r\n%s"	,ONENETDEVID,ONENETDEVAPI,strlen(LocationBuf),LocationBuf);						
					SendToTCP(SERVERIP,SERVERPROT,DataBuf);
					
					/* GET ������ */
				#endif

			 /* ���������ʱ��0������Ϊ�Զ�ģʽ,����1000ms */
				bsp_StartTimer(0, 10000);								
			}
		}
}


/*
*********************************************************************************************************
*	�� �� ��: AM21E_init
*	����˵��: AM21E������ʼ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AM21E_init(void)
{
	uint8_t i;
	
	/* �ڴ˴���ӿ��ƿ������� */
	
	
	/* AT ���� �ȴ� OK ��Ӧ */
	if(GPRS_PowerOn())
	{		
		#if ENABLEGPS		
		
				if(AT_SendToCmdData("AT+AMGNSSC=1","OK",2000))					
				{	
							/*GPSоƬ��һ����EOP��Ҫ����Download EPO ok and save success! ��Ϊ�ɹ����ٴ��ϵ�ͻ᷵�� OK */
							/* ��ȡ������������ģ�飬Ҳ���Բ�ʹ��EPO����ģ�鹤��û��̫��Ӱ�� */
							if(AT_SendToCmdData("AT+AMGNSSEPO=1","Download EPO ok",5000))
							{					
									if(AT_SendToCmdData("AT+AMGEPOAID","OK",5000))
									{
										return;
									}else if (AT_SendToCmdData("AT+AMGEPOAID","+CME ERROR",5000))
									{
										return;			//ʧ����Ҳ�Ƴ�����
									}						
							}else if (AT_SendToCmdData("AT+AMGNSSEPO=1","OK",5000))	//һ��EPO����Сʱ��Ч��ģ�����ú�EPO���ֱ�ӷ��� OK
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
	/* ��Ӧʧ�ܺ����ʧ�ܴ��� */
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
*	�� �� ��: AT_GetToGPSData
*	����˵��: �õ�GPS����
*	��    �Σ�_pATtr		 : AT ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AT_GetToGPSData(char *_pATstr)
{
	GPRS_SendAT(_pATstr);						   //���� AT ָ��
	bsp_StartTimer(GPRS_TMR_ID,10000); //�̶�ʱ��10000ms������ģ�鷴����Ϣ��ʱ��
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
*	�� �� ��: DispGPSStatus
*	����˵��: ��ӡGPS���ݰ�������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispGPSStatus(void)
{
	char buf[128];
	
	char _ddmmDuf[64];
	char _ddDuf[64];
	
	double   	MM = 0.0;	
	double   	DD = 0.0;		
		
	/* γ�Ȼ��� */
	sprintf(_ddmmDuf, "%02d.%05d",g_tGPS.WeiDu_Fen/100000,g_tGPS.WeiDu_Fen%100000);
	MM = atof(_ddmmDuf);										//�õ��ֵĸ�����
	MM = MM / 60.0;
	DD = MM + (double)g_tGPS.WeiDu_Du;
	sprintf(_ddDuf,"%lf",DD);							 //������γ�ȣ���λ ��
	/* ����ʱ����ֵд��Ҫ���͵����� */
	
	memcpy(t_ATReadCMD.LAT,_ddDuf,strlen(_ddDuf));
	
	/* �����ʱ���� */
	memset(_ddmmDuf,0,strlen(_ddmmDuf));
	memset(_ddDuf,0,strlen(_ddDuf));
	/* ���Ȼ��� */
	sprintf(_ddmmDuf, "%02d.%05d",g_tGPS.JingDu_Fen/100000,g_tGPS.JingDu_Fen%100000);
	MM = atof(_ddmmDuf);									//�õ��ֵĸ�����
	MM = MM / 60.0;
	DD = MM + (double)g_tGPS.JingDu_Du;
	sprintf(_ddDuf,"%lf",DD);							//�����ľ��ȣ���λ ��
	/* ����ʱ����ֵд��Ҫ���͵����� */
	
	memcpy(t_ATReadCMD.LON,_ddDuf,strlen(_ddDuf));
	
	/* �����ʱ���� */
	memset(_ddmmDuf,0,strlen(_ddmmDuf));
	memset(_ddDuf,0,strlen(_ddDuf));

	/* �ٶ� */
	sprintf(buf,"%5d.%d", g_tGPS.SpeedKM / 10, g_tGPS.SpeedKM % 10);

	memcpy(t_ATReadCMD.Speed,buf,strlen(buf));

	/* ���� */
	sprintf(buf, "%5d.%d", g_tGPS.Altitude / 10, g_tGPS.Altitude % 10);
	
	memcpy(t_ATReadCMD.Altitude,buf,strlen(buf));
	
}


/*
	AT+CREG?  ��ѯ��ǰ����״̬

	AT+CSQ ��ѯ�ź���������

	AT+CIMI ��ѯSIM ����IMSI �š�

*/
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitGprsToAM21E
*	����˵��: ��ʼ��AM21E��ر���
*	��    ��:  ��
*	�� �� ֵ: ��
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
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("\n\r");
	printf("*************************************************************\r\n");
	printf("* Routine name    : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* Routine version : %s\r\n", DEMO_VER);		  /* ��ӡ���̰汾 */
	printf("* publish date    : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST�̼���汾����3���������stm32f10x.h�ļ��� */
	printf("* Hardware version: V%d.%d.%d (STM32F10x_StdPeriph_Driver)\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	printf("*************************************************************\r\n");
}




