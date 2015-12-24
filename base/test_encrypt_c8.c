/*
 * A tool for c8 encrpyt and decrypt
 * time: 2013-11-08 13:37
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>		/* open */


#define INFOR_RET_SUCCESS    0
#define INFOR_RET_FAIL   	 1
#define INFOR_RET_NOACCESS  -1
#define INFOR_RET_WRONGPARA -2

#define MAX_LINE_LEN		(76)

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

int EncodeBase64(const unsigned char* pSrc, char* pDst, int nSrcLen,
		int nMaxLineLen) {
	unsigned char c1, c2, c3;    // è¾“å…¥ç¼“å†²åŒºè¯»å‡º3ä¸ªå­—èŠ‚
	int nDstLen = 0;             // è¾“å‡ºçš„å­—ç¬¦è®¡æ•°
	int nLineLen = 0;            // è¾“å‡ºçš„è¡Œé•¿åº¦è®¡æ•°
	int nDiv = nSrcLen / 3;      // è¾“å…¥æ•°æ®é•¿åº¦é™¤ä»¥3å¾—åˆ°çš„å€æ•°
	int nMod = nSrcLen % 3;      // è¾“å…¥æ•°æ®é•¿åº¦é™¤ä»¥3å¾—åˆ°çš„ä½™æ•°
	int i;
	// æ¯æ¬¡å–3ä¸ªå­—èŠ‚ï¼Œç¼–ç æˆ4ä¸ªå­—ç¬¦
	for (i = 0; i < nDiv; i++) {
		// å–3ä¸ªå­—èŠ‚
		c1 = *pSrc++;
		c2 = *pSrc++;
		c3 = *pSrc++;

		// ç¼–ç æˆ4ä¸ªå­—ç¬¦
		*pDst++ = EnBase64Tab[c1 >> 2];
		*pDst++ = EnBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3f];
		*pDst++ = EnBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3f];
		*pDst++ = EnBase64Tab[c3 & 0x3f];
		nLineLen += 4;
		nDstLen += 4;

		// è¾“å‡ºæ¢è¡Œï¼Ÿ
		if (nLineLen > nMaxLineLen - 4) {
			*pDst++ = '\r';
			*pDst++ = '\n';
			nLineLen = 0;
			nDstLen += 2;
		}
	}

	// ç¼–ç ä½™ä¸‹çš„å­—èŠ‚
	if (nMod == 1) {
		c1 = *pSrc++;
		*pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
		*pDst++ = EnBase64Tab[((c1 & 0x03) << 4)];
		*pDst++ = '=';
		*pDst++ = '=';
		nLineLen += 4;
		nDstLen += 4;
	} else if (nMod == 2) {
		c1 = *pSrc++;
		c2 = *pSrc++;
		*pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
		*pDst++ = EnBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
		*pDst++ = EnBase64Tab[((c2 & 0x0f) << 2)];
		*pDst++ = '=';
		nDstLen += 4;
	}

	// è¾“å‡ºåŠ ä¸ªç»“æŸç¬¦
	*pDst = '\0';

	return nDstLen;
}

int DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen) 
{
	int nDstLen;            // è¾“å‡ºçš„å­—ç¬¦è®¡æ•°
	int nValue;             // è§£ç ç”¨åˆ°çš„é•¿æ•´æ•°
	int i;

	i = 0;
	nDstLen = 0;

	// å–4ä¸ªå­—ç¬¦ï¼Œè§£ç åˆ°ä¸€ä¸ªé•¿æ•´æ•°ï¼Œå†ç»è¿‡ç§»ä½å¾—åˆ°3ä¸ªå­—èŠ‚
	while (i < nSrcLen) 
    {
		if (*pSrc != '\r' && *pSrc != '\n') 
        {
			nValue = DeBase64Tab[*pSrc++] << 18;
			nValue += DeBase64Tab[*pSrc++] << 12;
			*pDst++ = (nValue & 0x00ff0000) >> 16;
			nDstLen++;

			if (*pSrc != '=') 
            {
				nValue += DeBase64Tab[*pSrc++] << 6;
				*pDst++ = (nValue & 0x0000ff00) >> 8;
				nDstLen++;

				if (*pSrc != '=') 
                {
					nValue += DeBase64Tab[*pSrc++];
					*pDst++ = nValue & 0x000000ff;
					nDstLen++;
				}
			}

			i += 4;
		} 
        else        // å›è½¦æ¢è¡Œï¼Œè·³è¿‡
		{
			pSrc++;
			i++;
		}
	}

	// è¾“å‡ºåŠ ä¸ªç»“æŸç¬¦
	*pDst = '\0';

	return nDstLen;
}

char MakecodeChar(char c, int key) {
	return c = c ^ key;
}

//åŠ å¯†
void BitEncode(char *pstr) {
	int i;
	int len = strlen(pstr);        //è·å–é•¿åº¦
	for (i = 0; i < len; i++)
		*(pstr + i) = MakecodeChar(*(pstr + i), bitKey[i % 5]);
}

char CutcodeChar(char c, int key) {
	return c ^ key;
}

//è§£å¯†
void BitDecode(char *pstr) {
	int len = strlen(pstr);
    int i;
	for (i = 0; i < len; i++)
		*(pstr + i) = CutcodeChar(*(pstr + i), bitKey[i % 5]);
}

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

int FileBitDecrypt(char *pSrcPath, char *pDstPath)
{
	int ret = INFOR_RET_FAIL;
	
	if (pSrcPath == NULL || pSrcPath[0] == '\0')
		return ret;
	if (pDstPath == NULL || pDstPath[0] == '\0')
		return ret;
	
	struct stat st;
	if (stat(pSrcPath, &st) != 0 || st.st_size <= 0)
		return ret;

	int iFileSize = st.st_size;

	printf("iFileSize: %d %s %d\r\n", iFileSize, __FUNCTION__, __LINE__);

	char *pSrc = NULL;
	pSrc = (char *)malloc(iFileSize * sizeof(char));
	if (pSrc == NULL)
	{
		return ret;
	}

	memset(pSrc, 0, iFileSize);
	int iReadLen = ReadFile(pSrc, iFileSize, pSrcPath);

	printf("iReadLen: %d %s %d\r\n", iReadLen, __FUNCTION__, __LINE__);
	
	/* ¶ÁÈ¡ÎÄ¼şÊ§°Ü */
	if (iReadLen != iFileSize)
	{
		if (pSrc != NULL)
			free(pSrc);
		return ret;
	}
	
	char *pDst = NULL;
	int iDstLen = iReadLen * 2;
	pDst = (char *)malloc(iDstLen * sizeof(char));
	if (pDst == NULL)
	{
		if (pSrc != NULL)
			free(pSrc);
		return ret;
	}

	memset(pDst, 0, iDstLen);
	int iDstBase64Len = DecodeBase64(pSrc, (unsigned char *)pDst, iReadLen);
	BitDecode(pDst);
	int iWriteLen = WriteFile(pDst, iDstBase64Len, pDstPath);
	if (iWriteLen == iDstBase64Len)
	{
		ret = INFOR_RET_SUCCESS;
	}
    
	if (pSrc != NULL)
		free(pSrc);
	if (pDst != NULL)
		free(pDst);

	return ret;
}

/**********************************************************************************
º¯ÊıÃû³Æ£ºFileBitEncrypt
º¯Êı¹¦ÄÜ: ¶Ôsrc_fileÄÚÈİ½øĞĞ¼ÓÃÜ²Ù×÷£¬¼ÓÃÜºó½á¹û´æÔÚdst_fileÎÄ¼şÖĞ
Èë¿Ú²ÎÊı£º
			src_file: Ã÷ÎÄÎÄ¼ş
			dst_file: ÃÜÎÄÎÄ¼ş
³ö¿Ú²ÎÊı: ÎŞ
·µ»ØÖµ  £ºINFOR_RET_FAIL-Ê§°Ü£¬INFOR_RET_SUCCESS-³É¹¦
***********************************************************************************/
int FileBitEncrypt(const char *src_file, const char *dst_file)
{
	int ret = INFOR_RET_FAIL;
	int read_len = 0;
    int write_len = 0;
    int src_len;
    int dst_len;
    int base64_len;
    char *psrc_buf = NULL;
    char *pdst_buf = NULL;
    struct stat st;
    
	if (src_file == NULL || src_file[0] == '\0')
		return ret;

    if (dst_file == NULL || dst_file[0] == '\0')
		return ret;

	if (stat(src_file, &st) != 0 || st.st_size <= 0)
    {
		return ret;
    }

    printf("%s %d\r\n", __FUNCTION__, __LINE__);
    
   	src_len = st.st_size;
	dst_len = (src_len * 2);

    psrc_buf = (char *)malloc((src_len + 1) * sizeof(char));
    if (psrc_buf == NULL)
    {
        return ret;
    }

    printf("%s %d\r\n", __FUNCTION__, __LINE__);
    
    memset(psrc_buf, '\0', sizeof(src_len) + 1);

    read_len = ReadFile(psrc_buf, src_len, src_file);
    if (read_len != src_len)
    {
		free(psrc_buf);
        return ret;
    }

     printf("%s %d\r\n", __FUNCTION__, __LINE__);
    pdst_buf = (char *)malloc(dst_len * sizeof(char));
	if (pdst_buf == NULL)
    {
		free(psrc_buf);
        return ret;
    }

     printf("%s %d\r\n", __FUNCTION__, __LINE__);
     
    memset(pdst_buf, '\0', dst_len);
	BitEncode(psrc_buf);
    base64_len = EncodeBase64((unsigned char *)psrc_buf, pdst_buf, read_len, MAX_LINE_LEN);

 	write_len = WriteFile(pdst_buf, base64_len, dst_file);

     printf("write_len = %d, base64_len = %d %s %d\r\n", write_len, base64_len, __FUNCTION__, __LINE__);
     
    if (write_len == base64_len)
    {
		ret = INFOR_RET_SUCCESS;
    }

     printf("%s %d\r\n", __FUNCTION__, __LINE__);
     
    if (pdst_buf)
    {
		free(pdst_buf);
    }

    if (psrc_buf)
    {
		free(psrc_buf);
    }
    
	return ret;
}

int main(int argc, char *argv[])
{
    char cmd;
	char src_file[128];
    char dst_file[128];
	int src_len;
    int dst_len;
    
    while (1)
    {
		printf("\r\n");
        printf("1. File Decrypt. \r\n");
        printf("2. File Encrypt. \r\n");
        printf("3. quit. \r\n");
        printf("please input cmd (1-3): ");

        cmd = getchar();
		if(((cmd >= '0') && (cmd <= '9')) || ((cmd >= 'a') && (cmd <= 'z')))
		{
			while ((getchar()) != '\n')
			{
			}
		}

        if (cmd == '3')
		{
            printf("exit this program, thanks!\r\n");
			break;
		}

        if (cmd == '1')
        {
			printf("input src file name: ");
            fgets(src_file, sizeof(src_file) - 1, stdin);
            src_len = strlen(src_file);
            src_file[src_len - 1] = '\0';
			getchar();
            printf("input dst file name: ");
            fgets(dst_file, sizeof(dst_file) - 1, stdin);
            dst_len = strlen(dst_file);
            dst_file[dst_len - 1] = '\0';
            getchar();
            
            if (FileBitDecrypt(src_file, dst_file) == INFOR_RET_SUCCESS)
            {
				printf("%s decrypt success, dst file is %s. \r\n", src_file, dst_file);
            }
            else
            {
				printf("%s decrypt fail, try again. \r\n", src_file);		
            }

        }
        else if (cmd == '2')
        {
            
			printf("input src file name: ");
            fgets(src_file, sizeof(src_file) - 1, stdin);
            src_len = strlen(src_file);
            src_file[src_len - 1] = '\0';
            
			getchar();
            printf("input dst file name: ");
            fgets(dst_file, sizeof(dst_file) - 1, stdin);
			dst_len = strlen(dst_file);
            dst_file[dst_len - 1] = '\0';
            
            getchar();

            printf(" src_file %s, dst file %s. \r\n", src_file, dst_file);
            
            if (FileBitEncrypt(src_file, dst_file) == INFOR_RET_SUCCESS)
            {
				printf("%s encrypt success, dst file is %s. \r\n", src_file, dst_file);
            }
            else
            {
				printf("%s encrypt fail, try again. \r\n", src_file);		
            }
        }
    }
    
    return (0);        
}

