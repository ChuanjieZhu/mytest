/********************************************************************************
**  Copyright (c) 2010, 深圳市飞瑞斯科技有限公司
**  All rights reserved.
**	
**  文件说明: 此文件实现对基础的线程调用函数做了封装，并且实现了线程池基类。
**  创建日期: 2010.11.11
**
**  当前版本：1.0
**  作者：laizh
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "thread.h"

/*******************************************************************************
** 函数名称： PthreadCreateSched
** 函数功能： 线程创建包裹函数
** 传入参数： thread: 线程id, 
          start_rtn: 线程入口函数指针,
          arg: 传递给线程入口函数参数
          schedPolicy: 线程调度策略
          schedPriority: 线程运行优先级
** 传出参数:            
** 返回值： 0-成功，其他-失败 
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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
** 函数名称： PthreadCreateSchedCom
** 函数功能： 默认属性(调度策略，优先级)线程创建包裹函数
** 传入参数： *thread: 线程id, 
              start_rtn: 线程入口函数指针,
              arg: 传递给线程入口函数参数
** 传出参数:  
** 返回值： 
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
int PthreadCreateSchedCom(pthread_t * thread, void *(* start_rtn)(void *), void * arg)
{
    return PthreadCreateSched(thread, start_rtn, arg, SCHED_FIFO, 20);
}

/*******************************************************************************
** 函数名称： PthreadSelf
** 函数功能： 获取线程id函数pthread_self()函数包裹函数
** 返回参数： 
** 返回值： 线程自身id号
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
pthread_t PthreadSelf()
{
	return pthread_self();
}

/*******************************************************************************
** 函数名称： PthreadJoin
** 函数功能： 等待线程结束函数pthread_join()接口包裹函数，线程退出时调用
              pthread_join()函数等待线程资源释放，注意pthread_join会导致
              线程阻塞
** 传入参数： thread: 线程id号
** 传出参数： vaulePtr: 等待线程返回值
** 返回值：   0-成功，其他-失败
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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
** 函数名称： PthreadDetach
** 函数功能： 线程分离属性设置函数pthread_detach()接口包裹函数,将线程至于分离状态，
              在线程退出时，自动释放所占用资源
** 传入参数： thread: 线程id号
** 传出参数： 
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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
** 函数名称： PthreadExit
** 函数功能： 线程正常退出和线程终止函数pthread_exit()接口包裹函数,
** 传入参数： 
** 传出参数： vaulePtr: 保存线程退出后返回值
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
int PthreadExit(void *valuePtr)
{
    pthread_exit(valuePtr);
}

/*******************************************************************************
** 函数名称： PthreadCancel
** 函数功能： 线程取消函数pthread_cancel()接口包裹函数,线程取消有延迟取消和立即取消
              立即取消时，调用pthread_cancel线程马上结束，延迟取消时线程运行到一个
              取消点时才结束
** 传入参数： thread: 线程id
** 传出参数： 
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
int PthreadCancel(pthread_t thread)
{
    return pthread_cancel(thread);
}

/*******************************************************************************
** 函数名称： CreateTread
** 函数功能： 线程创建封装接口
** 传入参数： threadParam: pthread_create()接口相关参数
              threadName: 
** 传出参数： 
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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
** 函数名称： DestroyThread
** 函数功能： 线程销毁封装接口
** 传入参数： threadParam: pthread_create()接口相关参数
              threadName: 线程名
** 传出参数： 
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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
** 函数名称： CThreadPool
** 函数功能： CThreadPool类构造函数,初始化private变量
** 传入参数： 
** 传出参数： 
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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
** 函数名称： ~CThreadPool
** 函数功能： CThreadPool类析构函数
** 传入参数： 
** 传出参数： 
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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

