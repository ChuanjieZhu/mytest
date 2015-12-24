
#include <stdio.h>
#include <string.h>


#define LINUX_PROC_NET_DEV "/proc/net/dev"

int parse_proc_net_dev()
{
    FILE *fp = NULL;

    const char *pfile_path = LINUX_PROC_NET_DEV;
    
    fp = fopen(pfile_path, "r");
    if (fp == NULL)
    {
        printf("open %s fail. %s %d\r\n", pfile_path, __FUNCTION__, __LINE__);
        return -1;
    }

    char buff[1024] = {0};

    while (fgets(buff, sizeof(buff) - 1, fp) != NULL)
    {
        printf("%s \r\n", buff);
        if (strstr(buff, "eth11") != NULL)
        {
            printf("********* %s \r\n", buff);
        }
    }

    return 0;
}


int main()
{
    return parse_proc_net_dev();
}


