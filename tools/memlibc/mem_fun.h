/*
 * .h for mem_fun.c
 * time: 2013-11-11 10:24
 */

#ifndef _MEM_FUN_H_
#define _MEM_FUN_H_

#define PROTECT_TAIL_SIZE 10
#define MAGIC_NUMBER 0x5a

typedef struct MEM_BLOCK
{
    void *pAddr;
    int iSize;
    char *pStrFileName;
    int iFileLine;
    struct MEM_BLOCK *pNext;
} MEM_BLOCK, *LPMEM_BLOCK;

void Mem_Init();
void Mem_Realse();
void* Malloc_Mem(char* pFile, int iLine, int iSize);
void Free_Mem(char* pFile, int iLine, void* ptr);
void Print_Mem();

void *MallocMem(char *pFile, int iLine, int iSize);
void FreeMem(char *pFile, int iLine, void *ptr);
void PrintMem();

#endif /* end of _MEM_FUN_H_ */
