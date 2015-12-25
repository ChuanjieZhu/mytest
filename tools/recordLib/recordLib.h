/*********************************************************************************************
 *	��¼��
 *	��Ҫ����ά�������ڴ����ϵ����м�¼�����ڼ�¼����¼�յȣ�
 *
 *	���ߣ���˫��
 *	����ʱ�䣺2012-01-30
 *********************************************************************************************/

#ifndef __RECORD_LIB_H__
#define __RECORD_LIB_H__

#include "../Comm.h"

#define GENERAL_PATH_LEN			128      			//·���ĳ���

#define MAX_PHOTO_FILE_SIZE			(30*1024)			//�����Ƭ��С
#define STORAGE_MOUNT_SD_DIR		"/mnt/storage/sd"   //sd�����ص�Ŀ¼
#define STORAGE_MOUNT_FLASH_DIR		"/mnt/storage/flash"//flash���ص�Ŀ¼
#define RECORD_INDEX_DIR_NAME		"rcdIndex"			//��¼��������·��
#define RECORD_PIC_DIR_NAME			"rcdPic"			//��¼��Ƭ����·��

#define OPERATE_TYPE_RECORD_PIC		0x01				//��������  ��¼��Ƭ
#define OPERATE_TYPE_RECORD			0x02				//��������  ��¼��Ŀ

#define RECORD_START_FLAG			"[FIRS_START]"		//��¼��Ƭ�Ŀ�ʼ���
#define RECORD_END_FLAG				"[FIRS_END]"		//��¼��Ƭ�Ľ������




/* ��¼ά����Ĵ������� */
typedef enum recordLibErrorTypeE
{
	ERROR_RECORDLIB_INDEX_PATH_NOT_DIR = -100,			//��¼��������·������ ����Ŀ¼
	ERROR_RECORDLIB_INDEX_PATH_NOT_CREATE,				//��¼��������·������ ���ܴ���
	ERROR_RECORDLIB_PIC_PATH_NOT_DIR,					//��¼��Ƭ����·������ ����Ŀ¼
	ERROR_RECORDLIB_PIC_PATH_NOT_CREATE,				//��¼��Ƭ����·������ ���ܴ���

	ERROR_RECORDLIB_OTHER = -1,							//���������
	ERROR_RECORDLIB_SUCCESS = 0,						//�ɹ�
}recordLibErrorTypeE;

typedef enum operateStateE
{
	eOPERATE_STATUS_FREE = 0,			//����״̬
	eOPERATE_STATUS_RECORD,				//������¼
	eOPERATE_STATUS_USER,				//�����û�
}operateStateE;

/******************************************
 * ���ݿ�ȫ�ֽṹ��
 * ���ڱ������ݴ洢·�����洢״̬��
 ******************************************/
typedef struct __record_lib_t_
{
	int storageStatus;						//״̬ 0������

	unsigned int sdCardTotalSize;			//�洢�ռ��ܴ�С
	unsigned int sdCardFreeSize;			//ʣ��ռ�
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//�洢�ռ��ܴ�С
	unsigned int flashFreeSize;				//ʣ��ռ�
	char flashPath[GENERAL_PATH_LEN];		//

	char recordIndexPath[GENERAL_PATH_LEN];	//��¼��������·��
	char recordPicPath[GENERAL_PATH_LEN];	//��¼��Ƭ����·��

	int curOperateStatus;					//��ǰ����״̬

	int recordMaxBufNum;					//��¼��󻺳�����
	int recordSaveInterval;					//��¼������ʱ��
	int recordCurBufNum;					//��ǰ��¼��������
}record_lib_t;

/* �����ṹ�� */
typedef struct __file_index_t_
{
	unsigned int no;			//��0��ʼ����
	char devNo;					//�豸��� �Ŷ�ǰ׺
	char fileType;				//ָ����ļ����� 1����¼��Ƭ
	char fileStatus;			//�ļ�״̬  0������  1��ɾ��
	char userType;				//�û�����
	unsigned int id;			//�û�Id
	int nServerID;
	int nChannelID;
	char recogType;				//ʶ������	ˢ��/һ�Զ�/...
	char recogStatus;			//ʶ���״̬  �ɹ�/ʧ��
	char recogScores;			//ʶ�����
	char recogReason;			//ʶ��ʽ ��ݼ�
	unsigned int recordTime;	//��¼������ʱ��

	unsigned int startAddr;		//ָ����ļ�����ʼ��ַ
	int fileSize;				//ָ��һ����Ƭ��¼�Ĵ�С
	char cRecogMode;			//ʶ��ģʽ
	char unused[11];			//�����ֽ�
}file_index_t;



/* ��ѯ�ļ�¼���� */
typedef struct __record_info_t_
{
	int recordNo;					//��ǰ��¼��
	file_index_t fileIndex;
	struct __record_info_t_ *next;
	struct __record_info_t_ *cur;
}record_info_t;

typedef struct __record_list_t_
{
	int recordNum;					//�ܼ�¼��
	record_info_t head;
}record_list_t;


#ifdef __cplusplus 
extern "C" { 
#endif

#include "../recordOutLib.h"

/******************************************************************************
 * �������ƣ� getRecordLibInfo
 * ���ܣ� ��ȡ��¼�����Ϣ
 * ������ p_record_lib����¼��ָ��
 * ���أ� ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void getRecordLibInfo(record_lib_t *p_record_lib);

/******************************************************************************
 * �������ƣ� initRecordLib
 * ���ܣ� ���ݴ������Ϣ��ʼ����¼��
 * ������ p_record_lib����¼��ָ��
 * ���أ� ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void initRecordLib(record_lib_t *p_record_lib);


/******************************************************************************
 * �������ƣ� deinitRecordLib
 * ���ܣ��ͷ��ѱ���ʼ���ļ�¼��
 * ������ ��
 * ���أ� ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void deinitRecordLib();


/******************************************************************************
 * �������ƣ� addRecord
 * ���ܣ���ӿ��ڼ�¼
 * ������record����¼��Ϣ
 * ���أ� ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int addRecord(operate_event_t record);



/******************************************************************************
 * �������ƣ� searchRecordsNum
 * ���ܣ����Ҽ�¼��������
 * ������searchConditionFlag:	��ѯ������� ͨ��searchConditionFlag�����Ʋ�ѯ����
         searchCondition:	��ѯ�����ľ�������
 * ���أ� >=0����¼���� �� -1��ʧ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int searchRecordsNum(unsigned int searchConditionFlag, search_condition_t *pSearchCondition);


/******************************************************************************
 * �������ƣ� getNextRecordInfo
 * ���ܣ���ȡ��һ����¼��Ϣ
 * ������pRecordList
 * ���أ� >=0����¼���� �� -1��ʧ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
file_index_t * getNextRecordInfo(record_list_t *pRecordList);


/******************************************************************************
 * �������ƣ� freeSearchRecord
 * ���ܣ��ͷŲ�ѯ�ļ�¼
 * ������pRecordList:	��ѯ�ļ�¼����
 * ���أ� ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void freeSearchRecord(record_list_t *pRecordList);


/******************************************************************************
 * �������ƣ� getOneRecordPhoto
 * ���ܣ���ȡĳ����¼����Ƭ
 * ������ recordTime:	������¼��ʱ��
          startAddr:	������¼��Ƭ���ݵ���ʼ��ַ
          pPhotoBuf:	��Ƭ�����buf
 * ���أ� 0���ɹ�  ������ʧ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int getOneRecordPhoto(unsigned int recordTime, unsigned int startAddr, char *pPhotoBuf);


/******************************************************************************
 * �������ƣ� lockWriteData
 * ���ܣ�д���� ����
 * ��������
 * ���أ���
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void lockWriteData();


/******************************************************************************
 * �������ƣ� unlockWriteData
 * ���ܣ� д���� ���� 
 * ��������
 * ���أ���
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void unlockWriteData();

/******************************************************************************
 * �������ƣ� showRecordInfo
 * ���ܣ� ��ʾ��¼��Ϣ
 * ��������
 * ���أ���
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void showRecordInfo();


/******************************************************************************
 * �������ƣ� showRecordQueueInfo
 * ���ܣ� ��ʾ��¼���������Ϣ
 * ��������
 * ���أ���
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void showRecordQueueInfo();


/******************************************************************************
 * �������ƣ� checkStorageCapacity
 * ���ܣ� ����豸ʹ������
 * ������pStoragePath	�洢�豸��·��
         pTotalSize	�洢�豸�ܵĴ�С
         pFreeSize	�洢�豸ʣ���ݼ�Ĵ�С
 * ���أ�-1 ���� 0 ���� 1 ʹ�ÿռ䳬��90% 
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int checkStorageCapacity(char *pStoragePath, unsigned int *pTotalSize, unsigned int *pFreeSize);



/******************************************************************************
 * �������ƣ� getRecordFlag
 * ���ܣ� ��ȡ���
 * ��������
 * ���أ����
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int getRecordFlag();



/******************************************************************************
 * �������ƣ� setRecordFlag
 * ���ܣ� ���ñ��
 * ������flag�����
 * ���أ�0:����  1:ǿ��д���¼ 2:�˳�д���¼�߳�
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void setRecordFlag(int flag);



/******************************************************************************
 * �������ƣ� GetRecordPicPath
 * ���ܣ� ��ȡ��¼��Ƭ����·��
 * ������ pPath:����·��
 * ���أ� 0���ɹ�;	-1��ʧ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int GetRecordPicPath(char* pPath);


/******************************************************************************
 * �������ƣ� GetRecordIndexPath
 * ���ܣ� ��ȡ��¼��������·��
 * ������ pPath	��¼����·���洢�ռ�
 * ���أ� 0���ɹ�;	-1��ʧ�ܣ�
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int GetRecordIndexPath(char* pPath);

#ifdef __cplusplus 
}
#endif

#endif//__RECORD_LIB_H__

