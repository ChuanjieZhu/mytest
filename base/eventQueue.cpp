
/******************************************************************************************
*	��ʽ����ʵ��
*	
*
******************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eventQueue.h"

pthread_mutex_t g_FtpUploadqueueMutex;//����������
FTPLinkQueue g_FtpUploadQueue;//����

/* 1����ʼ����ʽ���� */
void InitQueue(LinkQueue *Q)
{
	memset(Q, 0, sizeof(LinkQueue));
	/* ��ͷ��βָ��ͬһ���ڵ� */
	Q->front = Q->rear = (QueuePtr)malloc(sizeof(QNode));
	if(Q->front == NULL)
	{
		printf("InitQueue malloc error\r\n");
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
		if(Q->front->data.pFileData != NULL)
		{
			free(Q->front->data.pFileData);
			Q->front->data.pFileData = NULL;
		}
		free(Q->front);
		Q->front = Q->rear;
	}
}

/* 3�������ʽ���� */
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

/* 9��������ʽ���в����Ԫ�� */
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

/* 1����ʼ����ʽ���� */
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

/* 2��������ʽ���� */
void DestroyFTPQueue(FTPLinkQueue *Q)
{
	while(Q->front) /* ���٣�����ָ�벻�ٱ���ԭ��˫�����ָ�� */
	{
		Q->rear = Q->front->next;
		free(Q->front);
		Q->front = Q->rear;
	}
}

/* 7������� */
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

/* 8�������� */
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


