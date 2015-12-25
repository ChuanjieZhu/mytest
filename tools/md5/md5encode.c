#include "md5.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

/* md5附加输入内容 */
const char *Mutex = "www.pfly.cn";

/**********************************************************************************
函数名称：GetFileMd5Code
函数功能: 获取指定文件的md5值
入口参数：	pSrcFile: 需要计算md5值文件路径
			pMd5File: md5结果存储文件路径,如果不需要将md5值存入文件，该参数设置为NULL
			iCodeSize: md5值返回缓存大小，不能小于32字节
出口参数:			
			pMd5Code: md5值返回缓存
返回值:	0 - 成功，其它: 失败
***********************************************************************************/
int GetFileMd5Code(const char *pSrcFile, const char *pMd5File, char *pMd5Code, int iCodeSize)
{
	int ret = -1;
	int hSrc = -1;
	int hDst = -1;
	char tmp[32];
	int i = 0;
	
	if (pSrcFile == NULL 
		|| pMd5Code == NULL 
		|| iCodeSize < 32)
	{
		return -1;
	}

	if (pMd5File != NULL && pMd5File[0] != '\0')
	{
		hDst = open(pMd5File, O_CREAT|O_WRONLY|O_TRUNC, 0777);
		if (hDst == -1)
		{
			if (hSrc != -1)
			{
				close(hSrc);
			}
			
			return ret;
		}
	}

	hSrc = open(pSrcFile, O_RDONLY);
	
	if(hSrc == -1) 
	{
		if (hDst != -1)
		{
			close(hDst);
		}
		
	    return ret;
	}
	else 
	{
		MD5_CONTEXT mtx;
		int len;
		char buf[50*1024], key[64];
		struct stat st;

		if(fstat(hSrc, &st) != 0 || st.st_size <= 0) 
		{
			printf("Source file %s invalid !\r\n", pSrcFile);
		} 
		else 
		{
			int	nSizeRead = 0;
			MD5Init(&mtx);
			while(st.st_size) 
			{
				len = read(hSrc, buf, 1024*50);
				if(len<=0)
				{
					break;
				}
				MD5Update(&mtx, (unsigned char *)buf, len);

				nSizeRead += len;
				ret = 0;
				if (nSizeRead >= st.st_size) 
				{
					ret = 0;
					break;
				}
				
				if(len < 1024*50) 
				{
					ret = 0;
					break;
				}
			}
		}

		if(ret == 0) 
		{
			MD5Final((unsigned char*)key, &mtx);

			memset(buf, 0, sizeof(buf));
			for( i=0; i<16; i++ )
			{ 
				sprintf(tmp, "%02x", key[i]); 
				strcat(buf,tmp); 
			}

			MD5Init(&mtx);
			
			sprintf(buf, "%s%s", buf, Mutex);
			
			MD5Update(&mtx, (unsigned char *)buf, strlen(buf));
			MD5Final((unsigned char*)key, &mtx);

			memset(buf, 0, sizeof(buf));
			for( i=0; i< 16; i++ )
			{ 
				sprintf(tmp,"%02x", key[i] ); 
				strcat(buf,tmp); 
			}

			memcpy(pMd5Code, buf, iCodeSize);

			if (hDst != -1)
			{
				write(hDst, buf, strlen(buf));
			}
		}
	}

	if(hSrc != -1)
    {
        close(hSrc);
	}

	if(hDst != -1) 
	{
		close(hDst);

        if(ret != 0)
        { 
            unlink(pMd5File);
		}
	}
	
	return ret;
}

/**********************************************************************************
函数名称：GetDataMd5Code
函数功能: 获取指定数据的md5值
入口参数：	pSrcData: 需要计算md5值的数据缓存buffer
			iDataSize: 需计算md5值数据大小
			iCodeSize: md5值返回缓存大小，不能小于32字节
出口参数:			
			pMd5Code: md5值返回缓存
返回值:	0 - 成功，其它: 失败
***********************************************************************************/
int GetDataMd5Code(const char *pSrcData, int iDataSize, char *pMd5code, int iCodeSize)
{
	char tmp[32];
	int i = 0;
	
	if (pSrcData == NULL 
		|| iDataSize <= 0 
		|| pMd5code == NULL
		|| iCodeSize < 32)
	{
		return -1;
	}
	
	MD5_CONTEXT mtx;
	char buf[50*1024], key[64];
	
	MD5Init(&mtx);
	MD5Update(&mtx, (unsigned char *)pSrcData, iDataSize);

	MD5Final((unsigned char*)key, &mtx);

	memset(buf, 0, sizeof(buf));
	for( i=0; i<16; i++ )
	{ 
		sprintf(tmp, "%02x", key[i]); 
		strcat(buf,tmp); 
	}

	MD5Init(&mtx);
	
	sprintf(buf, "%s%s", buf, Mutex);
	
	MD5Update(&mtx, (unsigned char *)buf, strlen(buf));
	MD5Final((unsigned char*)key, &mtx);

	memset(buf, 0, sizeof(buf));
	for( i=0; i< 16; i++ )
	{ 
		sprintf(tmp,"%02x", key[i] ); 
		strcat(buf,tmp); 
	}

	memcpy(pMd5code, buf, iCodeSize);
    
	return 0;
}
