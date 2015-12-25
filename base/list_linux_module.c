
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MODULE_FILE     "/proc/modules"

int main()
{
    int count = 0;
    char buf[256] = {0};
    FILE *fp = NULL;

    fp = fopen(MODULE_FILE, "r");
    if (fp == NULL)
    {
        printf("Open %s Fail, %s \r\n", MODULE_FILE, strerror(errno));
        return -1;
    }

    while (fgets(buf, 512, fp))
    {
        printf("buf: %s \r\n", buf);
        
        if (strstr(buf, "mtnet7601Usta") != NULL)
        {
            count++;
        }
        else if (strstr(buf, "mt7601Usta") != NULL)
        {
            count++;
        }
        else if (strstr(buf, "mtutil7601Usta") != NULL)
        {
            count++;
        }
            
        if (count == 3)
        {
            break;
        }
    }

    fclose(fp);

    return 0;
}

