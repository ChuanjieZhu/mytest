
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


int write_t(char *FileName)
{
    int WriteLen = 0;
    int FileFd = 0;
    int DataLen = 0;
    char DataBuff[128];

    FileFd = open(FileName, O_APPEND | O_CREAT | O_NONBLOCK | O_SYNC | O_WRONLY, 0666);
    if (FileFd < 0)
    {
        return -1;
    }

    memset(DataBuff, 0, sizeof(DataBuff));

    snprintf(DataBuff, sizeof(DataBuff), "%d,%d,%lf|", 100, 100, 99.99);
    DataLen = strlen(DataBuff);

    WriteLen = write(FileFd, DataBuff, DataLen);

    printf("writeLen %d, DataLen %d \r\n", WriteLen, DataLen);
    
    if (WriteLen != DataLen)
    {
        printf("write error. \r\n");
    }

    close(FileFd);

    return 0;
}


int main()
{
    char *FileName = "test.dat";
    write_t(FileName);

    return 0;
}

