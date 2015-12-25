#include "md5.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

/* md5������������ */
const char *Mutex = "www.pfly.cn";

/**********************************************************************************
�������ƣ�GetFileMd5Code
��������: ��ȡָ���ļ���md5ֵ
��ڲ�����	pSrcFile: ��Ҫ����md5ֵ�ļ�·��
			pMd5File: md5����洢�ļ�·��,�������Ҫ��md5ֵ�����ļ����ò�������ΪNULL
			iCodeSize: md5ֵ���ػ����С������С��32�ֽ�
���ڲ���:			
			pMd5Code: md5ֵ���ػ���
����ֵ:	0 - �ɹ�������: ʧ��
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
�������ƣ�GetDataMd5Code
��������: ��ȡָ�����ݵ�md5ֵ
��ڲ�����	pSrcData: ��Ҫ����md5ֵ�����ݻ���buffer
			iDataSize: �����md5ֵ���ݴ�С
			iCodeSize: md5ֵ���ػ����С������С��32�ֽ�
���ڲ���:			
			pMd5Code: md5ֵ���ػ���
����ֵ:	0 - �ɹ�������: ʧ��
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
