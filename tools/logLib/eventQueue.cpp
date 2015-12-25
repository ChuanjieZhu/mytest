#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eventQueue.h"

/* 1、初始化链式队列 */
void InitQueue(LinkQueue *Q)
{
	memset(Q, 0, sizeof(LinkQueue));
	Q->front = Q->rear = (QueuePtr)malloc(sizeof(QNode));
	if(Q->front == NULL)
	{
		printf("InitQueue malloc error\r\n");
		return;
	}
	memset(Q->front, 0, sizeof(QNode));
	Q->front->next = NULL;
}

void InitSysLogQueue(SysLogLinkQueue *Q)
{
	memset(Q, 0, sizeof(SysLogLinkQueue));
	Q->front = Q->rear = (SysLogQueuePtr)malloc(sizeof(SysLogQNode));
	if(Q->front == NULL)
	{
		printf("InitSysLogQueue malloc error\r\n");
		return;
	}
	memset(Q->front, 0, sizeof(SysLogQNode));
	Q->front->next = NULL;
}

void InitManLogQueue(ManLogLinkQueue *Q)
{
	memset(Q, 0, sizeof(ManLogLinkQueue));
	Q->front = Q->rear = (ManLogQueuePtr)malloc(sizeof(ManLogQNode));
	if(Q->front == NULL)
	{
		printf("InitSysLogQueue malloc error\r\n");
		return;
	}
	memset(Q->front, 0, sizeof(ManLogQNode));
	Q->front->next = NULL;
}

void InitAccessLogQueue(AccessLogLinkQueue *Q)
{
    memset(Q, 0, sizeof(AccessLogLinkQueue));
    Q->front = Q->rear = (AccessLogQueuePtr)malloc(sizeof(AccessLogQNode));
    if (Q->front == NULL)
    {
        printf("InitAccessLogQueue malloc error\r\n");
        return;
    }
    memset(Q->front, 0, sizeof(AccessLogQNode));
    Q->front->next = NULL;
}

/* 2、销毁链式队列 */
void DestroyQueue(LinkQueue *Q)
{
	while(Q->front)
	{
		Q->rear = Q->front->next;
		if(Q->front->data.pFileData != NULL)
		{
			free(Q->front->data.pFileData);
			Q->front->data.pFileData = NULL;
		}
		free(Q->front);
		Q->front = Q->rear;
	}
}

void DestroySysLogQueue(SysLogLinkQueue *Q)
{
	while(Q->front)
	{
		Q->rear = Q->front->next;
		free(Q->front);
		Q->front = Q->rear;
	}
}

void DestroyManLogQueue(ManLogLinkQueue *Q)
{
	while(Q->front)
	{
		Q->rear = Q->front->next;
		free(Q->front);
		Q->front = Q->rear;
	}
}

void DestroyAccessLogQueue(AccessLogLinkQueue *Q)
{
    while (Q->front)
    {
        Q->rear = Q->front->next;
        free(Q->front);
        Q->front = Q->rear;
    }
}

/* 3、清空链式队列 */
void ClearQueue(LinkQueue *Q)
{
	QueuePtr p;
	p = Q->front->next;
	while(p)
	{
		Q->front->next = p->next;
		if(p->data.pFileData != NULL)
		{
			free(p->data.pFileData);
			p->data.pFileData = NULL;
		}
		free(p);
		p = Q->front->next;
	}
	Q->rear = Q->front;
}

/* 4、判断空队列 */
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

/* 5、求链式队列长度 */
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

/* 6、取队头元素 */
ElemType *GetHead(LinkQueue Q)
{
	if(Q.front != Q.rear)
	{
		return &(Q.front->next->data);
	}
	return NULL;
}

/* 7、入队列 */
void EnQueue(LinkQueue *Q, ElemType e)
{
	QueuePtr p;
	p = (QueuePtr)malloc(sizeof(QNode));
	if(p == NULL)
	{
		printf("EnQueue malloc error\r\n");
		return;
	}
	memcpy((char *)&(p->data), (char *)&e, sizeof(ElemType) - 4);
	p->data.pFileData = (char *)malloc(((p->data.fileSize+3)/4)*4);
	memcpy(p->data.pFileData, e.pFileData, p->data.fileSize);
	p->next = NULL;
	Q->rear->next = p;
	Q->rear = p;
	Q->queueNum++;
}

/* 8、出队列 */
void DeQueue(LinkQueue *Q, ElemType *e)
{
	QueuePtr p;

	if(Q->front != Q->rear)
	{
		p = Q->front->next;
		if(p != NULL)
		{
			memcpy(e, &p->data, sizeof(ElemType) - 4);
			if(p->data.pFileData != NULL)
			{
				if(e->pFileData != NULL)
				{
					memcpy(e->pFileData, p->data.pFileData, p->data.fileSize);
					free(p->data.pFileData);
					p->data.pFileData = NULL;
				}
			}
			Q->front->next = p->next;
			if(Q->rear == p)
			{
				Q->rear = Q->front;
			}
			free(p);
			p = NULL;
			Q->queueNum--;
		}
	}
}

void EnAccessLogQueue(AccessLogLinkQueue *Q, AccessLogElemType e)
{
    int i;
    AccessLogQueuePtr p;
    p = (AccessLogQueuePtr)malloc(sizeof(AccessLogQNode));
    if (p == NULL)
    {
        printf("EnAccessLogQueue malloc error\r\n");
        return;
    }

    memcpy((char *)&(p->data), (char *)&e, (sizeof(AccessLogElemType) - (4 * 5)));

    for (i = 0; i < e.iPhotoNum; i++)
    {
	    p->data.pFileData[i] = (char *)malloc(((p->data.uiFileSize[i] + 3) / 4) * 4);
        memcpy(p->data.pFileData[i], e.pFileData[i], p->data.uiFileSize[i]);
    }
	
	p->next = NULL;
	Q->rear->next = p;
	Q->rear = p;
	Q->queueNum++;
    
}

void DeAccessLogQueue(AccessLogLinkQueue *Q, AccessLogElemType *e)
{
    int i;
    AccessLogQueuePtr p;
    
    if(Q->front != Q->rear)
	{
		p = Q->front->next;
		if(p != NULL)
		{
			memcpy(e, &p->data, sizeof(AccessLogElemType) - 4 * 5);

            for (i = 0; i < e->iPhotoNum; i++)
            {
			    if(p->data.pFileData[i] != NULL)
    			{
    				if(e->pFileData[i] != NULL)
    				{
    					memcpy(e->pFileData[i], p->data.pFileData[i], p->data.uiFileSize[i]);
    					free(p->data.pFileData[i]);
    					p->data.pFileData[i] = NULL;
    				}
    			}
            }
            
			Q->front->next = p->next;
			if(Q->rear == p)
			{
				Q->rear = Q->front;
			}
            
			free(p);
			p = NULL;
			Q->queueNum--;
		}
	}
    
}

void EnSysLogQueue(SysLogLinkQueue *Q, SysLogElemType e)
{
    SysLogQueuePtr p;
    p = (SysLogQueuePtr)malloc(sizeof(SysLogQNode));
    if (p == NULL)
    {
        printf("EnSysLogQueue malloc error %s %d\r\n", __FUNCTION__, __LINE__);
        return;
    }

    memcpy((char *)&(p->data), (char *)&e, sizeof(SysLogElemType));
    p->next = NULL;
    Q->rear->next = p;
    Q->rear = p;
    Q->queueNum++;
}

void DeSysLogQueue(SysLogLinkQueue *Q, SysLogElemType *e)
{
	SysLogQueuePtr p;

	if(Q->front != Q->rear)
	{
		p = Q->front->next;
		if(p != NULL)
		{
			memcpy(e, &p->data, sizeof(SysLogElemType));
			Q->front->next = p->next;
			if(Q->rear == p)
			{
				Q->rear = Q->front;
			}
			free(p);
			p = NULL;
			Q->queueNum--;
		}
	}
}

void EnManLogQueue(ManLogLinkQueue *Q, ManLogElemType e)
{
    ManLogQueuePtr p;
    p = (ManLogQueuePtr)malloc(sizeof(ManLogQNode));
    if (p == NULL)
    {
        printf("EnManLogQueue malloc error %s %d\r\n", __FUNCTION__, __LINE__);
        return;
    }

    memcpy((char *)&(p->data), (char *)&e, sizeof(ManLogElemType));
    p->next = NULL;
    Q->rear->next = p;
    Q->rear = p;
    Q->queueNum++;
}

void DeManLogQueue(ManLogLinkQueue *Q, ManLogElemType *e)
{
	ManLogQueuePtr p;

	if(Q->front != Q->rear)
	{
		p = Q->front->next;
		if(p != NULL)
		{
			memcpy(e, &p->data, sizeof(ManLogElemType));
			Q->front->next = p->next;
			if(Q->rear == p)
			{
				Q->rear = Q->front;
			}
			free(p);
			p = NULL;
			Q->queueNum--;
		}
	}
}


/* 9、遍历链式队列并输出元素 */
void QueueTraverse(LinkQueue Q)
{
	QueuePtr p;
	printf("Queue: ");
	p = Q.front->next;
	if(p != NULL)
	{
		printf("total num %d\r\n", Q.queueNum);
		while(p)
		{
			printf("%d\r\n", p->data.id);
			p = p->next;
		}
	}
	else
	{
		printf("total num 0\r\n");
	}
	printf("\r\n");
}

void QueueSysLogTraverse(SysLogLinkQueue Q)
{
	SysLogQueuePtr p;
	printf("Queue: ");
	p = Q.front->next;
	if(p != NULL)
	{
		printf("total num %d\r\n", Q.queueNum);
		while(p)
		{
			printf("%d %d\r\n", p->data.cType, p->data.cSubType);
			p = p->next;
		}
	}
	else
	{
		printf("total num 0\r\n");
	}
	printf("\r\n");
}

void QueueManLogTraverse(ManLogLinkQueue Q)
{
	ManLogQueuePtr p;
	printf("Queue: ");
	p = Q.front->next;
	if(p != NULL)
	{
		printf("total num %d\r\n", Q.queueNum);
		while(p)
		{
			printf("%d %d\r\n", p->data.cType, p->data.cSubType);
			p = p->next;
		}
	}
	else
	{
		printf("total num 0\r\n");
	}
	printf("\r\n");
}

/* 10、查询队列元素的个数 */
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

int GetQueueSysLogElemNum(SysLogLinkQueue Q)
{
	SysLogQueuePtr p;

	p = Q.front->next;
	if(p != NULL)
	{
		return Q.queueNum;
	}

	return 0;
}

int GetQueueManLogElemNum(ManLogLinkQueue Q)
{
    ManLogQueuePtr p;
    p = Q.front->next;
	if(p != NULL)
	{
		return Q.queueNum;
	}

	return 0;
}

int GetAccessLogQueueElemNum(AccessLogLinkQueue Q)
{
    AccessLogQueuePtr p;

    p = Q.front->next;
    if (p != NULL)
    {
        return Q.queueNum;
    }

    return 0;
}

