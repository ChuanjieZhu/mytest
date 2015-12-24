#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int iNum = 0;
    int iHead = 0;
    char c;
    char lineBuf[64] = {0};
    char buf[32] = {0};

    double d1 = 0.0;
    double d2 = 0.0;

    FILE *filefd = NULL;
    
    if (argv[1] == NULL)
    {
        printf("perror error!\r\n");
        return -1;
    }

    filefd = fopen(argv[1], "r");
    if (filefd == NULL)
    {
        printf("open file %s fail \r\n", argv[1]);
        return -2;
    }

    while ((c = fgetc(filefd)) != EOF)
    {
        if (c == '\n')
        {
            iNum++;
        }
    }

    printf("iNum %d \r\n", iNum);

    fseek(filefd, 0, SEEK_SET);

    while ((c = fgetc(filefd)) != EOF)
    {
        if (c == '\n')
        {
            iHead++;
        }

        if (iHead == iNum - 1)
        {
            break;
        }
    }

    fgets(lineBuf, 64, filefd);
    //fputs(lineBuf, stdout);
    fclose(filefd);
    
    printf("%s \r\n", lineBuf);

    char *cptr = strchr(lineBuf, '|');
    if (cptr != NULL)
    {
        int len = cptr - lineBuf;
        if (len != 20)
        {
            printf("postion error!\r\n");
            return -1;
        }
        else
        {
            printf("len %d \r\n", len);
            memcpy(buf, lineBuf, len);
            printf("buf %s \r\n", buf);
            sscanf(buf, "%lf,%lf", &d1, &d2);
            printf("d1: %lf, d2: %lf \r\n", d1, d2);
        }
    }

    //sscanf(lineBuf, "%lf,%lf", &d1, &d2);
    //printf("d1: %lf, d2: %lf \r\n", d1, d2);

    return 0;
}
