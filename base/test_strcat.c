
#include <stdio.h>
#include <string.h>

int testFunc(char *buff)
{
    strcat(buff, "111111111111\n");
    return 0;
}


int testFunc2(char *buff)
{
    strcat(buff, "22222222222\n");
    return 0;
}


int main()
{
    char buff[128] = {0};

    testFunc(buff);
    testFunc2(buff);

    printf("buff: %s \r\n", buff);

    return 0;
}

