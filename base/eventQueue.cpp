
/******************************************************************************************
*	链式队列实现
*	
*
******************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eventQueue.h"

pthread_mutex_t g_FtpUploadqueueMutex;//操作队列锁
FTPLinkQueue g_FtpUploadQueue;//队列

/* 1、初始化链式队列 */
void InitQueue(LinkQueue *Q)
{
	memset(Q, 0, sizeof(LinkQueue));
	/* 对头队尾指向同一个节点 */
	Q->front = Q->rear = (QueuePtr)malloc(sizeof(QNode));
	if(Q->front == NULL)
	{
		printf("InitQueue malloc error\r\n");
		return;
	}
	memset(Q->front, 0, sizeof(QNode));
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

/* 1、初始化链式队列 */
void InitFTPQueue(FTPLinkQueue *Q)
{
	memset(Q, 0, sizeof(FTPLinkQueue));
	Q->front = Q->rear = (FTPQueuePtr)malloc(sizeof(FTPQNode));
	if(Q->front == NULL)
	{
		printf("InitQueue malloc error\r\n");
		return;
	}
    
	memset(Q->front, 0, sizeof(FTPQNode));
    
	Q->front->next = NULL;
}

/* 2、销毁链式队列 */
void DestroyFTPQueue(FTPLinkQueue *Q)
{
	while(Q->front) /* 销毁，两个指针不再保持原来双链表的指向 */
	{
		Q->rear = Q->front->next;
		free(Q->front);
		Q->front = Q->rear;
	}
}

/* 7、入队列 */
void EnFTPQueue(FTPLinkQueue *Q, FtpElemType *e)
{
	FTPQueuePtr p;

    pthread_mutex_lock(&g_FtpUploadqueueMutex);
    
	p = (FTPQueuePtr)malloc(sizeof(FTPQNode));
	if(p == NULL)
	{
        pthread_mutex_unlock(&g_FtpUploadqueueMutex);
		printf("EnQueue malloc error\r\n");

		return;
	}

	memcpy((char *)&(p->data), (char *)e, sizeof(FtpElemType));

	p->next = NULL;
	Q->rear->next = p;
	Q->rear = p;
	Q->queueNum++;
    Q->iLastTime = time(NULL);

    pthread_mutex_unlock(&g_FtpUploadqueueMutex);

    return;
}

/* 8、出队列 */
void DeFTPQueue(FTPLinkQueue *Q, FtpElemType *e)
{
	FTPQueuePtr p;

    pthread_mutex_lock(&g_FtpUploadqueueMutex);

	if(Q->front != Q->rear)
	{
		p = Q->front->next;
		if(p != NULL)
		{
			memcpy(e, &p->data, sizeof(FtpElemType));

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

    pthread_mutex_unlock(&g_FtpUploadqueueMutex);

    return;
}


