
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

#include "Audio.h"
#include "g711.h"
#include "devLib.h"
#include "2451_test.h"
#include "CvsCore.h"

/* ������һ��3G���� */
BOOL gb3gFirstPpp = FALSE;

int giUsbIs3g = 0;
int giUsb3gStartFlag = 0;
int giMsgId;
pthread_attr_t gtAttr;

/* �Ƿ����U�� */
BOOL gbUsbExit = FALSE;
static int s_nStorageType = STORAGE_DEVICE_FLASH;

time_t gulUsb3gRePppTime = 0;
int giUsb3gRePppFlag = 0;			/* 3G���²��ű�ʶ */

int STORAGE_TYPE()
{
	return s_nStorageType;
}

/******************************************************************************
 * �������ƣ� CheckUsbExist
 * ���ܣ� check usb�豸�Ƿ����
 * ������ pDev: ����豸�� ��ΪNULL
 * ���أ� ����ΪTRUE ������ΪFALSE
 * �������ߣ� Jason
 * �������ڣ� 2012-10-09
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
BOOL CheckUsbExist(char *pDev)
{
	BOOL bRet = FALSE;
	struct stat st;
	char path[256] = {0};
	int i = 0;
	
	for(i = 0; i < 5; i ++)
	{
		sprintf(path, "/sys/block/sd%c", 'a'+i);
		if(stat(path, &st)==0)
		{
			break;
		}
	}

	if(i < 5)
	{
		sprintf(path, "/dev/sd%c", 'a'+i);
		if(stat(path, &st)==0)
		{
			if(pDev)
			{
				strcpy(pDev, path);
			}
			
			bRet = TRUE;
		}
	}

	return bRet;
}

/******************************************************************************
 * �������ƣ� MountDevice
 * ���ܣ� �����豸
 * ������ nType ����: 0 Ϊsd����nand �洢�豸 ����:usb�豸
          pDev: nType��Ϊ0ʱ usb���ص㣬NULL ʱ���ص�/mnt/usb
 * ���أ� 0 ���سɹ� ��0 ���ز��ɹ�
 * �������ߣ� Jason
 * �������ڣ� 2012-10-09
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int MountDevice(int nType, char *pDev)
{
	int ret = 0;
	char buf[256] = {0};

	if(nType == 0) 
	{
		umount(STORAGE_PATH);
		
		strcpy(buf, "SD card");

		if(STORAGE_TYPE()==STORAGE_DEVICE_FLASH)
		{
			ret = mount(FLASH_DEV_STORAGE, STORAGE_PATH, "yaffs", 0, NULL);
		}
	}
	else 
	{
		strcpy(buf, "USB");
		
		umount("/mnt/usb");

		if(pDev && *pDev != '\0')
		{
			struct stat st;
			char path[256] = {0};
			
			for(int i = 0; i < 10; i ++)
			{
				if(i == 0)
				{
					strcpy(path, pDev);
				}
				else
				{
					sprintf(path, "%s%d", pDev, i);
				}

				if(stat(path, &st) == -1)
				{
					TRACE("%s not exist! %s %d\r\n", path, __FUNCTION__, __LINE__);
				}
				else 
				{
					ret = mount(path, "/mnt/usb", "vfat", 0, NULL);
					if(ret == 0) 
					{
						printf("mount %s ok! %s %d\r\n", path,__FUNCTION__, __LINE__);
						
						break;
					}
					else
					{
						printf("mount %s Failed! err: %s. %s %d\r\n", path, strerror(errno), __FUNCTION__, __LINE__);
					}
				}
			}
		}
		else
		{
			ret = mount("/dev/sda", "/mnt/usb", "vfat", 0, NULL);
		}
	}

	if(ret==0)
	{
		printf("Mount %s OK !\r\n", buf);
	}
	else 
	{
		int err = errno;
		
		printf("mount %s Failed: %s(%d)\r\n", buf, strerror(err), err);		
	}
	
	return ret;
}


/* ��ȡIP */
int getIp(char *pIntface, unsigned int *ip)
{
	int sock = 0;
	struct ifreq ifr;

	if((pIntface == NULL) || (*pIntface == '\0'))
	{
		TRACE("set ip: pIntface == NULL %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pIntface, IFNAMSIZ);
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock <= 0)
	{
		TRACE("get ip: sock error, %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
		
		return -1;
	}

	((struct sockaddr_in*)&ifr.ifr_addr)->sin_family = PF_INET;

	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		close(sock);
		
		return -1;
	}
	else
	{
		*ip = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
	}
	
	close(sock);

	return 0;
}

/* ����·�� */
int setRoute(char *pIntface, unsigned int route)
{
	int sock = 0;
	struct ifreq ifr;

	if((pIntface == NULL) || (*pIntface == '\0'))
	{
		TRACE("set route: pIntface == NULL %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	if(route == 0)
	{
		TRACE("set route: route == 0 %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pIntface, IFNAMSIZ);
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock <= 0)
	{
		TRACE("set route: sock error, %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
		
		return -1;
	}

	struct rtentry rt;

	memset(&rt, 0, sizeof(struct rtentry));
	rt.rt_dev = pIntface;

	((struct sockaddr_in*)&rt.rt_dst)->sin_family = PF_INET;
	((struct sockaddr_in*)&rt.rt_dst)->sin_addr.s_addr = 0;
	((struct sockaddr_in*)&rt.rt_genmask)->sin_family = PF_INET;
	((struct sockaddr_in*)&rt.rt_genmask)->sin_addr.s_addr = 0;
	((struct sockaddr_in*)&rt.rt_gateway)->sin_family = PF_INET;
	((struct sockaddr_in*)&rt.rt_gateway)->sin_addr.s_addr = route;
	rt.rt_flags = RTF_GATEWAY | RTF_UP;
	
	if(ioctl(sock, SIOCADDRT, &rt) < 0)
	{
		TRACE("setRoute(%d.%d.%d.%d):%s %s %d\r\n",
				(route)&0xff, (route>>8)&0xff, (route>>16)&0xff, (route>>24)&0xff, strerror(errno), __FUNCTION__, __LINE__);

		close(sock);
		
		return -1;
	}
	else
	{
		TRACE("setRoute(%d.%d.%d.%d): success %s %d\r\n", 
			(route)&0xff, (route>>8)&0xff, (route>>16)&0xff, (route>>24)&0xff, __FUNCTION__, __LINE__);
	}
	
	close(sock);

	return 0;
}

/* ɾ��·�� */
int delRoute(char *pIntface, int gw)
{
	int sock = 0;
	struct rtentry rt;

	if((pIntface == NULL) || (*pIntface == '\0'))
	{
		TRACE("set route: pIntface == NULL %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock <= 0)
	{
		TRACE("set route: sock error, %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
		
		return -1;
	}

	memset(&rt, 0, sizeof(struct rtentry));
	rt.rt_dev = pIntface;
	((struct sockaddr_in*)&rt.rt_dst)->sin_family = PF_INET;
	((struct sockaddr_in*)&rt.rt_dst)->sin_addr.s_addr = 0;
	((struct sockaddr_in*)&rt.rt_genmask)->sin_family = PF_INET;
	((struct sockaddr_in*)&rt.rt_genmask)->sin_addr.s_addr = 0;
	((struct sockaddr_in*)&rt.rt_gateway)->sin_family = PF_INET;
	((struct sockaddr_in*)&rt.rt_gateway)->sin_addr.s_addr = gw;
	rt.rt_flags = RTF_GATEWAY | RTF_UP;

	if (ioctl(sock, SIOCDELRT, &rt) < 0)
	{
		TRACE("del route error %s %d\r\n", __FUNCTION__, __LINE__);

		close(sock);
		
		return -1;
	}
	
	close(sock);

	return 0;
}

/* ����DNS */
int setDns(unsigned int dns1, unsigned int dns2)
{
	int fd = 0;
	char tmpBuf[256] = {0};
	int iWriteLen = 0;

	if((dns1 == 0) && (dns2 == 0))
	{
		TRACE("set dns error %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	fd = open("/etc/resolv.conf", O_WRONLY|O_CREAT|O_TRUNC, 0777);
	if(fd <= 0)
	{
		TRACE("open resolv.conf error %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}

	if(dns1 != 0)
	{
		sprintf(tmpBuf,"nameserver %d.%d.%d.%d\n", (dns1)&0xff, (dns1>>8)&0xff, (dns1>>16)&0xff, (dns1>>24)&0xff);
		iWriteLen = strlen(tmpBuf);
		
		if(write(fd, tmpBuf, iWriteLen) != iWriteLen)
		{
			TRACE("write dns1 error %s %d\r\n", __FUNCTION__, __LINE__);
		}
	}
	
	if(dns2 != 0)
	{
		sprintf(tmpBuf,"nameserver %d.%d.%d.%d\n", (dns2)&0xff, (dns2>>8)&0xff, (dns2>>16)&0xff, (dns2>>24)&0xff);
		iWriteLen = strlen(tmpBuf);

		if(write(fd, tmpBuf, iWriteLen) != iWriteLen)
		{
			TRACE("write dns2 error %s %d\r\n", __FUNCTION__, __LINE__);
		}
	}

	close(fd);

	return 0;
}

/*************************************
 * ������Ϣ���ܵ�
 * type: 0����ʼ����  1��ֹͣ����
 * ����ֵ�� 0���ɹ�   �����ʾʧ��
 
    MACRO_PPP_START=1
    MACRO_PPP_STOP=2
    MACRO_PPP_RESTART=3
    USB_MODE_SWITCH=4
    SET_DEFAULT_ROUTE_PPPD=5
    USB_MODE_SWITCH1=41

 * ***********************************/
int sendPppFifoMsg(int type)
{
	//�˴���close������
	static int fd = 0;
	int nwrite = 0;
	
	fd = open(FIFO_SERVER, O_WRONLY, 0);
	if(fd <= 0)
	{
		TRACE("open myfifo error %s %d\r\n", __FUNCTION__, __LINE__);
		
		return -1;
	}
	
	type += '0';

	if((nwrite = write(fd, &type, 1)) == -1)
	{
		if(errno == EAGAIN)
		{
			TRACE("The FIFO has not been read yet.Please try later %s %d\r\n", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		TRACE("write msg to the FIFO ok %s %d\r\n", __FUNCTION__, __LINE__);
	}

	if(fd > 0)
	{
		close(fd);
		fd = 0;
	}

	return 0;
}

/**************************************************************\
** �������ƣ� FoundAndLoadUsb
** ���ܣ� 	���ֲ�����usb�豸
** ������ 	��
** ���أ� TRUE-�ɹ���FALSE-ʧ��
** �������ߣ� 
** �������ڣ� 2012-07-18
** �޸����ߣ�	jazhu
** �޸����ڣ�
\**************************************************************/
BOOL FoundAndLoadUsb()
{
	char dev[32 + 1] = {0};
	int i = 0;
	//���ż�����
	static unsigned int diagCount = 0;
	unsigned int ppp_ip = 0; 
    BOOL bDetectUsb;

	/* ��⵽u�� */
	if(CheckUsbExist(dev) == TRUE)
	{
		if(gbUsbExit == FALSE) 
		{
			gbUsbExit = TRUE;
			
			sleep(1);

			for(i = 0; i < 4; i ++)
			{
				if(MountDevice(1, dev) == 0)
				{
					break;
				}
				
				sleep(1);
			}

			if(i < 3) 
			{
				return TRUE;
			}
			else
			{
                giUsbIs3g = 1;

                /* ������ʱ�����ţ��ȴ�һ��ʱ�䣬���Ƿ������سɹ�ѧ�������ļ� */
#if 0
                
                if (giCurrentLinkType != 0) /* �������Ѿ��ɹ��������ݣ�����Ҫ�л�3gģ�� */
                {
    				if (giUsb3gStartFlag == 0)    
    				{
    					sendPppFifoMsg(4);//���͹ܵ���Ϣ����ʼ�л�USB 3Gģ��
    					giUsbIs3g = 1;
    				}

                    delRoute((char *)"eth0", inet_addr(strCfgNetWork.caGateway));
    				sendPppFifoMsg(1);
                }
#endif
			}
		}
	}
    
    /* ������й�����3g�����Ƿ�γ� */
    bDetectUsb = CheckUsbExist(dev);
    if (bDetectUsb == FALSE)
	{
		if((gbUsbExit == TRUE) && (giUsbIs3g == 1))
		{
			//USB�ɲ��뵽�γ�
			setRoute((char *)"eth0", inet_addr((char *)GATEWAY));
			setDns(inet_addr((char *)DNS1), inet_addr((char *)DNS2));

			//����ֹͣ�ź�
			sendPppFifoMsg(2);

			giUsbIs3g = 0;
            giUsb3gStartFlag = 0;
            gbUsbExit = FALSE;
		}

        if (CheckUsbExist(dev) == FALSE)
        {
    		gbUsbExit = FALSE;
        }
	}

    /* ���������ļ�ʧ�ܣ���3Gģ����ڣ��һ�û�н��е�һ��3G���� */
    if (giUsbIs3g == 1 && gb3gFirstPpp == FALSE)
    {        
        /* ���͹ܵ���Ϣ����ʼ�л�USB 3Gģ�� */
        sendPppFifoMsg(4);  
        delRoute((char *)"eth0", inet_addr(GATEWAY));
        sendPppFifoMsg(1);
        gb3gFirstPpp = TRUE;
        giUsb3gRePppFlag = 0;
        diagCount = 1;
    }

#if 0
    /* �����Ѿ������ļ��ɹ���˵���õ��ǹ���, ���ٽ���3g���� */
    if (giDownLoadErrCnt < MAX_DOWNLOAD_FAIL_TIMES
        && giDownLoadSucc == 1
        && giUsbIs3g == 1)
    {
        TRACE("\r\n-------->ethernet network. %s %d \r\n", __FUNCTION__, __LINE__);
        /* ���ﲻ���ڶ�̬�л���3G�����������豸�л���3G */
        giUsbIs3g = 0;
    }
#endif
    
	diagCount++;

    /* ����ȴ���һ�β������֮��,�ڽ��в��� */
	if (giUsbIs3g == 1 && giUsb3gRePppFlag == 1 && (gb3gFirstPpp == TRUE))
	{
		getIp((char *)"ppp0", &ppp_ip);
		
		TRACE("\r\n#################get ip(%d:%d:%d:%d) success %s %d!\r\n", ppp_ip&0xff, 
			(ppp_ip>>8)&0xff, (ppp_ip>>16)&0xff, (ppp_ip>>24)&0xff, __FUNCTION__, __LINE__);

		delRoute((char *)"eth0", inet_addr(GATEWAY));
		sendPppFifoMsg(1);
        giUsb3gRePppFlag = 0;
		diagCount = 1;
	}
    /* ����ȴ���һ�β������֮��,�ڽ��в��� */
	else if((diagCount % 200 == 0) && (giUsbIs3g == 1) && (gb3gFirstPpp == TRUE))
	{
		//ÿ5����һ��
		diagCount = 0;
		
		getIp((char *)"ppp0", &ppp_ip);

		TRACE("\r\n--------------->get ip(%d:%d:%d:%d) success! %s %d\r\n", ppp_ip&0xff, (ppp_ip>>8)&0xff, (ppp_ip>>16)&0xff, (ppp_ip>>24)&0xff, __FUNCTION__, __LINE__);

		if(1)
		{
			if(giUsbIs3g == 1)
			{
				//û������ip
				if(ppp_ip == 0)
				{
					delRoute((char *)"eth0", inet_addr(GATEWAY));
					sendPppFifoMsg(1);
				}
			}
			else
			{
				//�ָ�ԭ�е�·�ɺ�dns
				if(ppp_ip != 0)
				{
					//setRoute((char *)"eth0", inet_addr(strCfgNetWork.caGateway));
					//setDns(inet_addr(strCfgNetWork.caDns), inet_addr(strCfgNetWork.caDns2));

					//sendPppFifoMsg(2);
				
					sendPppFifoMsg(5); 
				}
			}
		}
	}
	
	return FALSE;
}

void ThreadInit()
{
	pthread_attr_init(&gtAttr);
	pthread_attr_setstacksize(&gtAttr, 64 * 1024);
}

int CreateClient()
{
    int sockfd = -1;
	struct sockaddr_in servaddr;
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return -1;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons((uint16_t)21);
	inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		if(sockfd != -1)
		{
			close(sockfd);
			sockfd = -1;
		}
		
		return -1;
	}
	
	return 0;
}

int writen(int fd, const void *vptr, int n)
{       
	int        nleft;
	int        nwritten;
	const char    *ptr;

	ptr = (char*)vptr;
	nleft = n;
	while (nleft > 0) 
	{                             
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {

			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;      
			else
				return(-1);

		}               

		nleft -= nwritten;

		ptr += nwritten;
	}   
	return(n);
}


/******************************************************************************
 * �������ƣ� NetErrorCallback
 * ���ܣ� ������̬����3G״̬�ص�����
 * ������ 
 * ���أ� 
 * �������ߣ� Jason
 * �������ڣ� 2012-10-12
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
static void NetErrorCallback(void)
{  
	time_t curTime = time(NULL);
	if (curTime - gulUsb3gRePppTime > 60)
	{
		giUsb3gRePppFlag = 1;
		gulUsb3gRePppTime = curTime;
		TRACE("\r\n---------->desheng 3g reppp. %s %d\r\n", __FUNCTION__, __LINE__);
	}
}

/* CVS-SDK��ʼ�� */
int CvsSDKInit(LPCVS_CONFIG_DS stCvsConfig)
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
	int allowMoreCardsPerSec;   //�����Ƿ�����1s��ͬһ�ſ�����ˢ���

	/*
	CvsInit(stCvsConfig->ftpserver, &(stCvsConfig->ftpport), stCvsConfig->ftpusr, stCvsConfig->ftppass, 
		&(stCvsConfig->hostId), &(stCvsConfig->schoolId), stCvsConfig->schoolName, &(stCvsConfig->resetHour), &(stCvsConfig->resetMin));
		*/
		
	CvsInit(ftpserver, &ftpport, ftpusr, ftppass, 
		&hostId, &schoolId, schoolName, &resetHour, &resetMin, &allowMoreCardsPerSec);

    CvsSetNetErrorFunc(NetErrorCallback);

    TRACE("\r\n---------->allowMoreCardsPerSec %d %s %d\r\n", allowMoreCardsPerSec, __FUNCTION__, __LINE__);
    
    //SetAllowMoreCardsPerSec(allowMoreCardsPerSec);

#if 0
	TRACE("\r\n \r\n \r\n \r\n", __FUNCTION__, __LINE__);
	TRACE("ftpserver: %s ... %s %d\r\n", stCvsConfig->ftpserver,__FUNCTION__, __LINE__);
	TRACE("ftpport: %d ... %s %d\r\n", stCvsConfig->ftpport,__FUNCTION__, __LINE__);
	TRACE("ftpusr: %s ... %s %d\r\n", stCvsConfig->ftpusr,__FUNCTION__, __LINE__);
	TRACE("ftppass: %s ... %s %d\r\n", stCvsConfig->ftppass,__FUNCTION__, __LINE__);
	TRACE("hostId: %d ... %s %d\r\n", stCvsConfig->hostId,__FUNCTION__, __LINE__);
	TRACE("schoolId: %d ... %s %d\r\n", stCvsConfig->schoolId,__FUNCTION__, __LINE__);
	TRACE("schoolName: %s %d ... %s %d\r\n", stCvsConfig->schoolName,strlen(stCvsConfig->schoolName),__FUNCTION__, __LINE__);
	//print_16((unsigned char * )stCvsConfig->schoolName, 256);
	TRACE("resetHour: %d ... %s %d\r\n", stCvsConfig->resetHour,__FUNCTION__, __LINE__);
	TRACE("resetMin: %d ... %s %d\r\n", stCvsConfig->resetMin,__FUNCTION__, __LINE__);
	TRACE("\r\n \r\n \r\n \r\n", __FUNCTION__, __LINE__);
#else
#if 0

    memset(ftpserver, 0, sizeof(ftpserver));
    sprintf(ftpserver, "%d.%d.%d.%d", 192,168,3,160);

    memset(ftpusr, 0, sizeof(ftpusr));
    memcpy(ftpusr, "user", 4);

    memset(ftppass, 0, sizeof(ftppass));
    memcpy(ftppass, "desheng", 7);

    schoolId = 33070001;
#endif

	TRACE("\r\n \r\n \r\n \r\n");
	TRACE("ftpserver: %s ... %s %d\r\n", ftpserver,__FUNCTION__, __LINE__);
	TRACE("ftpport: %d ... %s %d\r\n", ftpport,__FUNCTION__, __LINE__);
	TRACE("ftpusr: %s ... %s %d\r\n", ftpusr,__FUNCTION__, __LINE__);
	TRACE("ftppass: %s ... %s %d\r\n", ftppass,__FUNCTION__, __LINE__);
	TRACE("hostId: %d ... %s %d\r\n", hostId,__FUNCTION__, __LINE__);
	TRACE("schoolId: %d ... %s %d\r\n", schoolId,__FUNCTION__, __LINE__);
	TRACE("schoolName: %s %d ... %s %d\r\n", schoolName,    strlen(schoolName),__FUNCTION__, __LINE__);
	//print_16((unsigned char * )stCvsConfig->schoolName, 256);
	TRACE("resetHour: %d ... %s %d\r\n", resetHour,__FUNCTION__, __LINE__);
	TRACE("resetMin: %d ... %s %d\r\n", resetMin,__FUNCTION__, __LINE__);
	TRACE("\r\n \r\n \r\n \r\n");
#endif
	memcpy(stCvsConfig->ftpserver, ftpserver, sizeof(ftpserver));
	stCvsConfig->ftpport = ftpport;
	memcpy(stCvsConfig->ftpusr, ftpusr, sizeof(ftpusr));
	memcpy(stCvsConfig->ftppass, ftppass, sizeof(ftppass));
	stCvsConfig->hostId = hostId;
	stCvsConfig->schoolId = schoolId;
	memcpy(stCvsConfig->schoolName, schoolName, sizeof(schoolName));
	stCvsConfig->resetHour = resetHour;
	stCvsConfig->resetMin = resetMin;
	
	TRACE("\r\n \r\n \r\n \r\n");
	TRACE("ftpserver: %s ... %s %d\r\n", stCvsConfig->ftpserver,__FUNCTION__, __LINE__);
	TRACE("ftpport: %d ... %s %d\r\n", stCvsConfig->ftpport,__FUNCTION__, __LINE__);
	TRACE("ftpusr: %s ... %s %d\r\n", stCvsConfig->ftpusr,__FUNCTION__, __LINE__);
	TRACE("ftppass: %s ... %s %d\r\n", stCvsConfig->ftppass,__FUNCTION__, __LINE__);
	TRACE("hostId: %d ... %s %d\r\n", stCvsConfig->hostId,__FUNCTION__, __LINE__);
	TRACE("schoolId: %d ... %s %d\r\n", stCvsConfig->schoolId,__FUNCTION__, __LINE__);
	TRACE("schoolName: %s %d ... %s %d\r\n", stCvsConfig->schoolName, strlen(stCvsConfig->schoolName), __FUNCTION__, __LINE__);
	//print_16((unsigned char * )stCvsConfig->schoolName, 256);
	TRACE("resetHour: %d ... %s %d\r\n", stCvsConfig->resetHour,__FUNCTION__, __LINE__);
	TRACE("resetMin: %d ... %s %d\r\n", stCvsConfig->resetMin,__FUNCTION__, __LINE__);
	TRACE("\r\n \r\n \r\n \r\n");
	
	return 0;
}


void *AudioProcessThread(void *pArg)
{   
    CVS_CONFIG_DS g_strCvsConfigDs;
    memset(&g_strCvsConfigDs, 0, sizeof(CVS_CONFIG_DS));
    CvsSDKInit(&g_strCvsConfigDs);

    pthread_detach(pthread_self());

    giMsgId = msgget((key_t)1234, IPC_CREAT | 0666);

    int i = 0;
    my_msg_st myMsg;
    while (1)
    {
        if (giMsgId != -1)
        {
            memset(&myMsg, 0, sizeof(myMsg));
            myMsg.msg_type = 1;
            if (i == 0)
            {
                strcpy(myMsg.msg, (char *)"�߶���������");
                CvsSendData("10000001", "9888888", "�߶���������");
                i = 1;
            }
            else
            {
                strcpy(myMsg.msg, (char *)"�߶���������");
                CvsSendData("10000001", "9888888", "�߶���������");
                i = 0;
            }
            
            msgsnd(giMsgId, (void *)&myMsg, 64, 0);
        }

        usleep(200 * 1000);
    }
}

void ParentProcess()
{
    int running = 1;

    TRACE("giMsgId %d %s %d\r\n", giMsgId, __FUNCTION__, __LINE__);

    pthread_detach(pthread_self());
      
    //pthread_t pNetWorkId = 0;
    //pthread_create(&pNetWorkId, &gtAttr, NetworkThread, NULL);
    pthread_t pAudioId = 0;
    pthread_create(&pAudioId, &gtAttr, AudioProcessThread, NULL);
    
    while (running)
    {
        FoundAndLoadUsb();
        
        usleep(200 * 1000);
    }
}

void ChildProcess()
{
    AudioInit();

    AudioSetVolume(1);

    int running = 1;
    key_t key = 1234;
    my_msg_st msg;
    int msgid;
    
    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1)
    {
        printf("msgget error. %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        _exit(-1);
    }

    TRACE("msgid %d %s %d\r\n", msgid, __FUNCTION__, __LINE__);

    while (running)
    {
        memset(&msg, 0, sizeof(msg));
        if (msgrcv(msgid, (void *)&msg, sizeof(msg), 1, MSG_NOERROR) != -1)
        {
            TRACE("msgrcv %s %s %d\r\n", msg.msg, __FUNCTION__, __LINE__);
            
            xfMakePlayVoice(msg.msg);
        }

        usleep(200 * 1000);
    } 
}

int main()
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        printf("fork error. \r\n");
        _exit(-1);
    }
    else if (pid == 0) 
    {
        /* �ӽ��̲������� */
        ChildProcess();
    }
    else
    {
        ParentProcess();
    }

    return 0;
}

