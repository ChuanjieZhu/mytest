
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void TimeToDateTimeEx(time_t time, char *pDatetime)
{
	if (pDatetime) 
    {
		struct tm strTm;
		struct tm *pTm = &strTm;
		time_t ti = time;

		localtime_r(&ti, pTm);
		if (pTm) 
        {
			sprintf(pDatetime, "%d-%02d-%02d %02d:%02d:%02d", 
				pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
		}
	}

	return;
}

int main(int argc, char *argv[])
{
    char buff[32] = {0};

    if (argc != 2)
    {
        printf(" usage timeformat <time> \n");
        return -1;
    }

    unsigned long tt = strtoul(argv[1], NULL, 10);
    
    TimeToDateTimeEx((time_t)tt, buff);

    printf("time = %s \n", buff);

    return -1;
}

