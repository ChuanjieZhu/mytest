/*********************************************************************************************
 *	记录库
 *	主要用于维护保存在磁盘上的所有记录（考勤记录、记录照等）
 *
 *	作者：李双林
 *	创建时间：2012-01-30
 *********************************************************************************************/

#ifndef __RECORD_LIB_H__
#define __RECORD_LIB_H__

#include "../Comm.h"

#define GENERAL_PATH_LEN			128      			//路径的长度

#define MAX_PHOTO_FILE_SIZE			(30*1024)			//最大照片大小
#define STORAGE_MOUNT_SD_DIR		"/mnt/storage/sd"   //sd卡挂载的目录
#define STORAGE_MOUNT_FLASH_DIR		"/mnt/storage/flash"//flash挂载的目录
#define RECORD_INDEX_DIR_NAME		"rcdIndex"			//记录索引保存路径
#define RECORD_PIC_DIR_NAME			"rcdPic"			//记录照片保存路径

#define OPERATE_TYPE_RECORD_PIC		0x01				//数据类型  记录照片
#define OPERATE_TYPE_RECORD			0x02				//数据类型  记录条目

#define RECORD_START_FLAG			"[FIRS_START]"		//记录照片的开始标记
#define RECORD_END_FLAG				"[FIRS_END]"		//记录照片的结束标记




/* 记录维护库的错误类型 */
typedef enum recordLibErrorTypeE
{
	ERROR_RECORDLIB_INDEX_PATH_NOT_DIR = -100,			//记录索引保存路径错误， 不是目录
	ERROR_RECORDLIB_INDEX_PATH_NOT_CREATE,				//记录索引保存路径错误， 不能创建
	ERROR_RECORDLIB_PIC_PATH_NOT_DIR,					//记录照片保存路径错误， 不是目录
	ERROR_RECORDLIB_PIC_PATH_NOT_CREATE,				//记录照片保存路径错误， 不能创建

	ERROR_RECORDLIB_OTHER = -1,							//其他错误号
	ERROR_RECORDLIB_SUCCESS = 0,						//成功
}recordLibErrorTypeE;

typedef enum operateStateE
{
	eOPERATE_STATUS_FREE = 0,			//空闲状态
	eOPERATE_STATUS_RECORD,				//操作记录
	eOPERATE_STATUS_USER,				//操作用户
}operateStateE;

/******************************************
 * 数据库全局结构体
 * 用于保存数据存储路径、存储状态等
 ******************************************/
typedef struct __record_lib_t_
{
	int storageStatus;						//状态 0：正常

	unsigned int sdCardTotalSize;			//存储空间总大小
	unsigned int sdCardFreeSize;			//剩余空间
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//存储空间总大小
	unsigned int flashFreeSize;				//剩余空间
	char flashPath[GENERAL_PATH_LEN];		//

	char recordIndexPath[GENERAL_PATH_LEN];	//记录索引保存路径
	char recordPicPath[GENERAL_PATH_LEN];	//记录照片保存路径

	int curOperateStatus;					//当前操作状态

	int recordMaxBufNum;					//记录最大缓冲条数
	int recordSaveInterval;					//记录保存间隔时间
	int recordCurBufNum;					//当前记录缓冲条数
}record_lib_t;

/* 索引结构体 */
typedef struct __file_index_t_
{
	unsigned int no;			//从0开始计数
	char devNo;					//设备编号 号段前缀
	char fileType;				//指向的文件类型 1：记录照片
	char fileStatus;			//文件状态  0：正常  1：删除
	char userType;				//用户类型
	unsigned int id;			//用户Id
	int nServerID;
	int nChannelID;
	char recogType;				//识别类型	刷卡/一对多/...
	char recogStatus;			//识别的状态  成功/失败
	char recogScores;			//识别分数
	char recogReason;			//识别方式 快捷键
	unsigned int recordTime;	//记录产生的时间

	unsigned int startAddr;		//指向的文件的起始地址
	int fileSize;				//指向一条照片记录的大小
	char cRecogMode;			//识别模式
	char unused[11];			//保留字节
}file_index_t;



/* 查询的记录链表 */
typedef struct __record_info_t_
{
	int recordNo;					//当前记录号
	file_index_t fileIndex;
	struct __record_info_t_ *next;
	struct __record_info_t_ *cur;
}record_info_t;

typedef struct __record_list_t_
{
	int recordNum;					//总记录数
	record_info_t head;
}record_list_t;


#ifdef __cplusplus 
extern "C" { 
#endif

#include "../recordOutLib.h"

/******************************************************************************
 * 函数名称： getRecordLibInfo
 * 功能： 获取记录库的信息
 * 参数： p_record_lib：记录库指针
 * 返回： 无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void getRecordLibInfo(record_lib_t *p_record_lib);

/******************************************************************************
 * 函数名称： initRecordLib
 * 功能： 根据传入的信息初始化记录库
 * 参数： p_record_lib：记录库指针
 * 返回： 无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void initRecordLib(record_lib_t *p_record_lib);


/******************************************************************************
 * 函数名称： deinitRecordLib
 * 功能：释放已被初始化的记录库
 * 参数： 无
 * 返回： 无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void deinitRecordLib();


/******************************************************************************
 * 函数名称： addRecord
 * 功能：添加考勤记录
 * 参数：record：记录信息
 * 返回： 无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int addRecord(operate_event_t record);



/******************************************************************************
 * 函数名称： searchRecordsNum
 * 功能：查找记录的总条数
 * 参数：searchConditionFlag:	查询条件标记 通过searchConditionFlag来控制查询内容
         searchCondition:	查询条件的具体内容
 * 返回： >=0：记录条数 ， -1：失败
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int searchRecordsNum(unsigned int searchConditionFlag, search_condition_t *pSearchCondition);


/******************************************************************************
 * 函数名称： getNextRecordInfo
 * 功能：获取下一条记录信息
 * 参数：pRecordList
 * 返回： >=0：记录条数 ， -1：失败
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
file_index_t * getNextRecordInfo(record_list_t *pRecordList);


/******************************************************************************
 * 函数名称： freeSearchRecord
 * 功能：释放查询的记录
 * 参数：pRecordList:	查询的记录链表
 * 返回： 无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void freeSearchRecord(record_list_t *pRecordList);


/******************************************************************************
 * 函数名称： getOneRecordPhoto
 * 功能：获取某条记录的照片
 * 参数： recordTime:	该条记录的时间
          startAddr:	该条记录照片数据的起始地址
          pPhotoBuf:	照片保存的buf
 * 返回： 0：成功  其他：失败
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int getOneRecordPhoto(unsigned int recordTime, unsigned int startAddr, char *pPhotoBuf);


/******************************************************************************
 * 函数名称： lockWriteData
 * 功能：写数据 加锁
 * 参数：无
 * 返回：无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void lockWriteData();


/******************************************************************************
 * 函数名称： unlockWriteData
 * 功能： 写数据 解锁 
 * 参数：无
 * 返回：无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void unlockWriteData();

/******************************************************************************
 * 函数名称： showRecordInfo
 * 功能： 显示记录信息
 * 参数：无
 * 返回：无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void showRecordInfo();


/******************************************************************************
 * 函数名称： showRecordQueueInfo
 * 功能： 显示记录缓冲队列信息
 * 参数：无
 * 返回：无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void showRecordQueueInfo();


/******************************************************************************
 * 函数名称： checkStorageCapacity
 * 功能： 检查设备使用容量
 * 参数：pStoragePath	存储设备的路径
         pTotalSize	存储设备总的大小
         pFreeSize	存储设备剩余容间的大小
 * 返回：-1 出错 0 正常 1 使用空间超过90% 
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int checkStorageCapacity(char *pStoragePath, unsigned int *pTotalSize, unsigned int *pFreeSize);



/******************************************************************************
 * 函数名称： getRecordFlag
 * 功能： 获取标记
 * 参数：无
 * 返回：标记
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int getRecordFlag();



/******************************************************************************
 * 函数名称： setRecordFlag
 * 功能： 设置标记
 * 参数：flag：标记
 * 返回：0:正常  1:强制写入记录 2:退出写入记录线程
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void setRecordFlag(int flag);



/******************************************************************************
 * 函数名称： GetRecordPicPath
 * 功能： 获取记录照片保存路径
 * 参数： pPath:保存路径
 * 返回： 0：成功;	-1：失败
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int GetRecordPicPath(char* pPath);


/******************************************************************************
 * 函数名称： GetRecordIndexPath
 * 功能： 获取记录索引保存路径
 * 参数： pPath	记录索引路径存储空间
 * 返回： 0：成功;	-1：失败；
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int GetRecordIndexPath(char* pPath);

#ifdef __cplusplus 
}
#endif

#endif//__RECORD_LIB_H__

