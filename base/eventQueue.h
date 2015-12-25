/*************************************************************************************
*
*	��ʽ����ʵ��
*	
*
**************************************************************************************/

#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

typedef struct __operate_event_t_
{
	unsigned int operateType;//����  bit0�����ݿ����  bit1����¼��Ƭ
	unsigned int id;//�û�id
	int nServerID;
	int nChannelID;
	int nThreshold;//ʶ������
	int recogScores;//ʶ�����
	int nType;// 0:��  1:����
	int nStatus;//ʶ��״̬
	int nUserType;//�û�����
	int nLoginReason;//����ԭ��
	unsigned int time;//�¼�������ʱ��
	unsigned int fileSize;//��¼��Ƭ�ļ���С (������ʼͷ��������Ϣ��jpg��Ƭ������ͷ�Ĵ�С)
	char cRecogMode;//ʶ��ģʽ
	char *pFileData;//��¼��Ƭ��ͼƬ����
} operate_event_t;

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

/* ��¼��Ϣ */
typedef struct __upload_event_t_
{
    char acStudentID[20];	/* ѧ���߼����� */
    char acCardID[20];		/* ѧ�������� */
    char acPicName[64];		/* ץ��jpg��Ƭ·���� */
    char acStudentPic[128];	/* ѹ����Ƭ·���� */
    time_t tBaseTime;
	char cStatus;			/* ��Ƭ�ϴ�״̬ 0-δ�ϴ���1-���ϴ� */	
    char *pSaveJpg;
}ftp_upload_event_t;

/* ������ʽ�������� */
typedef ftp_upload_event_t FtpElemType;

typedef struct FTPQNode
{
	FtpElemType  data;
	struct FTPQNode *next;
}FTPQNode, *FTPQueuePtr;

typedef struct
{
	FTPQueuePtr front;//��һ��
	FTPQueuePtr rear;//���һ��
	int queueNum;//���е�ǰӵ�е�Ԫ����Ŀ
	time_t iLastTime; /* �ϴβ���ʱ�� */
}FTPLinkQueue;

extern pthread_mutex_t g_FtpUploadqueueMutex;//����������
extern FTPLinkQueue g_FtpUploadQueue;//����

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

