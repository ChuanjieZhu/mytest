/*
 * 
 */

#ifndef _CMUTEX_H_
#define _CMUTEX_H_

#include <pthread.h>

typedef pthtread_mutex_t Mutex;

void Mutex_Init(Mutex *pMutex);

void Mutex_Destroy(Mutex *pMutex);

void Mutex_Lock(Mutex *pMutex);

void Mutex_Unlock(Mutex *pMutex);

#endif /* end of _CMUTEX_H_ */

