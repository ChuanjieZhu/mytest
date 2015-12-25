#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include "recordLib.h"

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

#endif//__EVENT_QUEUE_H__

