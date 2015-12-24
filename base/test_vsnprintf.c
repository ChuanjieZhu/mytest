
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

char vspf_buff[1024] = {0};

typedef enum
{
    TRACE_OK = 0x00,
    TRACE_ERR = 0x01
    
} TRACEcode;

int vspf(char *fmt, ...)
{
    va_list argptr;
    int cnt;
    va_start(argptr, fmt);
    cnt = vsnprintf(vspf_buff, sizeof(vspf_buff) - 1, fmt, argptr);
    va_end(argptr);

    return cnt;
}

int Trace(int iTraceCode, char *pFormat, ...)
{
    int iRet = -1;
    static char acTmpBuff[512] = {0};
    va_list Argptr;

    if (pFormat != NULL)
    {
        if (iTraceCode == TRACE_OK)
        {
            snprintf(acTmpBuff, sizeof(acTmpBuff) - 1, "* %s %%s %%d\r\n", pFormat);
        }
        else 
        {
            snprintf(acTmpBuff, sizeof(acTmpBuff) - 1, "> %s %%s %%d\r\n", pFormat);
        }
    }
    
    va_start(Argptr, pFormat);
    iRet = vsnprintf(vspf_buff, sizeof(vspf_buff) - 1, acTmpBuff, Argptr);
    va_end(Argptr);

    printf("%s",  vspf_buff);
    
    return iRet;
}

int time_convert(long time, char *buffp, size_t buffsize)
{
    if (buffp) 
    {
		struct tm strTm;
		struct tm *pTm = &strTm;
		time_t ti = time;

		localtime_r(&ti, pTm);
		if (pTm) 
        {
			snprintf(buffp, buffsize - 1, "%d-%02d-%02d %02d:%02d:%02d", 
				pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

            return 0;
		}

        return -1;
	}

	return -1;    
}



int main()
{
    char time_buff[20] = {0};

    int i;

#if 0
    for (i = 0; i < 10; i++)
    {
        time_convert(time(0), time_buff, sizeof(time_buff));
        vspf("[%s-%s-%d] %d\r\n", time_buff, __FUNCTION__, __LINE__, i);
        printf("%s", vspf_buff);
        sleep(1);
    }
#endif

    Trace(TRACE_OK, "OOOOOOO. %s %d", "sssssss", 1000, __FUNCTION__, __LINE__);

    return 0;
}


