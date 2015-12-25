/*
 *
 *
 */

#ifndef _MUTEX_LIB_H_
#define _MUTEX_LIB_H_

#include <pthread.h>

/* ª•≥‚À¯¿‡ */
class CMutex
{
    public:
	    CMutex();
	    ~CMutex();
	    void Lock();
	    void Unlock();
    private:
        pthread_mutex_t m_Mutex;
};

#endif // _MUTEX_LIB_H_
