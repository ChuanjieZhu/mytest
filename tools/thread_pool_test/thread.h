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
** 函数名称： PthreadCreateSched
** 功能： 线程创建包裹函数
** 参数： thread: 线程id, 
          start_rtn: 线程入口函数指针,
          arg: 传递给线程入口函数参数
          schedPolicy: 线程调度策略
          schedPriority: 线程运行优先级
** 返回： 
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
int PthreadCreateSched(
                        pthread_t *thread, 
                        void *(*start_rtn)(void *),
                        void *arg,
                        int schedPolicy,
                        int schedPriority);

/*******************************************************************************
** 函数名称： PthreadCreateSchedCom
** 功能： 默认属性(调度策略，优先级)线程创建包裹函数
** 参数： *thread: 线程id, 
          start_rtn: 线程入口函数指针,
          arg: 传递给线程入口函数参数
** 返回： 
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
int PthreadCreateSchedCom(pthread_t * thread, void *(* start_rtn)(void *), void * arg);

/*******************************************************************************
** 函数名称： PthreadSelf
** 功能： 获取线程id函数pthread_self()函数包裹函数
** 参数： 
** 返回： 线程自身id号
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
pthread_t PthreadSelf();

/*******************************************************************************
** 函数名称： PthreadJoin
** 函数功能： 等待线程结束函数pthread_join()接口包裹函数，线程退出时调用
              pthread_join()函数等待线程资源释放，注意pthread_join会导致
              线程阻塞
** 传入参数： thread: 线程id号
** 传出参数： vaulePtr: 等待线程返回值
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
********************************************************************************/
int PthreadJoin(pthread_t thread, void **vaulePtr);

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
int PthreadDetach(pthread_t thread);

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
int PthreadExit(void *valuePtr);

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
int PthreadCancel(pthread_t thread);

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
int CreateTread(THREAD_PARAM *threadParam, const char *threadName);

/*******************************************************************************
** 函数名称： DestroyThread
** 函数功能： 线程销毁封装接口
** 传入参数： threadParam: pthread_create()接口相关参数
              threadName: 
** 传出参数： 
** 返回值：
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期： 
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

