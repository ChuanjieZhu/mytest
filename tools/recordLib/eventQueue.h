#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include "recordLib.h"

/* ������ʽ�������� */
typedef operate_event_t ElemType;

typedef struct QNode
{
	ElemType  data;
	struct QNode *next;
}QNode, *QueuePtr;

typedef struct
{
	QueuePtr front;//��һ��
	QueuePtr rear;//���һ��
	int queueNum;//���е�ǰӵ�е�Ԫ����Ŀ
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

