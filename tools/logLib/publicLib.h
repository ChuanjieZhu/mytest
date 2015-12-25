#ifndef __PUBLIC_LIB_H_
#define __PUBLIC_LIB_H_

#define MAX_LOG_PER_QUERY    2000

#define BOOL int
#define TRUE 1
#define FALSE 0

unsigned int DatetimeToTime(char *pDatetime);
unsigned int TimeToNextDay(unsigned int uiInTime);

#endif 

