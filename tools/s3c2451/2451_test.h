#ifndef __2451_TEST_H
#define __2451_TEST_H

#define TRACE printf
#define Malloc malloc
#define Free free

/* ���ӹܵ�ͨѶ�����Ϣ����3G���� */
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
/* 2451�û����� */
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
	char ftpserver[20];			//����FTP������IP��ַ
	int ftpport;				//����FTP�������˿ں�
	char ftpusr[256];			//����FTP�������û���
	char ftppass[256];			//����FTP����������
	int hostId;					//��������ID
	int schoolId;				//����ѧУID
	char schoolName[256];		//����ѧУ����
	int resetHour;				//����ϵͳ����ʱ��(Сʱ)
	int resetMin;				//����ϵͳ����ʱ��(����)
}PACK_ALIGN CVS_CONFIG_DS, *LPCVS_CONFIG_DS;

#endif //__2451_TEST_H