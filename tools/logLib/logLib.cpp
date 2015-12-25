
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/reboot.h>

#include "logLib.h"
#include "logOutLib.h"
#include "logInfo.h"

#include "appLib.h"
#include "eventQueue.h"
#include "publicLib.h"

manlog_lib_t  g_manlog_lib;
syslog_lib_t  g_syslog_lib;
accesslog_lib_t g_accesslog_lib;

static pthread_t g_saveSysLogId;
static pthread_t g_saveManLogId;
static pthread_t g_saveAccessLogId;

static pthread_mutex_t g_sysQueueMutex;
static pthread_mutex_t g_sysWriteMutex;
static pthread_mutex_t g_manQueueMutex;
static pthread_mutex_t g_manWriteMutex;
static pthread_mutex_t g_accessQueueMutex;
static pthread_mutex_t g_accessWriteMutex;

static SysLogLinkQueue g_sysLogQueue;
static ManLogLinkQueue g_manLogQueue;
static AccessLogLinkQueue g_accessLogQueue;

int gSysLogTotal = 0;           //系统日志总条数
int gManLogTotal = 0;           //管理日志总条数
int gAccessLogTotal = 0;

static int g_logFlag;

static int getSysLogIndexPath(char *pPath, unsigned int iLen)
{
    if (pPath == NULL || iLen < sizeof(g_syslog_lib.sysLogIndexPath))
    {
        return -1;
    }

    strncpy(pPath, g_syslog_lib.sysLogIndexPath, iLen - 1);

    return 0;
}

static int getManLogIndexPath(char *pPath, unsigned int iLen)
{
     if (pPath == NULL || iLen < sizeof(g_manlog_lib.manLogIndexPath))
    {
        return -1;
    }

    strncpy(pPath, g_manlog_lib.manLogIndexPath, iLen - 1);

    return 0;
}

static int checkLogPath(char *pIndexPath, char cLogType)
{
    if (pIndexPath == NULL || *pIndexPath == '\0')
    {
        return -1;
    }

    if ((strstr(pIndexPath, "sysIndex") != NULL && cLogType != eSysLog)
        || (strstr(pIndexPath, "manIndex") != NULL && cLogType != eManLog))
    {
        return -1;
    }

    return 0;
}

static int getLastFileIndexNo(char *pIndexPath, char cLogType)
{
    int iFd = 0;
    int iReadLen = 0;
    int iRet = 0;

    if (checkLogPath(pIndexPath, cLogType) != 0)
    {
        return iRet;
    }
    
    iFd = open(pIndexPath, O_RDONLY | O_NOATIME, 0666);
    if (iFd > 0)
    {
        if (cLogType == eSysLog)
        {    
            syslog_index_t strIndex;
            lseek(iFd, sizeof(syslog_index_t), SEEK_END);
            memset(&strIndex, '\0', sizeof(syslog_index_t));
            iReadLen = read(iFd, &strIndex, sizeof(syslog_index_t));
            if (iReadLen == sizeof(syslog_index_t))
            {
                iRet = strIndex.logNo;
            }
        }
        else if (cLogType == eManLog)
        {
            manlog_index_t strIndex;
            lseek(iFd, sizeof(manlog_index_t), SEEK_END);
            memset(&strIndex, '\0', sizeof(manlog_index_t));
            iReadLen = read(iFd, &strIndex, sizeof(manlog_index_t));
            if (iReadLen == sizeof(manlog_index_t))
            {
                iRet = strIndex.logNo;
            }
        }
        
        close(iFd);
        iFd = 0;
    }

    return iRet;
}

static void dealStorageCapacity(char cLogType)
{
    if (cLogType == eSysLog)
    {
        lockSysLogWriteData();

    	//容量使用率超过了90%，则删除最早的记录照片文件
    	//if(checkStorageCapacity(g_syslog_lib.sdCardPath, &g_syslog_lib.sdCardTotalSize, &g_syslog_lib.sdCardFreeSize) == 1
    	//	||  GetSysLogTotal() >= (GetDeviceAttrRecordMax() - g_syslog_lib.sysLogMaxBufNum))
    	{
    		delOldLogFile(g_syslog_lib.sysLogIndexPath, cLogType);
    	}

    	//容量使用率超过了90%，则删除最早的记录照片文件
    	//if(checkStorageCapacity(g_syslog_lib.flashPath, &g_syslog_lib.flashTotalSize, &g_syslog_lib.flashFreeSize) == 1
    	//	||  GetSysLogTotal() >= (GetDeviceAttrRecordMax() - g_syslog_lib.sysLogMaxBufNum))
    	{
    		delOldLogFile(g_syslog_lib.sysLogIndexPath, cLogType);
    	}
			
		unlockSysLogWriteData();
    }
    else if (cLogType == eManLog)
    {
       lockManLogWriteData();

    	//容量使用率超过了90%，则删除最早的记录照片文件
    	//if(checkStorageCapacity(g_manlog_lib.sdCardPath, &g_manlog_lib.sdCardTotalSize, &g_manlog_lib.sdCardFreeSize) == 1
    	//	||  GetManLogTotal() >= (GetDeviceAttrRecordMax() - g_manlog_lib.sysLogMaxBufNum))
    	{
    		delOldLogFile(g_manlog_lib.manLogIndexPath, cLogType);
    	}

    	//容量使用率超过了90%，则删除最早的记录照片文件
    	//if(checkStorageCapacity(g_manlog_lib.flashPath, &g_manlog_lib.flashTotalSize, &g_manlog_lib.flashFreeSize) == 1
    	//	||  GetSysLogTotal() >= (GetDeviceAttrRecordMax() - g_manlog_lib.manLogMaxBufNum))
    	{
    		delOldLogFile(g_manlog_lib.manLogIndexPath, cLogType);
    	}
			
		unlockManLogWriteData(); 
    }
}

/* 写数据 加锁 */
void lockSysLogWriteData()
{
	pthread_mutex_lock(&g_sysWriteMutex);
}

/* 写数据 解锁 */
void unlockSysLogWriteData()
{
	pthread_mutex_unlock(&g_sysWriteMutex);
}

/* 写数据 加锁 */
void lockManLogWriteData()
{
	pthread_mutex_lock(&g_manWriteMutex);
}

/* 写数据 解锁 */
void unlockManLogWriteData()
{
	pthread_mutex_unlock(&g_manWriteMutex);
}

void lockAccessLogWriteData()
{
    pthread_mutex_lock(&g_accessWriteMutex);
}

void unlockAccessLogWriteData()
{
    pthread_mutex_unlock(&g_accessWriteMutex);
}

/* 操作队列 加锁 */
static void lockSysLogQueue()
{
	pthread_mutex_lock(&g_sysQueueMutex);
}

/* 操作队列 解锁 */
static void unlockSysLogQueue()
{
	pthread_mutex_unlock(&g_sysQueueMutex);
}

/* 操作队列 加锁 */
static void lockManLogQueue()
{
	pthread_mutex_lock(&g_manQueueMutex);
}

/* 操作队列 解锁 */
static void unlockManLogQueue()
{
	pthread_mutex_unlock(&g_manQueueMutex);
}

/* 操作队列 加锁 */
static void lockAccessLogQueue()
{
	pthread_mutex_lock(&g_accessQueueMutex);
}

/* 操作队列 解锁 */
static void unlockAccessLogQueue()
{
	pthread_mutex_unlock(&g_accessQueueMutex);
}

int delOldLogFile(char *pPath, char cLogType)
{   
    if (checkLogPath(pPath, cLogType) != 0)
    {
        return -1;
    }
    
    DIR *pstrDir = NULL;
    char acOldFileName[32];
    char acOldFilePath[128];
    char acTmpFilePath[128];
    char acCurFileName[32] = {0};
    struct dirent *pstrDp = NULL;
    struct stat st;
    struct tm strTm;
    struct tm *pstrTm = &strTm;
    time_t tt;
    
    pstrDir = opendir(pPath);
    if (pstrDir != NULL)
    {
        while ((pstrDp = readdir(pstrDir)) != NULL)
        {
            if ((strcmp(pstrDp->d_name, ".") == 0) || (strcmp(pstrDp->d_name, "..")) == 0)
            {
                continue;
            }   

            memset(acTmpFilePath, 0, sizeof(acTmpFilePath));
            snprintf(acTmpFilePath, sizeof(acTmpFilePath) - 1, "%s/%s", pPath, pstrDp->d_name);

            if (stat(acTmpFilePath, &st) == 0)
            {
                if (S_ISDIR(st.st_mode))
                {
                    continue;
                }
                else
                {
                    if (strlen(acOldFileName) == 0)
                    {
                        memcpy(acOldFileName, pstrDp->d_name, sizeof(acOldFileName) - 1);
                    }
                    else
                    {
                        if (memcmp(pstrDp->d_name, acOldFileName, sizeof(acOldFileName)) < 0)
                        {
                            memset(acOldFileName, 0, sizeof(acOldFileName));
							memcpy(acOldFileName, pstrDp->d_name, sizeof(acOldFileName) - 1);
                        }
                    }
                }
            }
        }

        if (strlen(acOldFileName) > 0)
        {
            tt = time(NULL);
            localtime_r(&tt, pstrTm);

            snprintf(acCurFileName, (sizeof(acCurFileName) - 1), 
                "%04d%02d%02d.data", pstrTm->tm_year + 1900, pstrTm->tm_mon + 1, pstrTm->tm_mday);

            printf("curFileName %s oldFileName %s %s %d\r\n", acCurFileName, acOldFileName, __FUNCTION__, __LINE__);

            if(memcmp(acOldFileName, acCurFileName, sizeof(acCurFileName)) < 0)
			{
				snprintf(acOldFilePath, (sizeof(acOldFilePath) - 1), "%s/%s", pPath, acOldFileName);
				unlink(acOldFilePath);

                if (cLogType == eSysLog)
                {
				    SetSysLogTotal(searchSysLogTotal());
                }
                else if (cLogType == eManLog)
                {
                    SetManLogTotal(searchManLogTotal());
                }
			}
        }

        closedir(pstrDir);
    }

    return 0;
}

int addSysLog(syslog_event_t sysLog)
{
	lockSysLogQueue();

	EnSysLogQueue(&g_sysLogQueue, sysLog);

	unlockSysLogQueue();
	
	return 0;
}

int addManLog(manlog_event_t manLog)
{
    lockManLogQueue();

    EnManLogQueue(&g_manLogQueue, manLog);

    unlockManLogQueue();

    return 0;
}

/*
*   删除所有系统日志
*/
int delAllSysLog()
{
    int iRet = -1;
    struct stat st;
    struct dirent *pDirent = NULL;
    DIR *pDir = NULL;
    char acLogPath[GENERAL_PATH_LEN] = {0};
    char acTmpPath[GENERAL_PATH_LEN] = {0};
    
    memset(acLogPath, 0, sizeof(acLogPath));
    strcpy(acLogPath, g_syslog_lib.sysLogIndexPath);

    pDir = opendir(acLogPath);
    if (pDir != NULL)
    {
        while ((pDirent = readdir(pDir)) != NULL)
        {
            if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0)
            {
                continue;
            }

            sprintf(acTmpPath, "%s/%s", acLogPath, pDirent->d_name);

            if(stat(acTmpPath, &st) == 0)
            {
                unlink(acTmpPath);
                iRet = 0;
            }
        }

        closedir(pDir);
        pDir = NULL;
    }

    return iRet;
}

int delAllManLog()
{
    int iRet = -1;
    struct stat st;
    struct dirent *pDirent = NULL;
    DIR *pDir = NULL;
    char acLogPath[GENERAL_PATH_LEN] = {0};
    char acTmpPath[GENERAL_PATH_LEN] = {0};
    
    memset(acLogPath, 0, sizeof(acLogPath));
    strcpy(acLogPath, g_manlog_lib.manLogIndexPath);

    pDir = opendir(acLogPath);
    if (pDir != NULL)
    {
        while ((pDirent = readdir(pDir)) != NULL)
        {
            if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0)
            {
                continue;
            }

            sprintf(acTmpPath, "%s/%s", acLogPath, pDirent->d_name);

            if(stat(acTmpPath, &st) == 0)
            {
                unlink(acTmpPath);
                iRet = 0;
            }
        }

        closedir(pDir);
        pDir = NULL;
    }

    return iRet;
}

int delAllLog()
{
    int iRet = -1;

    if (delAllSysLog() == 0 && delAllManLog() == 0)
    {
        iRet = 0;
    }

    return iRet;
}

/******************************************************************************
 * 函数名称： delSysLogTime
 * 功能： 删除某个时间以前的log
 * 参数： 
 * 返回： 
 * 创建作者： LPH
 * 创建日期： 2012-11-05
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int delSysLogTime(char *pFile, time_t tTime)
{
	int n = 0;
	int ret = 0;
	int indexFd = 0;//存放记录索引
	syslog_index_t strLogIndex;
	int iIndexAddr = 0;

	/* 只要进行记录查询，就把记录队列中的记录全部更新存储到SD卡中 */
	//setRecordFlag(TRUE);

    if(pFile == NULL)
	{
		return -1;
	}

	indexFd = open(pFile, O_RDWR, 0666);
	if(indexFd <= 0)
	{
		return -1;
	}
	
	while(1)
	{
		n = read(indexFd, &strLogIndex, sizeof(syslog_index_t));

		if(n != sizeof(syslog_index_t))
		{
			break;
		}

		if (strLogIndex.logType == eSysLog
            && strLogIndex.logFileStatus == 0
			&& strLogIndex.logTime <= (unsigned int)tTime)
		{
		    strLogIndex.logFileStatus = 1;
			
			lseek(indexFd, iIndexAddr, SEEK_SET);
			n = write(indexFd, &strLogIndex, sizeof(syslog_index_t));
			if(n != sizeof(syslog_index_t))
			{
				break;
			}
		}
		
		iIndexAddr += sizeof(syslog_index_t);
		lseek(indexFd, iIndexAddr, SEEK_SET);
	}
	
	if(indexFd > 0)
	{
		close(indexFd);
		indexFd = 0;
	}	

	return ret;
}

/******************************************************************************
 * 函数名称： delSysLogsByTime
 * 功能： 删除某个时间以前的log
 * 参数： 
 * 返回： 
 * 创建作者： LPH
 * 创建日期： 2012-11-05
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int delSysLogsByTime(time_t tCurtime)
{
	struct dirent **namelist;
	int i = 0;
    int total = 0;
	struct tm strTm;
	struct tm *pTm = &strTm;
	int tmpTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	char cFilePath[GENERAL_PATH_LEN] = {0};
	char tmpPath[GENERAL_PATH_LEN] = {0};

	memset(tmpPath, 0, sizeof(tmpPath));
    strcpy(tmpPath, g_syslog_lib.sysLogIndexPath);

	total = scandir(tmpPath, &namelist, 0, alphasort);

	if (total < 0)
    {
    	return -1;
    }
    
	localtime_r(&tCurtime, pTm);
	tmpTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;
	sprintf(indexFile, "%08d.index", tmpTime);

	for (i = 0; i < total; i++)
    {
    	if ((strcmp(namelist[i]->d_name, ".") == 0) || (strcmp(namelist[i]->d_name, "..")==0))
        {
        	continue;
        }

    	if (strcmp(namelist[i]->d_name, indexFile) <= 0)
        {
        	sprintf(cFilePath, "%s/%s", tmpPath, namelist[i]->d_name);
        	delSysLogTime(cFilePath, tCurtime);
        }
    	else
        {
        	break;
        }
    }

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
    
	return 0;
}

int delManLogTime(char *pFile, time_t tTime)
{
	int n = 0;
	int ret = 0;
	int indexFd = 0;//存放记录索引
	manlog_index_t strLogIndex;
	int iIndexAddr = 0;

	/* 只要进行记录查询，就把记录队列中的记录全部更新存储到SD卡中 */
	//setRecordFlag(TRUE);

    if(pFile == NULL)
	{
		return -1;
	}

	indexFd = open(pFile, O_RDWR, 0666);
	if(indexFd <= 0)
	{
		return -1;
	}
	
	while(1)
	{
		n = read(indexFd, &strLogIndex, sizeof(manlog_index_t));

		if(n != sizeof(manlog_index_t))
		{
			break;
		}

		if (strLogIndex.logType == eManLog
            && strLogIndex.logFileStatus == 0
			&& strLogIndex.logTime <= (unsigned int)tTime)
		{
		    strLogIndex.logFileStatus = 1;
			
			lseek(indexFd, iIndexAddr, SEEK_SET);
			n = write(indexFd, &strLogIndex, sizeof(manlog_index_t));
			if(n != sizeof(manlog_index_t))
			{
				break;
			}
		}
		
		iIndexAddr += sizeof(manlog_index_t);
		lseek(indexFd, iIndexAddr, SEEK_SET);
	}
	
	if(indexFd > 0)
	{
		close(indexFd);
		indexFd = 0;
	}	

	return ret;
}

int delManLogsByTime(time_t tCurtime)
{
	struct dirent **namelist;
	int i = 0;
    int total = 0;
	struct tm strTm;
	struct tm *pTm = &strTm;
	int tmpTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	char cFilePath[GENERAL_PATH_LEN] = {0};
	char tmpPath[GENERAL_PATH_LEN] = {0};

	memset(tmpPath, 0, sizeof(tmpPath));
    strcpy(tmpPath, g_manlog_lib.manLogIndexPath);

	total = scandir(tmpPath, &namelist, 0, alphasort);

	if (total < 0)
    {
    	return -1;
    }
    
	localtime_r(&tCurtime, pTm);
	tmpTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;
	sprintf(indexFile, "%08d.index", tmpTime);

	for (i = 0; i < total; i++)
    {
    	if ((strcmp(namelist[i]->d_name, ".") == 0) || (strcmp(namelist[i]->d_name, "..")==0))
        {
        	continue;
        }

    	if (strcmp(namelist[i]->d_name, indexFile) <= 0)
        {
        	sprintf(cFilePath, "%s/%s", tmpPath, namelist[i]->d_name);
        	delManLogTime(cFilePath, tCurtime);
        }
    	else
        {
        	break;
        }
    }

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
    
	return 0;
}

/******************************************************************************
 * 函数名称： delSysLogUserNo
 * 功能： 删除某个指定工号用户的系统日志
 * 参数： 
 * 返回： 
 * 创建作者： LPH
 * 创建日期： 2012-11-05
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int delManLogUserNo(char *pFile, int iUserNo)
{
	int n = 0;
	int ret = 0;
	int indexFd = 0;//存放记录索引
	manlog_index_t strLogIndex;
	int iIndexAddr = 0;

	/* 只要进行记录查询，就把记录队列中的记录全部更新存储到SD卡中 */
	//setRecordFlag(TRUE);

    if(pFile == NULL)
	{
		return -1;
	}

	indexFd = open(pFile, O_RDWR, 0666);
	if(indexFd <= 0)
	{
		return -1;
	}
	
	while(1)
	{
		n = read(indexFd, &strLogIndex, sizeof(manlog_index_t));

		if(n != sizeof(manlog_index_t))
		{
			break;
		}

		if (strLogIndex.logType == eManLog
            && strLogIndex.logFileStatus == 0
			&& strLogIndex.logUserNo == iUserNo)
		{
		    strLogIndex.logFileStatus = 1;
			
			lseek(indexFd, iIndexAddr, SEEK_SET);
			n = write(indexFd, &strLogIndex, sizeof(manlog_index_t));
			if(n != sizeof(manlog_index_t))
			{
				break;
			}
		}
		
		iIndexAddr += sizeof(manlog_index_t);
		lseek(indexFd, iIndexAddr, SEEK_SET);
	}
	
	if(indexFd > 0)
	{
		close(indexFd);
		indexFd = 0;
	}	

	return ret;
}

int delManLogsByUserNo(int iUserNo)
{
	struct dirent **namelist;
	int i = 0;
    int total = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	char cFilePath[GENERAL_PATH_LEN] = {0};
	char tmpPath[GENERAL_PATH_LEN] = {0};

	memset(tmpPath, 0, sizeof(tmpPath));
    strcpy(tmpPath, g_manlog_lib.manLogIndexPath);

	total = scandir(tmpPath, &namelist, 0, alphasort);

	if (total < 0)
    {
    	return -1;
    }

	for (i = 0; i < total; i++)
    {
    	if ((strcmp(namelist[i]->d_name, ".") == 0) || (strcmp(namelist[i]->d_name, "..")==0))
        {
        	continue;
        }

        sprintf(cFilePath, "%s/%s", tmpPath, namelist[i]->d_name);
        delManLogTime(cFilePath, iUserNo);
    }

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
    
	return 0;
}

int searchAccessLog(unsigned int searchFlag, accesslog_search_condition_t *pSearchCondition, 
    accesslog_list_t *pLogList)
{
	int n = 0;
	int indexFd = 0;
	struct tm Tm;
	struct tm *pTm = &Tm;
	time_t tt;
	unsigned int tmpTime = 0;
	unsigned int tmpEndTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	accesslog_index_t strLogIndex;
	accesslog_info_t *pTmpInfo = NULL;
	accesslog_info_t *pstrLogInfo = NULL;
	int iLogNo = 0;
	int iLogNum = 0;
	int searchOverFlag = 0;     /* 查询结束标记   =1:表示查询结束 */
	int searchOverAddr = 0;     /* 结束时，当前文件已读取的位置 */
	
	/* 只要进行记录查询，就把记录队列中的记录全部更新存储到SD卡中 */
	//setRecordFlag(TRUE);

    if(pLogList == NULL)
	{
		return -1;
	}

	if(pSearchCondition == NULL)
	{
		return -2;
	}

	/* 时间非法 */
	if(pSearchCondition->startTime > pSearchCondition->endTime)
	{
        printf("pSearchCondition->startTime %u, pSearchCondition->endTime %u %s %d\r\n", pSearchCondition->startTime, pSearchCondition->endTime, __FUNCTION__, __LINE__);
		return -3;
	}

	pstrLogInfo = &pLogList->head;

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	tmpEndTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;

	while(1)
	{
		tt = pSearchCondition->startTime;
		localtime_r(&tt, pTm);
		tmpTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;
        
        if(tmpTime > tmpEndTime)
		{
			break;
		}
		
		sprintf(indexFile, "%s/%08d.index", g_accesslog_lib.accessLogIndexPath, tmpTime);
        
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
			n = read(indexFd, &strLogIndex, sizeof(accesslog_index_t));
			if(n != sizeof(accesslog_index_t))
			{
                printf("read error. %s %d\r\n", __FUNCTION__, __LINE__);
				break;
			}

			searchOverAddr += sizeof(accesslog_index_t);

            /* 符合条件日志 */
			if((strLogIndex.accessTime >= pSearchCondition->startTime) 
				&& (strLogIndex.accessTime <= pSearchCondition->endTime))
			{
				/* 日志类型不匹配 */
                if (strLogIndex.logType != eAccessLog)
                {
                    continue;
                }

				/* 记录已删除 */
				if (strLogIndex.logStatus == 1)
				{
					continue;
				}

                
				pTmpInfo = (accesslog_info_t *)malloc(sizeof(accesslog_info_t));
				if(pTmpInfo != NULL)
				{
					memset(pTmpInfo, 0, sizeof(accesslog_info_t));
					memcpy(&pTmpInfo->logIndex, &strLogIndex, sizeof(accesslog_index_t));
					
					iLogNo++;
					iLogNum++;
					
					pTmpInfo->logNo = iLogNo;
					pLogList->logNum = iLogNum;
					pstrLogInfo->next = pTmpInfo;
					
					if(iLogNo == 1)
					{
						pstrLogInfo->cur = pTmpInfo;
					}
					
					pstrLogInfo = pTmpInfo;

					/* 判断查询的记录数是否达到要求的记录数，达到后则本次查询结束 */
					if((pSearchCondition->searchNum > 0) 
						&& (iLogNum >= pSearchCondition->searchNum))
					{
						printf("search syslog num %d %s %d\r\n", iLogNum, __FUNCTION__, __LINE__);
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

		/* 查询结束 */
		if(searchOverFlag == 1)
		{
			pSearchCondition->startAddr += searchOverAddr;
			break;
		}

        /* 换天了，所以将起始地址清零 */
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
		pSearchCondition->startAddr = 0;
	}

	return 0;
}

int searchSysLog(unsigned int searchCondFlag, syslog_search_condition_t *pSearchCondition, syslog_list_t *pLogList)
{
	int n = 0;
	int indexFd = 0;
	struct tm Tm;
	struct tm *pTm = &Tm;
	time_t tt;
	unsigned int tmpTime = 0;
	unsigned int tmpEndTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	syslog_index_t strLogIndex;
	syslog_info_t *pTmpInfo = NULL;
	syslog_info_t *pstrLogInfo = NULL;
	int iLogNo = 0;
	int iLogNum = 0;
	int searchOverFlag = 0;     /* 查询结束标记   =1:表示查询结束 */
	int searchOverAddr = 0;     /* 结束时，当前文件已读取的位置 */
	
	/* 只要进行记录查询，就把记录队列中的记录全部更新存储到SD卡中 */
	//setRecordFlag(TRUE);

    if(pLogList == NULL)
	{
		return -1;
	}

	if(pSearchCondition == NULL)
	{
		return -2;
	}

	/* 时间非法 */
	if(pSearchCondition->startTime > pSearchCondition->endTime)
	{
        printf("pSearchCondition->startTime %u, pSearchCondition->endTime %u %s %d\r\n", pSearchCondition->startTime, pSearchCondition->endTime, __FUNCTION__, __LINE__);
		return -3;
	}

	pstrLogInfo = &pLogList->head;

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	tmpEndTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;

	while(1)
	{
		tt = pSearchCondition->startTime;
		localtime_r(&tt, pTm);
		tmpTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;
        
        if(tmpTime > tmpEndTime)
		{
			break;
		}
		
		sprintf(indexFile, "%s/%08d.index", g_syslog_lib.sysLogIndexPath, tmpTime);
        
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
			n = read(indexFd, &strLogIndex, sizeof(syslog_index_t));
			if(n != sizeof(syslog_index_t))
			{
                printf("read error. %s %d\r\n", __FUNCTION__, __LINE__);
				break;
			}

			searchOverAddr += sizeof(syslog_index_t);

            /* 符合条件日志 */
			if((strLogIndex.logTime >= pSearchCondition->startTime) 
				&& (strLogIndex.logTime <= pSearchCondition->endTime))
			{
				/* 日志类型不匹配 */
                if (strLogIndex.logType != eSysLog)
                {
                    continue;
                }

				/* 记录已删除 */
				if (strLogIndex.logFileStatus == 1)
				{
					continue;
				}

                
				pTmpInfo = (syslog_info_t *)malloc(sizeof(syslog_info_t));
				if(pTmpInfo != NULL)
				{
					memset(pTmpInfo, 0, sizeof(syslog_info_t));
					memcpy(&pTmpInfo->logIndex, &strLogIndex, sizeof(syslog_index_t));
                    pTmpInfo->next = NULL;
					
					iLogNo++;
					iLogNum++;
					
					pTmpInfo->logNo = iLogNo;
					pLogList->logNum = iLogNum;
					pstrLogInfo->next = pTmpInfo;
					
					if(iLogNo == 1)
					{
						pstrLogInfo->cur = pTmpInfo;
					}
					
					pstrLogInfo = pTmpInfo;

					/* 判断查询的记录数是否达到要求的记录数，达到后则本次查询结束 */
					if((pSearchCondition->searchNum > 0) 
						&& (iLogNum >= pSearchCondition->searchNum))
					{
						printf("search syslog num %d %s %d\r\n", iLogNum, __FUNCTION__, __LINE__);
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

		/* 查询结束 */
		if(searchOverFlag == 1)
		{
			pSearchCondition->startAddr += searchOverAddr;
			break;
		}

        /* 换天了，所以将起始地址清零 */
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
		pSearchCondition->startAddr = 0;
	}

	return 0;
}

/***********************************************************************
 * 查找记录的总条数
 * searchConditionFlag:	查询条件标记 通过searchConditionFlag来控制查询内容
 * searchCondition:	查询条件的具体内容
 *
 * 返回值： >=0：记录条数 ， -1：失败
 ***********************************************************************/
int searchSysLogNum(unsigned int searchCondFlag, syslog_search_condition_t *pSearchCondition)
{
	int n = 0;
	//存放记录索引
	int indexFd = 0;
	struct tm Tm;
	struct tm *pTm = &Tm;
	time_t tt;
	unsigned int tmpTime = 0;
	unsigned int tmpEndTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	syslog_index_t strLogIndex;
	//记录总数
	int logNum = 0;
	char timeBuf[32];

	if(pSearchCondition == NULL)
	{
		return -1;
	}

	//时间非法
	if(pSearchCondition->startTime > pSearchCondition->endTime)
	{
		return -2;
	}

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	//将时间转换成20120224格式
	tmpEndTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;

	memset(timeBuf, 0, 32);

	while(1)
	{
		tt = pSearchCondition->startTime;
		localtime_r(&tt, pTm);
		tmpTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;
		if(tmpTime > tmpEndTime)
		{
			break;
		}
		
		sprintf(indexFile, "%s/%08d.index", g_syslog_lib.sysLogIndexPath, tmpTime);
		indexFd = open(indexFile, O_RDONLY | O_NOATIME, 0666);
		if(indexFd < 0)
		{
			pSearchCondition->startAddr = 0;
			
			pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
			
			continue;
		}
		
		while(1)
		{
			n = read(indexFd, &strLogIndex, sizeof(syslog_index_t));
			if(n != sizeof(syslog_index_t))
			{
				break;
			}

			//记录合法
			if((strLogIndex.logTime >= pSearchCondition->startTime) 
				&& (strLogIndex.logTime <= pSearchCondition->endTime))
			{
                //日志类型不符合
                if (strLogIndex.logType != eSysLog)
                {
                    continue;
                }
                
				/* 记录已删除 */
				if (strLogIndex.logFileStatus == 1)
				{
					continue;
				}
				
				logNum++;
			}
		}
		
		if(indexFd > 0)
		{
			close(indexFd);
			indexFd = 0;
		}
		
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
		
		pSearchCondition->startAddr = 0;//换天了，所以将起始地址清零
	}

	return logNum;
}

/* 获取系统日志记录的条数 */
int searchSysLogTotal()
{
	struct dirent **namelist;
	int i = 0;
	int total = 0;
	BOOL flag = FALSE;
	char startTime[32];
	unsigned int sTime;	
	time_t tCurTime = 0;	
	syslog_search_condition_t searchCondition;
	char RcdPath[GENERAL_PATH_LEN] = {0};
	
	memset(RcdPath, 0, sizeof(RcdPath));

    if (getSysLogIndexPath(RcdPath, sizeof(RcdPath)) != 0)
    {
        return -1;
    }

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
    	
    	total = searchSysLogNum(0, &searchCondition);

    	if (total == -1)
    	{
    		printf("searchRecordsNum fail %s %d\r\n", __FUNCTION__, __LINE__);
    		
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

int searchManLog(unsigned int searchCondFlag, manlog_search_condition_t *pSearchCondition, 
                                                manlog_list_t *pstrLogList)
{
	int n = 0;
	int indexFd = 0;        /* 打开索引文件fd */
	struct tm Tm;
	struct tm *pTm = &Tm;
	time_t tt;
	unsigned int tmpTime = 0;
	unsigned int tmpEndTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	manlog_index_t strLogIndex;
	manlog_info_t *pTmpInfo = NULL;
	manlog_info_t *pstrLogInfo = NULL;
	int iLogNo = 0;
	int iLogNum = 0;
	int searchOverFlag = 0;     /* 查询结束标记   =1:表示查询结束 */
	int searchOverAddr = 0;     /* 结束时，当前文件已读取的位置 */    
	
	/* 只要进行记录查询，就把记录队列中的记录全部更新存储到SD卡中 */
	//setRecordFlag(TRUE);

    if(pstrLogList == NULL)
	{
		return -1;
	}

	if(pSearchCondition == NULL)
	{
		return -2;
	}

	/* 时间非法 */
	if(pSearchCondition->startTime > pSearchCondition->endTime)
	{
        printf("pSearchCondition->startTime %u, pSearchCondition->endTime %u %s %d\r\n", pSearchCondition->startTime, pSearchCondition->endTime, __FUNCTION__, __LINE__);
		return -3;
	}

	pstrLogInfo = &pstrLogList->head;

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	tmpEndTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;

	while(1)
	{
		tt = pSearchCondition->startTime;
		localtime_r(&tt, pTm);
		tmpTime = (pTm->tm_year + 1900) * 10000 + (pTm->tm_mon + 1) * 100 + pTm->tm_mday;
		if(tmpTime > tmpEndTime)
		{
			break;
		}
		
		sprintf(indexFile, "%s/%08d.index", g_manlog_lib.manLogIndexPath, tmpTime);

		indexFd = open(indexFile, O_RDONLY | O_NOATIME, 0666);
		if(indexFd <= 0)
		{
            /* 读取失败，换天 */
			pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
            pSearchCondition->startAddr = 0;
			continue;
		}

		if(pSearchCondition->startAddr > 0)
		{
			lseek(indexFd, pSearchCondition->startAddr, SEEK_SET);
		}
		
		searchOverAddr = 0;
        
		while(1)
		{
			n = read(indexFd, &strLogIndex, sizeof(manlog_index_t));
			if(n != sizeof(manlog_index_t))
			{
				break;
			}

			searchOverAddr += sizeof(manlog_index_t);

            //符合条件日志
			if((strLogIndex.logTime >= pSearchCondition->startTime) 
				&& (strLogIndex.logTime <= pSearchCondition->endTime))
			{
				//日志类型不匹配
                if (strLogIndex.logType != eManLog)
                {
                    continue;
                }

                //按名字检索
                if ((searchCondFlag & LOG_CONDTITION_USERNAME) 
                    && strcmp(pSearchCondition->userName, strLogIndex.logUserName) != 0)
                {
                    continue;
                }

                /* 按工号检索 */
                if ((searchCondFlag & LOG_CONDTITION_USERNO)
                    && pSearchCondition->userNo != strLogIndex.logUserNo)
                {
                    continue;
                }
                
				/* 记录已删除 */
				if (strLogIndex.logFileStatus == 1)
				{
					continue;
				}
                
				pTmpInfo = (manlog_info_t *)malloc(sizeof(manlog_info_t));
				if(pTmpInfo != NULL)
				{
					memset(pTmpInfo, 0, sizeof(manlog_info_t));
					memcpy(&pTmpInfo->logIndex, &strLogIndex, sizeof(manlog_index_t));
                    pTmpInfo->next = NULL;
					
					iLogNo++;
					iLogNum++;
					
					pTmpInfo->logNo = iLogNo;
					pstrLogList->logNum = iLogNum;
					pstrLogInfo->next = pTmpInfo;
					
					if(iLogNo == 1)
					{
						pstrLogInfo->cur = pTmpInfo;
					}
					
					pstrLogInfo = pTmpInfo;
                    pTmpInfo = NULL;
                    
					/* 判断查询的记录数是否达到要求的记录数，达到后则本次查询结束 */
					if((pSearchCondition->searchNum > 0) 
						&& (iLogNum >= pSearchCondition->searchNum))
					{
						printf("search manlog num %d %s %d\r\n", iLogNum, __FUNCTION__, __LINE__);
						searchOverFlag = 1;
						break;
					}
				}
                else
                {
                    printf("malloc error! %s \r\n", strerror(errno));
                }
			}
		}
		
		if(indexFd > 0)
		{
			close(indexFd);
			indexFd = 0;
		}
		
		/* 查询结束 */
		if(searchOverFlag == 1)
		{
			pSearchCondition->startAddr += searchOverAddr;
			break;
		}

        /* 换天了，所以将起始地址清零 */
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
		pSearchCondition->startAddr = 0;
	}

	return 0;
}

/***********************************************************************
 * 查找记录的总条数
 * searchConditionFlag:	查询条件标记 通过searchConditionFlag来控制查询内容
 * searchCondition:	查询条件的具体内容
 *
 * 返回值： >=0：记录条数 ， -1：失败
 ***********************************************************************/
int searchManLogNum(unsigned int searchCondFlag, manlog_search_condition_t *pSearchCondition)
{
	int n = 0;
	//存放记录索引
	int indexFd = 0;
	struct tm Tm;
	struct tm *pTm = &Tm;
	time_t tt;
	unsigned int tmpTime = 0;
	unsigned int tmpEndTime = 0;
	char indexFile[GENERAL_PATH_LEN] = {0};
	manlog_index_t manLogIndex;
	//记录总数
	int logNum = 0;
	char timeBuf[32];

	if(pSearchCondition == NULL)
	{
		return -1;
	}

	//时间非法
	if(pSearchCondition->startTime > pSearchCondition->endTime)
	{
		return -2;
	}

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	//将时间转换成20120224格式
	tmpEndTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;

	memset(timeBuf, 0, 32);

	while(1)
	{
		tt = pSearchCondition->startTime;
		localtime_r(&tt, pTm);
		tmpTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;
		if(tmpTime > tmpEndTime)
		{
			break;
		}
		
		sprintf(indexFile, "%s/%08d.index", g_manlog_lib.manLogIndexPath, tmpTime);
		indexFd = open(indexFile, O_RDONLY | O_NOATIME, 0666);
		if(indexFd < 0)
		{
			pSearchCondition->startAddr = 0;
			
			pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
			
			continue;
		}
		
		while(1)
		{
			n = read(indexFd, &manLogIndex, sizeof(manlog_index_t));
			if(n != sizeof(manlog_index_t))
			{
				break;
			}

			//记录合法
			if((manLogIndex.logTime >= pSearchCondition->startTime) 
				&& (manLogIndex.logTime <= pSearchCondition->endTime))
			{
                //日志类型不符合
                if (manLogIndex.logType != eManLog)
                {
                    continue;
                }
                
				/* 记录已删除 */
				if (manLogIndex.logFileStatus == 1)
				{
					continue;
				}
				
				logNum++;
			}
		}
		
		if(indexFd > 0)
		{
			close(indexFd);
			indexFd = 0;
		}
		
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
		
		pSearchCondition->startAddr = 0;//换天了，所以将起始地址清零
	}

	return logNum;
}

/* 获取系统日志记录的条数 */
int searchManLogTotal()
{
	struct dirent **namelist;
	int i = 0;
	int total = 0;
	BOOL flag = FALSE;
	char startTime[32];
	unsigned int sTime;	
	time_t tCurTime = 0;	
	manlog_search_condition_t searchCondition;
	char RcdPath[GENERAL_PATH_LEN] = {0};
	
	memset(RcdPath, 0, sizeof(RcdPath));

    if (getManLogIndexPath(RcdPath, sizeof(RcdPath)) != 0)
    {
        return -1;
    }

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
    	
    	total = searchManLogNum(0, &searchCondition);

    	if (total == -1)
    	{
    		printf("searchManLogNum fail %s %d\r\n", __FUNCTION__, __LINE__);
    		
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

/******************************************************************************
 * 函数名称： exportSysLogs
 * 功能： 导出所有系统log
 * 参数： 
 * 返回： 
 * 创建作者： LPH
 * 创建日期： 2012-11-05
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int exportSysLogs()
{
    
}

/******************************************************************************
 * 函数名称： exportManLogs
 * 功能： 导出所有管理日志log
 * 参数： 
 * 返回： 
 * 创建作者： LPH
 * 创建日期： 2012-11-05
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int exportManLogs()
{
    
}


/* 获取下一条记录信息 */
syslog_index_t * getNextSysLogInfo(syslog_list_t *pLogList)
{
	syslog_info_t *pstrLogInfo = NULL;
	syslog_index_t *pLogIndex = NULL;

	if(pLogList == NULL)
	{
		return NULL;
	}

	pstrLogInfo = pLogList->head.cur;
	if(pstrLogInfo == NULL)
	{
		return NULL;
	}

	pLogIndex = &pstrLogInfo->logIndex;
	pLogList->head.cur = pstrLogInfo->next;

	return pLogIndex;
}

/* 获取下一条记录信息 */
manlog_index_t * getNextManLogInfo(manlog_list_t *pLogList)
{
	manlog_info_t *pstrLogInfo = NULL;
	manlog_index_t *pLogIndex = NULL;

	if(pLogList == NULL)
	{
		return NULL;
	}

	pstrLogInfo = pLogList->head.cur;
	if(pstrLogInfo == NULL)
	{
		return NULL;
	}

	pLogIndex = &pstrLogInfo->logIndex;
	pLogList->head.cur = pstrLogInfo->next;

	return pLogIndex;
}



/* 释放查询的记录 */
void freeSearchSysLog(syslog_list_t *pLogList)
{
	syslog_info_t *pstrLogInfo = NULL;

	if(pLogList != NULL)
	{
		while(pLogList->head.next)
		{
			pstrLogInfo = pLogList->head.next;
			pLogList->head.next = pstrLogInfo->next;
			free(pstrLogInfo);
			pstrLogInfo = NULL;
		}
	}
}

/* 释放查询的记录 */
void freeSearchManLog(manlog_list_t *pstrLogList)
{
	manlog_info_t *pstrLogInfo = NULL;

	if(pstrLogList != NULL)
	{
		while(pstrLogList->head.next)
		{
			pstrLogInfo = pstrLogList->head.next;
			pstrLogList->head.next = pstrLogInfo->next;
			free(pstrLogInfo);
			pstrLogInfo = NULL;
		}
	}
}

/* 释放查询的记录 */
void freeSearchAccessLog(accesslog_list_t *pstrLogList)
{
	accesslog_info_t *pstrLogInfo = NULL;

	if(pstrLogList != NULL)
	{
		while(pstrLogList->head.next)
		{
			pstrLogInfo = pstrLogList->head.next;
			pstrLogList->head.next = pstrLogInfo->next;
			free(pstrLogInfo);
			pstrLogInfo = NULL;
		}
	}
}

/* 显示记录缓冲队列信息 */
void showSysLogQueueInfo()
{
	QueueSysLogTraverse(g_sysLogQueue);
}

void showManLogQueueInfo()
{
    QueueManLogTraverse(g_manLogQueue);
}

static void *saveSysLogThread(void *pArg)
{   
    int i, j;
    //int iCount = 0;
    int iCurQueueNum = 0;
    time_t new_tt, old_tt;
    time_t tmp_tt;
    struct tm strTm;
	struct tm *pTm = &strTm;
    struct stat st;
    SysLogElemType queueElem;
    syslog_index_t  log_index;
    syslog_buffer_t sysLogBuffer[MAX_OPERATE_QUEUE];
    int curDataDay = 0;
	int tmpDataDay = 0;
    int iIndexFd = -1;
    int iWriteLen = 0;
    int iErrorNum = 0;
    int iLogNum = GetSysLogTotal();

    memset(&sysLogBuffer, 0, sizeof(sysLogBuffer));
    old_tt = time(NULL);

    printf("%s %d\r\n", __FUNCTION__, __LINE__);
    
    while (1)
    {
#if 0
        //flash容量检测
        iCount++;
        //定时检测
		if((iCount % 100) == 0)
		{
			iCount = 0;
		    dealStorageCapacity(eSysLog);
		}
#endif  

        usleep(100 * 1000);

#if 0
        if (GetSysLogTotal() >= MAX_SYS_LOG_NUM)
        {
            continue;
        }
#endif
        lockSysLogQueue();
        iCurQueueNum = GetQueueSysLogElemNum(g_sysLogQueue);
        g_syslog_lib.sysLogCurBufNum = iCurQueueNum;
        unlockSysLogQueue();

        new_tt = time(NULL);

        if (((abs(new_tt - old_tt) > g_syslog_lib.sysLogSaveInterval)
                && iCurQueueNum > 0)
            /*|| (iCurQueueNum >= g_syslog_lib.sysLogMaxBufNum) */)
        {
            printf("save syslog ... %s %d\r\n", __FUNCTION__, __LINE__);

            old_tt = new_tt;

            for (i = 0; i < MAX_OPERATE_QUEUE; i++)
            {
                memset(&sysLogBuffer[i], 0, sizeof(syslog_buffer_t));
            }

            printf("iCurQueueNum %d %s %d \r\n", iCurQueueNum, __FUNCTION__, __LINE__);

            for (i = 0; i < iCurQueueNum; i++)
            {
                lockSysLogQueue();

                DeSysLogQueue(&g_sysLogQueue, &queueElem);

                unlockSysLogQueue();
                
                for (j = 0; j < iCurQueueNum; j++)
                {
                    if (sysLogBuffer[j].time == 0)
                    {   
                        tmp_tt = queueElem.uiTime;
                        localtime_r(&tmp_tt, pTm);
                        snprintf(sysLogBuffer[j].indexFileName, (MAX_RECORD_FILE_PATH_LEN - 1), "%s/%04d%02d%02d.index",
                            g_syslog_lib.sysLogIndexPath, pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);

                        lockSysLogWriteData();
                        sysLogBuffer[j].startIndexNo = getLastFileIndexNo(sysLogBuffer[j].indexFileName, eSysLog) + 1;
                        unlockSysLogWriteData();

                        break;
                    }
                    else
                    {   
                        tmp_tt = queueElem.uiTime;
						localtime_r(&tmp_tt, pTm);
						curDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

						tmp_tt = sysLogBuffer[j].time;
						localtime_r(&tmp_tt, pTm);
						tmpDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

						//找到合适的数据缓冲数组
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
                
                memset(&log_index, 0, sizeof(syslog_index_t));
                log_index.logNo = sysLogBuffer[j].startIndexNo;
                log_index.logFileStatus = 0;
                log_index.logType = queueElem.cType;
                log_index.logSubType = queueElem.cSubType;
                log_index.logOpType = queueElem.iOpType;
                log_index.logResult = queueElem.cResult;
                log_index.logTime = queueElem.uiTime;

                printf("msg: %s \r\n", queueElem.acMsg);
                if (queueElem.acMsg[0] != '\0')
                {
                    memcpy(log_index.logMsg, queueElem.acMsg, MAX_MSG_LEN - 1);
                }
                
                sysLogBuffer[j].time = queueElem.uiTime;
                
                memcpy(&sysLogBuffer[j].indexBuffer[sysLogBuffer[j].indexOffset], &log_index, sizeof(syslog_index_t));
                sysLogBuffer[j].indexOffset += sizeof(syslog_index_t);

                iLogNum = GetSysLogTotal();
                iLogNum++;
                SetSysLogTotal(iLogNum);
            }

            lockSysLogWriteData();
            
            for (i = 0; i < MAX_OPERATE_QUEUE; i++)
            {
                printf("time: %u %s %d\r\n", sysLogBuffer[i].time, __FUNCTION__, __LINE__);
                
                if (sysLogBuffer[i].time == 0)
                {
                    break;
                }

                //printf("indexFileName: %s %s %d\r\n", sysLogBuffer[i].indexFileName,  __FUNCTION__, __LINE__);
                
                /* 写入索引文件 */
				iIndexFd = open(sysLogBuffer[i].indexFileName, O_APPEND | O_CREAT | O_NONBLOCK | O_SYNC | O_WRONLY, 0666);
				if(iIndexFd <= 0)
				{
					printf("can not open index file (%s)\r\n", sysLogBuffer[i].indexFileName);
					unlockSysLogWriteData();
					goto error;
				}
				
				iWriteLen = write(iIndexFd, sysLogBuffer[i].indexBuffer, sysLogBuffer[i].indexOffset);
				if(iWriteLen != sysLogBuffer[i].indexOffset)
				{
					printf("write index buffer error:(%s)\r\n", strerror(errno));
					close(iIndexFd);
					iIndexFd = 0;
					unlockSysLogWriteData();
					goto error;
				}

				if(iIndexFd > 0)
				{
					close(iIndexFd);
					iIndexFd = 0;
				}
            }

            unlockSysLogWriteData();

		    /* 清空数据缓冲 */
		    for(i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				memset(&sysLogBuffer[i], 0, sizeof(syslog_buffer_t));
			}
        }

        continue;

error:
        /* 清空数据缓冲 */
		for(i = 0; i < MAX_OPERATE_QUEUE; i++)
		{
			memset(&sysLogBuffer[i], 0, sizeof(syslog_buffer_t));
		}

		iErrorNum++;
		
		if(iErrorNum >= 3)
		{
			printf("operate sd card error num %d, please fsck sd card. %s %d\r\n", iErrorNum, __FUNCTION__, __LINE__);

			sync();

			reboot(RB_AUTOBOOT);
		}   
    }

    sync();

	//setRecordFlag(3);
	
	return NULL;
}

static void *saveManLogThread(void *pArg)
{   
    int i, j;
    //int iCount = 0;
    int iCurQueueNum = 0;
    time_t new_tt, old_tt;
    time_t tmp_tt;
    struct tm strTm;
	struct tm *pTm = &strTm;
    struct stat st;
    ManLogElemType queueElem;
    manlog_index_t  log_index;
    manlog_buffer_t manLogBuffer[MAX_OPERATE_QUEUE];
    int curDataDay = 0;
	int tmpDataDay = 0;
    int iIndexFd = -1;
    int iWriteLen = 0;
    int iErrorNum = 0;
    int iLogNum = GetManLogTotal();

    memset(&manLogBuffer, 0, sizeof(manLogBuffer));
    old_tt = time(NULL);

    printf("%s %d\r\n", __FUNCTION__, __LINE__);
    
    while (1)
    {
#if 0
        //flash容量检测
        iCount++;
        //定时检测
		if((iCount % 100) == 0)
		{
			iCount = 0;
		    dealStorageCapacity(eSysLog);
		}
#endif  

        usleep(100 * 1000);

#if 0
        if (GetSysLogTotal() >= MAX_SYS_LOG_NUM)
        {
            continue;
        }
#endif
        
        lockManLogQueue();
        iCurQueueNum = GetQueueManLogElemNum(g_manLogQueue);
        g_manlog_lib.manLogCurBufNum = iCurQueueNum;
        unlockManLogQueue();
        
        new_tt = time(NULL);

        if (((abs(new_tt - old_tt) > g_manlog_lib.manLogSaveInterval)
                && iCurQueueNum > 0)
            /*|| (iCurQueueNum >= g_syslog_lib.sysLogMaxBufNum) */)
        {
            printf("save syslog ... %s %d\r\n", __FUNCTION__, __LINE__);

            old_tt = new_tt;

            for (i = 0; i < MAX_OPERATE_QUEUE; i++)
            {
                memset(&manLogBuffer[i], 0, sizeof(manlog_buffer_t));
            }

            printf("iCurQueueNum %d %s %d \r\n", iCurQueueNum, __FUNCTION__, __LINE__);

            for (i = 0; i < iCurQueueNum; i++)
            {
                lockManLogQueue();

                DeManLogQueue(&g_manLogQueue, &queueElem);

                unlockManLogQueue();

                for (j = 0; j < iCurQueueNum; j++)
                {
                    if (manLogBuffer[j].time == 0)
                    {
                        tmp_tt = queueElem.uiTime;
                        localtime_r(&tmp_tt, pTm);
                        sprintf(manLogBuffer[j].indexFileName, "%s/%04d%02d%02d.index",
                            g_manlog_lib.manLogIndexPath, pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);

                        lockManLogWriteData();
                        manLogBuffer[j].startIndexNo = getLastFileIndexNo(manLogBuffer[j].indexFileName, eManLog) + 1;
                        unlockManLogWriteData();

                        break;
                    }
                    else
                    {
                        tmp_tt = queueElem.uiTime;
						localtime_r(&tmp_tt, pTm);
						curDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

						tmp_tt = manLogBuffer[j].time;
						localtime_r(&tmp_tt, pTm);
						tmpDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

						//找到合适的数据缓冲数组
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

                memset(&log_index, 0, sizeof(manlog_index_t));
                log_index.logNo = manLogBuffer[j].startIndexNo;
                log_index.logFileStatus = 0;
                log_index.logType = queueElem.cType;
                log_index.logSubType = queueElem.cSubType;
                log_index.logOpType = queueElem.iOpType;
                log_index.logResult = queueElem.cResult;
                log_index.logTime = queueElem.uiTime;
                log_index.logUserNo = queueElem.uiUserNo;
                log_index.logUserId = queueElem.uiUserId;

                printf("name: %s \r\n", queueElem.acUserName);
                if (queueElem.acUserName[0] != '\0')
                {
                    memcpy(log_index.logUserName, queueElem.acUserName, MAX_USER_NAME_LEN - 1);
                }

                printf("msg: %s \r\n", queueElem.acMsg);
                if (queueElem.acMsg[0] != '\0')
                {
                    memcpy(log_index.logMsg, queueElem.acMsg, MAX_MSG_LEN - 1);
                }

                manLogBuffer[j].time = queueElem.uiTime;
                
                memcpy(&manLogBuffer[j].indexBuffer[manLogBuffer[j].indexOffset], &log_index, sizeof(manlog_index_t));
                manLogBuffer[j].indexOffset += sizeof(manlog_index_t);

                iLogNum = GetManLogTotal();
                iLogNum++;
                SetManLogTotal(iLogNum);
            }

            lockManLogWriteData();

            for (i = 0; i < MAX_OPERATE_QUEUE; i++)
            {
                if (manLogBuffer[i].time == 0)
                {
                    break;
                }

                /* 写入索引文件 */
				iIndexFd = open(manLogBuffer[i].indexFileName, O_APPEND | O_CREAT | O_NONBLOCK | O_SYNC | O_WRONLY, 0666);
				if(iIndexFd <= 0)
				{
					printf("can not open index file (%s)\r\n", manLogBuffer[i].indexFileName);
					unlockManLogWriteData();
					goto error;
				}
				
				iWriteLen = write(iIndexFd, manLogBuffer[i].indexBuffer, manLogBuffer[i].indexOffset);
				if(iWriteLen != manLogBuffer[i].indexOffset)
				{
					printf("write index buffer error:(%s)\r\n", strerror(errno));
					close(iIndexFd);
					iIndexFd = 0;
					unlockManLogWriteData();
					goto error;
				}

				if(iIndexFd > 0)
				{
					close(iIndexFd);
					iIndexFd = 0;
				}
            }

            unlockManLogWriteData();

		    /* 清空数据缓冲 */
		    for(i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				memset(&manLogBuffer[i], 0, sizeof(manlog_buffer_t));
			}
        }

        continue;

error:
        /* 清空数据缓冲 */
		for(i = 0; i < MAX_OPERATE_QUEUE; i++)
		{
			memset(&manLogBuffer[i], 0, sizeof(manlog_buffer_t));
		}

		iErrorNum++;
		
		if(iErrorNum >= 3)
		{
			printf("operate sd card error num %d, please fsck sd card. %s %d\r\n", iErrorNum, __FUNCTION__, __LINE__);

			sync();

			reboot(RB_AUTOBOOT);
		}
    }

    sync();

	//setRecordFlag(3);
	
	return NULL;
}

#if 1
void static saveAccessLogThread(void *pArg)
{   
    int i, j;
    //int iCount = 0;
    int iCurQueueNum = 0;
    time_t new_tt, old_tt;
    time_t tmp_tt;
    struct tm strTm;
	struct tm *pTm = &strTm;
    struct stat st;
    AccessLogElemType queueElem;
    accesslog_index_t  strLogIndex;
    accesslog_buffer_t strLogBuffer[MAX_OPERATE_QUEUE];
    int curDataDay = 0;
	int tmpDataDay = 0;
    int iIndexFd = -1;
    int iWriteLen = 0;
    int iErrorNum = 0;
    int iLogFlag = 0;
    int iLogNum = GetAccessLogTotal();

    memset(&strLogBuffer, 0, sizeof(strLogBuffer));
    old_tt = time(NULL);

    printf("%s %d\r\n", __FUNCTION__, __LINE__);
    
    while (1)
    {
#if 0
        //flash容量检测
        iCount++;
        //定时检测
		if((iCount % 100) == 0)
		{
			iCount = 0;
		    dealStorageCapacity(eSysLog);
		}
#endif  

        usleep(100 * 1000);

#if 0
        if (GetSysLogTotal() >= MAX_SYS_LOG_NUM)
        {
            continue;
        }
#endif

        //iLogFlag = getLogFlag();
        
        lockAccessLogQueue();
        iCurQueueNum = GetAccessLogQueueElemNum(g_accessLogQueue);
        g_accesslog_lib.accessLogCurBufNum = iCurQueueNum;
        unlockAccessLogQueue();
        
        new_tt = time(NULL);

        if (((abs(new_tt - old_tt) > g_manlog_lib.manLogSaveInterval)
                && iCurQueueNum > 0)
            /*|| (iCurQueueNum >= g_syslog_lib.sysLogMaxBufNum) */)
        {
            printf("save syslog ... %s %d\r\n", __FUNCTION__, __LINE__);

            old_tt = new_tt;

            for (i = 0; i < MAX_OPERATE_QUEUE; i++)
            {
                memset(&strLogBuffer[i], 0, sizeof(accesslog_buffer_t));
            }

            printf("iCurQueueNum %d %s %d \r\n", iCurQueueNum, __FUNCTION__, __LINE__);

            for (i = 0; i < iCurQueueNum; i++)
            {
                lockAccessLogQueue();

                DeAccessLogQueue(&g_accessLogQueue, &queueElem);

                unlockAccessLogQueue();

                for (j = 0; j < iCurQueueNum; j++)
                {
                    if (strLogBuffer[j].time == 0)
                    {
                        tmp_tt = queueElem.uiTime;
                        localtime_r(&tmp_tt, pTm);
                        sprintf(strLogBuffer[j].indexFileName, "%s/%04d%02d%02d.index",
                            g_manlog_lib.manLogIndexPath, pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);

                        lockAccessLogWriteData();
                        strLogBuffer[j].startIndexNo = getLastFileIndexNo(strLogBuffer[j].indexFileName, eAccessLog) + 1;
                        unlockAccessLogWriteData();

                        break;
                    }
                    else
                    {
                        tmp_tt = queueElem.uiTime;
						localtime_r(&tmp_tt, pTm);
						curDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

						tmp_tt = manLogBuffer[j].time;
						localtime_r(&tmp_tt, pTm);
						tmpDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

						//找到合适的数据缓冲数组
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
                
                memset(&strLogIndex, 0, sizeof(accesslog_index_t));
                strLogIndex.logNo = strLogBuffer[j].startIndexNo;
                strLogIndex.logStatus = 0;
                strLogIndex.logType = queueElem.cType;
                strLogIndex.accessMode = queueElem.iAccessMode;
                strLogIndex.accessTime = queueElem.uiTime;
                strLogIndex.recogMode = queueElem.cRecogMode;
                strLogIndex.result = queueElem.iResult;
                
                strLogIndex.userNum = (queueElem.iUserNum > 5 ? 5 : queueElem.iUserNum);
                for (j = 0; j < strLogIndex.userNum; j++)
                {
                    strLogIndex.userId[i] = queueElem.uiUserId[i];
                }

                strLogIndex.groupNum = (queueElem.iGroupNum > 5 ? 5 : queueElem.iGroupNum);
                for (j = 0; j < strLogIndex.groupNum; j++)
                {
                    strLogIndex.groupId[i] = queueElem.uiGroupId[i];
                }

                strLogIndex.photoNum = (queueElem.iPhotoNum > 5 ? 5 : queueElem.iPhotoNum);
                for (j = 0; j > strLogIndex.photoNum; j++)
                {
                    strLogIndex.photoSize[i] = queueElem.uiPhotoSize[i];
                    strLogIndex.photoTime[i] = queueElem.uiPhotoTime[i];
                    strLogIndex.photoStartAddr[i] = queueElem.uiPhotoStartAddr[i];
                }

                strLogBuffer[j].time = queueElem.uiTime;
                
                memcpy(&strLogBuffer[j].indexBuffer[strLogBuffer[j].indexOffset], &strLogIndex, sizeof(accesslog_index_t));
                strLogBuffer[j].indexOffset += sizeof(accesslog_index_t);

                iLogNum = GetAccessLogTotal();
                iLogNum++;
                SetAccessLogTotal(iLogNum);
            }

            lockAccessLogWriteData();

            for (i = 0; i < MAX_OPERATE_QUEUE; i++)
            {
                if (strLogBuffer[i].time == 0)
                {
                    break;
                }

                /* 写入索引文件 */
				iIndexFd = open(strLogBuffer[i].indexFileName, O_APPEND | O_CREAT | O_NONBLOCK | O_SYNC | O_WRONLY, 0666);
				if(iIndexFd <= 0)
				{
					printf("can not open index file (%s)\r\n", strLogBuffer[i].indexFileName);
					unlockAccessLogWriteData();
					goto error;
				}
				
				iWriteLen = write(iIndexFd, strLogBuffer[i].indexBuffer, strLogBuffer[i].indexOffset);
				if(iWriteLen != strLogBuffer[i].indexOffset)
				{
					printf("write index buffer error:(%s)\r\n", strerror(errno));
					close(iIndexFd);
					iIndexFd = 0;
					unlockAccessLogWriteData();
					goto error;
				}

				if(iIndexFd > 0)
				{
					close(iIndexFd);
					iIndexFd = 0;
				}
            }

            unlockAccessLogWriteData();

		    /* 清空数据缓冲 */
		    for(i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				memset(&strLogBuffer[i], 0, sizeof(accesslog_buffer_t));
			}
        }

        continue;

error:
        /* 清空数据缓冲 */
		for(i = 0; i < MAX_OPERATE_QUEUE; i++)
		{
			memset(&strLogBuffer[i], 0, sizeof(accesslog_buffer_t));
		}

		iErrorNum++;
		
		if(iErrorNum >= 3)
		{
			printf("operate sd card error num %d, please fsck sd card. %s %d\r\n", iErrorNum, __FUNCTION__, __LINE__);

			sync();

			reboot(RB_AUTOBOOT);
		}
    }

    sync();

	//setRecordFlag(3);
	
	return NULL;
}
#endif

void showSysLogInfo()
{
	printf("syslog lib info:\r\n");
	printf("	storage status:		%d\r\n", g_syslog_lib.storageStatus);
	printf("	sd mount path:		(%s)\r\n", g_syslog_lib.sdCardPath);
	printf("	sd total size:		%d\r\n", g_syslog_lib.sdCardTotalSize);
	printf("	sd free size:		%d\r\n", g_syslog_lib.sdCardFreeSize);
	printf("	flash mount path:	(%s)\r\n", g_syslog_lib.flashPath);
	printf("	flash total size:	%d\r\n", g_syslog_lib.flashTotalSize);
	printf("	flash free size:	%d\r\n", g_syslog_lib.flashFreeSize);
	printf("	log index path:	(%s)\r\n", g_syslog_lib.sysLogIndexPath);
}

void showManLogInfo()
{
	printf("manlog lib info:\r\n");
	printf("	storage status:		%d\r\n", g_manlog_lib.storageStatus);
	printf("	sd mount path:		(%s)\r\n", g_manlog_lib.sdCardPath);
	printf("	sd total size:		%d\r\n", g_manlog_lib.sdCardTotalSize);
	printf("	sd free size:		%d\r\n", g_manlog_lib.sdCardFreeSize);
	printf("	flash mount path:	(%s)\r\n", g_manlog_lib.flashPath);
	printf("	flash total size:	%d\r\n", g_manlog_lib.flashTotalSize);
	printf("	flash free size:	%d\r\n", g_manlog_lib.flashFreeSize);
	printf("	log index path:	(%s)\r\n", g_manlog_lib.manLogIndexPath);
}

void initLogLib(syslog_lib_t *pSysLogLib, manlog_lib_t *pManLogLib)
{
    memcpy(&g_manlog_lib, pManLogLib, sizeof(manlog_lib_t));
	memcpy(&g_syslog_lib, pSysLogLib, sizeof(syslog_lib_t));

    if (g_manlog_lib.manLogMaxBufNum > MAX_OPERATE_QUEUE)
    {
        g_manlog_lib.manLogMaxBufNum = MAX_OPERATE_QUEUE;
    }

	//如果记录最大缓冲条目数大于最大条目
	if (g_syslog_lib.sysLogMaxBufNum > MAX_OPERATE_QUEUE)
	{
		g_syslog_lib.sysLogMaxBufNum = MAX_OPERATE_QUEUE;
	}

    sprintf(g_manlog_lib.manLogIndexPath, "%s/%s", g_manlog_lib.manLogIndexPath, MANLOG_INDEX_DIR_NAME);
    mkdir(g_manlog_lib.manLogIndexPath,0666);

	sprintf(g_syslog_lib.sysLogIndexPath, "%s/%s", g_syslog_lib.sysLogIndexPath, SYSLOG_INDEX_DIR_NAME);
	mkdir(g_syslog_lib.sysLogIndexPath, 0666);

	printf("%s \r\n", g_syslog_lib.sysLogIndexPath);
	printf("%s \r\n", g_manlog_lib.manLogIndexPath);
	
    printf("manLogIndexPath: %s, sysLogIndexPath: %s %s %d\r\n", g_manlog_lib.manLogIndexPath, g_syslog_lib.sysLogIndexPath, __FUNCTION__, __LINE__);
    
    pthread_mutex_init(&g_manQueueMutex, NULL);
    pthread_mutex_init(&g_manWriteMutex, NULL);
    pthread_mutex_init(&g_sysQueueMutex, NULL);
    pthread_mutex_init(&g_sysWriteMutex, NULL);
	
    InitSysLogQueue(&g_sysLogQueue);
	InitManLogQueue(&g_manLogQueue);

    if (pthread_create(&g_saveManLogId, NULL, saveManLogThread, NULL) < 0)
    {
        printf("create saveManLogThread error\r\n");
    }
    
	if (pthread_create(&g_saveSysLogId, NULL, saveSysLogThread, NULL) < 0)
	{
		printf("create saveSysLogThread error\r\n");
	}
}


void deinitLogLib()
{
	//setRecordFlag(2);

	pthread_join(g_saveSysLogId, NULL);
    pthread_join(g_saveManLogId, NULL);

    /* 释放队列 */
	lockManLogQueue();
	DestroySysLogQueue(&g_sysLogQueue);
	unlockManLogQueue();

    /* 释放队列 */
	lockSysLogQueue();
	DestroyManLogQueue(&g_manLogQueue);
	unlockSysLogQueue();

	pthread_mutex_destroy(&g_sysQueueMutex);
	pthread_mutex_destroy(&g_sysWriteMutex);
    pthread_mutex_destroy(&g_manQueueMutex);
    pthread_mutex_destroy(&g_manWriteMutex);
}




