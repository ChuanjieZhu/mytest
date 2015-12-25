
#ifndef _MEM_DEBUG_H_
#define _MEM_DEBUG_H_

#include "public.h"

#define WHILE_FALSE  while(0)

#define MRJ_safefree(ptr) \
  do {if((ptr)) {free((ptr)); (ptr) = NULL;}} WHILE_FALSE

#endif /* _MEM_DEBUG_H_ */

