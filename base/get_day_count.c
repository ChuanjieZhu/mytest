/**
 *
 *
 *
 **/

#include <stdio.h>
#include <time.h>

#define LEAP_YEAR(y) ((y) % 400 == 0 || ((y) % 100 && (y) % 4 == 0)) 
#define MAX_MONTHDAY(y, m) (((m) == 1 || (m) == 3 || (m) == 5 || (m) == 7 || (m) == 8 || (m) == 10 || (m) == 12) ? 31 : ((m) == 2 ? (LEAP_YEAR(y) ? 29 : 28) : 30))

int Getdaycount()
{
    int i = 0;
    int days = 0;
    struct tm tmTime;

    time_t tRefTime = time(NULL);

    gmtime_r(&tRefTime, &tmTime);

    int year = tmTime.tm_year + 1900;

    if (year < 1900)
    {
        return -1;
    }

    for (i = 2000; i < year; i++)
    {
        days += LEAP_YEAR(i) ? 366 : 365;
    }

    for (i = 1; i < tmTime.tm_mon + 1; i++)
    {
        days += MAX_MONTHDAY(year, i);
    }

    days += tmTime.tm_mday - 1;

    return days;
}

int main()
{
    int days = Getdaycount();

    printf("days %d \r\n", days);

    return 0;
}
