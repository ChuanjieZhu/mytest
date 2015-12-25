
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <time.h>

#include "publicLib.h"
#include "logLib.h"

#define STORAGE_SD_PATH "/mnt/sd"
#define STORAGE_PATH    "/mnt/storage"

static int gSdCardExist = 0;
static int gSysLogTotalNum = 0;
static int gManLogTotalNum = 0;
static int gLogTotalNum = 0;

int AddSysOpAttendance(char cSubType, int iOpType, char cResult, char *pMsg)
{
    int iRet = -1;
    syslog_event_t log_info;
    time_t curTime = time(NULL);

    memset(&log_info, 0, sizeof(syslog_event_t));

    log_info.cType = eSysLog;
    log_info.cSubType = cSubType;
    log_info.uiTime = (unsigned int)curTime;
    log_info.iOpType = iOpType;
    log_info.cResult = cResult;

    if (pMsg != NULL)
    {
        memcpy(log_info.acMsg, pMsg, sizeof(log_info.acMsg) - 1);
    }

    printf("syslog: type %d, subType %d, time: %u, op: %d, reslut: %d, msg: %s %s %d\r\n",
        log_info.cType, log_info.cSubType, log_info.uiTime, log_info.iOpType, log_info.cResult,
        log_info.acMsg, __FUNCTION__, __LINE__);

    iRet = addSysLog(log_info);

    return iRet; 
}

int AddManOpAttendance(char cSubType, int iOpType, char cResult, char *pMsg, int iUserId)
{
    int iRet = -1;
    manlog_event_t log_info;
    time_t curTime = time(NULL);

#if 0
    LPUSERINFO pUserInfo = NULL;
    pUserInfo = GetUserInfoByUserId(iUserId);
    if (pUserInfo == NULL)
    {
        printf("getuserinfo error! %s %d\r\n", __FUNCTION__, __LINE__);
        return iRet;
    }

    
#else
    char name[] = "Admin";
    unsigned int uiUserId = (unsigned int)iUserId;
    unsigned int uiUserNo = 1;
#endif

    memset(&log_info, 0, sizeof(manlog_event_t));

    log_info.cType = eManLog;
    log_info.cSubType = cSubType;
    log_info.uiTime = (unsigned int)curTime;
    log_info.iOpType = iOpType;
    log_info.cResult = cResult;
    log_info.uiUserId = uiUserId;
    log_info.uiUserNo = uiUserNo;
    
    memcpy(log_info.acUserName, name, sizeof(log_info.acUserName) - 1);
    
    if (pMsg != NULL)
    {
        memcpy(log_info.acMsg, pMsg, sizeof(log_info.acMsg) - 1);
    }

    printf("syslog: type %d, subType %d, time: %u, name: %s, userid: %u, userno: %u, op: %d, reslut: %d, msg: %s %s %d\r\n",
        log_info.cType, log_info.cSubType, log_info.uiTime, log_info.acUserName, 
        log_info.uiUserId, log_info.uiUserNo, log_info.iOpType, log_info.cResult,
        log_info.acMsg, __FUNCTION__, __LINE__);

    iRet = addManLog(log_info);

    return iRet; 
}

void InitLogLib()
{
    syslog_lib_t syslog_lib;
    manlog_lib_t manlog_lib;

    memset(&syslog_lib, 0, sizeof(syslog_lib_t));
    memset(&manlog_lib, 0, sizeof(manlog_lib_t));

    if (gSdCardExist == 1)
    {
        strcpy(syslog_lib.sdCardPath, STORAGE_SD_PATH);
		strcpy(manlog_lib.sdCardPath, STORAGE_SD_PATH);
    }
    else
    {
        strcpy(syslog_lib.sdCardPath, STORAGE_PATH);
		strcpy(manlog_lib.sdCardPath, STORAGE_PATH);
    }

    strcpy(syslog_lib.flashPath, STORAGE_PATH);
	strcpy(syslog_lib.sysLogIndexPath, STORAGE_PATH);
    
    strcpy(manlog_lib.flashPath, STORAGE_PATH);
	strcpy(manlog_lib.manLogIndexPath, STORAGE_PATH);

    printf("sysLogSDpath: %s, sysIndexPath: %s, flash: %s %s %d\r\n", syslog_lib.sdCardPath, 
        syslog_lib.sysLogIndexPath, syslog_lib.flashPath, __FUNCTION__, __LINE__);

    printf("manlogSdpath: %s, manIndexPath: %s, flash: %s %s %d\r\n", manlog_lib.sdCardPath, 
        manlog_lib.manLogIndexPath, manlog_lib.flashPath, __FUNCTION__, __LINE__);
    
    //checkStorageCapacity((char *)STORAGE_PATH, &record_lib.flashTotalSize, &record_lib.flashFreeSize);
    
    syslog_lib.sysLogMaxBufNum = manlog_lib.manLogMaxBufNum = 10;
	syslog_lib.sysLogSaveInterval = manlog_lib.manLogSaveInterval = 5;

    initLogLib(&syslog_lib, &manlog_lib);

    gSysLogTotalNum = searchSysLogTotal();
    gManLogTotalNum = searchManLogTotal();
    gLogTotalNum = gSysLogTotalNum + gManLogTotalNum;

    printf("gSysLogTotalNum: %d, gManLogTotalNum: %d, gLogTotalNum: %d %s %d\r\n",
              gSysLogTotalNum, gManLogTotalNum, gLogTotalNum, __FUNCTION__, __LINE__);
}


int SearchAccessLog(unsigned int uiUserId, char *pBeginTime, char *pEndTime)
{
    int iRet = 0;
    time_t startTime = 0;
    time_t endTime = 0;
    char acTime[64] = {0};
    unsigned int searchConditionFlag = 0;
    
    accesslog_search_condition_t accessCond;

    if (pBeginTime && *pBeginTime != '\0')
    {
        startTime = DatetimeToTime(pBeginTime);    
    }
    else
    {
        sprintf(acTime, "2012-08-01 00:00:00");
        startTime = DatetimeToTime(acTime);
    }

    if (pEndTime && *pEndTime != '\0')
    {
        endTime = DatetimeToTime(pEndTime);
    }
    else
    {
        endTime = time(NULL);
    }

    memset(&accessCond, 0, sizeof(accesslog_search_condition_t));
    
    accessCond.startTime = startTime;
    accessCond.endTime = endTime;
    accessCond.searchNum = 2000;

    int i = 0;
    accesslog_list_t strLogList;

    if (uiUserId)
    {
        accessCond.userId = uiUserId;
        searchConditionFlag = LOG_CONDTITION_USERID;
    }
    
    printf("searchCondition.startTime:%d, searchCondition.endTime:%d \r\n", sysCond.startTime, sysCond.endTime);

    while (1)  
    {
        memset(&strLogList, 0, sizeof(accesslog_list_t));
            
        iRet = searchAccessLog(searchConditionFlag, &accessCond, &strLogList);

        printf("iRet: %d %s %d\r\n", iRet, __FUNCTION__, __LINE__);
        
        if (iRet)
        {
            break;
        }

        accesslog_info_t *pLogInfo = NULL;
        if (strLogList.head.cur != NULL)
        {
            pLogInfo = strLogList.head.next;
            while (pLogInfo)
            {
                printf("logtype: %d, accesstime: %u. \r\n", pLogInfo->logIndex.logType, pLogInfo->logIndex.accessTime);
                i++;
                pLogInfo = pLogInfo->next;
            }

            printf("%s %d\r\n", __FUNCTION__, __LINE__);
            
            freeSearchAccessLog(&strLogList);

            if(strLogList.logNum < 2000) 
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    return iRet;
}

/* 检索系统日志接口 */
int  SearchSysLog(char *pBeginTime, char *pEndTime)
{
    int iRet = 0;
    time_t startTime = 0;
    time_t endTime = 0;
    char acTime[64] = {0};
    
    syslog_search_condition_t sysCond;

    if (pBeginTime && *pBeginTime != '\0')
    {
        startTime = DatetimeToTime(pBeginTime);    
    }
    else
    {
        sprintf(acTime, "2012-08-01 00:00:00");
        startTime = DatetimeToTime(acTime);
    }

    if (pEndTime && *pEndTime != '\0')
    {
        endTime = DatetimeToTime(pEndTime);
    }
    else
    {
        endTime = time(NULL);
    }

    memset(&sysCond, 0, sizeof(syslog_search_condition_t));
    
    sysCond.startTime = startTime;
    sysCond.endTime = endTime;
    sysCond.searchNum = 2000;

    int i = 0;
    syslog_list_t strLogList;

    printf("searchCondition.startTime:%d, searchCondition.endTime:%d \r\n", sysCond.startTime, sysCond.endTime);

    while (1)  
    {
        memset(&strLogList, 0, sizeof(syslog_list_t));
            
        iRet = searchSysLog(0, &sysCond, &strLogList);

        printf("iRet: %d %s %d\r\n", iRet, __FUNCTION__, __LINE__);
        
        if (iRet)
        {
            break;
        }

        syslog_info_t *pLogInfo = NULL;
        if (strLogList.head.cur != NULL)
        {
            pLogInfo = strLogList.head.next;
            while (pLogInfo)
            {
                printf("logtype: %d, logsubtype: %d. \r\n", pLogInfo->logIndex.logType, pLogInfo->logIndex.logSubType);
                i++;
                pLogInfo = pLogInfo->next;
            }

            printf("%s %d\r\n", __FUNCTION__, __LINE__);
            
            freeSearchSysLog(&strLogList);

            if(strLogList.logNum < 2000) 
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    return iRet;
}

static void CheckSearchCondTime(char *pStartTime, char *pEndTime, time_t *tStartTime, time_t *tEndTime)
{
    if (pStartTime && *pStartTime != '\0')
    {
        *tStartTime = DatetimeToTime(pStartTime);
    }
    else
    {
        *tStartTime = DatetimeToTime((char *)"2012-08-01 00:00:00");
    }

    if (pEndTime && *pEndTime != '\0')
    {
        *tEndTime = DatetimeToTime(pEndTime);
    }
    else
    {
        *tEndTime = time(NULL);
    }
}


/*
 *  查询全部记录   
 */
int SearchManLogAll()
{
    int iRet = 0;
    int iCondFlag = LOG_CONDITION_ALL;
    time_t tStartTime = 0;
    time_t tEndTime = 0;
    manlog_list_t strLogList;
    manlog_search_condition_t strSearchCond;
    
    memset(&strSearchCond, 0, sizeof(manlog_search_condition_t));    
    
    tStartTime = DatetimeToTime((char *)"2012-08-01 00:00:00");
    tEndTime = time(NULL);
    
    strSearchCond.startTime = tStartTime;
    strSearchCond.endTime = tEndTime;
    strSearchCond.startAddr = 0;
    strSearchCond.searchNum = MAX_LOG_PER_QUERY;

    while (1)
    {
        memset(&strLogList, 0, sizeof(manlog_list_t));
        
        iRet = searchManLog(iCondFlag, &strSearchCond, &strLogList);
        if (iRet)
        {
            printf("searchSysLog fail. %s %d\r\n", __FUNCTION__, __LINE__);   
            break;
        }
        else
        {
            manlog_info_t *pstrLogInfo = NULL;
            
            if (strLogList.head.cur != NULL)
            {
                pstrLogInfo = strLogList.head.next;

                while (pstrLogInfo)
                {
                    /*
                     *  这里增加查询结果处理代码
                     */
                    printf("Msg: %s \r\n", pstrLogInfo->logIndex.logMsg); 
                    pstrLogInfo = pstrLogInfo->next;
                }

                freeSearchManLog(&strLogList);

                if (strLogList.logNum < MAX_LOG_PER_QUERY)
                {
                    break;
                }
            }
            else
            {
                printf("searchManLog no log for search! %s %d \r\n", __FUNCTION__, __LINE__);
                break;
            }
        }
    }
    
    return iRet;
}

int SearchManLogByTime(char *pStartTime, char *pEndTime)
{
    int iRet = 0;
    int iCondFlag = LOG_CONDITION_TIME;
    time_t tStartTime = 0;
    time_t tEndTime = 0;
    manlog_list_t strLogList;
    manlog_search_condition_t strSearchCond;

    CheckSearchCondTime(pStartTime, pEndTime, &tStartTime, &tEndTime);

    printf("start: %lu, end: %lu \r\n", tStartTime, tEndTime);

    memset(&strSearchCond, 0, sizeof(manlog_search_condition_t));
    strSearchCond.startTime = tStartTime;
    strSearchCond.endTime = tEndTime;
    strSearchCond.startAddr = 0;
    strSearchCond.searchNum = MAX_LOG_PER_QUERY;

    while (1)
    {
        memset(&strLogList, 0, sizeof(manlog_list_t));

        iRet = searchManLog(iCondFlag, &strSearchCond, &strLogList);
        if (iRet)
        {
            printf("searchManLog error! \r\n");
            break;
        }

        manlog_info_t *pstrLogInfo = NULL;
        
        if (strLogList.head.cur != NULL)
        {
            pstrLogInfo = strLogList.head.next;
            while (pstrLogInfo)
            {
                /*
                 * 这里增加查询结果处理
                 */
                printf("Msg: %s \r\n", pstrLogInfo->logIndex.logMsg);
                pstrLogInfo = pstrLogInfo->next;
            }
            
            freeSearchManLog(&strLogList);

            if(strLogList.logNum < MAX_LOG_PER_QUERY) 
            {
                break;
            }
        }
        else
        {
            printf("searchManLog no log for search! %s %d \r\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    return iRet;
}

int SearchManLogByName(char *pUserName)
{
    int iRet = 0;
    int iCondFlag = LOG_CONDTITION_USERNAME;
    time_t tStartTime = 0;
    time_t tEndTime = 0;
    manlog_list_t strLogList;
    manlog_search_condition_t strSearchCond;

    tStartTime = DatetimeToTime((char *)"2012-08-01 00:00:00");
    tEndTime = time(NULL);

    printf("start: %lu, end: %lu \r\n", tStartTime, tEndTime);

    memset(&strSearchCond, 0, sizeof(manlog_search_condition_t));
    strSearchCond.startTime = tStartTime;
    strSearchCond.endTime = tEndTime;
    memcpy(strSearchCond.userName, pUserName, MAX_USER_NAME_LEN - 1);
    strSearchCond.startAddr = 0;
    strSearchCond.searchNum = MAX_LOG_PER_QUERY;

    while (1)
    {
        memset(&strLogList, 0, sizeof(manlog_list_t));

        iRet = searchManLog(iCondFlag, &strSearchCond, &strLogList);
        if (iRet)
        {
            printf("searchManLog error! \r\n");
            break;
        }

        manlog_info_t *pstrLogInfo = NULL;
        
        if (strLogList.head.cur != NULL)
        {
            pstrLogInfo = strLogList.head.next;
            while (pstrLogInfo)
            {
                /*
                 * 这里增加查询结果处理
                 */
                pstrLogInfo = pstrLogInfo->next;
            }
            
            freeSearchManLog(&strLogList);

            if(strLogList.logNum < MAX_LOG_PER_QUERY) 
            {
                break;
            }
        }
        else
        {
            printf("searchManLog no log for search! %s %d\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    return iRet;
}

int SearchManLogByUserNo(int iUserNo)
{
    int iRet = 0;
    int iCondFlag = LOG_CONDTITION_USERNO;
    time_t tStartTime = 0;
    time_t tEndTime = 0;
    manlog_list_t strLogList;
    manlog_search_condition_t strSearchCond;

    tStartTime = DatetimeToTime((char *)"2012-08-01 00:00:00");
    tEndTime = time(NULL);

    printf("start: %lu, end: %lu \r\n", tStartTime, tEndTime);

    memset(&strSearchCond, 0, sizeof(manlog_search_condition_t));
    strSearchCond.startTime = tStartTime;
    strSearchCond.endTime = tEndTime;
    strSearchCond.userNo = iUserNo;
    strSearchCond.startAddr = 0;
    strSearchCond.searchNum = MAX_LOG_PER_QUERY;

    while (1)
    {
        memset(&strLogList, 0, sizeof(manlog_list_t));

        iRet = searchManLog(iCondFlag, &strSearchCond, &strLogList);
        if (iRet)
        {
            printf("searchManLog error! \r\n");
            break;
        }

        manlog_info_t *pstrLogInfo = NULL;
        
        if (strLogList.head.cur != NULL)
        {
            pstrLogInfo = strLogList.head.next;
            
            while (pstrLogInfo)
            {
                /*
                 * 这里增加查询结果处理
                 */
                pstrLogInfo = pstrLogInfo->next;
            }
            
            freeSearchManLog(&strLogList);

            if(strLogList.logNum < MAX_LOG_PER_QUERY) 
            {
                break;
            }
        }
        else
        {
            printf("searchManLog no log for search! %s %d\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    return iRet;
}


int main()
{
    int iRet = -1;
    int i;
    int iFlag = 0;
    int iSysLogTotal = 0;
    int iManLogTotal = 0;
    
    InitLogLib();

    usleep(200 * 1000);
#if 1
    
    while (1)
    {
        if (iFlag == 0)
        {
            for (i = 0; i < 10; i++)
            {
                printf("i: %d \r\n", i);
                //AddSysOpAttendance(eNetSdk, 1, eSucc, (char *)"add user");
                AddManOpAttendance(eAdmin, 1, eSucc, (char *)"add user", 10000);
                sleep(1);      
            }

            if (i >= 10)
            {
                iFlag = 1;
            }
        }

        if (iFlag == 1)
        {
            sleep(5);
            break;
        }
        
        usleep(200 * 1000);
    }
#endif

    iSysLogTotal = searchSysLogTotal();
    iManLogTotal = searchManLogTotal();

    printf("syslogTotal: %d %s %d\r\n", iSysLogTotal, __FUNCTION__, __LINE__);
    printf("manlogTotal: %d %s %d\r\n", iManLogTotal, __FUNCTION__, __LINE__);

    //SearchSysLog(NULL, NULL);
    SearchManLogAll();
    sleep(1);
    SearchManLogByName((char *)"Admin");
    sleep(1);
    SearchManLogByTime(NULL, NULL);
    sleep(1);
    SearchManLogByUserNo(1000);
    
    deinitLogLib();
    
    return 0;
}


