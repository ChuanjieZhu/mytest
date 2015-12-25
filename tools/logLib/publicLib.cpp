#include "publicLib.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

/******************************************************************************
 * 函数名称： DatetimeToTime
 * 功能： 将日期时间格式转换为时间
 * 参数： pDatetime:要转换的日期时间
 		  
 * 返回： 转换后的时间格式
 * 创建作者： 
 * 创建日期： 2012-6-21
 * 修改作者：
 * 修改日期：
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
 * 函数名称： TimeToNextDay
 * 功能： 将时间换算为第2天的0点0分0秒
 * 参数： 	uiInTime : 输入的秒数
 * 返回： 换算为第2天0点0分0秒的秒数
 * 创建作者：
 * 创建日期： 2012-6-25
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
unsigned int TimeToNextDay(unsigned int uiInTime)
{
	/* 把当天的时间变为凌晨，如2012-07-19 00:00:00 再到第二天的凌晨开始*/
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

