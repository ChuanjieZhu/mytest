
#if 0
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#endif

#include "recordLib.h"
#include "recordInfo.h"
#include "eventQueue.h"

record_lib_t g_record_lib;
static pthread_t g_saveRecordId;
static pthread_mutex_t g_queueMutex;    //����������
static pthread_mutex_t g_writeMutex;    //д�������
static LinkQueue g_operateQueue;        //����
static int g_recordFlag = 0;            //ǿ��д����

#define Malloc malloc
#define Free free
#define TRUE 1
#define FALSE 0
#define BOOL int

time_t DatetimeToTime(char *pDatetime)
{
	struct tm tmTime;
	time_t ti = 0;

	if (pDatetime) 
    {
		sscanf(pDatetime, 
            "%d-%02d-%02d %02d:%02d:%02d", 
			&tmTime.tm_year, 
			&tmTime.tm_mon, 
			&tmTime.tm_mday, 
			&tmTime.tm_hour, 
			&tmTime.tm_min, 
			&tmTime.tm_sec);
        
		tmTime.tm_year -= 1900;
		tmTime.tm_mon -= 1;
		ti = mktime(&tmTime);
	}

	return ti;
}


unsigned int TimeToNextDay(unsigned int uiInTime)
{
	/* �ѵ����ʱ���Ϊ�賿����2012-07-19 00:00:00 �ٵ��ڶ�����賿��ʼ*/
	char timeBuf[32] = {0};
	time_t tt;
	struct tm Tm;
	struct tm *pTm = &Tm;
	unsigned int uiRetTime;
	
	tt = uiInTime;
  	localtime_r(&tt, pTm);

	sprintf(timeBuf, "%d-%02d-%02d %02d:%02d:%02d", pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, 0, 0, 0);
	uiRetTime = DatetimeToTime(timeBuf);
	uiRetTime += (24 * 60 * 60);

	return uiRetTime;
}


int getRecordPath(char* pPath)
{
	if (pPath == NULL)
	{
		return -1;
	}

	strcpy(pPath, g_record_lib.recordPath);

	return 0;
}


/* ��ȡ���������� */
static int getLastFileIndexNo(char *pIndexPath)
{
	int fd = 0;
	file_index_t index;
	int n = 0;
	int ret = 0;

	fd = open(pIndexPath, O_RDONLY | O_NOATIME, 0666);
	if(fd > 0)
	{
		lseek(fd, sizeof(file_index_t), SEEK_END);
		memset(&index, 0, sizeof(file_index_t));
		n = read(fd, &index, sizeof(file_index_t));
		if(n == sizeof(file_index_t))
		{
			ret = index.no;
		}
		close(fd);
		fd = 0;
	}

	return ret;
}


/* �������� ���� */
static void lockOperateQueue()
{
	pthread_mutex_lock(&g_queueMutex);
}

/* �������� ���� */
static void unlockOperateQueue()
{
	pthread_mutex_unlock(&g_queueMutex);
}

/* ���洢�豸������ */
int checkStorageCapacity(char *pStoragePath, unsigned int *pTotalSize, unsigned int *pFreeSize)
{
	int ret = 0;
	struct statvfs st;

	if(pStoragePath == NULL)
	{
		return -1;
	}

	if (statvfs(pStoragePath, &st) == 0)
	{
		//ʹ�ÿռ䳬��90%
		if (((float)st.f_blocks-(float)st.f_bfree)/(float)st.f_blocks*100 > 90.0f)
		{
			TRACE("sd total block %ld, Free block %ld\r\n", st.f_blocks, st.f_bfree);
			
			ret = 1;
		}
		
		if(pTotalSize != NULL)
		{
			*pTotalSize = st.f_blocks * st.f_bsize;
		}

		if(pFreeSize != NULL)
		{
			*pFreeSize = st.f_bfree * st.f_bsize;
		}
	}
	else
	{
		if(pTotalSize != NULL)
		{
			*pTotalSize = 0;
		}
		
		if(pFreeSize != NULL)
		{
			*pFreeSize = 0;
		}
	}

	return ret;
}


static void delOldRecordFile(char *path)
{
	DIR *pDir;
	char oldFileName[32] = {0};
	char oldFilePath[128] = {0};
	char tmpFilePath[128] = {0};
	struct dirent *dp = NULL;
	struct stat st;
	time_t tt;
	struct tm strTm;
	struct tm *pTm = &strTm;
	char curFileName[32] = {0};

	pDir = opendir(path);
	if(pDir != NULL)
	{
		while((dp = readdir(pDir)) != NULL)
		{
			if ((strcmp(dp->d_name,".") == 0) || (strcmp(dp->d_name,"..")==0))
			{
				continue;
			}

			memset(tmpFilePath, 0 ,sizeof(tmpFilePath));
			sprintf(tmpFilePath, "%s/%s", path, dp->d_name);

			if(0 == stat(tmpFilePath, &st))
			{
				//��Ŀ¼
				if(S_ISDIR(st.st_mode))
				{
					continue;
				}
				else 
				{
					if(strlen(oldFileName) == 0)
					{
						memcpy(oldFileName, dp->d_name, sizeof(oldFileName));
					}
					else
					{
						//���ļ�����
						if(memcmp(dp->d_name, oldFileName, sizeof(oldFileName)) < 0)
						{
							memset(oldFileName, 0, sizeof(oldFileName));
							memcpy(oldFileName, dp->d_name, sizeof(oldFileName));
						}
					}
				}
			}
		}

		if (strlen(oldFileName) > 0)
		{
			tt = time(0);
			localtime_r(&tt, pTm);
			
			sprintf(curFileName, "%04d%02d%02d.data", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday);

			TRACE("curFileName %s oldFileName %s %s %d\r\n", curFileName, oldFileName, __FUNCTION__, __LINE__);

			if(memcmp(oldFileName, curFileName, sizeof(curFileName)) < 0)
			{
				sprintf(oldFilePath, "%s/%s", path, oldFileName);
				unlink(oldFilePath);

				/* ɾ����¼�ļ������»�ȡ��¼���������� */
				//setRecordTotal(searchRecordTotal());
			}
		}
		
		closedir(pDir);
	}
}


/* д���� ���� */
void lockWriteData()
{
	pthread_mutex_lock(&g_writeMutex);
}

/* д���� ���� */
void unlockWriteData()
{
	pthread_mutex_unlock(&g_writeMutex);
}

/* ��ȡǿ��д��� */
int getRecordFlag()
{
	return g_recordFlag;
}

/* ����ǿ��д��� */
void setRecordFlag(int flag)
{
	lockWriteData();
	
	g_recordFlag = flag;
	
	unlockWriteData();
}


int addRecord(operate_event_t record)
{
	if((record.operateType & OPERATE_TYPE_RECORD == 0)
        || (record.recordType & RECORD_TYPE_SECOND == 0
            && record.recordType & RECORD_TYPE_HOUR == 0))
	{
        TRACE("addRecord Error! %s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	lockOperateQueue();
    
	EnQueue(&g_operateQueue, record);

	unlockOperateQueue();
	
	return 0;
}


int s_addSecondRecord()
{
    int iRet = 0;
    //CONFIG_BASE_STR strCfgBase;
    time_t tCurTime = 0;
    static time_t tLastSaveTime = 0;
    int rcdSecond = 0;
    int inCount = 0;             //��ǰ��������
    int outCount = 0;            //��ǰ��������   
    int lastInCount = 0;         //��һ�α����¼ʱ��������
    int lastOutCount = 0;        //��һ�α����¼ʱ��������
    static int count = 0;

    /* ÿ5�����һ��time */
    count++;
    if (count % 125 == 0)
    {
        tCurTime = time(0);
        count = 0;
    }
    
    //memset(&strCfgBase, 0, sizeof(strCfgBase));
    //GetConfigBase(&strCfgBase);
    
    rcdSecond = 60;//strCfgBase.rcdSecond;
    if (tCurTime - tLastSaveTime >= rcdSecond)
    {
        tLastSaveTime = tCurTime;

        //��ȡ��ǰ������
        //s_GetChannelPeopleNum(&inCount, &outCount);

        //��ȡ�ϴα����¼ʱ������
        //GetSecondLastPeopleNum(&lastInCount, &lastOutCount);
        
        operate_event_t record;
        record.operateType = OPERATE_TYPE_RECORD;
        record.recordTime = tCurTime;
        record.recordType = RECORD_TYPE_SECOND;
        record.inCount = abs(inCount - lastInCount);
        record.outCount = abs(outCount - lastOutCount);

        TRACE("inCount:%d, outCount:%d %s %d \r\n", record.inCount, record.outCount, __FUNCTION__, __LINE__);
        
        iRet = addRecord(record);
        if (0 == iRet)
        {
            //SetSecondLastPeopleNum(inCount, outCount);
        }
    }

    return iRet;
}


int s_addHourRecord()
{
    int iRet = 0;
    //CONFIG_BASE_STR strCfgBase;
    time_t tCurTime = 0;
    static time_t tLastSaveTime = 0;
    int rcdHour = 0;
    int inCount = 0;             //��ǰ��������
    int outCount = 0;            //��ǰ��������   
    int lastInCount = 0;         //��һ�α����¼ʱ��������
    int lastOutCount = 0;        //��һ�α����¼ʱ��������
    static int count = 0;

    /* ÿ4�����һ��time */
    count++;
    if (count % 125 == 0)
    {
        tCurTime = time(NULL);
        count = 0;
    }
    
    //memset(&strCfgBase, 0, sizeof(strCfgBase));
    //GetConfigBase(&strCfgBase);
    rcdHour = 1;
    
    if (tCurTime - tLastSaveTime >= (rcdHour * 3600))
    {
        tLastSaveTime = tCurTime;

        //��ȡ��ǰ������
        //s_GetChannelPeopleNum(&inCount, &outCount);

        //��ȡ�ϴα����¼ʱ������
        //GetHourLastPeopleNum(&lastInCount, &lastOutCount);
        
        operate_event_t record;
        record.operateType = OPERATE_TYPE_RECORD;
        record.recordTime = tCurTime;
        record.recordType = RECORD_TYPE_HOUR;
        record.inCount = abs(inCount - lastInCount);
        record.outCount = abs(outCount - lastOutCount);

        TRACE("inCount:%d, outCount:%d %s %d \r\n", record.inCount, record.outCount, __FUNCTION__, __LINE__);
        
        iRet = addRecord(record);
        if (0 == iRet)
        {
            //SetHourLastPeopleNum(inCount, outCount);
        }
    }

    return iRet;
}


 
int addSecondRecord()
{
    int iRet = 0;
    //CONFIG_BASE_STR strCfgBase;
    time_t tCurTime = 0;
    static time_t tLastSaveTime = 0;
    int rcdSecond = 0;
    int iCurIn = 0;
    int iCurOut = 0;
    int iLastIn = 0;
    int iLastOut = 0;
    static int count = 0;

    /* ÿ4�����һ��time */
    count++;
    if (count % 125 == 0)
    {
        tCurTime = time(NULL);
        count = 0;
    }

    //memset(&strCfgBase, 0, sizeof(strCfgBase));
    //GetConfigBase(&strCfgBase);
    rcdSecond = 60;//strCfgBase.rcdSecond;
    
    if (tCurTime - tLastSaveTime >= rcdSecond)
    {
        tLastSaveTime = tCurTime;
        //GetSecondLastPeopleNum(&iLastIn, &iLastOut);
        //GetTotalPeopleNum(&iCurIn, &iCurOut);
        
        operate_event_t record;
        record.operateType = OPERATE_TYPE_RECORD;
        record.recordTime = tCurTime;
        record.recordType = RECORD_TYPE_SECOND;
        record.inCount = abs(iCurIn - iLastIn);
        record.outCount = abs(iCurOut - iLastOut);
        
        TRACE("record.inCount:%d, record.outCount:%d %s %d \r\n", record.inCount, record.outCount, __FUNCTION__, __LINE__);
        
        iRet = addRecord(record);
        if (iRet == 0)
        {
            //SetSecondLastPeopleNum(iCurIn, iCurOut);
        }
    }

    return iRet;
}


int addHourRecord()
{
    int iRet = 0;
    //CONFIG_BASE_STR strCfgBase;
    time_t tCurTime = 0;
    static time_t tLastSaveTime = 0;
    int rcdHour = 0;
    int iCurIn = 0;
    int iCurOut = 0;
    int iLastIn = 0;
    int iLastOut = 0;
    static int count = 0;

    /* ÿ5�����һ��time */
    count++;
    if (count % 125 == 0)
    {
        tCurTime = time(NULL);
        count = 0;
    }
    
    //memset(&strCfgBase, 0, sizeof(strCfgBase));
    //GetConfigBase(&strCfgBase);
    rcdHour = 1;//strCfgBase.rcdHour;

    if (tCurTime - tLastSaveTime >= (rcdHour * 3600))
    {
        tLastSaveTime = tCurTime;
        //GetHourLastPeopleNum(&iLastIn, &iLastOut);
        //GetTotalPeopleNum(&iCurIn, &iCurOut);
        
        operate_event_t record;
        record.operateType = OPERATE_TYPE_RECORD;
        record.recordTime = tCurTime;
        record.recordType = RECORD_TYPE_HOUR;
        record.inCount = abs(iCurIn - iLastIn);
        record.outCount = abs(iCurOut - iLastOut);

        TRACE("record.inCount:%d, record.outCount:%d %s %d \r\n", record.inCount, record.outCount, __FUNCTION__, __LINE__);
                
        iRet = addRecord(record);
        if (iRet == 0)
        {
            //SetHourLastPeopleNum(iCurIn, iCurOut);
        }
    }

    return iRet;
}



/***********************************************************************
 * ���Ҽ�¼����ѯ��Ϻ������freeSearchRecord()���������ͷ�,�������ͷ�
 * startTime:		��ʼʱ�� ������ģ�
 * endTime:		����ʱ�� ������ģ�
 *
 * searchConditionFlag:	��ѯ������� ͨ��searchConditionFlag�����Ʋ�ѯ����
 * searchCondition:	��ѯ�����ľ�������
 *
 * pRecordInfo:		���صĲ�ѯ�������
 ***********************************************************************/
int searchRecord(unsigned int searchConditionFlag, search_condition_t *pSearchCondition, record_list_t *pRecordList)
{
	int n = 0;
	int indexFd = 0;
	struct tm Tm;
	struct tm *pTm = &Tm;
	time_t tt;
	unsigned int tmpTime = 0;
	unsigned int tmpEndTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	file_index_t fileIndex;
	record_info_t *pTmpInfo = NULL;
	record_info_t *pRecordInfo = NULL;
	int recordNo = 0;
	int recordNum = 0;
	//��ѯ�������   =1:��ʾ��ѯ����
	int searchOverFlag = 0;
	//����ʱ����ǰ�ļ��Ѷ�ȡ��λ��
	int searchOverAddr = 0;
	
	/* ֻҪ���м�¼��ѯ */
	setRecordFlag(TRUE);

    if(pRecordList == NULL)
	{
		return -1;
	}

	if(pSearchCondition == NULL)
	{
		return -2;
	}

	//ʱ��Ƿ�
	if (pSearchCondition->startTime > pSearchCondition->endTime)
	{
        TRACE("pSearchCondition->startTime %u, pSearchCondition->endTime %u %s %d\r\n", pSearchCondition->startTime, pSearchCondition->endTime, __FUNCTION__, __LINE__);
		return -3;
	}

	pRecordInfo = &pRecordList->head;

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	//��ʱ��ת����20120224��ʽ
	tmpEndTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;

	while (1)
	{
		tt = pSearchCondition->startTime;
		localtime_r(&tt, pTm);
		tmpTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;
		if(tmpTime > tmpEndTime)
		{
			break;
		}
        
		sprintf(indexFile, "%s/%08d.data", g_record_lib.recordPath, tmpTime);

		indexFd = open(indexFile, O_RDONLY | O_NOATIME, 0666);
		if(indexFd <= 0)
		{
			pSearchCondition->startAddr = 0;
			pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
			continue;
		}

		if(pSearchCondition->startAddr > 0)
		{
			lseek(indexFd, pSearchCondition->startAddr, SEEK_SET);
		}
		
		searchOverAddr = 0;
        
		while(1)
		{
			n = read(indexFd, &fileIndex, sizeof(file_index_t));
			if (n != sizeof(file_index_t))
			{
				break;
			}

			searchOverAddr += sizeof(file_index_t);

            TRACE("pSearchCondition->startTime: %d, pSearchCondition->endTime: %d \r\n", 
                        pSearchCondition->startTime, pSearchCondition->endTime);

            TRACE("fileIndex.recordTime: %d\r\n", fileIndex.recordTime);
            
			//��¼�Ϸ�
			if ((fileIndex.recordTime >= pSearchCondition->startTime) 
				&& (fileIndex.recordTime <= pSearchCondition->endTime))
			{
                #if 0
				if (((searchConditionFlag & RECORD_CONDITION_TYPE) > 0) 
					&& (fileIndex.recordType != pSearchCondition->recordType))
				{
                    TRACE("============ %s %d\r\n", __FUNCTION__, __LINE__);    
					continue;
				}
                #endif

                printf("########## %s %d\r\n", __FUNCTION__, __LINE__);
                
				/* ��¼��ɾ�� */
				if (fileIndex.recordStatus == 1)
				{   
					continue;
				}

				pTmpInfo = (record_info_t *)Malloc(sizeof(record_info_t));
				if(pTmpInfo != NULL)
				{
					memset(pTmpInfo, 0, sizeof(record_info_t));
					memcpy(&pTmpInfo->fileIndex, &fileIndex, sizeof(file_index_t));
					
					recordNo++;
					recordNum++;
					
					pTmpInfo->recordNo = recordNo;
					pRecordList->recordNum = recordNum;
					pRecordInfo->next = pTmpInfo;
					
					if (recordNo == 1)
					{
						pRecordInfo->cur = pTmpInfo;
					}
					
					pRecordInfo = pTmpInfo;

					//�жϲ�ѯ�ļ�¼���Ƿ�ﵽҪ��ļ�¼�����ﵽ���򱾴β�ѯ����
					if ((pSearchCondition->searchNum > 0) 
						&& (recordNum >= pSearchCondition->searchNum))
					{
						TRACE("search record num %d %s %d\r\n", recordNum, __FUNCTION__, __LINE__);
						
						searchOverFlag = 1;
						
						break;
					}
				}
			}
		}
		
		if(indexFd > 0)
		{
			close(indexFd);
			indexFd = 0;
		}

        printf("########## %s %d\r\n", __FUNCTION__, __LINE__);
        
		//��ѯ����
		if(searchOverFlag == 1)
		{
			pSearchCondition->startAddr += searchOverAddr;
			
			break;
		}

        printf("########## %s %d\r\n", __FUNCTION__, __LINE__);
            
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);

        printf("########## %s %d\r\n", __FUNCTION__, __LINE__);
        
		//�����ˣ����Խ���ʼ��ַ����
		pSearchCondition->startAddr = 0;
	}

	return 0;
}

/***********************************************************************
 * ���Ҽ�¼��������
 * searchConditionFlag:	��ѯ������� ͨ��searchConditionFlag�����Ʋ�ѯ����
 * searchCondition:	��ѯ�����ľ�������
 *
 * ����ֵ�� >=0����¼���� �� -1��ʧ��
 ***********************************************************************/
int searchRecordsNum(unsigned int searchConditionFlag, search_condition_t *pSearchCondition)
{
	int n = 0;
	//��ż�¼����
	int indexFd = 0;
	struct tm Tm;
	struct tm *pTm = &Tm;
	time_t tt;
	unsigned int tmpTime = 0;
	unsigned int tmpEndTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	file_index_t fileIndex;
	//��¼����
	int recordNum = 0;
	char timeBuf[32];

	if(pSearchCondition == NULL)
	{
		return -1;
	}

	//ʱ��Ƿ�
	if(pSearchCondition->startTime > pSearchCondition->endTime)
	{
		return -2;
	}

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	//��ʱ��ת����20120224��ʽ
	tmpEndTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;

	memset(timeBuf, 0, 32);

	while (1)
	{
		tt = pSearchCondition->startTime;
		localtime_r(&tt, pTm);
		tmpTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;
		if(tmpTime > tmpEndTime)
		{
			break;
		}
		
		sprintf(indexFile, "%s/%08d.index", g_record_lib.recordPath, tmpTime);
		indexFd = open(indexFile, O_RDONLY | O_NOATIME, 0666);
		if (indexFd < 0)
		{
			pSearchCondition->startAddr = 0;
			pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
			continue;
		}
		
		while(1)
		{
			n = read(indexFd, &fileIndex, sizeof(file_index_t));
			if(n != sizeof(file_index_t))
			{
				break;
			}

			//��¼�Ϸ�
			if ((fileIndex.recordTime >= pSearchCondition->startTime) 
				&& (fileIndex.recordTime <= pSearchCondition->endTime))
			{
				if (((searchConditionFlag & RECORD_CONDITION_TYPE) > 0) 
					&& (fileIndex.recordType != pSearchCondition->recordType))
				{
					continue;
				}

				/* ��¼��ɾ�� */
				if (fileIndex.recordStatus == 1)
				{
					continue;
				}
				
				recordNum++;
			}
		}
		
		if(indexFd > 0)
		{
			close(indexFd);
			indexFd = 0;
		}
		
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
		
		pSearchCondition->startAddr = 0;//�����ˣ����Խ���ʼ��ַ����
	}

	return recordNum;
}


/* ��ȡ��һ����¼��Ϣ */
file_index_t * getNextRecordInfo(record_list_t *pRecordList)
{
	record_info_t *pRecordInfo = NULL;
	file_index_t *pFileIndex = NULL;

	if(pRecordList == NULL)
	{
		return NULL;
	}

	pRecordInfo = pRecordList->head.cur;
	if(pRecordInfo == NULL)
	{
		return NULL;
	}

	pFileIndex = &pRecordInfo->fileIndex;
	pRecordList->head.cur = pRecordInfo->next;

	return pFileIndex;
}

/* �ͷŲ�ѯ�ļ�¼ */
void freeSearchRecord(record_list_t *pRecordList)
{
	record_info_t *pRecordInfo = NULL;

	if(pRecordList != NULL)
	{
		while(pRecordList->head.next)
		{
			pRecordInfo = pRecordList->head.next;
			pRecordList->head.next = pRecordInfo->next;
			
			Free(pRecordInfo);
			pRecordInfo = NULL;
		}
	}
}


/* ��ʾ��¼���������Ϣ */
void showRecordQueueInfo()
{
	QueueTraverse(g_operateQueue);
}

void showRecordInfo()
{
	TRACE("record lib info:\r\n");
	TRACE("	storage status:		%d\r\n", g_record_lib.storageStatus);
	TRACE("	flash mount path:	(%s)\r\n", g_record_lib.flashPath);
	TRACE("	flash total size:	%d\r\n", g_record_lib.flashTotalSize);
	TRACE("	flash Free size:	%d\r\n", g_record_lib.flashFreeSize);
	TRACE("	record data path:	(%s)\r\n", g_record_lib.recordPath);
}

/* ��ȡ��¼�����Ϣ */
void getRecordLibInfo(record_lib_t *p_record_lib)
{
	memcpy(p_record_lib, &g_record_lib, sizeof(record_lib_t));
}


void deinitRecordLib()
{
	setRecordFlag(2);

	pthread_join(g_saveRecordId, NULL);

	/* �ͷŶ��� */
	lockOperateQueue();
	DestroyQueue(&g_operateQueue);
	unlockOperateQueue();

	pthread_mutex_destroy(&g_queueMutex);
	pthread_mutex_destroy(&g_writeMutex);
}
 
/* ��ȡ��¼�����ܼ�¼������ */
int searchRecordTotal()
{
	struct dirent **namelist;
	int i = 0;
	int total = 0;
	BOOL flag = FALSE;
	char startTime[32];
	unsigned int sTime;	
	time_t tCurTime = 0;	
	search_condition_t searchCondition;
	char RcdPath[GENERAL_PATH_LEN] = {0};
	
	memset(RcdPath, 0, sizeof(RcdPath));
	(void)getRecordPath(RcdPath);

	total = scandir(RcdPath, &namelist, 0, alphasort);
	
	for(i=0; i<total; i++)
	{
		if ((strcmp(namelist[i]->d_name,".") == 0) || (strcmp(namelist[i]->d_name,"..")==0))
		{
			continue;
		}
		else
		{
			flag = TRUE;

			break;
		}
	}

	if(flag)
	{
		sprintf(startTime,"%.*s-%.*s-%.*s 00:00:00",4, namelist[i]->d_name, 2, namelist[i]->d_name+4, 2, namelist[i]->d_name+6);

        sTime = DatetimeToTime(startTime);
	
    	tCurTime = time(NULL);

    	searchCondition.startTime = sTime;
    	searchCondition.endTime= tCurTime;
    	
    	if (total < 0)
    	{
    		perror("scandir");
    	}
    	else
    	{
    		while (total--)
    		{
    			free(namelist[total]);
    		}
    		
    		free(namelist);
    	}
    	
    	total = searchRecordsNum(0, &searchCondition);

    	if (total == -1)
    	{
    		TRACE("searchRecordsNum fail %s %d\r\n", __FUNCTION__, __LINE__);
    		
    		total = 0;
    	}

    }
	else
	{
        if (total < 0)
    	{
    		perror("scandir");
    	}
    	else
    	{
    		while (total--)
    		{
    			free(namelist[total]);
    		}
    		
    		free(namelist);
    	}

		total = 0;
	}
		
	return total;	
}


void *writeRecordFlag(void *pArg)
{
    setRecordFlag(1);
    return NULL;
}


static void *saveRecordThread(void *param)
{
	int i = 0;
	int j = 0;
	int n = 0;
	int index_fd = 0;
	time_t old_tt, new_tt;
	file_index_t file_index;
	struct tm strTm;
	struct tm *pTm = &strTm;
	//��¼д����̳���Ĵ���
	int errorNum = 0;
	//���ݻ��壬�Ա�һ��д��
	record_buffer_t recordBuffer[MAX_OPERATE_QUEUE];
	//ǿ�Ʊ����־
	int recordFlag = 0;
	//��ǰ�¼�����Ŀ��
	int curQueueNum = 0;
	//�����е�Ԫ��
	ElemType queueElem;
	//������
	int count = 0;
	//���ڲ��Һ��ʵ����ݻ�������
	int curDataDay = 0;
	int tmpDataDay = 0;
	time_t tmp_tt;

	old_tt = time(0);

	memset(&recordBuffer, 0, sizeof(recordBuffer));

    TRACE("%s have been created! %s %d\r\n", __FUNCTION__, __FUNCTION__, __LINE__);
    
	while(1)
	{
		count++;

		//��ʱ���
		if ((count % 100) == 0)
		{
			count = 0;
			
			lockWriteData();

			//����ʹ���ʳ�����90%����ɾ������ļ�¼��Ƭ�ļ�
			if (checkStorageCapacity(g_record_lib.flashPath, 
                    &g_record_lib.flashTotalSize, 
                    &g_record_lib.flashFreeSize) == 1)
			{	
                TRACE("disk usage is more 90 percentage, delete old record file! %s %d\r\n", __FUNCTION__, __LINE__);
				delOldRecordFile(g_record_lib.recordPath);
			}
			
			unlockWriteData();
		}
        
		usleep(100 * 1000);
        
		recordFlag = getRecordFlag();
        
		lockOperateQueue();

		curQueueNum = GetQueueElemNum(g_operateQueue);
		g_record_lib.recordCurBufNum = curQueueNum;
            
		unlockOperateQueue();

		new_tt = time(0);

		//���������ʱ�����򳬹���󻺳���Ŀ����ǿ��д���־����λ
		if (((abs(new_tt - old_tt) > g_record_lib.recordSaveInterval) && (curQueueNum > 0))
			|| (curQueueNum >= g_record_lib.recordMaxBufNum)
			|| ((recordFlag > 0) && (curQueueNum > 0)))
		{
			TRACE(".save record.. %s %d\r\n", __FUNCTION__, __LINE__);
		
			old_tt = new_tt;

			/* ������ݻ��� */
			for(i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				memset(&recordBuffer[i], 0, sizeof(record_buffer_t));
			}

			TRACE("curQueueNum %d %s %d \r\n", curQueueNum, __FUNCTION__, __LINE__);

			for (i = 0; i < curQueueNum; i++)
			{
				lockOperateQueue();

				DeQueue(&g_operateQueue, &queueElem);

				unlockOperateQueue();

				if (queueElem.operateType > 0)
				{
					if ((queueElem.operateType & OPERATE_TYPE_RECORD) > 0)
					{
						//����ƥ������ݻ���
						for (j = 0; j < curQueueNum; j++)
						{
							//δ�ҵ�ƥ������ݻ�������
							if (recordBuffer[j].time == 0)
							{
								tmp_tt = queueElem.recordTime;
								
								localtime_r(&tmp_tt, pTm);

								sprintf(recordBuffer[j].indexFileName, "%s/%04d%02d%02d.data",
										g_record_lib.recordPath, pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);

								lockWriteData();

								recordBuffer[j].startIndexNo = getLastFileIndexNo(recordBuffer[j].indexFileName) + 1;

								unlockWriteData();

								break;
							}
							//�Ƚ������Ƿ�һ��
							else
							{
								tmp_tt = queueElem.recordTime;
								localtime_r(&tmp_tt, pTm);
								curDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

								tmp_tt = recordBuffer[j].time;
								localtime_r(&tmp_tt, pTm);
								tmpDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

								//�ҵ����ʵ����ݻ�������
								if(curDataDay == tmpDataDay)
								{
									break;
								}
								else
								{
									continue;
								}
							}
						}

						/* ����������Ϣ */
						memset(&file_index, 0, sizeof(file_index_t));
						file_index.no = recordBuffer[j].startIndexNo;
						file_index.recordStatus = 0;
                        file_index.recordType = queueElem.recordType;
                        file_index.inCount = queueElem.inCount;
                        file_index.outCount = queueElem.outCount;
						file_index.recordTime = queueElem.recordTime;

						recordBuffer[j].time = queueElem.recordTime;

						memcpy(&recordBuffer[j].indexBuffer[recordBuffer[j].indexOffset], &file_index, sizeof(file_index_t));
						recordBuffer[j].indexOffset += sizeof(file_index_t);
					}
				}
			}
            
			lockWriteData();

			g_record_lib.curOperateStatus = eOPERATE_STATUS_RECORD;

			for (i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				//������д��
				if(recordBuffer[i].time == 0)
				{
					break;
				}

				/* д�������ļ� */
				index_fd = open(recordBuffer[i].indexFileName, O_APPEND | O_CREAT | O_NONBLOCK | O_SYNC | O_WRONLY, 0666);
				if(index_fd <= 0)
				{	
					TRACE("can not open index file (%s)\r\n", recordBuffer[i].indexFileName);
					unlockWriteData();
					goto error;
				}
				
				n = write(index_fd, recordBuffer[i].indexBuffer, recordBuffer[i].indexOffset);
				if(n != recordBuffer[i].indexOffset)
				{	
					TRACE("write index buffer error:(%s)\r\n", strerror(errno));

					close(index_fd);
					index_fd = 0;
					unlockWriteData();
					goto error;
				}

				if(index_fd > 0)
				{
					close(index_fd);
					index_fd = 0;
				}
			}

			g_record_lib.curOperateStatus = eOPERATE_STATUS_FREE;
			
			unlockWriteData();

			/* ������ݻ��� */
			for (i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				memset(&recordBuffer[i], 0, sizeof(record_buffer_t));
			}
		}
		
		if(recordFlag == 2)
		{
			break;
		}

		/* �����ǿ��д��¼��д��ѱ�־������Ϊ0 */
		if(recordFlag == 1)
		{
			setRecordFlag(0);
		}
		
		recordFlag = 0;
        
		continue;
error:
		g_record_lib.curOperateStatus = eOPERATE_STATUS_FREE;

		/* ������ݻ��� */
		for (i = 0; i < MAX_OPERATE_QUEUE; i++)
		{
			memset(&recordBuffer[i], 0, sizeof(record_buffer_t));
		}

		if(recordFlag == 2)
		{
			break;
		}

		/* �����ǿ��д��¼��д��ѱ�־������Ϊ0 */
		if(recordFlag == 1)
		{
			setRecordFlag(0);
		}
		
		recordFlag = 0;

		errorNum++;
		
		if(errorNum >= 3)
		{
			TRACE("operate sd card error num %d, please fsck sd card\r\n", errorNum);

			sync();
			sleep(3);
			reboot(RB_AUTOBOOT);
		}
	}

	sync();

	setRecordFlag(3);
    
	return NULL;
}


void initRecordLib(record_lib_t *p_record_lib)
{
	memcpy(&g_record_lib, p_record_lib, sizeof(record_lib_t));

	//�����¼��󻺳���Ŀ�����������Ŀ
	if (g_record_lib.recordMaxBufNum > MAX_OPERATE_QUEUE)
	{
		g_record_lib.recordMaxBufNum = MAX_OPERATE_QUEUE;
	}

	sprintf(g_record_lib.recordPath, "%s/%s", STORAGE_PATH, RECORD_DIR_NAME);
	mkdir(g_record_lib.recordPath, 0666);

	pthread_mutex_init(&g_queueMutex, NULL);
	pthread_mutex_init(&g_writeMutex, NULL);
	
	InitQueue(&g_operateQueue);

	if (pthread_create(&g_saveRecordId, NULL, saveRecordThread, NULL) < 0)
	{
		TRACE("create saveRecordThread error\r\n");
	}
}


void InitRecordLib()
{
	/* ��ʼ����¼�� */
	record_lib_t record_lib;
	
	memset(&record_lib, 0, sizeof(record_lib_t));

    snprintf(record_lib.flashPath, sizeof(record_lib.flashPath) - 1, "%s/%s", STORAGE_PATH, RECORD_DIR_NAME);
	snprintf(record_lib.recordPath, sizeof(record_lib.recordPath) - 1, "%s/%s", STORAGE_PATH, RECORD_DIR_NAME);
    mkdir(record_lib.recordPath, 0666);
    
    TRACE("flashPath: %s, recordPath: %s \r\n", record_lib.flashPath, record_lib.recordPath);
    
	checkStorageCapacity(record_lib.flashPath, &record_lib.flashTotalSize, &record_lib.flashFreeSize);

    TRACE("flashTotalSize: %d, flashFreeSize: %d \r\n", record_lib.flashTotalSize, record_lib.flashFreeSize);
    
	record_lib.recordMaxBufNum = 10;
	record_lib.recordSaveInterval = 120;

	initRecordLib(&record_lib);

	//��¼���еļ�¼������
	//gRecordTotal = searchRecordTotal();
}

