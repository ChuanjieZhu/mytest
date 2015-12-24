/* base function lib */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

ssize_t readn(int fd, void *vptr, size_t n)
{
	ssize_t nread;
	size_t nleft;
	char *ptr;

	nleft = n;
	ptr = vptr;

	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			/* interrupt by system signal */
			if (errno == EINTR) {
				nread = 0;				/* call read again */
			} else {
				return (-1);
			}
		} else if (0 == nread){		
			break;						/* EOF */
		}

		nleft -= nread;
		ptr	  += nread;
	}

	return (n - nleft);
}

ssize_t writen(int fd, void *vptr, size_t n)
{
	ssize_t nwrite;
	size_t nleft;
	char *ptr;

	ptr = vptr;
	nleft = n;

	while (nleft > 0) {
		if ((nwrite = write(fd, ptr, nleft)) <= 0) {
			/* interrupt by system singal */
			if (nwrite < 0 && errno == EINTR) {
				nwrite = 0;				/* call write() again */
			} else {
				return (-1);
			}
		}

		nleft -= nwrite;
		ptr   += nwrite;
	}

	return n;
}
