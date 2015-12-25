
#ifndef __RECORD_COMM_H__
#define __RECORD_COMM_H__

/* 记录信息 */
typedef struct __operate_event_t_
{
	unsigned int operateType;       //类型  bit0：数据库操作
	unsigned int recordTime;        //记录产生的时间
	char recordType;        //记录类型，0-秒间隔，1-小时间隔
    unsigned int inCount;     //进人数
    unsigned int outCount;    //出人数
} operate_event_t;

#define MAX_TMP_LEN 512

#endif  /* __COMM_H__ */

