/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  文件说明: ftp curl lib 接口封装
**  创建日期: 2014.02.26
**
**  当前版本：1.0
**  作者：
********************************************************************************/


#ifndef _MY_CURL_H_
#define _MY_CURL_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int CurlFtpGetFileLenth(const char *file, const char *dir, const char *ipaddr, int port, const char *username, const char *password);
extern int CurlFtpUploadFile(const char *localfilename, const char *file, int file_len, const char *dir, const char *ipaddr, int port, const char *username, const char *password, int iTmpStamp);

#ifdef __cplusplus
}
#endif

#endif //_MY_CURL_H_

