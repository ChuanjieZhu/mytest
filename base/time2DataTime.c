#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
void TimeToDataTime(unsigned long time, char *pDataTime)
{
    if (pDataTime)
    {
        struct tm pTm;
        time_t ti = (time_t)time;

        if (localtime_r(&ti, &pTm) != NULL)
        {
            sprintf(pDataTime, "%d-%02d-%02d %02d:%02d:%02d",
                    pTm.tm_year+1900, pTm.tm_mon+1, pTm.tm_mday, pTm.tm_hour, pTm.tm_min, pTm.tm_sec);
        }
    }

    return;
}


int main()
{
    unsigned long t = 1375457585;
    char data[32] = {0};
    TimeToDataTime(t, data);
    printf("Time: %s \r\n", data);

    return 0;
}
