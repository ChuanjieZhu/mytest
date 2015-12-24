
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>


#define MAX_LINE 1024

#define _LINE_LENGTH 300

#define TRACE printf


int get_path_total(const char *path, long long* total)
{
    int err = -1;
    FILE *file;
    char line[_LINE_LENGTH];
    char *p;
    char tmp[100];
    char *token;
    sprintf(tmp, "df %s", path);
    file = popen(tmp, "r");

    if (file != NULL) 
    {
        if (fgets(line, _LINE_LENGTH, file) != NULL) 
        {
            if (fgets(line, _LINE_LENGTH, file) != NULL) 
            {
                token = strtok(line, " ");
                if (token != NULL) 
                {
                    printf("token=%s\n", token);
                }

                token = strtok(NULL, " ");
                if (token != NULL) 
                {
                    printf("token=%s\n", token);
                    *total=atoll(token)/1024;//k/1024
                    err=0;
                }
            }
        }
        pclose(file);
    }
    return err;
}

static int ExecCmd(const char *pCommand)
{
    if (pCommand == NULL || pCommand[0] == '\0')
    {
        TRACE("Error, Param Cmd is NULL! %s %d \r\n", __FUNCTION__, __LINE__);
        return -1;    

    }

    int ret = -1;
    char cmd[256];
    int rc = 0;
    FILE *fp = NULL;

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd) - 1, "%s", pCommand);
    TRACE("Cmd: %s %s %d\r\n", cmd, __FUNCTION__, __LINE__);

    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        TRACE("popen fail, %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        return -1;
    }

    rc = pclose(fp);
    if (rc == -1)
    {
        TRACE("pclose fail, %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        return -1;
    }

    TRACE("Parent: rc == %d, WIFEXITED(rc) == %d, WEXITSTATUS(rc) == %d \r\n", rc, WIFEXITED(rc), WEXITSTATUS(rc));
    
    if (WIFEXITED(rc) != 0)
    {
        if (WEXITSTATUS(rc) != 0)
        {
            ret = -1;
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        ret = -1;    

    }
    
    return ret;
}


static int ExecLoadWifiStaDriver()
{
    int iRet = -1;
    char cmd[256];

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd) - 1, "%s", "rmmod rtnet7601Uap");
    iRet = ExecCmd(cmd);
    TRACE("iRet: %d \r\n", iRet);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd) - 1, "%s", "rmmod mt7601Uap");
    iRet = ExecCmd(cmd);
    TRACE("iRet: %d \r\n", iRet);
    
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd) - 1, "%s", "rmmod rtutil7601Uap");
    iRet = ExecCmd(cmd);
    TRACE("iRet: %d \r\n", iRet);
    
    return iRet;
}

int main(int argc, char **argv)
{
    int iRet = ExecLoadWifiStaDriver();

    return iRet;
}
