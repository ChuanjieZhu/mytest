
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <dirent.h>

int GetStorageFreeSize(const char *storage, unsigned int *freesize)
{
    int ret = -1;
	struct statvfs st;

    if (statvfs(storage, &st) == 0)
    {
		if (freesize != NULL)
		{
			*freesize = st.f_bfree * st.f_bsize;
            ret = 0;
		}
	}

    return ret;        
}

#define STORAGE_DATA_PATH   "/mnt/storage/record"
#define EXTERN_DOWNLOAD_SIZE   5 * 1024


int DelOldRecordFile(char *path)
{
    int ret = -1;
	DIR *pDir;
	char oldFileName[32] = {0};
	char oldFilePath[128] = {0};
	char tmpFilePath[128] = {0};
	struct dirent *dp = NULL;
	struct stat st;
	time_t tt;
	struct tm strTm;
	struct tm *pTm = &strTm;
	char curFileName[32] = {0};

	pDir = opendir(path);
	if(pDir != NULL)
	{
		while((dp = readdir(pDir)) != NULL)
		{
			if ((strcmp(dp->d_name, ".") == 0) || (strcmp(dp->d_name, "..")==0))
			{
				continue;
			}

            memset(tmpFilePath, 0 ,sizeof(tmpFilePath));
			sprintf(tmpFilePath, "%s/%s", path, dp->d_name);

			if (0 == stat(tmpFilePath, &st))
			{
				//是目录
				if (S_ISDIR(st.st_mode))
				{
					continue;
				}
				else 
				{   
					if (strlen(oldFileName) == 0)
					{
						memcpy(oldFileName, dp->d_name, sizeof(oldFileName));
					}
					else
					{
						//该文件更旧
						if (memcmp(dp->d_name, oldFileName, sizeof(oldFileName)) < 0)
						{
							memset(oldFileName, 0, sizeof(oldFileName));
							memcpy(oldFileName, dp->d_name, sizeof(oldFileName));
						}
					}
				}
			}
		}

		if (strlen(oldFileName) > 0)
		{
			tt = time(0);
			localtime_r(&tt, pTm);
			
			sprintf(curFileName, "%04d%02d%02d.data", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday);

			printf("* cur filename(%s) old filename(%s). %s %d\r\n", curFileName, oldFileName, __FUNCTION__, __LINE__);

			if (memcmp(oldFileName, curFileName, sizeof(curFileName)) < 0)
			{
				sprintf(oldFilePath, "%s/%s", path, oldFileName);
                printf("* del record file(%s) %s %d\r\n", oldFileName, __FUNCTION__, __LINE__);
				unlink(oldFilePath);
                ret = 0;
			}
		}
		
		closedir(pDir);
	}

    return ret;
}


int testFunc()
{
    /* 检测flash空间大小，如果flash空间小于2M，删除记录文件 */
    unsigned int freeSize = 0;
    unsigned int downloadSize = 3894611; /* 需要下载的文件大小 */
    const char *storage = "/";
    GetStorageFreeSize(storage, &freeSize);

    printf("* download size: %d, storage free size: %d. %s %d\r\n", downloadSize, freeSize, __FUNCTION__, __LINE__);

    while (freeSize < downloadSize + EXTERN_DOWNLOAD_SIZE)
    {   
        if (DelOldRecordFile((char *)STORAGE_DATA_PATH) != 0)
        {
            printf("> delete record fail, break. \n");
            break;
        }
        
        GetStorageFreeSize(storage, &freeSize);

        printf("* download size: %u, storage free size: %u. %s %d\r\n", downloadSize, freeSize, __FUNCTION__, __LINE__);
        
        usleep(10 * 1000);
    }

    GetStorageFreeSize(storage, &freeSize);
    
    printf("* download size: %d, storage free size: %d. %s %d\r\n", downloadSize, freeSize, __FUNCTION__, __LINE__);
    
    return 0;
}


int main(int argc, char *argv[])
{
    const char *storage = "/";
    unsigned int freesize = 0;

    //GetStorageFreeSize(storage, &freesize);
    
    //printf("* freesize: %u \r\n", freesize / (1024 * 1024));

    testFunc();

    return 0;
}

