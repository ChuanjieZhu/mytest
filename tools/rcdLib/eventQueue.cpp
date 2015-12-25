#include "stdlib.h"
#include "stdio.h"
#include <string.h>

#include "eventQueue.h"

#define Malloc malloc
#define Free free
#define TRACE printf

/* 1����ʼ����ʽ���� */
void InitQueue(LinkQueue *Q)
{
	memset(Q, 0, sizeof(LinkQueue));
	Q->front = Q->rear = (QueuePtr)Malloc(sizeof(QNode));
	if(Q->front == NULL)
	{
		TRACE("InitQueue Malloc error\r\n");
		return;
	}
	memset(Q->front, 0, sizeof(QNode));
	Q->front->next = NULL;
}


/* 2��������ʽ���� */
void DestroyQueue(LinkQueue *Q)
{
	while(Q->front)
	{
		Q->rear = Q->front->next;
		Free(Q->front);
		Q->front = Q->rear;
	}
}


/* 3�������ʽ���� */
void ClearQueue(LinkQueue *Q)
{
	QueuePtr p;
	p = Q->front->next;

    while (p)
	{
		Q->front->next = p->next;
		Free(p);
		p = Q->front->next;
	}
    
	Q->rear = Q->front;
}


/* 4���жϿն��� */
int QueueEmpty(LinkQueue Q)
{
	if(Q.front == Q.rear)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}


/* 5������ʽ���г��� */
int QueueLength(LinkQueue Q)
{
	QueuePtr p;
	int n = 0;
	p = Q.front;
	while(p != Q.rear)
	{
		n++;
		p = p->next;
	}
	return n;
}

/* 6��ȡ��ͷԪ�� */
ElemType *GetHead(LinkQueue Q)
{
	if(Q.front != Q.rear)
	{
		return &(Q.front->next->data);
	}
	return NULL;
}

/* 7������� */
void EnQueue(LinkQueue *Q, ElemType e)
{
	QueuePtr p;
	p = (QueuePtr)Malloc(sizeof(QNode));
	if(p == NULL)
	{
		TRACE("EnQueue Malloc error\r\n");
		return;
	}
	memcpy((char *)&(p->data), (char *)&e, sizeof(ElemType));
	p->next = NULL;
	Q->rear->next = p;
	Q->rear = p;
	Q->queueNum++;
}

/* 8�������� */
void DeQueue(LinkQueue *Q, ElemType *e)
{
	QueuePtr p;

	if(Q->front != Q->rear)
	{
		p = Q->front->next;
		if(p != NULL)
		{
			memcpy(e, &p->data, sizeof(ElemType) - 4);
			Q->front->next = p->next;
			if(Q->rear == p)
			{
				Q->rear = Q->front;
			}
			Free(p);
			p = NULL;
			Q->queueNum--;
		}
	}
}

/* 9��������ʽ���в����Ԫ�� */
void QueueTraverse(LinkQueue Q)
{
	QueuePtr p;
	TRACE("Queue: ");
	p = Q.front->next;
	if(p != NULL)
	{
		TRACE("total num %d\r\n", Q.queueNum);
		while(p)
		{
			//TRACE("%d\r\n", p->data.id);
			p = p->next;
		}
	}
	else
	{
		TRACE("total num 0\r\n");
	}
	TRACE("\r\n");
}

/* 10����ѯ����Ԫ�صĸ��� */
int GetQueueElemNum(LinkQueue Q)
{
	QueuePtr p;

	p = Q.front->next;
	if(p != NULL)
	{
		return Q.queueNum;
	}
	else
	{
		return 0;
	}

	return 0;
}

