#include <stdio.h>

int main()
{
    int size = 12345678;
    
    float msize = (size / (1000000)) + ((size % 1000000) / 1000000);

    printf("msize: %01fM \r\n", msize);

    return 0;
}
