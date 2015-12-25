
#ifndef __RECORD_OUT_LIB_H__
#define __RECORD_OUT_LIB_H__

#define     RECORD_CONDITION_TYPE				0x01 //根据记录类型查询, 秒间隔记录已小时间隔记录

typedef struct __search_condition_t_
{
    char recordType;                //记录类型,秒间隔记录和小时间隔记录
	unsigned int startTime;			//起始时间
	unsigned int endTime;			//结束时间
	unsigned int startAddr;			//起始地址
	int searchNum;					//要查询的条目数
} search_condition_t;


#ifdef __cplusplus
extern "C" {
#endif

int searchRecord(unsigned int searchConditionFlag, search_condition_t *pSearchCondition, record_list_t *pRecordList);

#ifdef __cplusplus
}
#endif

#endif /* __RECORD_OUT_LIB_H__ */

