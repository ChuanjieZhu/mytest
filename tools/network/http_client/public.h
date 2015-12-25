
#ifndef __MRJ_PUBLIC_H_
#define __MRJ_PUBLIC_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <iostream>


#define STR_ERRNO   (strerror(errno))
#define MDL __FILE__, __LINE__
#define TRACE printf

#endif /* __MRJ_PUBLIC_H_ */

