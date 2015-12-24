
#include <stdio.h>
#include <string.h>

int GetTopCmdResult(const char *pCmdString)
{
    static char buf[10 * 1024];
    FILE *popenStream = NULL;
	FILE *writeStream = NULL;

    popenStream = popen(pCmdString, "r");
    if (popenStream == NULL)
    {
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    fread(buf, sizeof(char), sizeof(buf), popenStream);

    printf("buf: %s %s %d\r\n", buf, __FUNCTION__, __LINE__);

    return 0;
}


int main()
{
    int ret = -1;

    ret = GetTopCmdResult("top");

    return ret;
}

