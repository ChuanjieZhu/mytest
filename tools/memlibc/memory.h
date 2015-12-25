/*
 * My own memory function
 */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "mem_fun.h"

/*-------------------------------c function ----------------------------------*/
#ifdef __cplusplus
    extern "C" {
#endif

#ifdef DEBUG_MEMORY

#define  Malloc(s) MallocMem((char *)__FILE__, (int)__LINE__, s);

#define Free(p) if (p != NULL) \
                {\
                    FreeMem((char *)__FILE__, (int)__LINE__, p);\
                }
#else

#define Malloc(s) malloc(s)

#define Free(p) if (p != NULL)\
                {\
                    free(p);\
                    p = NULL;\
                }

#endif

#ifdef __cplusplus
}
#endif

/*-------------------------------end c function ----------------------------------*/

#ifdef DEBUG_MEMORY

inline void* operator new(size_t s, char* pFile, int nLineNo)
{
	void* p = MallocMem(pFile, nLineNo, s);
	
	return p;
}

inline void* operator new [](size_t s, char* pFile, int nLineNo)
{
	void* p = MallocMem(pFile, nLineNo, s);	
	
	return p;
}

extern char* gpcFile;
extern int giLine;

inline void operator delete(void * p)
{
    FreeMem(gpcFile, giLine, p);	
}

inline void operator delete [] (void * p)
{
    FreeMem(gpcFile, giLine, p);
}

#define new new((char *)__FILE__, (int)__LINE__)
#define delete (gpcFile = (char *)__FILE__, giLine = (int)__LINE__), delete

#else
	#define NEW new
	#define DELETE delete
#endif

#endif /* end of _MEMORY_H_ */

