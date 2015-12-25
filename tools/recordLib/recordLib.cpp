#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/statvfs.h>
#include <dirent.h>

#include "userLib.h"
#include "recordLib.h"
#include "eventQueue.h"
#include "recordInfo.h"
#include "maintainLib.h"
#include "devLib.h"
#include "appLib.h"
#include "publicLib.h"

record_lib_t g_record_lib;

static pthread_t g_saveRecordId;
static pthread_mutex_t g_queueMutex;//操作队列锁
static pthread_mutex_t g_writeMutex;//写入磁盘锁
static LinkQueue g_operateQueue;//队列
static int g_recordFlag = 0;//强制写入标记

static int getLastFileIndexNo(char *pIndexPath);
static void *saveRecordThread(void *param);
static void lockOperateQueue();
static void unlockOperateQueue();
static void delOldRecordFile(char *path);

/******************************************************************************
 * 函数名称： GetRecordPicPath
 * 功能： 
 * 参数： 
 * 返回： 
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int GetRecordPicPath(char* pPath)
{
	if (pPath == NULL)
	{
		return -1;
	}

	strcpy(pPath, g_record_lib.recordPicPath);

	return 0;
}

/******************************************************************************
 * 函数名称： GetRecordIndexPath
 * 功能： 
 * 参数： 
 * 返回： 
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int GetRecordIndexPath(char* pPath)
{
	if (pPath == NULL)
	{
		return -1;
	}

	strcpy(pPath, g_record_lib.recordIndexPath);

	return 0;
}

/* 获取最后的索引号 */
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

/******************************************************************************
 * 函数名称： saveRecordThread
 * 功能： 记录保存线程
 * 参数： param : 线程参数
 * 返回： 无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
static void *saveRecordThread(void *param)
{
	int i = 0;
	int j = 0;
	int n = 0;
	//事件数组中，操作记录文件的次数
	int photoNum = 0;
	int data_fd = 0;
	int index_fd = 0;
	time_t old_tt, new_tt;
	file_index_t file_index;
	struct tm strTm;
	struct tm *pTm = &strTm;
	//记录写入磁盘出错的次数
	int errorNum = 0;
	//数据缓冲，以便一次写入
	record_buffer_t recordBuffer[MAX_OPERATE_QUEUE];
	//强制保存标志
	int recordFlag = 0;
	//当前事件的条目数
	int curQueueNum = 0;
	//队列中的元素
	ElemType queueElem;
	//数据文件的状态
	struct stat data_st;
	//计数器
	int count = 0;
	//用于查找合适的数据缓冲数组
	int curDataDay = 0;
	int tmpDataDay = 0;
	time_t tmp_tt;
	static char tmpPicBuf[30 * 1024] = {0};
	int iRecordNum = GetRecordTotal();

	old_tt = time(0);

	memset(&recordBuffer, 0, sizeof(recordBuffer));
	
	while(1)
	{
		count++;

		//定时检测
		if((count % 100) == 0)
		{
			count = 0;
			
			lockWriteData();

			//容量使用率超过了90%，则删除最早的记录照片文件
			if(checkStorageCapacity(g_record_lib.sdCardPath, &g_record_lib.sdCardTotalSize, &g_record_lib.sdCardFreeSize) == 1
				||  GetRecordTotal() >= (GetDeviceAttrRecordMax() - g_record_lib.recordMaxBufNum))
			{
				delOldRecordFile(g_record_lib.recordPicPath);
				delOldRecordFile(g_record_lib.recordIndexPath);
			}

			//容量使用率超过了90%，则删除最早的记录照片文件
			if(checkStorageCapacity(g_record_lib.flashPath, &g_record_lib.flashTotalSize, &g_record_lib.flashFreeSize) == 1
				||  GetRecordTotal() >= (GetDeviceAttrRecordMax() - g_record_lib.recordMaxBufNum))
			{
				delOldRecordFile(g_record_lib.recordPicPath);
				delOldRecordFile(g_record_lib.recordIndexPath);
			}
			
			unlockWriteData();
		}

		usleep(100 * 1000);

		/* 如果记录总数达到容量，不予添加记录 */
		if(GetRecordTotal() >= GetDeviceAttrRecordMax() - g_record_lib.recordMaxBufNum)
		{
			continue;
		}

		recordFlag = getRecordFlag();

		lockOperateQueue();

		curQueueNum = GetQueueElemNum(g_operateQueue);
		g_record_lib.recordCurBufNum = curQueueNum;

		unlockOperateQueue();

		new_tt = time(0);

		//超过最大间隔时长，或超过最大缓冲条目，或强制写入标志被置位
		if(((abs(new_tt - old_tt) > g_record_lib.recordSaveInterval) 
					&& (curQueueNum > 0))
				|| (curQueueNum >= g_record_lib.recordMaxBufNum)
				|| ((recordFlag > 0) 
					&& (curQueueNum > 0)))
		{
			TRACE(".save record.. %s %d\r\n", __FUNCTION__, __LINE__);
			
			//超过sd操作的最大间隔时间，则将数据写入sd卡
			old_tt = new_tt;
			photoNum = 0;

			/* 清空数据缓冲 */
			for(i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				if(recordBuffer[i].pPicBuffer != NULL)
				{
					free(recordBuffer[i].pPicBuffer);
					recordBuffer[i].pPicBuffer = NULL;
				}
				
				memset(&recordBuffer[i], 0, sizeof(record_buffer_t));
			}

			TRACE("curQueueNum %d %s %d \r\n", curQueueNum, __FUNCTION__, __LINE__);

			for(i = 0; i < curQueueNum; i++)
			{
				queueElem.pFileData = tmpPicBuf;
				
				lockOperateQueue();

				DeQueue(&g_operateQueue, &queueElem);

				unlockOperateQueue();

				if(queueElem.operateType > 0)
				{
					//保存记录照片
					if((queueElem.operateType&OPERATE_TYPE_RECORD_PIC) > 0
						|| (queueElem.operateType&OPERATE_TYPE_RECORD) > 0)
					{
						//查找匹配的数据缓冲
						for(j = 0; j < curQueueNum; j++)
						{
							//未找到匹配的数据缓冲数组
							if(recordBuffer[j].time == 0)
							{
								tmp_tt = queueElem.time;    
								
								localtime_r(&tmp_tt, pTm);

								sprintf(recordBuffer[j].picFileName, "%s/%04d%02d%02d.data",
										g_record_lib.recordPicPath, pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);

								sprintf(recordBuffer[j].indexFileName, "%s/%04d%02d%02d.index",
										g_record_lib.recordIndexPath, pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);

								lockWriteData();
								
								if(stat(recordBuffer[j].picFileName, &data_st) == 0)
								{
									recordBuffer[j].startAddr = data_st.st_size;
								}
								else
								{
									recordBuffer[j].startAddr = 0;
								}

								recordBuffer[j].startIndexNo = getLastFileIndexNo(recordBuffer[j].indexFileName) + 1;

								unlockWriteData();

								break;
							}
							//比较日期是否一致
							else
							{
								tmp_tt = queueElem.time;
								localtime_r(&tmp_tt, pTm);
								curDataDay = pTm->tm_year*10000 + pTm->tm_mon*100 + pTm->tm_mday;

								tmp_tt = recordBuffer[j].time;
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

						/* 生成索引信息 */
						memset(&file_index, 0, sizeof(file_index_t));
						file_index.no = recordBuffer[j].startIndexNo;
						file_index.fileType = 1;
						file_index.fileStatus = 0;
						file_index.userType = queueElem.nUserType;
						file_index.id = queueElem.id;
						file_index.recogType = queueElem.nType;
						file_index.recogStatus = queueElem.nStatus;
						file_index.recogScores = queueElem.recogScores;
						file_index.recogReason = queueElem.nLoginReason;
						file_index.startAddr = recordBuffer[j].startAddr + recordBuffer[j].picOffset;
						file_index.fileSize = strlen(RECORD_START_FLAG) + sizeof(file_index_t) + queueElem.fileSize + strlen(RECORD_END_FLAG);
						file_index.recordTime = queueElem.time;
                        file_index.cRecogMode = queueElem.cRecogMode;

						recordBuffer[j].time = queueElem.time;
			
						recordBuffer[j].pPicBuffer = (char *)realloc(recordBuffer[j].pPicBuffer, ((recordBuffer[j].picOffset + file_index.fileSize + 3)/4) * 4);

						if(recordBuffer[j].pPicBuffer == NULL)
						{
							TRACE("realloc pic buffer error\r\n");

							usleep(100*1000);

							goto error;
						}

						/* 保存的记录的格式： RECORD_START_FLAG + 索引信息 + 照片 + RECORD_END_FLAG */
						memcpy(recordBuffer[j].pPicBuffer + recordBuffer[j].picOffset, RECORD_START_FLAG, strlen(RECORD_START_FLAG));
						recordBuffer[j].picOffset += strlen(RECORD_START_FLAG);
						
						memcpy(recordBuffer[j].pPicBuffer + recordBuffer[j].picOffset, &file_index, sizeof(file_index_t));
						recordBuffer[j].picOffset += sizeof(file_index_t);

						memcpy(recordBuffer[j].pPicBuffer + recordBuffer[j].picOffset, queueElem.pFileData, queueElem.fileSize);
						recordBuffer[j].picOffset += queueElem.fileSize;

						memcpy(recordBuffer[j].pPicBuffer + recordBuffer[j].picOffset, RECORD_END_FLAG, strlen(RECORD_END_FLAG));
						recordBuffer[j].picOffset += strlen(RECORD_END_FLAG);

						memcpy(&recordBuffer[j].indexBuffer[recordBuffer[j].indexOffset], &file_index, sizeof(file_index_t));
						recordBuffer[j].indexOffset += sizeof(file_index_t);

						/* 记录总数自加 */
						iRecordNum = GetRecordTotal();
						iRecordNum++;
						
						SetRecordTotal(iRecordNum);
					}
				}
			}

			lockWriteData();

			g_record_lib.curOperateStatus = eOPERATE_STATUS_RECORD;

			for(i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				//数据已写完
				if(recordBuffer[i].time == 0)
				{
					break;
				}

				/* 写入数据文件 */
				data_fd = open(recordBuffer[i].picFileName, O_APPEND | O_CREAT | O_NONBLOCK | O_SYNC | O_WRONLY, 0666);
				if(data_fd <= 0)
				{
					TRACE("can not open data file (%s)\r\n", recordBuffer[i].picFileName);

					unlockWriteData();

					goto error;
				}

				n = write(data_fd, recordBuffer[i].pPicBuffer, recordBuffer[i].picOffset);
				if(n != recordBuffer[i].picOffset)
				{
					TRACE("write data buffer error:(%s)\r\n", strerror(errno));
					
					close(data_fd);
					data_fd = 0;

					unlockWriteData();

					goto error;
				}

				if(data_fd > 0)
				{
					close(data_fd);
					data_fd = 0;
				}

				/* 写入索引文件 */
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

			/* 清空数据缓冲 */
			for(i = 0; i < MAX_OPERATE_QUEUE; i++)
			{
				if(recordBuffer[i].pPicBuffer != NULL)
				{
					free(recordBuffer[i].pPicBuffer);
					recordBuffer[i].pPicBuffer = NULL;
				}

				memset(&recordBuffer[i], 0, sizeof(record_buffer_t));
			}
		}
		
		if(recordFlag == 2)
		{
			break;
		}

		/* 如果是强制写记录，写完把标志重新置为0 */
		if(recordFlag == 1)
		{
			setRecordFlag(0);
		}
		
		recordFlag = 0;

		continue;
error:
		g_record_lib.curOperateStatus = eOPERATE_STATUS_FREE;

		/* 清空数据缓冲 */
		for(i = 0; i < MAX_OPERATE_QUEUE; i++)
		{
			if(recordBuffer[i].pPicBuffer != NULL)
			{
				free(recordBuffer[i].pPicBuffer);
				recordBuffer[i].pPicBuffer = NULL;
			}

			memset(&recordBuffer[i], 0, sizeof(record_buffer_t));
		}

		if(recordFlag == 2)
		{
			break;
		}

		/* 如果是强制写记录，写完把标志重新置为0 */
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

			reboot(RB_AUTOBOOT);
		}
	}

	sync();

	setRecordFlag(3);
	
	return NULL;
}

/* 操作队列 加锁 */
static void lockOperateQueue()
{
	pthread_mutex_lock(&g_queueMutex);
}

/* 操作队列 解锁 */
static void unlockOperateQueue()
{
	pthread_mutex_unlock(&g_queueMutex);
}

/* 检测存储设备的容量 */
int checkStorageCapacity(char *pStoragePath, unsigned int *pTotalSize, unsigned int *pFreeSize)
{
	int ret = 0;
	struct statvfs st;

	if(pStoragePath == NULL)
	{
		return -1;
	}

	if(statvfs(pStoragePath, &st) == 0)
	{
		//使用空间超过90%
		if(((float)st.f_blocks-(float)st.f_bfree)/(float)st.f_blocks*100 > 90.0f)
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
				//是目录
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
						//该文件更旧
						if(memcmp(dp->d_name, oldFileName, sizeof(oldFileName)) < 0)
						{
							memset(oldFileName, 0, sizeof(oldFileName));
							memcpy(oldFileName, dp->d_name, sizeof(oldFileName));
						}
					}
				}
			}
		}

		if(strlen(oldFileName) > 0)
		{
			tt = time(0);
			localtime_r(&tt, pTm);
			
			sprintf(curFileName, "%04d%02d%02d.data", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday);

			TRACE("curFileName %s oldFileName %s %s %d\r\n", curFileName, oldFileName, __FUNCTION__, __LINE__);

			if(memcmp(oldFileName, curFileName, sizeof(curFileName)) < 0)
			{
				sprintf(oldFilePath, "%s/%s", path, oldFileName);
				unlink(oldFilePath);

				/* 删除记录文件，重新获取记录总数并设置 */
				SetRecordTotal(searchRecordTotal());
			}
		}
		
		closedir(pDir);
	}
}

/* 写数据 加锁 */
void lockWriteData()
{
	pthread_mutex_lock(&g_writeMutex);
}

/* 写数据 解锁 */
void unlockWriteData()
{
	pthread_mutex_unlock(&g_writeMutex);
}

/* 获取强制写标记 */
int getRecordFlag()
{
	return g_recordFlag;
}

/* 设置强制写标记 */
void setRecordFlag(int flag)
{
	lockWriteData();
	
	g_recordFlag = flag;
	
	unlockWriteData();
}

int addRecord(operate_event_t record)
{
	if(((record.operateType & OPERATE_TYPE_RECORD_PIC) == 0
			&& (record.operateType & OPERATE_TYPE_RECORD) == 0)
		|| (record.id < 0)
		|| ((record.fileSize != 0) 
			&& (record.pFileData == NULL))
		)
	{
		return -1;
	}
	
	lockOperateQueue();

	EnQueue(&g_operateQueue, record);

	unlockOperateQueue();
	
	return 0;
}


/***********************************************************************
 * 查找记录，查询完毕后需调用freeSearchRecord()函数进行释放,或自行释放
 * startTime:		起始时间 （必须的）
 * endTime:		结束时间 （必须的）
 *
 * searchConditionFlag:	查询条件标记 通过searchConditionFlag来控制查询内容
 * searchCondition:	查询条件的具体内容
 *
 * pRecordInfo:		返回的查询结果链表
 ***********************************************************************/
int searchRecord(unsigned int searchConditionFlag, search_condition_t *pSearchCondition, record_list_t *pRecordList)
{
	int n = 0;
	//int ret = 0;
	//存放数据到excel表
	int excelFd = 0;
	//存放记录索引
	int indexFd = 0;
	//存放记录照的数据文件
	int dataFd = 0;
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
	//查询结束标记   =1:表示查询结束
	int searchOverFlag = 0;
	//结束时，当前文件已读取的位置
	int searchOverAddr = 0;
	
	/* 只要进行记录查询，就把记录队列中的记录全部更新存储到SD卡中 */
	setRecordFlag(TRUE);

    if(pRecordList == NULL)
	{
		return -1;
	}

	if(pSearchCondition == NULL)
	{
		return -2;
	}

	//时间非法
	if(pSearchCondition->startTime > pSearchCondition->endTime)
	{
        TRACE("pSearchCondition->startTime %u, pSearchCondition->endTime %u %s %d\r\n", pSearchCondition->startTime, pSearchCondition->endTime, __FUNCTION__, __LINE__);

		return -3;
	}

	pRecordInfo = &pRecordList->head;

	tt = pSearchCondition->endTime;
	localtime_r(&tt, pTm);
	//将时间转换成20120224格式
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
		
		sprintf(indexFile, "%s/%08d.index", g_record_lib.recordIndexPath, tmpTime);

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
			if(n != sizeof(file_index_t))
			{
				break;
			}

			searchOverAddr += sizeof(file_index_t);

			//记录合法
			if((fileIndex.recordTime >= pSearchCondition->startTime) 
				&& (fileIndex.recordTime <= pSearchCondition->endTime))
			{
				if(((searchConditionFlag & RECORD_CONDITION_ID) > 0) 
					&& (fileIndex.id != pSearchCondition->userId))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_USER_TYPE) > 0) 
					&& (fileIndex.userType != pSearchCondition->userType))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_TYPE) > 0) 
					&& (fileIndex.recogType != pSearchCondition->recogType))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_STATUS) > 0) 
					&& (fileIndex.recogStatus != pSearchCondition->recogStatus))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_SCORES) > 0)
						&& (fileIndex.recogScores >= pSearchCondition->recogMinScores) 
						&& (fileIndex.recogScores <= pSearchCondition->recogMaxScores))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_REASON) > 0) 
					&& (fileIndex.recogReason != pSearchCondition->recogReason))
				{
					continue;
				}

				//判断用户是否存在，ID为0是存放无法找到用户的失败记录，这里允许id为0的记录导出
				if((fileIndex.id != 0) &&(searchUserById(fileIndex.id, NULL) == NULL))
				{
					continue;
				}

				/* 记录已删除 */
				if (fileIndex.fileStatus == 1)
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
					
					if(recordNo == 1)
					{
						pRecordInfo->cur = pTmpInfo;
					}
					
					pRecordInfo = pTmpInfo;

					//判断查询的记录数是否达到要求的记录数，达到后则本次查询结束
					if((pSearchCondition->searchNum > 0) 
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
		
		if(dataFd > 0)
		{
			close(dataFd);
			dataFd = 0;
		}

		//查询结束
		if(searchOverFlag == 1)
		{
			pSearchCondition->startAddr += searchOverAddr;
			
			break;
		}
		
		pSearchCondition->startTime = TimeToNextDay(pSearchCondition->startTime);
		
		//换天了，所以将起始地址清零
		pSearchCondition->startAddr = 0;
	}

	if(excelFd > 0)
	{
		close(excelFd);
		excelFd = 0;
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
int searchRecordsNum(unsigned int searchConditionFlag, search_condition_t *pSearchCondition)
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
	file_index_t fileIndex;
	//记录总数
	int recordNum = 0;
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
		
		sprintf(indexFile, "%s/%08d.index", g_record_lib.recordIndexPath, tmpTime);
		indexFd = open(indexFile, O_RDONLY | O_NOATIME, 0666);
		if(indexFd < 0)
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

			//记录合法
			if((fileIndex.recordTime >= pSearchCondition->startTime) 
				&& (fileIndex.recordTime <= pSearchCondition->endTime))
			{
				if(((searchConditionFlag & RECORD_CONDITION_ID) > 0) 
					&& (fileIndex.id != pSearchCondition->userId))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_USER_TYPE) > 0) 
					&& (fileIndex.userType != pSearchCondition->userType))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_TYPE) > 0) 
					&& (fileIndex.recogType != pSearchCondition->recogType))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_STATUS) > 0) 
					&& (fileIndex.recogStatus != pSearchCondition->recogStatus))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_SCORES) > 0)
						&& (fileIndex.recogScores >= pSearchCondition->recogMinScores) 
						&& (fileIndex.recogScores <= pSearchCondition->recogMaxScores))
				{
					continue;
				}
				
				if(((searchConditionFlag & RECORD_CONDITION_RECOG_REASON) > 0) 
					&& (fileIndex.recogReason != pSearchCondition->recogReason))
				{
					continue;
				}

				//判断用户是否存在
				if((fileIndex.id != 0) &&(searchUserById(fileIndex.id, NULL) == NULL))
				{
					continue;
				}

				/* 记录已删除 */
				if (fileIndex.fileStatus == 1)
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
		
		pSearchCondition->startAddr = 0;//换天了，所以将起始地址清零
	}

	return recordNum;
}

/* 获取下一条记录信息 */
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

/* 释放查询的记录 */
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

/************************************************************
 * 获取某条记录的照片
 * recordTime:	该条记录的时间
 * startAddr:	该条记录照片数据的起始地址
 * pPhotoBuf:	照片保存的buf(为NULL，则表示只校验记录数据是否正常，图片是否存在)
 *
 * 返回值:	>0:图片大小， -1：失败
 ************************************************************/
int getOneRecordPhoto(unsigned int recordTime, unsigned int startAddr, char *pPhotoBuf)
{
	int n = 0;
	int ret = -1;
	//存放记录照的数据文件
	int dataFd = 0;
	struct tm strTm;
	struct tm *pTm = &strTm;
	time_t tt;
	unsigned int tmpRecordTime = 0;
	file_index_t fileIndex;

	char startFlag[16] = {0};
	char endFlag[16] = {0};
	char dataFile[GENERAL_PATH_LEN] = {0};

	tt = recordTime;
	localtime_r(&tt, pTm);
	//将时间转换成20120224格式
	tmpRecordTime = (pTm->tm_year + 1900)*10000 + (pTm->tm_mon + 1)*100 + pTm->tm_mday;

	/* 打开记录照片数据 */
	sprintf(dataFile, "%s/%08d.data", g_record_lib.recordPicPath, tmpRecordTime);
	dataFd = open(dataFile, O_RDONLY | O_NOATIME, 0666);

	//照片数据存在
	if((dataFd > 0) && (lseek(dataFd, startAddr, SEEK_SET) >= 0))
	{
		/* 保存的记录的格式： RECORD_START_FLAG + 索引信息 + 照片 + RECORD_END_FLAG */
		n = read(dataFd, startFlag, strlen(RECORD_START_FLAG));
		//数据开始记录正确
		if((n == (int)(strlen(RECORD_START_FLAG))) 
			&& (memcmp(startFlag, RECORD_START_FLAG, strlen(RECORD_START_FLAG)) == 0))
		{
			n = read(dataFd, &fileIndex, sizeof(file_index_t));
			//索引大小正确
			if(n == sizeof(file_index_t))
			{
				//照片大小正常
				if((fileIndex.fileSize > 0) && (fileIndex.fileSize < MAX_PHOTO_FILE_SIZE))
				{
					//只校验图片是否正常
					if(pPhotoBuf == NULL)
					{
						n = lseek(dataFd, fileIndex.fileSize - strlen(RECORD_START_FLAG) - sizeof(file_index_t) - strlen(RECORD_END_FLAG), SEEK_CUR);
						//lseek返回成功，则将n赋值为要偏移的长度，以方便下面的判断
						if(n >= 0)
						{
							n = fileIndex.fileSize - strlen(RECORD_START_FLAG) - sizeof(file_index_t) - strlen(RECORD_END_FLAG);
						}
					}
					else
					{
						n = read(dataFd, pPhotoBuf, fileIndex.fileSize - strlen(RECORD_START_FLAG) - sizeof(file_index_t) - strlen(RECORD_END_FLAG));
					}

					//读取的文件大小正确
					if(n == (int)(fileIndex.fileSize - strlen(RECORD_START_FLAG) - sizeof(file_index_t) - strlen(RECORD_END_FLAG)))
					{
						n = read(dataFd, endFlag, strlen(RECORD_END_FLAG));
						//数据结束记录正确
						if((n == (int)(strlen(RECORD_END_FLAG))) && (memcmp(endFlag, RECORD_END_FLAG, strlen(RECORD_END_FLAG)) == 0))
						{
							ret = fileIndex.fileSize - strlen(RECORD_START_FLAG) - sizeof(file_index_t) - strlen(RECORD_END_FLAG);
						}
					}
				}
			}
		}

		if(dataFd > 0)
		{
			close(dataFd);
			dataFd = 0;
		}
	}

	return ret;
}

/* 显示记录缓冲队列信息 */
void showRecordQueueInfo()
{
	QueueTraverse(g_operateQueue);
}

void showRecordInfo()
{
	TRACE("record lib info:\r\n");
	TRACE("	storage status:		%d\r\n", g_record_lib.storageStatus);
	TRACE("	sd mount path:		(%s)\r\n", g_record_lib.sdCardPath);
	TRACE("	sd total size:		%d\r\n", g_record_lib.sdCardTotalSize);
	TRACE("	sd Free size:		%d\r\n", g_record_lib.sdCardFreeSize);
	TRACE("	flash mount path:	(%s)\r\n", g_record_lib.flashPath);
	TRACE("	flash total size:	%d\r\n", g_record_lib.flashTotalSize);
	TRACE("	flash Free size:	%d\r\n", g_record_lib.flashFreeSize);
	TRACE("	record pic path:	(%s)\r\n", g_record_lib.recordPicPath);
	TRACE("	record index path:	(%s)\r\n", g_record_lib.recordIndexPath);
}

/* 获取记录库的信息 */
void getRecordLibInfo(record_lib_t *p_record_lib)
{
	memcpy(p_record_lib, &g_record_lib, sizeof(record_lib_t));
}

void initRecordLib(record_lib_t *p_record_lib)
{
	memcpy(&g_record_lib, p_record_lib, sizeof(record_lib_t));

	//如果记录最大缓冲条目数大于最大条目
	if(g_record_lib.recordMaxBufNum > MAX_OPERATE_QUEUE)
	{
		g_record_lib.recordMaxBufNum = MAX_OPERATE_QUEUE;
	}

	sprintf(g_record_lib.recordIndexPath, "%s/%s", g_record_lib.recordIndexPath, RECORD_INDEX_DIR_NAME);
	mkdir(g_record_lib.recordIndexPath, 0666);
	
	sprintf(g_record_lib.recordPicPath, "%s/%s", g_record_lib.recordPicPath, RECORD_PIC_DIR_NAME);
	mkdir(g_record_lib.recordPicPath, 0666);

	pthread_mutex_init(&g_queueMutex, NULL);
	pthread_mutex_init(&g_writeMutex, NULL);
	
	InitQueue(&g_operateQueue);

	if(pthread_create(&g_saveRecordId, NULL, saveRecordThread, NULL) < 0)
	{
		TRACE("create saveRecordThread error\r\n");
	}
}

void deinitRecordLib()
{
	setRecordFlag(2);

	pthread_join(g_saveRecordId, NULL);

	/* 释放队列 */
	lockOperateQueue();
	DestroyQueue(&g_operateQueue);
	unlockOperateQueue();

	pthread_mutex_destroy(&g_queueMutex);
	pthread_mutex_destroy(&g_writeMutex);
}
 
/* 获取记录库中总记录的条数 */
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
	(void)GetRecordIndexPath(RcdPath);

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

