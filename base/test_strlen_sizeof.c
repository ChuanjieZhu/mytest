/*
 * A test program for test strlen function vs sizeof
 * time: 2013-11-08 10:13
 * strlen(): count the string length don't contain the termianl char of "null";
 *         : strlen函数遇到null结束符时停止           
 * sizeof(): is a operate, count string length contain the termianl char of "null"
 */

#include <string.h>    /* strlen() */
#include <stdio.h>

struct capability {
		int a;
		int b;
		int c;
		int e;
		int f;
} capability;

int main()
{
    char buf[] = "aaaaaaaaaaaaa";

    char *p = "hello,world";
	
    printf("strlen: %d, sizeof: %d\n", strlen(buf), sizeof(buf));
	printf("strlen(p): %d \r\n", strlen(p));

    
    struct capability cap;

    printf("sizoef(capability): %d, sizeof(struct capability): %d\r\n", sizeof(capability), sizeof(struct capability));

    return 0;
}

/*
 * result: strlen: 13, sizeof: 14
 *
 */
