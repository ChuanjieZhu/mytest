
#ifndef __LOG_LIB_H_
#define __LOG_LIB_H_

#include "com.h"

#define GENERAL_PATH_LEN			128      			//路径的长度

#define SYSLOG_INDEX_DIR_NAME		"sysIndex"			//记录索引保存路径
#define MANLOG_INDEX_DIR_NAME       "manIndex"

//日志类型
enum __log_type_e
{
    eSysLog,        //系统日志
    eManLog,        //管理日志
    eUserLog,       //用户日志
    eAccessLog        //开门日志
};

//日志子类型
enum __log_sub_type_e
{
    eNormal,       //无管理员设备操作
    eAdmin,         //管理员设备操作
    eNetSdk,       //网络sdk操作 
    eUsb,           //usb操作
    eUseDevice,     //usbdevice操作
    eRs232,         //Rs232操作
    eRs485,         //Rs485操作
    eNetSync        //网络同步操作
};

enum __log_result_e
{
    eSucc,
    eFail
};

/* 操作类型定义 */

/* 用户管理操作 */
#define LOG_USER_ADD                0x00000001
#define LOG_USER_EDIT               0x00000002
#define LOG_USER_DEL_               0x00000003
#define LOG_USER_DEL_ALL            0x00000004

/* 管理员配置 */
#define LOG_ADMIN_ADD               0x0000000A
#define LOG_ADMIN_EDIT              0x0000000B
#define LOG_ADMIN_DEL               0x0000000C

/* 记录管理 */
#define LOG_RECORD_QUERY            0x00000010
#define LOG_RECORD_CLEAN            0x00000011
#define LOG_RECORD_SYSLOG_QUERY     0x00000012
#define LOG_RECORD_MANLOG_QUERY1    0x00000013
#define LOG_RECORD_DOORLOG_QUERY    0x00000014

/* u盘管理 */
#define LOG_USB_EXPORT_USER         0x00000020
#define LOG_USB_EXPORT_ALL_USER     0x00000021
#define LOG_USB_IMPORT_USER_TEMP    0x00000022
#define LOG_USB_IMPORT_USER_LIST    0x00000023
#define LOG_USB_SYSTEM_UPDATE       0x00000024

/* 系统设置 */
#define LOG_SYS_TIME_ZONE           0x00000030
#define LOG_SYS_TIME_DATE           0x00000031
#define LOG_SYS_TIME_TIME           0x00000032
#define LOG_SYS_TIME_FORMAT         0x00000033
#define LOG_SYS_TIME_NET_TIME       0x00000034
#define LOG_SYS_TIME_SYNC_TIME      0x00000035

#define LOG_SYS_SOUND_TOUCH         0x00000036
#define LOG_SYS_SOUND_PLAY          0x00000037

#define LOG_SYS_SLEEP_SETTING       0x00000038

#define LOG_SYS_RING_SETTING        0x00000039
#define LOG_SYS_RING_COUNT          0x0000003A
        
#define LOG_SYS_IO_SWITCH_OUT       0x0000003B   
#define LOG_SYS_IO_DURATION_RELAY   0x0000003C
#define LOG_SYS_DOOR_BELL           0x0000003D
#define LOG_SYS_LANGUAGE            0x0000003E
#define LOG_SYS_TIME_INTERVAL       0x0000003F
#define LOG_SYS_VERIFY_MANAGE       0x00000040
#define LOG_SYS_SECURITY            0x00000041
#define LOG_SYS_SCREEN_CALIBRATION  0x00000042
#define LOG_SYS_NETWORK             0x00000043

typedef struct __syslog_lib_t_
{
    int storageStatus;						//状态 0：正常

	unsigned int sdCardTotalSize;			//存储空间总大小
	unsigned int sdCardFreeSize;			//剩余空间
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//存储空间总大小
	unsigned int flashFreeSize;				//剩余空间
	char flashPath[GENERAL_PATH_LEN];		//

	char sysLogIndexPath[GENERAL_PATH_LEN];	//记录索引保存路径

	int curOperateStatus;					//当前操作状态

	int sysLogMaxBufNum;					//记录最大缓冲条数
	int sysLogSaveInterval;					//记录保存间隔时间
	int sysLogCurBufNum;					//当前记录缓冲条数
} syslog_lib_t;
    
typedef struct __manlog_lib_t_
{
    int storageStatus;						//状态 0：正常

	unsigned int sdCardTotalSize;			//存储空间总大小
	unsigned int sdCardFreeSize;			//剩余空间
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//存储空间总大小
	unsigned int flashFreeSize;				//剩余空间
	char flashPath[GENERAL_PATH_LEN];		//

	char manLogIndexPath[GENERAL_PATH_LEN];	//记录索引保存路径

	int curOperateStatus;					//当前操作状态

	int manLogMaxBufNum;					//记录最大缓冲条数
	int manLogSaveInterval;					//记录保存间隔时间
	int manLogCurBufNum;					//当前记录缓冲条数    
} manlog_lib_t;

typedef struct __accesslog_lib_t_
{
    int storageStatus;						//状态 0：正常

	unsigned int sdCardTotalSize;			//存储空间总大小
	unsigned int sdCardFreeSize;			//剩余空间
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//存储空间总大小
	unsigned int flashFreeSize;				//剩余空间
	char flashPath[GENERAL_PATH_LEN];		//

	char accessLogIndexPath[GENERAL_PATH_LEN];	//记录索引保存路径

	int curOperateStatus;					//当前操作状态

	int accessLogMaxBufNum;					//记录最大缓冲条数
	int accessLogSaveInterval;					//记录保存间隔时间
	int accessLogCurBufNum;					//当前记录缓冲条数    
} accesslog_lib_t;

/* 索引结构体 */
typedef struct __syslog_index_t_
{
	unsigned int logNo;			//从0开始计数
	char logFileStatus;			//日志状态  0：正常  1：删除
	unsigned int logStartAddr;	//指向的文件的起始地址
	int logFileSize;			//指向的文件的大小 
	char logType;               //日志类型
    char logSubType;            //日志子类型
    unsigned int logTime;       //日志时间
    int  logOpType;             //操作类型
    char logResult;             //操作结果
    char logMsg[MAX_MSG_LEN];   //操作详细结果
    char unused[64];            //保留 
} syslog_index_t;

typedef struct __syslog_info_t_
{
    int logNo;
    syslog_index_t logIndex;
    struct __syslog_info_t_ *next;
    struct __syslog_info_t_ *cur;
    
} syslog_info_t;

typedef struct __syslog_list_t_
{
    int logNum;
    syslog_info_t head;
} syslog_list_t;

/* 索引结构体 */
typedef struct __manlog_index_t_
{
	unsigned int logNo;			//从0开始计数
	char logFileStatus;			//文件状态  0：正常  1：删除
	unsigned int logStartAddr;	//指向的文件的起始地址
	int logFileSize;			//指向的文件的大小 
	char logType;               //日志类型
    char logSubType;            //日志子类型
    unsigned int logTime;       //日志时间
    char logUserName[MAX_USER_NAME_LEN]; //操作者姓名
    unsigned int logUserNo;          //用户工号
    unsigned int logUserId;          //用户id号
    int  logOpType;             //操作类型
    char logResult;             //操作结果
    char logMsg[MAX_MSG_LEN];   //操作详细结果
    char unused[64];            //保留 
} manlog_index_t;

typedef struct __manlog_info_t_
{
    int logNo;
    manlog_index_t logIndex;
    struct __manlog_info_t_ *next;
    struct __manlog_info_t_ *cur;
    
} manlog_info_t;

typedef struct __manlog_list_t_
{
    int logNum;
    manlog_info_t head;
} manlog_list_t;

typedef struct __accesslog_index_t_
{
	unsigned int logNo;			//从0开始计数
	char logStatus;				//文件状态  0：正常  1：删除
	unsigned int logStartAddr;	//指向的文件的起始地址
	char logType;               //日志类型
	char accessMode;               //开门类型
	char recogMode;				//识别模式
	int userNum;				//开门人数
	int userId[5];				//开门用户id，最多5个人一起开门
	int groupNum;				//开门组数
	int groupId[5];				//开门组id
    unsigned int accessTime;    //开门时间
    int result;					//开门结果
	int photoNum;               //照片数量
	unsigned int photoSize[5];  //照片大小
	unsigned int photoTime[5];  //照片记录生成时间
	unsigned int photoStartAddr[5]; //照片在记录中偏移位置
    char unused[12];            //保留 
} accesslog_index_t;

typedef struct __accesslog_info_t_
{
    int logNo;
    accesslog_index_t logIndex;
    struct __accesslog_index_t_ *next;
    struct __accesslog_index_t_ *cur;
} accesslog_info_t;

typedef struct __accesslog_list_t_
{
    int logNum;
    accesslog_info_t head;
} accesslog_list_t;


#ifdef __cplusplus
extern "C" {
#endif

#include "logOutLib.h"

void lockSysLogWriteData();
void unlockSysLogWriteData();
void lockManLogWriteData();
void unlockManLogWriteData();
int addSysLog(syslog_event_t sysLog);
int addManLog(manlog_event_t manLog);
int searchSysLogNum(unsigned int searchCondFlag, syslog_search_condition_t *pSearchCondition);
int searchManLogNum(unsigned int searchCondFlag, manlog_search_condition_t *pSearchCondition);
void showSysLogInfo();
void showManLogInfo();
int delOldLogFile(char *pPath, char cLogType);
void initLogLib(syslog_lib_t *pSysLogLib, manlog_lib_t *pManLogLib);
void deinitLogLib();

#ifdef __cplusplus
}
#endif

#endif
