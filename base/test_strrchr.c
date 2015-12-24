

#include <string.h>
#include <stdio.h>

int main()
{
    const char *p = "http://app.5mrj.com/upload/softVersion/1416453806933.bin";
    char *token = strrchr(p, '/');
    if (token == NULL)
    {
        printf("Not found! \r\n");
    }
    else
    {
        token++;
        printf("token: %s \r\n", token);
    }

    return 0;
}

