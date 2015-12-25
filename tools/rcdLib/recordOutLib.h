
#ifndef __RECORD_OUT_LIB_H__
#define __RECORD_OUT_LIB_H__

#define     RECORD_CONDITION_TYPE				0x01 //���ݼ�¼���Ͳ�ѯ, ������¼��Сʱ�����¼

typedef struct __search_condition_t_
{
    char recordType;                //��¼����,������¼��Сʱ�����¼
	unsigned int startTime;			//��ʼʱ��
	unsigned int endTime;			//����ʱ��
	unsigned int startAddr;			//��ʼ��ַ
	int searchNum;					//Ҫ��ѯ����Ŀ��
} search_condition_t;


#ifdef __cplusplus
extern "C" {
#endif

int searchRecord(unsigned int searchConditionFlag, search_condition_t *pSearchCondition, record_list_t *pRecordList);

#ifdef __cplusplus
}
#endif

#endif /* __RECORD_OUT_LIB_H__ */

