#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
    unsigned char buf[256] = {0}; 
    unsigned char ucCode[2] = {0};
    ucCode[0] = 80;
    ucCode[1] = 200;
    
    snprintf(buf, sizeof(buf), "code: %02X%02X", ucCode[0], ucCode[1]);

    printf("%s\r\n", buf);

    return 0;
}
