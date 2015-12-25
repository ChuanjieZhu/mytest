#include "publicLib.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

/******************************************************************************
 * �������ƣ� DatetimeToTime
 * ���ܣ� ������ʱ���ʽת��Ϊʱ��
 * ������ pDatetime:Ҫת��������ʱ��
 		  
 * ���أ� ת�����ʱ���ʽ
 * �������ߣ� 
 * �������ڣ� 2012-6-21
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
unsigned int DatetimeToTime(char *pDatetime)
{
	struct tm tmTime;
	time_t ti = 0;

	if(pDatetime) {
		sscanf(pDatetime, "%d-%02d-%02d %02d:%02d:%02d", 
			&tmTime.tm_year, &tmTime.tm_mon, &tmTime.tm_mday, &tmTime.tm_hour, &tmTime.tm_min, &tmTime.tm_sec);
		tmTime.tm_year-=1900;
		tmTime.tm_mon-=1;
		ti = mktime(&tmTime);
	}

	return ti;
}

/******************************************************************************
 * �������ƣ� TimeToNextDay
 * ���ܣ� ��ʱ�任��Ϊ��2���0��0��0��
 * ������ 	uiInTime : ���������
 * ���أ� ����Ϊ��2��0��0��0�������
 * �������ߣ�
 * �������ڣ� 2012-6-25
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
unsigned int TimeToNextDay(unsigned int uiInTime)
{
	/* �ѵ����ʱ���Ϊ�賿����2012-07-19 00:00:00 �ٵ��ڶ�����賿��ʼ*/
	char timeBuf[32] = {0};
	time_t tt;
	struct tm Tm;
	struct tm *pTm = &Tm;
	unsigned int uiRetTime;
	
	tt = uiInTime;
  	localtime_r(&tt, pTm);

	sprintf(timeBuf, "%d-%02d-%02d %02d:%02d:%02d", pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, 0, 0, 0);
	uiRetTime = DatetimeToTime(timeBuf);
	uiRetTime += (24 * 60 * 60);

	return uiRetTime;
}

