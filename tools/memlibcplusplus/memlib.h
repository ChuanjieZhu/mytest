/*
 *
 *
 */

#ifndef _MEMLIB_H_
#define _MEMLIB_H_

#include "mutexlib.h"

typedef struct MEM_BLOCK {
	void* pAddr;
	int iSize;
	char* pStrFileName;
	int iFileLine;
	struct MEM_BLOCK* pNext;
} MEM_BLOCK, *LPMEM_BLOCK;

/* 内存管理类 */
class CMem 
{
    public:
	    CMem();
	    ~CMem();
	    void* MallocMem(char* pFile, int nLine, int size);
	    void FreeMem(char* pFile, int nLine, void* ptr);
        void PrintMem();
    private:
        LPMEM_BLOCK m_pMem;
    	CMutex m_mutex;       
};

void* MallocMem(char* pFile, int nLine, int size);
void FreeMem(char * pFile, int nLine, void * ptr);
void PrintMem();

#endif //_MEMLIB_H_
