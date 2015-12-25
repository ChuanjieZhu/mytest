/*
 * memory manager function for malloc and free
 * time: 2013-11-11 10:22
 */

#include "mem_fun.h"
#include "cmutex.h"


#include <stdio.h>

static LPMEM_BLOCK m_pMem;
static Mutex m_mutex;

void Mem_Init()
{
    m_pMem = NULL;
    Mutex_Init(&m_mutex)
}

void Mem_Realse()
{
    Mutex_Lock(&m_mutex);

    if (m_pMem)
    {
        LPMEM_BLOCK pMem = m_pMem;

        sleep(1);

        printf("!!! Memory not free !!! %s %d\r\n", __FUNCTION__, __LINE__);

        while (m_pMem)
        {
            printf("%s:%d, size=%d %s %d\r\n", m_pMem->pStrFileName, m_pMem->iFileLine,
				m_pMem->iSize, __FUNCTION__, __LINE__);

            m_pMem = pMem->pNext;

            free(pMem);
            pMem = m_pMem;
        }

        printf("\r\n");
        sleep(1);
    }
    else
    {
        printf("Memory test ok. %s %d\r\n", __FUNCTION__, __LINE__);
    }

    Mutex_Unlock(&m_mutex);
    sleep(1);
}

void *Malloc_Mem(char * pFile, int iLine, int iSize)
{
    LPMEM_BLOCK pMemBlock = (LPMEM_BLOCK)malloc(sizeof(MEM_BLOCK) + iSize + PROTECT_TAIL_SIZE);
	if(pMemBlock == NULL)
	{
		printf("!!! Malloc Fail %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	else
	{
		Mutex_Lock(&m_mutex);
		LPMEM_BLOCK pMem = m_pMem;
		if(pMem == NULL)
		{
			m_pMem = pMemBlock;
		}
		else
		{
			while(pMem->pNext)
			{
				pMem = pMem->pNext;
			}
			pMem->pNext = pMemBlock;
		}
		
		pMemBlock->pAddr = ((char*)pMemBlock) + sizeof(MEM_BLOCK);
		pMemBlock->pStrFileName = pFile;
		pMemBlock->nFileLine = iLine;
		pMemBlock->nSize = iSize;
		pMemBlock->pNext = NULL;

		memset((char*)pMemBlock->pAddr + iSize, MAGIC_NUMBER, PROTECT_TAIL_SIZE);
		Mutex_Unlock(&m_mutex);

		return pMemBlock->pAddr;
	}
}

void Free_Mem(char* pFile, int iLine, void* ptr)
{
    Mutex_Lock(&m_mutex);
	
	LPMEM_BLOCK pMem = m_pMem;
	LPMEM_BLOCK pPrev=NULL;
	char aCheck[16];

	memset(aCheck, MAGIC_NUMBER, sizeof(aCheck));
	
	while(pMem) 
	{
		if(pMem->pAddr == ptr)
		{
			if(pPrev == NULL)
			{
				m_pMem = pMem->pNext;
			}
			else
			{
				pPrev->pNext = pMem->pNext;
			}

			if (memcmp(aCheck, (char*)pMem->pAddr + pMem->iSize, PROTECT_TAIL_SIZE) != 0)
			{
				printf("!!! Memory over flow %s %d\r\n", pFile, iLine);
			}
            
			free(pMem);

            break;
		}
		
		pPrev = pMem;
		pMem = pMem->pNext;
	}

	if(pMem == NULL)
	{
		TRACE("\r\n!!! %s:%d => Bad memory Free !!!\r\n", pFile, iLine);
	}
    
	Mutex_Unlock(&m_mutex);
}

void Print_Mem()
{
    Mutex_Lock(&m_mutex);
	
	LPMEM_BLOCK pMem = m_pMem;
	int iTotalSize = 0;
	
	while(pMem) 
	{
		//printf("pAddr %p nSize %d pStrFileName %s nFileLine %d %s %d \r\n",
		//	pMem->pAddr, pMem->nSize, pMem->pStrFileName, pMem->nFileLine, __FUNCTION__, __LINE__);
		iTotalSize += pMem->iSize;
		
		pMem = pMem->pNext;
	}

	printf("iTotalSize %d %s %d \r\n", iTotalSize, __FUNCTION__, __LINE__);

	Mutex_Unlock(&m_mutex);
}

void *MallocMem(char *pFile, int iLine, int iSize)
{
    void *p = Malloc_Mem(pFile, iLine, iSize);
    return (p);
}


void FreeMem(char *pFile, int iLine, void *ptr)
{
    Free_Mem(pFile, iLine, ptr);
}

void PrintMem()
{
    Print_Mem();
}


