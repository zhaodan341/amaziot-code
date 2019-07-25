/*
*********************************************************************************************************
*
*	ģ������ : GPS��λģ����������
*	�ļ����� : bsp_g_tGPS.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*********************************************************************************************************
*/

#ifndef __BSP_GPS_H
#define __BSP_GPS_H


#define DEBUG_GPS_TO_COM1
typedef struct
{
	uint8_t UartOk;		/* 1��ʾ�������ݽ����������������յ�GPS�������ַ��� */
	
	/* ��GGA ���� : ʱ�䡢λ�á���λ����  (λ����Ϣ���� RMC������) */

	/* ��λ��Ч��־, 0:δ��λ 1:SPSģʽ����λ��Ч 2:��֣�SPSģʽ����λ��Ч 3:PPSģʽ����λ��Ч */
	uint8_t PositionOk;
	uint32_t Altitude;	/* ���θ߶ȣ���Ժ�ƽ�������   x.x  */

	/* GLL�����ȡ�γ�ȡ�UTCʱ�� */
	/* �������ݣ�$GPGLL,3723.2475,N,12158.3416,W,161229.487,A*2C */


	/* GSA��GPS���ջ�����ģʽ����λʹ�õ����ǣ�DOPֵ */
	/* �������ݣ�$GPGSA,A,3,07,02,26,27,09,04,15, , , , , ,1.8,1.0,1.5*33 */
	uint8_t ModeAM;				/* M=�ֶ���ǿ�Ʋ�����2D��3Dģʽ����A=�Զ� */
	uint8_t Mode2D3D;			/* ��λ���� 1=δ��λ��2=2D��λ��3=3D��λ */
	uint8_t SateID[12];			/* ID of 1st satellite used for fix */
	uint16_t PDOP;				/* λ�þ���, 0.1�� */
	uint16_t HDOP;				/* ˮƽ����, 0.1�� */
	uint16_t VDOP;				/* ��ֱ����, 0.1�� */

	/* GSV���ɼ�GPS������Ϣ�����ǡ���λ�ǡ�����ȣ�SNR�� */
	/*
		$GPGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
		$GPGSV,2,2,07,09,23,313,42,04,19,159,41,15,12,041,42*41
	*/
	uint8_t ViewNumber;			/* �ɼ����Ǹ��� */
	uint8_t Elevation[12];		/* ���� elevation in degrees  ��,���90��*/
	uint16_t Azimuth[12];		/* ��λ�� azimuth in degrees to true  ��,0-359��*/
	uint8_t SNR[12];			/* ����ȣ�C/No��  ����ȣ� �� dBHz   ��Χ0��99��û�и���ʱΪ�� */

	/* RMC��ʱ�䡢���ڡ�λ��	���ٶ� */
	/* �������ݣ�$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*10 */
	uint16_t WeiDu_Du;			/* γ�ȣ��� */
	uint32_t WeiDu_Fen;			/* γ�ȣ���. 232475��  С�����4λ, ��ʾ 23.2475 �� */
	char     NS;				/* γ�Ȱ���N�������򣩻�S���ϰ��� */

	uint16_t JingDu_Du;			/* ���ȣ���λ�� */
	uint32_t JingDu_Fen;		/* ���ȣ���λ�� */
	char     EW;				/* ���Ȱ���E����������W�������� */

	uint16_t Year;				/* ���� 120598 ddmmyy */
	uint8_t  Month;
	uint8_t  Day;
	uint8_t  Hour;				/* UTC ʱ�䡣 hhmmss.sss */
	uint8_t  Min;				/* �� */
	uint8_t  Sec;				/* �� */
	uint16_t mSec;				/* ���� */
	char   TimeOk;				/* A=UTCʱ��������Ч��V=������Ч */

	/* VTG�������ٶ���Ϣ */
	/* �������ݣ�$GPVTG,309.62,T, ,M,0.13,N,0.2,K*6E */
	uint16_t TrackDegTrue;		/* ���汱Ϊ�ο���׼�ĵ��溽��000~359�ȣ�ǰ���0Ҳ�������䣩 */
	uint16_t TrackDegMag;		/* �Դű�Ϊ�ο���׼�ĵ��溽��000~359�ȣ�ǰ���0Ҳ�������䣩 */
	uint32_t SpeedKnots;		/* �������ʣ�000.0~999.9�ڣ�ǰ���0Ҳ�������䣩 */
	uint32_t SpeedKM;			/* �������ʣ�000.0~999.9�ڣ�ǰ���0Ҳ�������䣩 */

}GPS_T;

void bsp_InitGPS(void);
void gps_pro(void);
void GPS_ParsingData(char *_pData);
uint32_t gps_FenToDu(uint32_t _fen);
uint16_t gps_FenToMiao(uint32_t _fen);

extern GPS_T g_tGPS;

#endif


