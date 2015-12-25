/*
 *
 *
 */

#include "cmutex.h"

void Mutex_Init(Mutex *pMutex)
{
    pthread_mutex_init(pMutex, NULL);
}

void Mutex_Destroy(Mutex *pMutex)
{
    pthread_mutex_destroy(pMutex);
}

void Mutex_Lock(Mutex *pMutex)
{
    pthread_mutex_lock(pMutex);
}

void Mutex_Unlock(Mutex *pMutex)
{
    pthread_mutex_unlock(pMutex);
}

