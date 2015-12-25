#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include "logLib.h"

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


typedef syslog_event_t SysLogElemType;

typedef struct SysLogQNode
{
    SysLogElemType data;
    struct SysLogQNode *next;
} SysLogQNode, *SysLogQueuePtr;

typedef struct
{
   SysLogQueuePtr front;
   SysLogQueuePtr rear;
   int queueNum;
} SysLogLinkQueue;

typedef manlog_event_t ManLogElemType;

typedef struct ManLogQNode
{
    ManLogElemType data;
    struct ManLogQNode *next;
} ManLogQNode, *ManLogQueuePtr;

typedef struct
{
   ManLogQueuePtr front;
   ManLogQueuePtr rear;
   int queueNum;
} ManLogLinkQueue;

typedef accesslog_event_t AccessLogElemType;

typedef struct AccessLogQNode
{
	AccessLogElemType data;
	struct AccessLogQNode *next;
} AccessLogQNode, *AccessLogQueuePtr;

typedef struct
{
	AccessLogQueuePtr front;
	AccessLogQueuePtr rear;
	int queueNum;
} AccessLogLinkQueue;

void InitQueue(LinkQueue *Q);
void InitSysLogQueue(SysLogLinkQueue * Q);
void InitManLogQueue(ManLogLinkQueue * Q);

void DestroyQueue(LinkQueue *Q);
void DestroySysLogQueue(SysLogLinkQueue * Q);
void DestroyManLogQueue(ManLogLinkQueue * Q);

void ClearQueue(LinkQueue *Q);
int QueueEmpty(LinkQueue Q);
int QueueLength(LinkQueue Q);
ElemType *GetHead(LinkQueue Q);
void EnQueue(LinkQueue *Q, ElemType e);
void DeQueue(LinkQueue *Q, ElemType *e);

void EnSysLogQueue(SysLogLinkQueue *Q, SysLogElemType e);
void DeSysLogQueue(SysLogLinkQueue *Q, SysLogElemType *e);

void EnManLogQueue(ManLogLinkQueue *Q, ManLogElemType e);
void DeManLogQueue(ManLogLinkQueue *Q, ManLogElemType *e);

void QueueTraverse(LinkQueue Q);
void QueueSysLogTraverse(SysLogLinkQueue Q);
void QueueManLogTraverse(ManLogLinkQueue Q);

int GetQueueElemNum(LinkQueue Q);
int GetQueueSysLogElemNum(SysLogLinkQueue Q);
int GetQueueManLogElemNum(ManLogLinkQueue Q);

void InitAccessLogQueue(AccessLogLinkQueue *Q);
void DestroyAccessLogQueue(AccessLogLinkQueue *Q);
void EnAccessLogQueue(AccessLogLinkQueue *Q, AccessLogElemType e);
void DeAccessLogQueue(AccessLogLinkQueue *Q, AccessLogElemType e);
int GetAccessLogQueueElemNum(AccessLogLinkQueue Q);

#endif//__EVENT_QUEUE_H__

