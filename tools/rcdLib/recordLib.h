
#ifndef __RECORD_LIB_H__
#define __RECORD_LIB_H__

#define TRACE printf 

#include "recordComm.h"

#define GENERAL_PATH_LEN			128      			    //路径的长度


#define STORAGE_PATH                "/mnt/storage"
#define STORAGE_MOUNT_FLASH_DIR		"/mnt/storage/flash"    //flash挂载的目录
#define RECORD_DIR_NAME		        "record"		        //记录数据保存路径

#define OPERATE_TYPE_RECORD			0x01				    //操作类型  记录条目

#define RECORD_TYPE_SECOND          0x01                    //记录类型，按秒间隔记录
#define RECORD_TYPE_HOUR            0x02                    //记录类型，按小时间隔记录

typedef enum operateStateE
{
	eOPERATE_STATUS_FREE = 0,			//空闲状态
	eOPERATE_STATUS_RECORD,				//操作记录
	eOPERATE_STATUS_USER,				//操作用户
}operateStateE;


/* 数据库全局结构体, 用于保存数据存储路径、存储状态等 */
typedef struct __record_lib_t_
{
    int storageStatus;

    unsigned int flashTotalSize;
    unsigned int flashFreeSize;
    char flashPath[GENERAL_PATH_LEN];
    char recordPath[GENERAL_PATH_LEN];	    //记录索引保存路径
    
    int curOperateStatus;
    int recordMaxBufNum;					//记录最大缓冲条数
	int recordSaveInterval;					//记录保存间隔时间
	int recordCurBufNum;					//当前记录缓冲条数
} record_lib_t;


/* 索引结构体 */
typedef struct __file_index_t_
{
    unsigned int no;			    //从0开始计数
    char recordStatus;			    //记录状态  0：正常  1：删除
    char recordType;                //记录类型，1-按秒为单位的记录，2-按小时为单位的记录
    unsigned int inCount;           //进人数
    unsigned int outCount;          //出人数
    unsigned int recordTime;	    //记录产生的时间
	char unused[10];			    //保留字节
} file_index_t;


/* 查询的记录链表 */
typedef struct __record_info_t_
{
	int recordNo;					//当前记录号
	file_index_t fileIndex;
	struct __record_info_t_ *next;
	struct __record_info_t_ *cur;
} record_info_t;

typedef struct __record_list_t_
{
	int recordNum;					//总记录数
	record_info_t head;
} record_list_t;


#ifdef __cplusplus
extern "C" {
#endif

#include "recordOutLib.h"

void InitRecordLib();

void getRecordLibInfo(record_lib_t *p_record_lib);

void initRecordLib(record_lib_t *p_record_lib);

void deinitRecordLib();

int addRecord(operate_event_t record);

void *writeRecordFlag(void *pArg);

int s_addSecondRecord();

int s_addHourRecord();

int addSecondRecord();

int addHourRecord();

int searchRecordsNum(unsigned int searchConditionFlag, search_condition_t *pSearchCondition);

file_index_t * getNextRecordInfo(record_list_t *pRecordList);

void freeSearchRecord(record_list_t *pRecordList);

void lockWriteData();

void unlockWriteData();

void showRecordInfo();

void showRecordQueueInfo();

int checkStorageCapacity(char *pStoragePath, unsigned int *pTotalSize, unsigned int *pFreeSize);

int getRecordFlag();

void setRecordFlag(int flag);

int getRecordPath(char* pPath);

#ifdef __cplusplus
}
#endif

#endif /* _RECORD_LIB_H_ */

