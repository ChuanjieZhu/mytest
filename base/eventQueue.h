/*************************************************************************************
*
*	链式队列实现
*	
*
**************************************************************************************/

#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

typedef struct __operate_event_t_
{
	unsigned int operateType;//类型  bit0：数据库操作  bit1：记录照片
	unsigned int id;//用户id
	int nServerID;
	int nChannelID;
	int nThreshold;//识别门限
	int recogScores;//识别分数
	int nType;// 0:卡  1:其它
	int nStatus;//识别状态
	int nUserType;//用户类型
	int nLoginReason;//考勤原因
	unsigned int time;//事件产生的时间
	unsigned int fileSize;//记录照片文件大小 (包括开始头、索引信息、jpg照片、结束头的大小)
	char cRecogMode;//识别模式
	char *pFileData;//记录照片的图片数据
} operate_event_t;

/* 定义链式队列类型 */
typedef operate_event_t ElemType;

typedef struct QNode
{
	ElemType  data;
	struct QNode *next;
}QNode, *QueuePtr;

typedef struct
{
	QueuePtr front;//第一个
	QueuePtr rear;//最后一个
	int queueNum;//队列当前拥有的元素数目
}LinkQueue;

/* 记录信息 */
typedef struct __upload_event_t_
{
    char acStudentID[20];	/* 学生逻辑卡号 */
    char acCardID[20];		/* 学生物理卡号 */
    char acPicName[64];		/* 抓拍jpg照片路径名 */
    char acStudentPic[128];	/* 压缩照片路径名 */
    time_t tBaseTime;
	char cStatus;			/* 照片上传状态 0-未上传，1-已上传 */	
    char *pSaveJpg;
}ftp_upload_event_t;

/* 定义链式队列类型 */
typedef ftp_upload_event_t FtpElemType;

typedef struct FTPQNode
{
	FtpElemType  data;
	struct FTPQNode *next;
}FTPQNode, *FTPQueuePtr;

typedef struct
{
	FTPQueuePtr front;//第一个
	FTPQueuePtr rear;//最后一个
	int queueNum;//队列当前拥有的元素数目
	time_t iLastTime; /* 上次操作时间 */
}FTPLinkQueue;

extern pthread_mutex_t g_FtpUploadqueueMutex;//操作队列锁
extern FTPLinkQueue g_FtpUploadQueue;//队列

void InitQueue(LinkQueue *Q);
void DestroyQueue(LinkQueue *Q);
void ClearQueue(LinkQueue *Q);
int QueueEmpty(LinkQueue Q);
int QueueLength(LinkQueue Q);
ElemType *GetHead(LinkQueue Q);
void EnQueue(LinkQueue *Q, ElemType e);
void DeQueue(LinkQueue *Q, ElemType *e);
void QueueTraverse(LinkQueue Q);
int GetQueueElemNum(LinkQueue Q);

void InitFTPQueue(FTPLinkQueue *Q);
void DestroyFTPQueue(FTPLinkQueue *Q);
void EnFTPQueue(FTPLinkQueue *Q, FtpElemType *e);
void DeFTPQueue(FTPLinkQueue *Q, FtpElemType *e);

#endif//__EVENT_QUEUE_H__

