/*******************************************************************************
 *  for delete desheng record pictures
 *  2013-12-22 9:58
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>


#define MAX_TOTAL_PIC_NUM           10000               //ÕÕÆ¬×ÜÊýÁ¿
#define DEL_PIC_TIME_INTRVAL 		(3 * 60)			//ÕÕÆ¬É¾³ý¼ä¸ô3·ÖÖÓ

static int delRecordPictures(const char *pRcdPath)
{
    struct dirent **namelist;
    int i;
    int iTotal = 0;
    int iRcds = 0;
    int iTime;
    int iShouldDels = 0;
    int iHaveDels = 0;
    int iRet = 0;
    char acStudentId[20];
    char acTemp[128];
    char acPath[128];
    time_t tBaseTime;
    time_t tCurrTime;
    struct tm strTm;
    struct stat strSt;

	if (pRcdPath == NULL || pRcdPath[0] == '\0')
    {
        return iTotal;
    }
    
    tCurrTime = time(NULL);

    //printf("tCurrTime %lu \r\n", tCurrTime);
    
    iTotal = scandir(pRcdPath, &namelist, 0, alphasort);

    printf("--------->total picture numbers %d %s %d\r\n", iTotal, __FUNCTION__, __LINE__);

    if (iTotal == 0)
    {
        free(namelist);
    }
    else if (iTotal > 0)
    {
        if (iTotal > MAX_TOTAL_PIC_NUM)
        {
			iShouldDels = iTotal - MAX_TOTAL_PIC_NUM;
        }

        for (i = 0; i < iTotal; i++)
        {
			if ((strcmp(namelist[i]->d_name,".") == 0) || (strcmp(namelist[i]->d_name,"..")==0))
			{
				continue;
			}
            else
            {
                memset(acPath, 0, sizeof(acPath));
				snprintf(acPath, sizeof(acPath) - 1, "%s/%s", pRcdPath, namelist[i]->d_name);
				
                if (stat(acPath, &strSt) == 0)
                {
                    /*
                    printf("st_atime %lu, st_mtime %lu, st_ctime %lu \r\n", 
                    strSt.st_atime, strSt.st_mtime, strSt.st_ctime);;
                    */

                    if ((iShouldDels > 0) || ((strSt.st_mtime + (30 * 24 * 60 * 60)) < tCurrTime))
                    {
						unlink(acPath);
                        if (iShouldDels > 0)
                        {
							iShouldDels--;
                        }
                    }                    
                }
           	}
        }

        for (i = 0; i < iTotal; i++)
		{
			free(namelist[i]);
		}

		free(namelist);
    }

    return iTotal;
}

int main()
{
    time_t tCurTime = 0;
    time_t tLastDelTime = 0;
    
    while (1)
    {
        tCurTime = time(NULL);
	    if ((tCurTime - tLastDelTime) > DEL_PIC_TIME_INTRVAL)
	    {
	        tLastDelTime = tCurTime;
			delRecordPictures("/mnt/storage/stuData/pic");
	    }

        usleep(200 * 1000);
	}
    
    return 0;
}


