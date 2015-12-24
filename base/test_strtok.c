#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int test()
{
    const char *pstr = "set_network 0 ssid xiaozhou";
    char buffer[100] = {0};
    snprintf(buffer, sizeof(buffer) - 1, "%s", pstr);

    char *p = strtok(buffer, " ");
    printf("%s \r\n", p);
    while (p != NULL)
    {
        printf("buffer: %s \r\n", buffer);
        printf("p: %s \r\n", p);
        p = strtok(NULL, " ");
    }

    return 0;
}

int main()
{
    char *str = "1111.2222.3333.4444.5555";
    char buf[100] = {0};
    strcpy(buf, str);
    /* 这里不能直接对str进行strtok操作，str为常量字符串，不能被修改 */
    char *p = strtok(buf, ".");
    printf("p: %s \r\n", p);
    while (p != NULL)
    {
        p = strtok(NULL, ".");
        printf("p: %s \r\n", p);
    }

    return 0;
}
