/*
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mutexlib.h"

CMem::CMem()
{
	m_pMem = NULL;
}

CMem::~CMem()
{
	m_mutex.Lock();
	if(m_pMem)
	{
		LPMEM_BLOCK pMem = m_pMem;
		
		sleep(1);
		printf("$$$ !!!  Memory not Free !!! %s %d\r\n", __FUNCTION__, __LINE__);
		
		while(m_pMem)
		{
			printf("%s:%d, size=%d %s %d\r\n", m_pMem->pStrFileName, m_pMem->iFileLine,
				m_pMem->iSize, __FUNCTION__, __LINE__);

			m_pMem = m_pMem->pNext;
			
			free(pMem);
			pMem = m_pMem;
		}
		
		printf("\r\n\r\n\r\n");
		sleep(1);
	} 
	else 
	{
		printf("$$$ MEMORY TEST OK %s %d\r\n", __FUNCTION__, __LINE__);
	}
	m_mutex.Unlock();
	
	sleep(1);
}


#define PROTECT_TAIL_SIZE 10
#define MAGIC_NUMBER 0x5a
/******************************************************************************
 * 函数名称： CMem.MallocMem
 * 功能： 申请内存，并将内存信息插入链表，析构函数会检查是否有内存没有被释放
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 
 * 修改作者：
 * 修改日期：2012-6-15
 ******************************************************************************/
void* CMem::MallocMem(char* pFile, int nLine, int size)
{
	LPMEM_BLOCK pMemBlock = (LPMEM_BLOCK)malloc(sizeof(MEM_BLOCK) + size + PROTECT_TAIL_SIZE);
	if(pMemBlock==NULL)
	{
		printf("$$$ Malloc Fail %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	else
	{
		m_mutex.Lock();
		LPMEM_BLOCK pMem = m_pMem;
		if(pMem==NULL)
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
		pMemBlock->iFileLine = nLine;
		pMemBlock->iSize = size;
		pMemBlock->pNext = NULL;

		memset((char*)pMemBlock->pAddr + size, MAGIC_NUMBER, PROTECT_TAIL_SIZE);
		m_mutex.Unlock();

		return pMemBlock->pAddr;
	}
}

/******************************************************************************
 * 函数名称： CMem.FreeMem
 * 功能： 释放内存，并检查内存是否溢出
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 
 * 修改作者：
 * 修改日期：2012-6-15
 ******************************************************************************/
void CMem::FreeMem(char* pFile, int nLine, void* ptr)
{
	m_mutex.Lock();
	
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
				printf("$$$ memory over flow %s %d\r\n", pFile, nLine);
			}
			free(pMem);
			break;
		}
		
		pPrev = pMem;
		pMem = pMem->pNext;
	}

	if(pMem==NULL)
	{
		printf("\r\n\r\n\r\n!!! %s:%d => Bad memory Free !!!\r\n\r\n\r\n", pFile, nLine);
	}
	m_mutex.Unlock();
}

/******************************************************************************
 * 函数名称： CMem.Print
 * 功能： 运行过程中可调此函数将申请的内存打印出来
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 
 * 修改作者：
 * 修改日期：2012-6-15
 ******************************************************************************/
void CMem::PrintMem()
{
	m_mutex.Lock();
	
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

	m_mutex.Unlock();
}

class CMem s_Mem;

/******************************************************************************
 * 函数名称： MallocMem
 * 功能： 申请内存
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 
 * 修改作者：
 * 修改日期：2012-6-15
 ******************************************************************************/
void* MallocMem(char* pFile, int nLine, int size)
{
	void* p = s_Mem.MallocMem(pFile, nLine, size);
	return p;
}

/******************************************************************************
 * 函数名称： FreeMem
 * 功能： 释放内存
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 
 * 修改作者：
 * 修改日期：2012-6-15
 ******************************************************************************/
void FreeMem(char * pFile, int nLine, void * ptr)
{
	s_Mem.FreeMem(pFile, nLine, ptr);
}


/******************************************************************************
 * 函数名称： PrintMem
 * 功能： 释放内存
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 
 * 修改作者：
 * 修改日期：2012-6-15
 ******************************************************************************/
void PrintMem()
{
	s_Mem.PrintMem();
}

