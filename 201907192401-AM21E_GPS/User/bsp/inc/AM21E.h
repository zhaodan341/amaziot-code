#ifndef _AM21E_H_
#define _AM21E_H

		
#define _rATCMDCEREG	"+CEREG: 1, 1"
#define _rATCMDCSQ		"+CSQ: "


/* AM20E 功能使能项 */
#define ENABLEGPS			1 	//GPS功能使能

#define ENABLETCP			1 	//TCP功能使能	（暂未实现）
#define ENABLEUDP			1 	//UDP功能使能	

#define ENABLEHTTP		0 	//HTTP功能使能

/* oneNET平台相关数据 */
#define ONENETDEVID   "535759705"
#define ONENETDEVAPI  "kKAUkW0z8AHiPa9P7EYIQPKyPuM="

/* 私有服务器相关数据 */
#define SERVERIP  	 "183.230.40.33"
#define SERVERPROT   80

typedef struct
{
	uint8_t *CSQ;		//信号值
	uint8_t CEREG;	//注册信息，0：注册成功，1：注册失败
	uint8_t *CIMI;  //CIMI信息	
	
	
	uint8_t *LAT;		//纬度信息
	uint8_t *LON;		//经度信息	
	
	uint8_t *Speed;		//速度信息
	uint8_t *Altitude;	//海拔信息

	uint8_t _vTemp;		//温度信息
	uint8_t _vPm25;		//PM2.5信息
		
	char *strLAT;		//纬度字符串
	char *strLON;		//经度字符串
	
	char *str_vTemp;	//温度字符串
	char *str_vPm25;	//PM2.5字符串
	
  char *strSpeed;			//速度字符串
	char *strAltitude;	//海拔字符串
	
}AM21E_T;


/* 向oneNET 平台发送数据 HTTP 协议 */
uint8_t SendToHTTP(char *_pDevIDstr,char *_pAPIkey,char *_pData);

/* 向指定的TCP端口发送数据 */
int8_t SendToTCP(char *_pIPstr,uint32_t _vPortstr,char *_pData);

#endif
