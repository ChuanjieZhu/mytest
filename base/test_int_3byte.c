/*
 *  A simple test program to transfer a int type to 3 bytes
 */

#include <stdio.h>

void Int2ThreeBytes(int i)
{
    unsigned char buf[3];

    buf[0] = (i >> 16) & 0xff;
    buf[1] = (i >> 8) & 0xff;
    buf[2] = i & 0xff;

    printf("buf[0]: 0x%02X, buf[1]: 0x%02X, buf[2]: 0x%02X \r\n", buf[0], buf[1], buf[2]);
}

int main(void)
{
    int i = 1024;

    Int2ThreeBytes(i);

    return 0;
}

