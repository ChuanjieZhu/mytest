
#ifndef __RECORD_LIB_H__
#define __RECORD_LIB_H__

#define TRACE printf 

#include "recordComm.h"

#define GENERAL_PATH_LEN			128      			    //·���ĳ���


#define STORAGE_PATH                "/mnt/storage"
#define STORAGE_MOUNT_FLASH_DIR		"/mnt/storage/flash"    //flash���ص�Ŀ¼
#define RECORD_DIR_NAME		        "record"		        //��¼���ݱ���·��

#define OPERATE_TYPE_RECORD			0x01				    //��������  ��¼��Ŀ

#define RECORD_TYPE_SECOND          0x01                    //��¼���ͣ���������¼
#define RECORD_TYPE_HOUR            0x02                    //��¼���ͣ���Сʱ�����¼

typedef enum operateStateE
{
	eOPERATE_STATUS_FREE = 0,			//����״̬
	eOPERATE_STATUS_RECORD,				//������¼
	eOPERATE_STATUS_USER,				//�����û�
}operateStateE;


/* ���ݿ�ȫ�ֽṹ��, ���ڱ������ݴ洢·�����洢״̬�� */
typedef struct __record_lib_t_
{
    int storageStatus;

    unsigned int flashTotalSize;
    unsigned int flashFreeSize;
    char flashPath[GENERAL_PATH_LEN];
    char recordPath[GENERAL_PATH_LEN];	    //��¼��������·��
    
    int curOperateStatus;
    int recordMaxBufNum;					//��¼��󻺳�����
	int recordSaveInterval;					//��¼������ʱ��
	int recordCurBufNum;					//��ǰ��¼��������
} record_lib_t;


/* �����ṹ�� */
typedef struct __file_index_t_
{
    unsigned int no;			    //��0��ʼ����
    char recordStatus;			    //��¼״̬  0������  1��ɾ��
    char recordType;                //��¼���ͣ�1-����Ϊ��λ�ļ�¼��2-��СʱΪ��λ�ļ�¼
    unsigned int inCount;           //������
    unsigned int outCount;          //������
    unsigned int recordTime;	    //��¼������ʱ��
	char unused[10];			    //�����ֽ�
} file_index_t;


/* ��ѯ�ļ�¼���� */
typedef struct __record_info_t_
{
	int recordNo;					//��ǰ��¼��
	file_index_t fileIndex;
	struct __record_info_t_ *next;
	struct __record_info_t_ *cur;
} record_info_t;

typedef struct __record_list_t_
{
	int recordNum;					//�ܼ�¼��
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

