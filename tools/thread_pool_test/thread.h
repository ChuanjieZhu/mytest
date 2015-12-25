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

#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>

#define FI_FAILED        (-1)
#define FI_SUCCESSFUL    (0)

#define FI_FALSE         (0)
#define FI_TRUE          (1)

typedef void *(*THREAD_FUN)(void *);
typedef struct tagThreadParam
{
    int         thdRunFlag;
    pthread_t   thdId;
    void *      thdParam;
    int         thdPolicy;
    int         thdPriority;
    THREAD_FUN  thdFunc;
} THREAD_PARAM;

#ifdef __cpluscplus
extern "C" {
#endif

/*******************************************************************************
** �������ƣ� PthreadCreateSched
** ���ܣ� �̴߳�����������
** ������ thread: �߳�id, 
          start_rtn: �߳���ں���ָ��,
          arg: ���ݸ��߳���ں�������
          schedPolicy: �̵߳��Ȳ���
          schedPriority: �߳��������ȼ�
** ���أ� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadCreateSched(
                        pthread_t *thread, 
                        void *(*start_rtn)(void *),
                        void *arg,
                        int schedPolicy,
                        int schedPriority);

/*******************************************************************************
** �������ƣ� PthreadCreateSchedCom
** ���ܣ� Ĭ������(���Ȳ��ԣ����ȼ�)�̴߳�����������
** ������ *thread: �߳�id, 
          start_rtn: �߳���ں���ָ��,
          arg: ���ݸ��߳���ں�������
** ���أ� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadCreateSchedCom(pthread_t * thread, void *(* start_rtn)(void *), void * arg);

/*******************************************************************************
** �������ƣ� PthreadSelf
** ���ܣ� ��ȡ�߳�id����pthread_self()������������
** ������ 
** ���أ� �߳�����id��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
pthread_t PthreadSelf();

/*******************************************************************************
** �������ƣ� PthreadJoin
** �������ܣ� �ȴ��߳̽�������pthread_join()�ӿڰ����������߳��˳�ʱ����
              pthread_join()�����ȴ��߳���Դ�ͷţ�ע��pthread_join�ᵼ��
              �߳�����
** ��������� thread: �߳�id��
** ���������� vaulePtr: �ȴ��̷߳���ֵ
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int PthreadJoin(pthread_t thread, void **vaulePtr);

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
int PthreadDetach(pthread_t thread);

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
int PthreadExit(void *valuePtr);

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
int PthreadCancel(pthread_t thread);

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
int CreateTread(THREAD_PARAM *threadParam, const char *threadName);

/*******************************************************************************
** �������ƣ� DestroyThread
** �������ܣ� �߳����ٷ�װ�ӿ�
** ��������� threadParam: pthread_create()�ӿ���ز���
              threadName: 
** ���������� 
** ����ֵ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ� 
********************************************************************************/
int DestroyThread(THREAD_PARAM *threadParam, const char *threadName);

#ifdef __cpluscplus
}
#endif

typedef void *(*THREAD_FUN)(void *);

typedef struct workNode
{
    THREAD_FUN fun;
    void *args;
    struct workNode *next;
} WORK_NODE;

class CThreadPool
{
public:        
    CThreadPool();
    
    ~CThreadPool();
        
    int Create(int priority, int threadNum);

    int Add(THREAD_FUN fun, void *args);

    int Destroy();

private:
    static void *ThreadProcess(void *args); 
    void ThreadRoutine();
    WORK_NODE *NewWork(THREAD_FUN fun, void *args);

private:
    bool m_Shutdown;
    int m_MaxThreadNum;
    pthread_t *m_ThreadId;
    int m_WorkQueueSize;
    WORK_NODE *m_WorkQueueFront;
    WORK_NODE *m_WorkQueueRear;
    pthread_mutex_t m_Mutex;
    pthread_cond_t m_Cond;
};


#endif  /* _THREAD_H_ */

