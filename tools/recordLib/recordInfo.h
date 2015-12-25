#ifndef __RECORD_INFO_H__
#define __RECORD_INFO_H__

#define MAX_OPERATE_STORAGE_INTERVAL_TIME	300//保存数据到存储设备上的间隔时间 单位秒
#define MAX_OPERATE_QUEUE			30//最大缓冲的记录条目数
#define MAX_FILE_INDEX_SIZE			64//最大记录索引信息长度
#define MAX_RECORD_FILE_NAME_LEN		16//保存记录照片的文件名的最大长度
#define MAX_RECORD_FILE_PATH_LEN		128//存储照片的文件的路径

/* 数据缓冲 */
typedef struct __record_buffer_t_
{
	unsigned int time;//缓冲的时间  只取年月日

	char picFileName[MAX_RECORD_FILE_PATH_LEN];//照片存储的文件
	int startAddr;//要写入的照片的文件的起始地址
	int picOffset;//照片偏移的大小
	char *pPicBuffer;//照片缓冲

	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//索引存储的文件
	int startIndexNo;//要写入的索引的起始编号
	int indexOffset;//索引偏移的大小
	char indexBuffer[MAX_OPERATE_QUEUE*MAX_FILE_INDEX_SIZE];//索引缓冲
}record_buffer_t;

#if 0
/* 获取磁盘总大小 */
int getStorageTotalSize(int *pSdTotalSize, int *pFlashTotalSize);

/* 获取磁盘空闲空间大小 */
int getStorageFreeSize(int *pSdTotalSize, int *pFlashTotalSize);
#endif

#endif//__RECORD_INFO_H__

