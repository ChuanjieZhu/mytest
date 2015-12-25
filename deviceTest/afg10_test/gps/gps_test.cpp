/********************************************************************************
**  Copyright (c) 2010, �����з���˹�Ƽ����޹�˾
**  All rights reserved.
**	
**  �ļ�˵��: afg10 gps/���� ���Դ���ӿ�ʵ�֡�
**  ��������: 2014.02.28
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <semaphore.h>
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include "Logger.h"
#include "gps_test.h"

using namespace FrameWork;

static int g_gpsFlag = 0;
static int g_signalFlag = 0;

unsigned char g_recvBuffer[MAX_BUFF_LEN];				        //gps���ݽ��ջ�����
unsigned char *g_pRecvHead = NULL;
unsigned char *g_pRecvTail = NULL;
nmeaPartPacket nmeaPacket;

static int speed_value[] = 
{
    230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600, 300
};

static int speed_param[] = 
{
    B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300
};

/*******************************************************************************
** �������ƣ� gps_param_init
** �������ܣ� gps ���ڲ�����ʼ��
** ��������� param: ���ڳ�ʼ��ֵ
** ��������:  
** ����ֵ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
static void gps_param_init(uart_param *param)
{
	memset(param, '\0', sizeof(uart_param));
	param->speed = 9600;
	param->databits = 8;
	param->stopbits = 1;
	param->parity	= 'n';
	param->opostflag = 0;	
}

/*******************************************************************************
** �������ƣ� gps_set_param
** �������ܣ� gps ���ڲ�������
** ��������� fd: gps�豸��������param: ���ڳ�ʼ��ֵ
** ��������:  
** ����ֵ��   0-�ɹ�������-ʧ��
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
static int gps_set_param(int fd, uart_param *param)
{
    int i; 
	int speednum = (int)(sizeof(speed_param)/sizeof(int));
	struct termios options;
	if (tcgetattr(fd, &options) != 0)
	{
		printf("## Get fd param fail! %s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	for (i = 0; i < speednum; i++)
	{
		if (param->speed == speed_value[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&options, speed_param[i]);
			cfsetospeed(&options, speed_param[i]);
			if (tcsetattr(fd, TCSANOW, &options) != 0)
			{
				return -1;
			}
		} 
	}

	options.c_cflag &= ~CSIZE;
	switch (param->databits)
	{
		case 5:
			options.c_cflag |= CS5;
			break;
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			break;
	}

	switch (param->stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			break;
	}

	switch (param->parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB; /*   Clear   parity   enable   */
			options.c_iflag &= ~INPCK; /*   CLEAR   parity   checking   */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK; /*   ENABLE   parity   checking   */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB; /*   Enable   parity   */
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK; /*   ENABLE   parity   checking   */
			break;
		case 'S':
		case 's':
			/*as   no   parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			options.c_iflag &= ~INPCK; /*   Disnable   parity   checking   */
			break;
		default:
			options.c_iflag &= ~INPCK;
			break;
	}
	options.c_iflag &= ~IXON; /* disable XON/XOFF control */

	options.c_iflag |= IGNBRK | IGNPAR;
	//options.c_iflag |= BRKINT | IGNBRK | IGNPAR;
	options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
	
	options.c_lflag = 0;
	options.c_oflag = 0;

	if(param->opostflag)
	{
		options.c_oflag |= OPOST;
	}
	else 
	{
		options.c_oflag &= ~OPOST;
	}
	
	options.c_oflag &= ~(ONLCR | OCRNL | ONOCR | ONLRET);

	options.c_cflag |= CLOCAL | CREAD;
	options.c_cc[VTIME] = 1; /*  0: 15   seconds   */
	options.c_cc[VMIN] = 1;  /* 1 */

	tcflush(fd, TCIFLUSH); /*   Update   the   options   and   do   it   NOW   */
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		return -1;
	}

	return 0;
}

/*******************************************************************************
** �������ƣ� gps_check
** �������ܣ� gps �豸��⣬���豸�������ж�ȡ���ݣ�����ܶ������ݣ�˵����ʵ��
              gps�豸���ڣ�����˵��gps�豸������
** ��������� fd: gps�豸������
** ��������:  
** ����ֵ��   ����0-����, -1-������
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
static int gps_check(int fd)
{
    char readchar = -1;
    int ret = -1;
    int count = 0;
    int result = 0;

    tcflush(fd, TCIOFLUSH);

    while ((ret = read(fd, &readchar, 1)) < 0)
	{			
        count++;
		if (count > MAX_CHECK_TIMES)
		{
			result = -1;
			break;
		}
        
        usleep(100 * 1000);
	}

    printf("ret %d, readchar %d %s %d\r\n", ret, readchar, __FUNCTION__, __LINE__);

    return result;
}

/*******************************************************************************
** �������ƣ� gps_open
** �������ܣ� ��GPS�豸������ʼ�����ڲ���
** ��������� pDevice: gps�豸��
** ��������:  
** ����ֵ��   gps�豸�ļ�������
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
int gps_open(const char *pDevice)
{
    int fd = -1;
    if (pDevice == NULL)
    {
        return -1;
    }

    int flags = O_RDWR | O_NOCTTY | O_NONBLOCK;    
    fd = open(pDevice, flags);
    
    if (fd < 0)
    {
        printf("open gps device fail. %s %d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* ���ô��ڲ����� */
	uart_param param;
	gps_param_init(&param);
	if (gps_set_param(fd, &param) != 0)
	{
		printf("Set uart param fail. %s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

    if (gps_check(fd) != 0)
    {
        printf("check uart device fail. %s %d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) != 0) 
	{
		printf("Unable to turn UART back to blocking mode: %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
		return -1;
	}

	return fd;
}

/*******************************************************************************
** �������ƣ� gps_nmea_parse
** �������ܣ� GPS nmea���ݽ����ӿ�
** ��������� 
** ��������:  packRMC: nmea����������棬
              posData: ҵ���������Ϣ����
** ����ֵ��   
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
int gps_nmea_parse(nmeaGPRMC *packRMC, gpsPosData *posData)
{
    int ret = -1;
    int crcRes = -1;
    unsigned char dataBuffer[512];
    int dataLen = 0;

    memset(packRMC, 0, sizeof(nmeaGPRMC));
    packRMC->status = 'V';
    
    memset(dataBuffer, 0, sizeof(dataBuffer));
    
    ret = nmea_parse_packet(dataBuffer, &dataLen);
    if (ret < 0 || dataLen == 0)
    {
        return 0;
    }

    crcRes = -1;
    ret = nmea_checksum((char *)dataBuffer, dataLen, &crcRes);
    if (ret == 0 || crcRes < 0)
    {
        return 0;
    }

    dataBuffer[dataLen - 2] = '\0';
    printf("len %d, %s \r\n", dataLen, dataBuffer);         /* dataLen - 74/68 */

    /* gps���� */
    if (dataBuffer[2] == NMEA_HEAD_50)
    {
        ret = nmea_parse_gprmc((char *)dataBuffer, dataLen, packRMC);
    }
    else
    {   /* �������� */
        ret = nmea_parse_gnrmc((char *)dataBuffer, dataLen, packRMC);
    }
    
    if (ret < 0)
    {
        packRMC->status = 'V';
        return 0;
    }

    memset(posData, 0, sizeof(gpsPosData));
    ret = nmea_parse_result(packRMC, &(posData->deglat), &(posData->deglon), 
        &(posData->speed), &(posData->ultime));
   
    return ret;
}

/*******************************************************************************
** �������ƣ� gpsParserThread
** �������ܣ� GPS/���� nmea���ݽ����߳�
** ��������� 
** ��������:  
** ����ֵ��   
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
void *gpsParserThread(void *arg)
{
    unsigned char *pRecvHeadBak = NULL;
	unsigned char *pRecvTailBak = NULL;

    nmeaGPRMC nmeaDataPacket;

    int ret = 0;
    gpsPosData posData;
    int iNmeaCount = 0;
    
    pthread_detach(pthread_self());

    memset(&nmeaDataPacket, 0, sizeof(nmeaGPRMC));
    nmeaDataPacket.status = 'V';
    memset(&nmeaPacket, 0, sizeof(nmeaPartPacket));

    while (1)
    {
        if (g_pRecvHead != g_pRecvTail
			&& (pRecvHeadBak != g_pRecvHead 
				|| pRecvTailBak != g_pRecvTail))
		{
			pRecvHeadBak = g_pRecvHead;
			pRecvTailBak = g_pRecvTail;
            
            ret = gps_nmea_parse(&nmeaDataPacket, &posData);
            if (ret == 0 && nmeaDataPacket.status == 'A')
            {
                iNmeaCount = 0;
                g_signalFlag = 1;
            }
            else if (ret == -1 && nmeaDataPacket.status == 'V')
            {
                iNmeaCount = 0;
                g_signalFlag = 0;
            }
            else
            {
                iNmeaCount++;
                if (iNmeaCount > MAX_ERROR_COUNT)
                {
                    g_signalFlag = 0;
                }
            }
		}

        usleep(100 * 1000);
    }

    pthread_exit(NULL);
}

/*******************************************************************************
** �������ƣ� gpsReadThread
** �������ܣ� GPS �������ݶ�ȡ�߳�
** ��������� 
** ��������:  
** ����ֵ��   
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
void *gpsReadThread(void *arg)
{
    int fd = -1;
    int ret = -1;
    int readlen = 0;
    int offset = 0;

    fd_set readfd;
    struct timeval tv;

    pthread_detach(pthread_self());
    
    fd = gps_open((const char *)GPS_DEVICE);
    if (fd < 0)
    {
        printf("open gps device fail. \r\n");
        g_gpsFlag = 0;
        g_signalFlag = 0;
    }

    memset(g_recvBuffer, 0, sizeof(g_recvBuffer));
	g_pRecvHead = g_recvBuffer;
	g_pRecvTail = g_recvBuffer;

    while (fd >= 0)
    {
        tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&readfd);
		FD_SET(fd, &readfd);

        ret = select(fd + 1, &readfd, NULL, NULL, &tv);
		if (ret > 0 && FD_ISSET(fd, &readfd))
		{
            g_gpsFlag = 1;
           
			readlen = read(fd, g_pRecvHead, MAX_BUFF_LEN - offset);
			offset += readlen;
			if (offset >= MAX_BUFF_LEN)
			{
				offset = 0;
			}

            //printf("\r\n%s \r\n", g_pRecvHead);
            
			g_pRecvHead = g_recvBuffer + offset;
		}

        usleep(100 * 1000);
    }

    if(fd >= 0) 
	{
		close(fd);
	}

    g_gpsFlag = 0;
	g_signalFlag = 0;
	memset(g_recvBuffer, 0, sizeof(g_recvBuffer));	
    
    pthread_exit(NULL);
}

/*******************************************************************************
** �������ƣ� main
** �������ܣ� GPS ����main����
** ��������� 
** ��������:  
** ����ֵ��   
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
int main(int argc, char **argv)
{
    pthread_t recvId;
    pthread_t parseId;

    InitLogging("gps_", INFO, "/root/");
    
    pthread_create(&recvId, NULL, &gpsReadThread, NULL);
    sleep(2);
    pthread_create(&parseId, NULL, &gpsParserThread, NULL);  
    
    while (1)
    {
        usleep(200 * 1000);
    }

    return 0;
}

