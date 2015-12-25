#include "recordLib.h"

#define STORAGE_PATH    "/mnt/storage"
#define STORAGE_SD_PATH "/mnt/sd"

int gRecordTotal = 0;

void InitRecordLib()
{
	/* 初始化记录库 */
	record_lib_t record_lib;
	
	memset(&record_lib, 0, sizeof(record_lib_t));

	strcpy(record_lib.sdCardPath, STORAGE_PATH);
	strcpy(record_lib.recordPicPath, STORAGE_PATH);

	strcpy(record_lib.flashPath, STORAGE_PATH);
	strcpy(record_lib.recordIndexPath, STORAGE_PATH);
	
	//checkStorageCapacity((char *)STORAGE_PATH, &record_lib.flashTotalSize, &record_lib.flashFreeSize);

	record_lib.recordMaxBufNum = 30;
	record_lib.recordSaveInterval = 300;

	initRecordLib(&record_lib);

	//记录库中的记录总条数
	gRecordTotal = searchRecordTotal();
}

int SearchRecord(int iUserId, char *pBeginTime, char *pEndTime)
{
    time_t startTime = 0;
    time_t endTime = 0;
    record_list_t recordList;
    search_condition_t searchCondition;
    unsigned int searchConditionFlag = 0;
    user_data_t userData;
    user_index_t *pUserIndex = NULL;
    int ret = -1;
    int iSearchUserId = 0;

    //struct item *pRec = NULL;

    //giAttendanceRec = 0;

    //feature_q_init(&g_item_list);
    
    if(pBeginTime && *pBeginTime != '\0')
    {
        startTime = DatetimeToTime(pBeginTime);
    }

    if(pEndTime && *pEndTime != '\0') 
    {
        endTime = DatetimeToTime(pEndTime);
    }
    else
    {
        endTime = time(NULL);
    }

    memset(&searchCondition, 0, sizeof(search_condition_t));
    searchCondition.startTime = startTime;
    searchCondition.endTime = endTime;
    searchCondition.searchNum = MAX_RECORD_PER_QUERY;

    if(iUserId)
    {
        searchCondition.userId = iUserId;
        
		/* 根据用户ID查询记录 */
		searchConditionFlag = RECORD_CONDITION_ID;
    }

    TRACE("searchCondition.startTime:%d, searchCondition.endTime:%d, iUserId:%d\r\n", searchCondition.startTime, searchCondition.endTime, iUserId);

    while(1)
    {	
		if (MessageGet(MSG_SEARCH_RECORD_QUIT, NULL, 0, 0) >= 0)
		{
			break;
		}
		
        memset(&recordList, 0, sizeof(record_list_t));
        ret = searchRecord(searchConditionFlag, &searchCondition, &recordList);
        if(ret)
        {
            break;
        }

        record_info_t *p_recordInfo = NULL;
        if(recordList.head.cur!= NULL)
        {
        	p_recordInfo = recordList.head.next;
        	while(p_recordInfo)
        	{
				if (MessageGet(MSG_SEARCH_RECORD_QUIT, NULL, 0, 0) >= 0)
				{
					MessageSend(MSG_SEARCH_RECORD_QUIT, NULL, 0);
					break;
				}
				
        		iSearchUserId = p_recordInfo->fileIndex.id;

                memset(&userData, 0, sizeof(user_data_t));
                pUserIndex = searchUserById(iSearchUserId, &userData);
                if((pUserIndex) || (iSearchUserId == 0))    /* id非零，且找不到用户的记录过滤掉，ID为0的记录保留 */
                {
                    pRec = (struct item *)Malloc(sizeof(struct item));
                    if(pRec == NULL)
                    {
                        break;
                    }
                    
                    memset(pRec, 0, sizeof(struct item));
                    if (pUserIndex)
                    {
                        strncpy(pRec->caName, userData.name, 16);
                        pRec->iUserNo = userData.userno;
                    }
                    else
                    {
                        pRec->caName[0] = '0';
                        pRec->iUserNo = 0;
                    }

                    pRec->recordTime = p_recordInfo->fileIndex.recordTime;
                    feature_q_add(&g_item_list, (FEATURE_OBJECT*)pRec);
                    giAttendanceRec++;
                }
                else
                {
                    TRACE("$$$ searchUserById failed %s %d\r\n", __FUNCTION__, __LINE__);
                }

                p_recordInfo = p_recordInfo->next;
        	}

        	freeSearchRecord(&recordList);
        	
            if(recordList.recordNum < MAX_RECORD_PER_QUERY) 
            {
                break;
            }
        }
        else
        {
			/* 没找到记录 */
            break;
        }
    }

	if (giAttendanceRec == 0)
	{
		giAttendanceRec = -1;
	}
    
    return ret;
}
