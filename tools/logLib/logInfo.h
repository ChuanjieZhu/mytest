#ifndef __LOG_INFO_H_
#define __LOG_INFO_H_

#define MAX_OPERATE_STORAGE_INTERVAL_TIME	300//保存数据到存储设备上的间隔时间 单位秒
#define MAX_OPERATE_QUEUE			10          //最大缓冲的记录条目数
#define MAX_FILE_INDEX_SIZE			256          //最大记录索引信息长度
#define MAX_RECORD_FILE_NAME_LEN		16      //保存记录照片的文件名的最大长度
#define MAX_RECORD_FILE_PATH_LEN		128     //存储照片的文件的路径

typedef struct __syslog_buffer_t_
{
	unsigned int time;//缓冲的时间  只取年月日
	
	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//索引存储的文件
	int startIndexNo;//要写入的索引的起始编号
	int indexOffset;//索引偏移的大小
	char indexBuffer[MAX_OPERATE_QUEUE * MAX_FILE_INDEX_SIZE];//索引缓冲
} syslog_buffer_t;

typedef struct __manlog_buffer_t_
{
	unsigned int time;//缓冲的时间  只取年月日
	
	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//索引存储的文件
	int startIndexNo;//要写入的索引的起始编号
	int indexOffset;//索引偏移的大小
	char indexBuffer[MAX_OPERATE_QUEUE * MAX_FILE_INDEX_SIZE];//索引缓冲
} manlog_buffer_t;

typedef struct __accesslog_buffer_t_
{
    unsigned int time;//缓冲的时间  只取年月日
	
	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//索引存储的文件
	int startIndexNo;//要写入的索引的起始编号
	int indexOffset;//索引偏移的大小
	char indexBuffer[MAX_OPERATE_QUEUE * MAX_FILE_INDEX_SIZE];//索引缓冲
} accesslog_buffer_t;

#endif //__LOG_INFO_H_
