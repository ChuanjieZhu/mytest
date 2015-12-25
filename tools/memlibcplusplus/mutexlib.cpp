/*
 *
 *
 */

#include "mutexlib.h"

/* 构造函数无返回值 */
CMutex::CMutex()
{
	pthread_mutex_init(&m_Mutex, NULL);
}

/* 析构函数无返回值 */
CMutex::~CMutex()
{
	pthread_mutex_destroy(&m_Mutex);
}

void CMutex::Lock()
{
	pthread_mutex_lock(&m_Mutex);
}

void CMutex::Unlock()
{
	pthread_mutex_unlock(&m_Mutex);
}

