/***************************************************************************\
*
*	A simple program test the va_start, va_end, va_list function
*	time: 2013-10-27 20:59:00
*
****************************************************************************/

#include <stdio.h>
#include <stdarg.h>

/* for getpid function */
#include <sys/types.h>
#include <unistd.h>
	   
#define DEBUG(fmt, ...)			debug(__FILE__, __LINE__, fmt, __VA_ARGS__)

static void 
debug(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "(%s:%d:%d) ", file, line, getpid());
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\r\n");
}

int main(int argc, char *argv[])
{
	DEBUG("va_func test %s : %d\r\n", argv[0], argc);
	return 0;
}

