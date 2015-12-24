/*
 * This is a simple program for testing the macro using in c program 
 * time: 2013-11-11 14:11
 */

#include <stdio.h>


#define swap(x, y) \
    do { unsigned long _temp = x; x = y; y = _temp; } while(0)


int main()
{
    int x = 10;
    int y = 9;

    printf("before swap, x = %d, y = %d. \r\n", x, y);

    swap(x, y);

    printf("after swap, x= %d, y = %d. \r\n", x, y);

    return 0;
}

