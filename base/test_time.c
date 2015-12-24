#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
    unsigned short expDays = 0;
    
    unsigned short year = 0;
    unsigned char month = 0;
    unsigned char day = 0;

    char tmpBuf[5] = {0};
    char buf[26] = {0};

    snprintf(buf, sizeof(buf), "%s", "0810812000019201201010365");
    
    char *ptr = buf;
    
    printf("ptr %s \r\n", ptr);

    sscanf(&buf[13], "%4s", tmpBuf);
    year = atoi(tmpBuf);
    
    printf("tmpBuf %s year %d \r\n", tmpBuf, year);
    
    sscanf(ptr+17, "%2s", tmpBuf);
    month = atoi(tmpBuf);

    sscanf(ptr+19, "%2s", tmpBuf);
    day = atoi(tmpBuf);

    sscanf(ptr+21, "%4s", tmpBuf);
    expDays = atoi(tmpBuf);

    printf("year %d month %d day %d expDays %d\r\n", year, month, day, expDays);

    return 0;
}
