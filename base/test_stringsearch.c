/*
 * A simple code for test
 * create time: 2012-09-28
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>      /* for EINTR */
#include <sys/types.h>  /* for stat */
#include <sys/stat.h>   /* for stat */
#include <errno.h>      /* for errno */

/* MACROS define */
#define MAX_FILE_BUF_SIZE (2 * 1024)
#define MAX_DST_FILE_BUF_SIZE (2 * 1024)

#define FILE_PATH "firs_auth.txt"

/* global variable define */
char g_cFileBuffer[MAX_FILE_BUF_SIZE] = {0};
char g_cDstFileBuffer[MAX_DST_FILE_BUF_SIZE] = {0};

/* local functions */
static int readn(int ifilefd, void *vptr, int ireadsize)
{
    int ileft = 0;
    int iread = 0;

    char *cptr = NULL;

    cptr = vptr;
    ileft = ireadsize;

    while (ileft > 0)
    {
        if ((iread = read(ifilefd, cptr, ileft)) < 0)
        {
            if (errno == EINTR)
            {
                iread = 0;
            }
            else
            {
                return -1;
            }
        }
        else if (iread == 0)
        {
            break;
        }
        
        ileft -= iread;
        cptr  += iread;
    }

    return (ireadsize - ileft);   /* 返回读取到的字节数 */
}

static int writen(int ifilefd, const void *cvptr, int iwritesize)
{
    int ileft = 0;
    int iwrite = 0;

    const char *ccptr = NULL;

    ccptr = cvptr;
    ileft = iwritesize;

    while (ileft > 0)
    {
        if ((iwrite = write(ifilefd, ccptr, ileft)) <= 0)
        {
            if (iwrite < 0 && errno == EINTR)
            {
                iwrite = 0;
            }
            else
            {
                return -1;
            }
        }

        ileft -= iwrite;
        ccptr += iwrite;
    }

    return iwritesize;
}

/* 
 * return value:
 * -1: pfilebuf is null
 * -2: pfilepath is null
 * -3: open file fail
 * >0: read file length
 *
 **/
int ReadFile(char *pfilebuf, int ibuffsize, char *pfilepath)
{
    int ifilefd = -1;
    int ifilelen = 0;
    int ireadlen = 0;

    char *ptr = NULL;

    struct stat st;

    if (pfilebuf == NULL)
    {
        printf("pfilebuf is null %d \r\n", __LINE__);
        return -1;
    }

    if (pfilepath == NULL || *pfilepath == '\0')
    {
        printf("pfilepath is null %d \r\n", __LINE__);
        return -2;
    }
    
    ifilefd = open(pfilepath, O_RDONLY);
    
    if (ifilefd < 0)
    {
        printf("open %d fail \r\n", pfilepath);
        return -3;
    }

    if (stat(pfilepath, &st) == 0)
    {
        ifilelen = st.st_size;
    }

    if (ifilelen > ibuffsize)
    {
        ifilelen = ibuffsize;
    }

    ireadlen = readn(ifilefd, pfilebuf, ifilelen);

    close(ifilefd);

    return ireadlen;
}

/**
 *
 * return value:
 * -1: pbuf is null
 * -2: pfilepath is null
 * -3: open file fail
 * >0: write data len
 *
 **/

int WriteFile(char *pbuf, int iwritesize, char *pfilepath)
{
    int ifilefd = -1;
    int iwritelen = 0;

    char *ptr = NULL;

    if (pbuf == NULL)
    {
        printf("pBuf is null %d \r\n", __LINE__);
        return -1;
    }

    if (pfilepath == NULL || *pfilepath == '\0')
    {
        printf("pfilepath is null %d \r\n", __LINE__);
        return -2;
    }

    ifilefd = open(pfilepath, O_RDWR | O_CREAT | O_APPEND, 0777);

    if (ifilefd < 0)
    {
        printf("open %s fail %d \r\n", pfilepath, __LINE__);
        return -3;
    }

    iwritelen = write(ifilefd, pbuf, iwritesize);

    close(ifilefd);

    return iwritelen;
}

/**
 * 
 * return value
 * -1: psearch buf is null;
 * -2: psrcbuf is null
 * -3: pdstbuf is null
 * -4: not find the src string
 *  0: find src string success
 *
 **/
int SearchString(char *psearchbuf, int inum, const char *psrcbuf, char *pdstbuf)
{
    int iret = -1;
    int i;
    int ioffset = 0;
    char *ptr = NULL;
    char tempbuf[32] = {0};

    if (psearchbuf == NULL)
    {
        printf("search buf is null %d \r\n", __LINE__);
        iret = -1;
    }

    if (psrcbuf == NULL)
    {
        printf("src buf is null %d \r\n", __LINE__);
        iret = -2;
    }

    if (pdstbuf == NULL)
    {
        printf("dst buf is null %d \r\n", __LINE__);
        iret = -3;
    }
    
    for (i = 0; i < inum; i++)
    {
        memset(tempbuf, 0, sizeof(tempbuf));
        memcpy(tempbuf, psearchbuf + ioffset, 30);
        //ioffset += 30;

        printf("%s \r\n", tempbuf);
        if (strstr(tempbuf, psrcbuf) != NULL)
        {
            memcpy(pdstbuf, tempbuf, 30);
            iret = 0;
            break;
        }
        
        ioffset += 30;
    }

#if 0
    ptr = strstr(psrcbuf, psearchbuf);

    if (ptr == NULL)
    {
        printf("no find string %d \r\n", __LINE__);
        iret = -4;
    }
    else
    {
        printf("%s \r\n", ptr);
        memcpy(pdstbuf, ptr, 30);
        iret = 0;
    }
#endif
    return iret;
}

int main()
{
    int iretvalue = -1;
    int inum = 0;
    char dststring[16] = {0};
    
    snprintf(dststring, sizeof(dststring), "0810812000040");

    memset(g_cFileBuffer, 0, sizeof(g_cFileBuffer));
    memset(g_cDstFileBuffer, 0, sizeof(g_cDstFileBuffer));

    iretvalue = ReadFile(g_cFileBuffer, MAX_FILE_BUF_SIZE, FILE_PATH);
    inum = iretvalue / 30;

    printf("iretvalue %d inum %d %d \r\n", iretvalue, inum, __LINE__);

    if (iretvalue > 0)
    {
#if 1
        printf("g_cFileBuffer: \r\n");
        printf("%s \r\n", g_cFileBuffer);

        printf("dststring: \r\n");
        printf("%s \r\n", dststring);
#endif

        iretvalue = SearchString(g_cFileBuffer, 7, dststring, g_cDstFileBuffer);

        printf("iretvalue %d %d \r\n", iretvalue, __LINE__);

        if (0 == iretvalue)
        {
            printf("find dst string success %d \r\n", __LINE__);

            printf("%s %d \r\n", g_cDstFileBuffer, __LINE__);
        }
        else
        {
            printf("not find dst string %d \r\n", __LINE__);
        }
    }
    else
    {
        printf("read file fail %d \r\n", __LINE__);
    }

    return 0;
}

