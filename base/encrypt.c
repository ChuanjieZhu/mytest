#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#include <errno.h>
#include <string.h>

const int bitKey[] = { 1, 2, 3, 4, 5 };

const char EnBase64Tab[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        const char DeBase64Tab[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0,
                62,        // '+'
                0, 0, 0,
                63,        // '/'
                52, 53, 54, 55, 56, 57, 58, 59, 60,
                61,        // '0'-'9'
                0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                25,        // 'A'-'Z'
                0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,        // 'a'-'z'
                };

#define BASE64_LINE     (76)


int ReadFile(char *pBuf, int nBufSize, char *pFileName)
{
    int filefd = -1;
    int nfilelen = 0;
    int nreadlen = 0;
    struct stat st;

    if (pBuf == NULL || pFileName == NULL)
    {
    
            return -1;
        }

    filefd = open(pFileName, O_RDONLY);
    if (filefd < 0)
    {
            printf("open %s error, %s! %s %d\r\n", pFileName, strerror(errno), __FUNCTION__, __LINE__);
            return -1;
        }

    if (stat(pFileName, &st) == 0)
    {
            nfilelen = st.st_size;
        }

    if (nfilelen > nBufSize)
    {
            nfilelen = nBufSize;
        }

    nreadlen = read(filefd, pBuf, nfilelen);

    close(filefd);

    return nreadlen;
}



int WriteFile(char *pBuf, int nWriteSize, char *pFileName)
{
    int filefd = -1;
    int writelen = 0;
    int nleft = 0;
    struct stat st;
    char *ptr = NULL;

    if (pBuf == NULL || pFileName == NULL)
    {
            return -1;
        }

    filefd = open(pFileName, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    if (filefd < 0)
    {
            return -1;
        }

    /* 判断文件大小 */
    if (stat(pFileName, &st) == 0)
    {
            
        }
        
    ptr = pBuf;
    nleft = nWriteSize;

    while(nleft > 0)
    {
            if((writelen = write(filefd, ptr, nleft)) <= 0)
            {
                        if (writelen < 0 && errno == EINTR)
                        {
                                        writelen = 0;
                                    }
                        else
                        {
                                        return -1;
                                    }
                    }
    
            nleft -= writelen;
            ptr += writelen;
        }

    if (filefd != -1)
    {
            close(filefd);
        }
    
    return nWriteSize;
}

int DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen) {
    int nDstLen;            // 杈撳嚭鐨勫瓧绗﹁鏁?
    int nValue;             // 瑙ｇ爜鐢ㄥ埌鐨勯暱鏁存暟
    int i;

    i = 0;
    nDstLen = 0;

    while (i < nSrcLen) {
            if (*pSrc != '\r' && *pSrc != '\n') {
                        nValue = DeBase64Tab[*pSrc++] << 18;
                        nValue += DeBase64Tab[*pSrc++] << 12;
                        *pDst++ = (nValue & 0x00ff0000) >> 16;
                        nDstLen++;
            
                        if (*pSrc != '=') {
                                        nValue += DeBase64Tab[*pSrc++] << 6;
                                        *pDst++ = (nValue & 0x0000ff00) >> 8;
                                        nDstLen++;
                        
                                        if (*pSrc != '=') {
                                                            nValue += DeBase64Tab[*pSrc++];
                                                            *pDst++ = nValue & 0x000000ff;
                                                            nDstLen++;
                                                        }
                                    }
            
                        i += 4;
                    } else        // 鍥炶溅鎹㈣锛岃烦杩?
            {
                        pSrc++;
                        i++;
                    }
        }

    *pDst = '\0';

    return nDstLen;
}

char MakecodeChar(char c, int key) {
    return c = c ^ key;
}

char CutcodeChar(char c, int key) {
    return c ^ key;
}

void BitDecode(char *pstr) {
    int len = strlen(pstr);
    int i;
    for (i = 0; i < len; i++)
        *(pstr + i) = CutcodeChar(*(pstr + i), bitKey[i % 5]);
}

int file_bit_decrypt(const char *src_file_path, const char *dst_file_path)
{
    struct stat st;
    if (stat(src_file_path, &st) != 0 || st.st_size <= 0)
    {
        return -1;
    }

    int size = st.st_size;
    printf("size: %d \r\n", size);

    char *psrc = (char *)malloc((size + 1) * sizeof(char));
    if (psrc == NULL)
    {
        return -1;
    }

    int src_len = ReadFile(psrc, size + 1, src_file_path);
    if (src_len != size)
    {
        free(psrc);
        return -1;
    }

    char *pdst = NULL;
    int dst_len = src_len * 2;
    pdst = (char *)malloc(dst_len * sizeof(char));
    if (pdst == NULL)
    {
        free(psrc);
        return -1;
    }

    int dst_base64_len = DecodeBase64(psrc, (unsigned char *)pdst, src_len);
    BitDecode(pdst);
    WriteFile(pdst, dst_base64_len, dst_file_path);

    if (psrc)
        free(psrc);

    if (pdst)
        free(pdst);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        return -1;
    }

    file_bit_decrypt(argv[1], argv[2]);

    return 0;
}
