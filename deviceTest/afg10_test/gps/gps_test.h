/********************************************************************************
**  Copyright (c) 2010, 深圳市飞瑞斯科技有限公司
**  All rights reserved.
**	
**  文件说明: 此文件实现对基础的线程调用函数做了封装，并且实现了线程池基类。
**  创建日期: 2014.02.28
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#ifndef _GPS_TEST_H_
#define _GPS_TEST_H_

#include "nmea.h"

/* gps设备串口1 */
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
	unsigned long ultime;			/* gps时间,本地时间戳 */
	double deglat;			/* gps经度 */
	double deglon;			/* gps纬度 */
	double speed;			/* gps速度(千米/小时) */
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
