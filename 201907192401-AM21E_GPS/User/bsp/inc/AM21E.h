#ifndef _AM21E_H_
#define _AM21E_H

		
#define _rATCMDCEREG	"+CEREG: 1, 1"
#define _rATCMDCSQ		"+CSQ: "


/* AM20E ����ʹ���� */
#define ENABLEGPS			1 	//GPS����ʹ��

#define ENABLETCP			1 	//TCP����ʹ��	����δʵ�֣�
#define ENABLEUDP			1 	//UDP����ʹ��	

#define ENABLEHTTP		0 	//HTTP����ʹ��

/* oneNETƽ̨������� */
#define ONENETDEVID   "535759705"
#define ONENETDEVAPI  "kKAUkW0z8AHiPa9P7EYIQPKyPuM="

/* ˽�з������������ */
#define SERVERIP  	 "183.230.40.33"
#define SERVERPROT   80

typedef struct
{
	uint8_t *CSQ;		//�ź�ֵ
	uint8_t CEREG;	//ע����Ϣ��0��ע��ɹ���1��ע��ʧ��
	uint8_t *CIMI;  //CIMI��Ϣ	
	
	
	uint8_t *LAT;		//γ����Ϣ
	uint8_t *LON;		//������Ϣ	
	
	uint8_t *Speed;		//�ٶ���Ϣ
	uint8_t *Altitude;	//������Ϣ

	uint8_t _vTemp;		//�¶���Ϣ
	uint8_t _vPm25;		//PM2.5��Ϣ
		
	char *strLAT;		//γ���ַ���
	char *strLON;		//�����ַ���
	
	char *str_vTemp;	//�¶��ַ���
	char *str_vPm25;	//PM2.5�ַ���
	
  char *strSpeed;			//�ٶ��ַ���
	char *strAltitude;	//�����ַ���
	
}AM21E_T;


/* ��oneNET ƽ̨�������� HTTP Э�� */
uint8_t SendToHTTP(char *_pDevIDstr,char *_pAPIkey,char *_pData);

/* ��ָ����TCP�˿ڷ������� */
int8_t SendToTCP(char *_pIPstr,uint32_t _vPortstr,char *_pData);

#endif
