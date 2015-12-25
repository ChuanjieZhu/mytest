/********************************************************************************
**  Copyright (c) 2010, �����з���˹�Ƽ����޹�˾
**  All rights reserved.
**	
**  �ļ�˵��: ���ļ�ʵ�ֶԻ������̵߳��ú������˷�װ������ʵ�����̳߳ػ��ࡣ
**  ��������: 2014.02.28
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#ifndef _GPS_TEST_H_
#define _GPS_TEST_H_

#include "nmea.h"

/* gps�豸����1 */
#define GPS_DEVICE                 "/dev/ttySAC1"

#define MAX_BUFF_LEN                2048
#define MAX_CHECK_TIMES             50
#define MAX_ERROR_COUNT             50

typedef struct uart_param
{
	int speed;
	int databits;
	int stopbits;
	int parity;
	int opostflag;
} uart_param;

typedef struct _gpsPosData
{
	unsigned long ultime;			/* gpsʱ��,����ʱ��� */
	double deglat;			/* gps���� */
	double deglon;			/* gpsγ�� */
	double speed;			/* gps�ٶ�(ǧ��/Сʱ) */
}gpsPosData;


#ifdef __cplusplus
extern "C" {
#endif

extern int gps_open(const char *pDevice);
extern int gps_nmea_parse(nmeaGPRMC *packRMC, gpsPosData *posData);


#ifdef __cplusplus
}
#endif

#endif  //_GPS_TEST_H_
