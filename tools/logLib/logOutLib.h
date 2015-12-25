#ifndef __LOG_OUT_LIB_H_
#define __LOG_OUT_LIB_H_

#define LOG_CONDITION_ALL                   0x01
#define LOG_CONDITION_TIME                  0x02
#define LOG_CONDTITION_USERID                0x04
#define LOG_CONDTITION_USERNAME              0x08

typedef struct __syslog_search_condition_t_
{
	unsigned int startTime;			//起始时间
	unsigned int endTime;			//结束时间
	unsigned int startAddr;			//起始地址
	int searchNum;					//要查询的条目数
}syslog_search_condition_t;

typedef struct __manlog_search_condition_t_
{
    unsigned int userNo;			//用户工号
    char userName[MAX_USER_NAME_LEN];   //用户姓名
	unsigned int startTime;			//起始时间
	unsigned int endTime;			//结束时间
	unsigned int startAddr;			//起始地址
	int searchNum;					//要查询的条目数
}manlog_search_condition_t;

typedef struct __accesslog_search_condition_t_
{
    unsigned int userId;
    unsigned int userNo;			//用户工号
    char userName[MAX_USER_NAME_LEN];   //用户姓名
	unsigned int startTime;			//起始时间
	unsigned int endTime;			//结束时间
	unsigned int startAddr;			//起始地址
	int searchNum;					//要查询的条目数
}accesslog_search_condition_t;


#ifdef __cplusplus 
extern "C" {
#endif

int searchSysLog(unsigned int searchCondFlag, syslog_search_condition_t *pSearchCondition, syslog_list_t *pSysLogList);
int searchManLog(unsigned int searchCondFlag, manlog_search_condition_t *pSearchCondition, manlog_list_t *pManLogList);

void freeSearchSysLog(syslog_list_t * pLogList);
void freeSearchManLog(manlog_list_t * pLogList);


int searchSysLogTotal();
int searchManLogTotal();





#ifdef __cplusplus 
}
#endif

#endif

