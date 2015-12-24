#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void test_sscanf_0()
{
    char acName[128] = {0};
    char buf[] = "2013-11-08_095730_2012110612000007223.jpg";

    sscanf(buf, "%s.jpg", acName);

    printf("acName: %s \r\n", acName);
}

void test_sscanf_1()
{
    struct tm strTm;
    time_t iCurTime = time(NULL);
    localtime_r(&iCurTime, &strTm);

    int iCurDate = ((strTm.tm_year + 1900) * 10000) + ((strTm.tm_mon + 1) * 100) + strTm.tm_mday;
    strTm.tm_hour = 0;
    strTm.tm_min = 0;
    strTm.tm_sec = 0;

    iCurTime = mktime(&strTm);

    printf("iCurTime %d \r\n", iCurTime);

    int iDate = 0; 
    sscanf("20131021.index", "%d.index", &iDate);
    printf("iDate %d \r\n", iDate);
    struct tm tm;
    tm.tm_year = iDate / 10000 - 1900;
    tm.tm_mon = (iDate % 10000) / 100 - 1;
    tm.tm_mday = (iDate % 10000) % 100;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    time_t tBaseTime = mktime(&tm);
    //printf("year: %d, month: %d, day: %d \r\n", iYear, iMon, iDay);
    printf("tbasetime: %d \r\n", tBaseTime);

    if (iCurTime == tBaseTime)
    {
        printf("time equal !################ \r\n");
    }
}

int main()
{
    test_sscanf_0();
    test_sscanf_1();
    return 0;
}


