

#include <stdio.h>
#include <stdarg.h>

#define MAX_LINE 1024

void LoggerMessage(const char *fmt, ...)
{
    char buf[MAX_LINE + 1] = {0};
    
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, MAX_LINE, fmt, ap);	/* safe */
    va_end(ap);
    printf("buf: %s \r\n", buf);
}

int main()
{
    int i = 10;
    LoggerMessage("i = %d, str = %s\r\n", i, "sdfsdfd");

    return 0;
}
