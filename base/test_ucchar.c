#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    short size = 6650;

    unsigned char ucSize1 = (size >> 8) & 0x00FF;
    unsigned char ucSize2 = size & 0x00FF;

    printf("ucSize[0] %02X \r\n", ucSize1);
    printf("ucSize[1] %02X \r\n", ucSize2);

    return 0;
}
