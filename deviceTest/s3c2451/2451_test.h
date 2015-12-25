#ifndef __2451_TEST_H
#define __2451_TEST_H

#define TRACE printf
#define Malloc malloc
#define Free free

/* 增加管道通讯相关信息用于3G网络 */
#define FIFO_SERVER "/tmp/ppp.fifo"

#define DNS1    "202.96.134.133"
#define DNS2    "202.96.128.166"
#define GATEWAY "192.168.18.1"

#define PORT    25
#define SERV_IP "192.168.3.139"

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

typedef struct CVS_CONFIG_DS
{
	char ftpserver[20];			//返回FTP服务器IP地址
	int ftpport;				//返回FTP服务器端口号
	char ftpusr[256];			//返回FTP服务器用户名
	char ftppass[256];			//返回FTP服务器密码
	int hostId;					//返回主机ID
	int schoolId;				//返回学校ID
	char schoolName[256];		//返回学校名字
	int resetHour;				//返回系统重启时间(小时)
	int resetMin;				//返回系统重启时间(分钟)
}PACK_ALIGN CVS_CONFIG_DS, *LPCVS_CONFIG_DS;

#endif //__2451_TEST_H