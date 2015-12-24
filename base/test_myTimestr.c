
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define TRACE printf

void TimeToDateTime(unsigned long ulTime, int *pYear, int *pMonth, int *pDay,
							int *pHour, int *pMinute, int *pSecond)
{
	struct tm strTm;
    struct tm *pTm = &strTm;
	
	if (ulTime < 0)
	{
		TRACE("Invalid UTC time,do not set time before 1970-1-1 00:00:00.\r\n");
		ulTime = 0;
	}
    
	time_t ti = ulTime;
    if (localtime_r(&ti, pTm) != NULL)
    {
    	if (pYear != NULL)	
        {
            *pYear = strTm.tm_year + 1900;
    	}
        
    	if (pMonth != NULL)
    	{
            *pMonth = strTm.tm_mon + 1;
    	}
        
    	if (pDay != NULL)
        {
            *pDay = strTm.tm_mday;
    	}
        
    	if (pHour != NULL)
    	{
            *pHour = strTm.tm_hour;
    	}
        
    	if (pMinute != NULL)
        {
            *pMinute = strTm.tm_min;
    	}
        
    	if (pSecond != NULL)
        {   
            *pSecond = strTm.tm_sec;
    	}
    }
}

time_t DateTimeToTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
{
	time_t iTime;
	struct tm tmTime;

    tmTime.tm_year = iYear - 1900;
	tmTime.tm_mon = iMonth - 1;
	tmTime.tm_mday = iDay;
	tmTime.tm_hour = iHour;
	tmTime.tm_min = iMinute;
	tmTime.tm_sec = iSecond;

    iTime = mktime(&tmTime);
    if (iTime < 0)
	{
		TRACE("%04d-%02d-%02d %02d:%02d:%02d\r\n", iYear, iMonth, iDay, iHour, iMinute, iSecond);
		return 0;
	}
    
	return iTime;
}

static int MakeTimeString(char *pTimeBuff, int iBuffSize, unsigned int uiRcdTime)
{   
    int pYear = 0;
    int pMonth = 0;
    int pDay = 0;
    int pHour = 0;
    int pMinute = 0;
    int pSecond = 0;
    
    TimeToDateTime(uiRcdTime, &pYear, &pMonth, &pDay, &pHour, &pMinute, &pSecond);

    memset(pTimeBuff, 0, iBuffSize);
    
    snprintf(pTimeBuff, iBuffSize - 1, "%04d-%02d-%02d %02d:%02d:%02d", 
        pYear, pMonth, pDay, pHour, pMinute, pSecond);

    return 0;
}

static int getCurTime(char *timeBuffer, int size)
{
    if (!timeBuffer)
    {
        return -1;    

    }
    
	struct tm strTm;
	struct tm *pTm = &strTm;
	time_t ti = time(NULL);

	localtime_r(&ti, pTm);

    memset(timeBuffer, 0, size);
	strftime(timeBuffer, size - 1, "%Y-%m-%d %H:%M:%S", pTm);

    printf("* timebuffer->%s \r\n", timeBuffer);
    
    return 0;
}

int main()
{
    unsigned long tt = 1439284863;
    char buff[32] = {0};
    //MakeTimeString(buff, sizeof(buff), tt);

    getCurTime(buff, sizeof(buff));
    printf("########## buf: %s \r\n", buff);

    return 0;
}
