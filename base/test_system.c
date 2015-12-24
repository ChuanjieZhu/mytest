

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int ret = system("dhclient eth10");
    printf("ret: %d \r\n", ret);

    return 0;
}

