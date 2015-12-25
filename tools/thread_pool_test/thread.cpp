/********************************************************************************
**  Copyright (c) 2010, �����з���˹�Ƽ����޹�˾
**  All rights reserved.
**	
**  �ļ�˵��: ���ļ�ʵ�ֶԻ������̵߳��ú������˷�װ������ʵ�����̳߳ػ��ࡣ
**  ��������: 2010.11.11
**
**  ��ǰ�汾��1.0
**  ���ߣ�laizh
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "thread.h"

/*******************************************************************************
** �������ƣ� PthreadCreateSched
** �������ܣ� �̴߳�����������
** ��������� thread: �߳�id, 
          start_rtn: �߳���ں���ָ��,
          arg: ���ݸ��߳���ں�������
          schedPolicy: �̵߳��Ȳ���
          schedPriority: �߳��������ȼ�
** ��������:            
** ����ֵ�� 0-�ɹ�������-ʧ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadCreateSched(pthread_t * thread, 
                            void *(* start_rtn)(void *), 
                            void * arg, 
                            int schedPolicy, 
                            int schedPriority)
{
    int iRet = -1;
    pthread_attr_t attr;

    printf("pthread create with priority! \r\n");

    iRet = pthread_attr_init(&attr);
    if (iRet != 0)
    {
        printf("pthread attr init error. \r\n");
    }

    /* set thread schedpolicy, SCHED_OTHER(default), SCHED_FIFO, SCHED_RR */
    iRet = pthread_attr_setschedpolicy(&attr, schedPolicy);
    if (iRet != 0)
    {
        printf("pthread_attr_setschedpolicy error. \r\n");
    }

    /* schedpolicy is SCHED_FIFO or SCHED_RR, setschedparam is effective */
    if (schedPolicy == SCHED_FIFO | schedPolicy == SCHED_RR)
    {
        sched_param param;
        param.sched_priority = schedPriority;
        
        /* set thread sched priority, default is 0 */
        iRet = pthread_attr_setschedparam(&attr, &param);
        if (iRet != 0)
        {
            printf("pthread_attr_setschedparam error .\r\n");
        }
    }

    /* create a new thread, return 0 success, other error */
    iRet = pthread_create(thread, &attr, start_rtn, arg);
    if (iRet != 0)
    {
        printf("pthread_create error .\r\n");
    }

    return iRet;
}

/*******************************************************************************
** �������ƣ� PthreadCreateSchedCom
** �������ܣ� Ĭ������(���Ȳ��ԣ����ȼ�)�̴߳�����������
** ��������� *thread: �߳�id, 
              start_rtn: �߳���ں���ָ��,
              arg: ���ݸ��߳���ں�������
** ��������:  
** ����ֵ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadCreateSchedCom(pthread_t * thread, void *(* start_rtn)(void *), void * arg)
{
    return PthreadCreateSched(thread, start_rtn, arg, SCHED_FIFO, 20);
}

/*******************************************************************************
** �������ƣ� PthreadSelf
** �������ܣ� ��ȡ�߳�id����pthread_self()������������
** ���ز����� 
** ����ֵ�� �߳�����id��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
pthread_t PthreadSelf()
{
	return pthread_self();
}

/*******************************************************************************
** �������ƣ� PthreadJoin
** �������ܣ� �ȴ��߳̽�������pthread_join()�ӿڰ����������߳��˳�ʱ����
              pthread_join()�����ȴ��߳���Դ�ͷţ�ע��pthread_join�ᵼ��
              �߳�����
** ��������� thread: �߳�id��
** ���������� vaulePtr: �ȴ��̷߳���ֵ
** ����ֵ��   0-�ɹ�������-ʧ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadJoin(pthread_t thread, void **valuePtr)
{
    int iRet = -1;

    iRet = ptread_join(thread, valuePtr);
    if (iRet != 0)
    {
        printf("pthread_join error %s(%d).\r\n", strerror(iRet), iRet);
    }

    return iRet;
}

/*******************************************************************************
** �������ƣ� PthreadDetach
** �������ܣ� �̷߳����������ú���pthread_detach()�ӿڰ�������,���߳����ڷ���״̬��
              ���߳��˳�ʱ���Զ��ͷ���ռ����Դ
** ��������� thread: �߳�id��
** ���������� 
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadDetach(pthread_t thread)
{
    int iRet = -1;

    iRet = pthread_detach(thread);
    if (iRet != 0)
    {
        printf("pthread_detach error %s(%d).\r\n", strerror(iRet), iRet);
    }

    return iRet;
}

/*******************************************************************************
** �������ƣ� PthreadExit
** �������ܣ� �߳������˳����߳���ֹ����pthread_exit()�ӿڰ�������,
** ��������� 
** ���������� vaulePtr: �����߳��˳��󷵻�ֵ
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadExit(void *valuePtr)
{
    pthread_exit(valuePtr);
}

/*******************************************************************************
** �������ƣ� PthreadCancel
** �������ܣ� �߳�ȡ������pthread_cancel()�ӿڰ�������,�߳�ȡ�����ӳ�ȡ��������ȡ��
              ����ȡ��ʱ������pthread_cancel�߳����Ͻ������ӳ�ȡ��ʱ�߳����е�һ��
              ȡ����ʱ�Ž���
** ��������� thread: �߳�id
** ���������� 
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadCancel(pthread_t thread)
{
    return pthread_cancel(thread);
}

/*******************************************************************************
** �������ƣ� CreateTread
** �������ܣ� �̴߳�����װ�ӿ�
** ��������� threadParam: pthread_create()�ӿ���ز���
              threadName: 
** ���������� 
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int CreateTread(THREAD_PARAM *threadParam, const char *threadName)
{
    int iRet = FI_FAILED;

    if (threadParam == NULL)
    {
        printf("CreateThread threadParam is null. \r\n");
        return iRet;
    }

    iRet = PthreadCreateSched(&threadParam->thdId, 
                              threadParam->thdFunc, 
                              (void *)threadParam->thdParam, 
                              threadParam->thdPolicy,
                              threadParam->thdPriority);
    if (iRet != 0)
    {
        threadParam->thdRunFlag = FI_FALSE;
        printf("CreateThread() %s  error:%s\r\n", threadName, STRERROR_ERRNO);
        iRet = FI_FAILED;
    }
    else
    {
        iRet = FI_SUCCESSFUL;
    }

    return iRet;
}

/*******************************************************************************
** �������ƣ� DestroyThread
** �������ܣ� �߳����ٷ�װ�ӿ�
** ��������� threadParam: pthread_create()�ӿ���ز���
              threadName: �߳���
** ���������� 
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int DestroyThread(THREAD_PARAM *threadParam, const char *threadName)
{
    int iRet = FI_SUCCESSFUL;

    if (FI_TRUE == threadParam->thdRunFlag)
    {
        threadParam->thdRunFlag = FI_FALSE;

        if (0 != PthreadJoin(threadParam->thdId, NULL))
        {
            printf("DestroyThread %s error:%s!\r\n", threadName, STRERROR_ERRNO);
            iRet = FI_FAILED
        }
    }

    return iRet;
}

/*******************************************************************************
** �������ƣ� CThreadPool
** �������ܣ� CThreadPool�๹�캯��,��ʼ��private����
** ��������� 
** ���������� 
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
CThreadPool::CThreadPool()
{
    m_Shutdown          = false;
    m_MaxThreadNum      = 0;
    m_ThreadId          = NULL;
    m_WorkQueueSize     = 0;
    m_WorkQueueFront    = NULL;
    m_WorkQueueRear     = NULL;
}

/*******************************************************************************
** �������ƣ� ~CThreadPool
** �������ܣ� CThreadPool����������
** ��������� 
** ���������� 
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/

CThreadPool::~CThreadPool()
{
    
}

int CThreadPool::Create(int priority, int threadNum) 
{
	m_Shutdown = false;
	m_MaxThreadNum = threadNum;

    m_ThreadId = (pthread_t *)malloc(m_MaxThreadNum * sizeof(pthread_t));
	if (m_ThreadId == NULL)
	{
        printf("CThreadPool::Create malloc error. \r\n");
        return -1;
	}
	
	pthread_mutex_init(&m_Mutex, NULL);
	pthread_cond_init(&m_Cond, NULL);

    int i;
	for (i = 0; i < m_MaxThreadNum; ++i)
	{
		PthreadCreateSched(&m_ThreadId[i], ThreadProcess, this, SCHED_FIFO, priority );
	}
    
	return 0;
}


int CThreadPool::Add(THREAD_FUN fun, void *args) 
{ 
	WORK_NODE *worker = NewWork(fun, args);
	if (worker == NULL)
	{
        return -1;
	}

	pthread_mutex_lock(&m_Mutex);	

    if (m_WorkQueueRear == NULL)
	{
		m_WorkQueueFront = worker;
		m_WorkQueueRear = worker;
	}
	else
	{
		m_WorkQueueRear->next = worker;
		m_WorkQueueRear = worker;
	}
	m_WorkQueueSize++;
    
	pthread_mutex_unlock(&m_Mutex);
	pthread_cond_signal(&m_Cond);

    return 0;
}

int CThreadPool::Destroy()
{
	if (m_Shutdown || m_ThreadId == NULL)
    { 
        return -1;
	}

	m_Shutdown = true;
    
	pthread_cond_broadcast(&m_Cond);

    int i;
	for (i = 0; i < m_MaxThreadNum; ++i)
	{
		PthreadJoin(m_ThreadId[i], NULL);
		printf("Thread %d is exit to work !\r\n", m_ThreadId[i]);
	}

	m_MaxThreadNum = 0;
	free(m_ThreadId);
	
	WORK_NODE *worker = NULL;
	while (m_WorkQueueFront != NULL)
	{
		worker = m_WorkQueueFront;
		m_WorkQueueFront = worker->next;
		free(worker);
	}
    
	m_WorkQueueRear = NULL;
	m_WorkQueueSize = 0;

    pthread_mutex_destroy(&m_Mutex);
	pthread_cond_destroy(&m_Cond);
    
	return 0;
}

void *CThreadPool::ThreadProcess(void *args)
{
	if (args != NULL)
	{
		CThreadPool *pThis = ( CThreadPool * )args;
		pThis->ThreadRoutine();
	}
    
	return NULL;
}

void CThreadPool::ThreadRoutine()
{
    while (1)
    {
		pthread_mutex_lock(&m_Mutex);	

        while (m_WorkQueueSize == 0 && m_Shutdown = false)
		{
			printf("Thread %d is waiting !\r\n", PthreadSelf());
			pthread_cond_wait(&m_Cond, &m_Mutex);
		}
        
		if (m_Shutdown == true)
		{
			pthread_mutex_unlock(&m_Mutex);
			printf("Thread %d will exit !\r\n", PthreadSelf());
			break;
		}
        
		printf("Thread %d is starting to work !\r\n", PthreadSelf());
		
		m_WorkQueueSize--;
        
		WORK_NODE *worker = m_WorkQueueFront;
		m_WorkQueueFront = worker->next;

        if (m_WorkQueueSize == 0)
        {
            m_WorkQueueRear = NULL;
        }
        
		pthread_mutex_unlock(&m_Mutex);
        
		(*(worker->fun))(worker->args);
        free( worker );
	}
}

WORK_NODE *CThreadPool::NewWork( THREAD_FUN fun, void *args )
{
	WORK_NODE *worker = (WORK_NODE* )malloc(sizeof(WORK_NODE));
	if (worker != NULL)
	{
		worker->fun = fun;
		worker->args = args;
		worker->next = NULL;
	}

    return worker;
}

