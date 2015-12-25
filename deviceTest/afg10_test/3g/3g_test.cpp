/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  �ļ�˵��: afg10 3gģ����Դ���
**  ��������: 2014.03.02
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <linux/kd.h>
#include <linux/route.h>
#include <sys/types.h>
#include <dirent.h>
#include <termios.h>
#include "3g_test.h"
#include "my_curl.h"

#include <iostream>
#include "log/Logger.h"

using namespace FrameWork;

static int g_3gOnline = 0;
static int g_found3gDevice = 0;
static CONFIG_FTP ftpConfig;

/*******************************************************************************
** �������ƣ� initFtpConfig
** �������ܣ� ��ʼ��ftp���Ӳ���
** ��������� 
** ��������:  
** ����ֵ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
void initFtpConfig()
{
    memset(&ftpConfig, 0, sizeof(ftpConfig));
    snprintf(ftpConfig.ftpserver, sizeof(ftpConfig.ftpserver) - 1, "%s", SERV_IP);
    ftpConfig.ftpport = PORT;
    snprintf(ftpConfig.ftpusr, sizeof(ftpConfig.ftpusr) - 1, "%s", USER);
    snprintf(ftpConfig.ftppass, sizeof(ftpConfig.ftppass) - 1, "%s", PASSWD);
}

/*******************************************************************************
** �������ƣ� ftpUploadFile
** �������ܣ� ftp�ļ��ϴ�����
** ��������� localFileName: �����ļ�����remoteFileName: Զ���ļ�����dirbase: ftp�������ļ����Ŀ¼��
              iTmpStamp: �ش�����
** ��������:  0-�ϴ��ɹ�������-�ϴ�ʧ��
** ����ֵ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
int ftpUploadFile(char *localFileName, char *dirbase, char *remoteFileName, int iTmpStamp)
{
    int iRet;
	struct stat st;
	int iFileLen = 0;
 
	stat(localFileName, &st);
	iFileLen = st.st_size;
	
	iRet = CurlFtpUploadFile(localFileName, remoteFileName, iFileLen, dirbase, ftpConfig.ftpserver, ftpConfig.ftpport, 
		ftpConfig.ftpusr, ftpConfig.ftppass, iTmpStamp);

	return iRet;
}

#if 0
void *threadgThread(void *arg)
{
    static unsigned int diagCount = 0;
    unsigned int ppp_ip = 0;
    
    pthread_detach(pthread_self());
    
    if (0 == g_found3gDevice)
    {
        g_found3gDevice = 1;
    }

    while (1)
    {
        if ((0 == diagCount % 100) && (1 == g_found3gDevice))
        {
            diagCount = 0;
            getIp((char *)"ppp0", &ppp_ip);
            
            TRACE("\r\nget ip(%d:%d:%d:%d) success! %s %d\r\n", 
                (ppp_ip & 0xff), 
                (ppp_ip >> 8) & 0xff, 
                (ppp_ip >> 16) & 0xff, 
                (ppp_ip >> 24) & 0xff, 
                __FUNCTION__, __LINE__);

            if (0 == ppp_ip)
            {
                delRoute((char *)"eth0", inet_addr(GATEWAY));
				sendPppFifoMsg(1);
                g_3gOnline = 0;
            }
            else
            {
                TRACE("\r\n 3g online. %s %d\r\n", __FUNCTION__, __LINE__);
                g_3gOnline = 1;
            }
        }

        usleep(200 * 1000);
    }

    printf("threadgThread exit. \r\n");
    pthread_exit(NULL);
}
#endif

/*******************************************************************************
** �������ƣ� sendDataThread
** �������ܣ� ���ݷ����߳�
** ��������� 
** ��������: 
** ����ֵ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
void *sendDataThread(void *arg)
{
    pthread_detach(pthread_self());
    time_t tStart = 0;
    int count = 0;
    int ret = 0;
    char fnamebuf[128] = {0};

    snprintf(fnamebuf, sizeof(fnamebuf) - 1, "ftp_test_%d.tar.gz", time(NULL));

    while (1)
    {
        ret = ftpUploadFile((char *)LOCAL_FILE, (char *)"school", fnamebuf, count);
        if (0 == ret)
        {
            printf("ftp upload success. %s %d\r\n", __FUNCTION__, __LINE__);
            count = 0;
            sleep(2);
            
            snprintf(fnamebuf, sizeof(fnamebuf) - 1, "ftp_test_%d.tar.gz", time(NULL));
        }
        else
        {
            count++;
            printf("ftp upload fail. %s %d\r\n", __FUNCTION__, __LINE__);
            LOG(LINFO) << count <<  " ftp upload fail.";
            sleep(2);
        }
        
        usleep(200 * 1000);
    }
}

/*******************************************************************************
** �������ƣ� main
** �������ܣ� ���Դ�����main����
** ��������� 
** ��������: 
** ����ֵ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
int main(int argc, char *argv[])
{
    pthread_t threegThreadId = 0;
    pthread_t sendThreadId = 0;

    /* ��ʼ��ftp�ϴ����� */
    initFtpConfig();

    /* ��ʼ����־ģ�� */
    InitLogging("3g_", LINFO, "/root/");

    /* �������������߳� */
    pthread_create(&sendThreadId, NULL, sendDataThread, NULL);
    
    while (1)
    {
        usleep(200 * 1000);
    }

    return 0;
    
}
