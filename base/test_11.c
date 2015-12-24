#include <stdio.h>

int main()
{
    char buf[4];
    buf[0] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    buf[4] = 0x04;

    char t = buf[2] << 8;

    printf("t %d \r\n", t);

    return 0;
}
