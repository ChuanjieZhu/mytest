/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  文件说明: afg10 3g测试接口
**  创建日期: 2014.02.26
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#ifndef _3G_TEST_H_
#define _3G_TEST_H_

#define TRACE printf
#define Malloc malloc
#define Free free

/* 增加管道通讯相关信息用于3G网络 */
#define FIFO_SERVER "/tmp/ppp.fifo"

#define DNS1    "202.96.134.133"
#define DNS2    "202.96.128.166"
#define GATEWAY "192.168.18.1"
#define PORT    21
#define SERV_IP "112.124.29.254"
#define USER    "user1"
#define PASSWD  "password1"

#define LOCAL_FILE     "/root/ftp_test.tar.gz"
#define REMOTE_FILE    "/root/ftp_test.tar.gz" 

#ifndef BOOL
#define BOOL int
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#define STORAGE_PATH    "/mnt/storage"
#define STORAGE_DEVICE_FLASH   1
/* 2451用户分区 */
#define FLASH_DEV_STORAGE 	   "/dev/mtdblock8"

#ifdef WIN32
#define PACK_ALIGN
#else
#define PACK_ALIGN __attribute__((packed))
#endif

struct my_msg_st
{
    long msg_type;
    char msg[64];
};

typedef struct CONFIG_FTP
{
	char ftpserver[20];			//返回FTP服务器IP地址
	int ftpport;				//返回FTP服务器端口号
	char ftpusr[256];			//返回FTP服务器用户名
	char ftppass[256];			//返回FTP服务器密码
	int hostId;					//返回主机ID
	int schoolId;				//返回学校ID
	char schoolName[256];		//返回学校名字
}PACK_ALIGN CONFIG_FTP, *LPCONFIG_FTP;

#endif //_3G_TEST_H_
