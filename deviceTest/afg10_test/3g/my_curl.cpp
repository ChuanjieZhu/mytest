/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  �ļ�˵��: ftp curl lib �ӿڷ�װ
**  ��������: 2014.02.26
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "curl/curl.h"
#include "my_curl.h"


/* ����ftp��������ʱ�� */
static long ulFtpTime = 30;

static size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data)
{
  (void)ptr;
  (void)data;
  /* we are not interested in the headers itself,
     so we only return the size we would have saved ... */
  return (size_t)(size * nmemb);
}

/******************************************************************************
 * �������ƣ� CurlFtpGetFileLenth
 * ���ܣ� FTP��ȡ������ָ���ļ�����
 * ������ remoteFile: ����������ȡ�����ļ�����dir:ftp��������Ŀ¼����ipaddr:������ip��ַ��
          port: �˿ںţ�username: ftp�û�����password:ftp����
 * ���أ� �ɹ�-��ȡ�ļ�����, -1-ʧ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-12
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int CurlFtpGetFileLenth(const char *remoteFile, const char *dir, const char *ipaddr, int port,
	const char *username, const char *password)
{
	CURL *pstrhandle;
	char acURL[128] = {0};	
	char acArg[32] = {0};
	double dFileSize = -1;
    int iRet;

	if(remoteFile == NULL)
	{
		return -1;
	}

	if ((dir == NULL) || (*dir == 0))
	{
		if (port == 21)
		{
			snprintf(acURL, 127, "ftp://%s/%s", ipaddr, remoteFile);
		}
		else
		{
			snprintf(acURL, 127, "ftp://%s/%s", ipaddr, remoteFile);
		}
	}
	else
	{
		if (port == 21)
		{
			snprintf(acURL, 127, "ftp://%s/%s/%s", ipaddr, dir, remoteFile);
		}
		else
		{
			snprintf(acURL, 127, "ftp://%s/%s/%s", ipaddr, dir, remoteFile);
		}
	}	

	if ((port < 1) || (port > 65535))
	{
		printf("port %d invalid %s %d\n", port, __FUNCTION__, __LINE__);
		
		return -1;
	}
	
	if ((ipaddr == NULL) || (*ipaddr == '\0') || (username == NULL) || (*username == '\0')
	   || (password == NULL) || (*password == '\0'))
	{
		printf("param invalid %s %d\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	if (curl_global_init(CURL_GLOBAL_ALL) != 0)
	{
		printf("curl_global_init fail %s %d\n", __FUNCTION__, __LINE__);
		
		return -1;
	}
    
	pstrhandle = curl_easy_init();
    if (pstrhandle == NULL)
    {
		printf("curl_easy_init fail %s %d\n", __FUNCTION__, __LINE__);
		curl_global_cleanup();

		return -1;
	}

    memset(acArg, 0, sizeof(acArg));
	snprintf(acArg, 31, "%s:%s", username, password);

	curl_easy_setopt(pstrhandle, CURLOPT_PORT, port);
	curl_easy_setopt(pstrhandle, CURLOPT_USERPWD, acArg);
    curl_easy_setopt(pstrhandle, CURLOPT_URL, acURL);

    /* No download if the file */
    curl_easy_setopt(pstrhandle, CURLOPT_NOBODY, 1L);
    
    /* Ask for lfiletime */
    curl_easy_setopt(pstrhandle, CURLOPT_FILETIME, 1L);
    
    /* No header output: TODO 14.1 http-style HEAD output for ftp */
    curl_easy_setopt(pstrhandle, CURLOPT_HEADERFUNCTION, throw_away);
    
    curl_easy_setopt(pstrhandle, CURLOPT_HEADER, 0L);

	curl_easy_setopt(pstrhandle, CURLOPT_TIMEOUT, ulFtpTime);
	curl_easy_setopt(pstrhandle, CURLOPT_CONNECTTIMEOUT, ulFtpTime);

	
    /* Switch on full protocol/debug output */
    /* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */

	//curl_easy_setopt(pstrhandle, CURLOPT_LOW_SPEED_LIMIT, 1L);
	//curl_easy_setopt(pstrhandle, CURLOPT_LOW_SPEED_TIME, 5L);
    
    iRet = curl_easy_perform(pstrhandle);
    if(CURLE_OK == iRet) 
    {
      //(void)curl_easy_getinfo(pstrhandle, CURLINFO_FILETIME, &lfiletime);
      (void)curl_easy_getinfo(pstrhandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dFileSize);
    } 
    else 
    {
      /* we failed */
      fprintf(stderr, "curl told us %d\n", iRet);
    }

	curl_easy_cleanup(pstrhandle);
	curl_global_cleanup();

	return dFileSize;
}

/******************************************************************************
 * �������ƣ� CurlFtpUploadFile
 * ���ܣ� FTP�ϴ��ļ�
 * ������ localFile: �ϴ����ļ�����; file_len: �ϴ����ļ��ĳ���; 
          remoteFile: �ϴ����������ļ����� dir:�ϴ���FTP�ĸ�Ŀ¼;
 	ipaddr: FTP��������IP��ַ; username: FTP���������û���; password: FTP������������;
 * ���أ� 
 * �������ߣ� Jason
 * �������ڣ� 2012-10-12
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int CurlFtpUploadFile(const char *localFile, const char *remoteFile, int file_len, const char *dir, const char *ipaddr, int port,
	const char *username, const char *password, int iTmpStamp)
{
	FILE *fp;
	char arg[32];
	char url[128];	
	char temp[128];	
	char gb2312_filename[128];	
	CURL *curl;
    int iFileSize;

    char acBuf1[128] = {0};
    char acBuf2[128] = {0};
    struct curl_slist *headerlist=NULL;
    
	if ((remoteFile == NULL) || (*remoteFile == '\0'))
	{
		printf("file NULL %s %d\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	if (file_len < 0)
	{
		printf("file_len %d invalid %s %d\n", file_len, __FUNCTION__, __LINE__);
		
		return -1;
	}
	
	if (file_len == 0)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}
	
	if ((port < 1) || (port > 65535))
	{
		printf("port %d invalid %s %d\n", port, __FUNCTION__, __LINE__);
		
		return -1;
	}
	
	if ((ipaddr == NULL) || (*ipaddr == '\0') || (username == NULL) || (*username == '\0')
	   || (password == NULL) || (*password == '\0'))
	{
		printf("param invalid %s %d\n", __FUNCTION__, __LINE__);
		
		return -1;
	}
	
	memset(temp, 0, 128);
    if (iTmpStamp == 0)
    {
	    snprintf(temp, sizeof(temp) - 1, "%s.tmp", remoteFile);
    }
    else
    {
        snprintf(temp, sizeof(temp) - 1, "%s.tmp.%d", remoteFile, iTmpStamp);
    }
	
	memset(gb2312_filename, 0, 128);
	strcpy(gb2312_filename, temp);
	
	if (*gb2312_filename == '\0')
	{
		printf("gb2312_filename invalid %s %d\n", __FUNCTION__, __LINE__);
		
		return -1;
	}
		
	memset(url, 0, 128);
	
	if ((dir == NULL) || (*dir == 0))
	{
		if (port == 21)
		{
			snprintf(url, 127, "ftp://%s/%s", ipaddr, gb2312_filename);
		}
		else
		{
			snprintf(url, 127, "ftp://%s/%s", ipaddr, gb2312_filename);
		}
	}
	else
	{
		if (port == 21)
		{
			snprintf(url, 127, "ftp://%s/%s/%s", ipaddr, dir, gb2312_filename);
		}
		else
		{
			snprintf(url, 127, "ftp://%s/%s/%s", ipaddr, dir, gb2312_filename);
		}
	}	

    /* �ϴ��ļ�������FTPָ�� */
    snprintf(acBuf1, 127, "RNFR %s", gb2312_filename);
    snprintf(acBuf2, 127, "RNTO %s", remoteFile);
	
	memset(arg, 0, 32);
	snprintf(arg, 31, "%s:%s", username, password);
	
	fp = fopen(localFile, "rb+");
	if (fp == NULL)
	{
		printf("fopen localfile %s err: %s %s %d\n", localFile, strerror(errno), __FUNCTION__, __LINE__);

		return -1;
	}
	
	if (curl_global_init(CURL_GLOBAL_ALL) != 0)
	{
		printf("curl_global_init fail %s %d\n", __FUNCTION__, __LINE__);
		
		fclose(fp);
		
		return -1;
	}
	
	curl = curl_easy_init();
	if (curl == NULL)
	{
		printf("curl_easy_init fail %s %d\n", __FUNCTION__, __LINE__);
		
		fclose(fp);
		curl_global_cleanup();
		
		return -1;
	}

     /* build a list of commands to pass to libcurl */ 
    headerlist = curl_slist_append(headerlist, acBuf1);
    headerlist = curl_slist_append(headerlist, acBuf2);
    
	/* ����������1��ʾtrue */
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
	curl_easy_setopt(curl, CURLOPT_PORT, port);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_USERPWD, arg);
	curl_easy_setopt(curl, CURLOPT_READDATA, fp);
	curl_easy_setopt(curl, CURLOPT_INFILESIZE, file_len);

	/* ����ftp�ϴ��ļ�Ϊ׷��ģʽ */
	curl_easy_setopt(curl, CURLOPT_FTPAPPEND, 0);
    
    /* ftp�������30s��� */
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, ulFtpTime);

    /* ���ӽ����������10s��� */
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, ulFtpTime);

    //curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
	//curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5L);
    
	 /* pass in that last of FTP commands */ 
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
     
	/*
	 * CURLOPT_VERBOSE:
	 * Set the parameter to 1 to get the library to display a lot of verbose
	 * information about its operations. The default value for this
	 * parameter is 0. 	
	 */
	#ifdef DEBUG_V
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	#else
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	#endif
    
	if (curl_easy_perform(curl) != CURLE_OK)
	{
		printf("curl_easy_perform %s fail %s %d\n", remoteFile, __FUNCTION__, __LINE__);
		
		fclose(fp);
        
        if (headerlist) 
        {
            curl_slist_free_all(headerlist);
        }
        
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		
		return -1;
	}

#if 0
	double speed_upload, total_time; 
    /* now extract transfer info */
    curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
  
    TRACE("\r\nSpeed: %.3f bytes/sec during %.3f seconds %s %d\r\n", speed_upload, total_time, __FUNCTION__, __LINE__);
#endif
    
	fclose(fp);
    if (headerlist) 
    {
        curl_slist_free_all(headerlist);
    }
	curl_easy_cleanup(curl);
	curl_global_cleanup();    
    
    iFileSize = (int)CurlFtpGetFileLenth(remoteFile, dir, ipaddr, port, username, password);
    /* У���ļ� */
    if (file_len != iFileSize)
    {
        printf("file %s file_len %d iFileSize %d %s %d\r\n", remoteFile, file_len, iFileSize, __FUNCTION__, __LINE__);
        return -1;
    }

	return 0;
}

