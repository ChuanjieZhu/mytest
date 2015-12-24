#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 256

void ScandirFiles(const char *path, char *pName)
{
    struct dirent **namelist = NULL;
    int i, n = 0;
    
    n = scandir(path, &namelist, NULL, NULL);

    if (n >= 0)
    {
        char cwd[MAX_LEN];
        getcwd(cwd, MAX_LEN);
        chdir(path);

        for (i = 0; i < n; i++)
        {
            if (namelist[i]->d_type & 0x04)
            {
                continue;
            }
            else
            {
                if (0 == strncmp(namelist[i]->d_name, "12345678", 7))
                {
                    printf("Name: %s \r\n", namelist[i]->d_name);
                    strncpy(pName, namelist[i]->d_name, strlen(namelist[i]->d_name));
                    break;
                }
            }
        }

        while (n--)
        {
            free(namelist[n]);
        }

        if (namelist)
        {
            free(namelist);
        }

        chdir(cwd);
    }
}

void ListFiles(const char *path)
{
    struct dirent **namelist = NULL;
    int i, n = 0;
    
    n = scandir(path, &namelist, NULL, alphasort);

    printf("n: %d \r\n", n);
    
    if (n >= 0)
    {
        printf("namelast: %s \r\n", namelist[n - 1]->d_name);
        
        for (i = 0; i < n; i++)
        {
            printf("name: %s \r\n", namelist[i]->d_name);
        }

        while (n--)
        {
            free(namelist[n]);
        }

        if (namelist)
        {
            free(namelist);
        }
    }
}

int main(int c, char **v)
{
    //char name[MAX_LEN] = {0};
    //ScandirFiles(".", name);
    
    //printf("name##: %s \r\n", name);
    ListFiles("/root/mytest/test");
    return 0;
}
