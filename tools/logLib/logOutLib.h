#ifndef __LOG_OUT_LIB_H_
#define __LOG_OUT_LIB_H_

#define LOG_CONDITION_ALL                   0x01
#define LOG_CONDITION_TIME                  0x02
#define LOG_CONDTITION_USERID                0x04
#define LOG_CONDTITION_USERNAME              0x08

typedef struct __syslog_search_condition_t_
{
	unsigned int startTime;			//��ʼʱ��
	unsigned int endTime;			//����ʱ��
	unsigned int startAddr;			//��ʼ��ַ
	int searchNum;					//Ҫ��ѯ����Ŀ��
}syslog_search_condition_t;

typedef struct __manlog_search_condition_t_
{
    unsigned int userNo;			//�û�����
    char userName[MAX_USER_NAME_LEN];   //�û�����
	unsigned int startTime;			//��ʼʱ��
	unsigned int endTime;			//����ʱ��
	unsigned int startAddr;			//��ʼ��ַ
	int searchNum;					//Ҫ��ѯ����Ŀ��
}manlog_search_condition_t;

typedef struct __accesslog_search_condition_t_
{
    unsigned int userId;
    unsigned int userNo;			//�û�����
    char userName[MAX_USER_NAME_LEN];   //�û�����
	unsigned int startTime;			//��ʼʱ��
	unsigned int endTime;			//����ʱ��
	unsigned int startAddr;			//��ʼ��ַ
	int searchNum;					//Ҫ��ѯ����Ŀ��
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

