/**
 *
 *
 *
 *
 **/

#include <time.h>
#include <stdio.h>

typedef struct _ourTIME
{
    int year;
    int mon;
    int day;
    int hour;
    int min;
    int sec;
    int hsec;
} ourTIME;

void our_time_now(ourTIME *stm)
{ 
    time_t lt;
    struct tm *tt;

    time(&lt);
    tt = gmtime(&lt);

    stm->year = tt->tm_year;
    stm->mon = tt->tm_mon;
    stm->day = tt->tm_mday;
    stm->hour = tt->tm_hour;
    stm->min = tt->tm_min;
    stm->sec = tt->tm_sec;
    stm->hsec = 0;
}

void our_time_print(ourTIME *stm)
{
    if(stm != NULL)
    {
        printf("year---> %d\r\n", stm->year + 1900);
        printf("mon----> %d\r\n", stm->mon + 1);
        printf("day----> %d\r\n", stm->day);
        printf("hour---> %d\r\n", stm->hour + 8);
        printf("min----> %d\r\n", stm->min);
        printf("sec----> %d\r\n", stm->sec);
    }
}

int main()
{
    ourTIME ourtime;
    our_time_now(&ourtime);
    our_time_print(&ourtime);
    return 0;
}
