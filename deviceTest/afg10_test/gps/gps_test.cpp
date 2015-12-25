/********************************************************************************
**  Copyright (c) 2010, 深圳市飞瑞斯科技有限公司
**  All rights reserved.
**	
**  文件说明: afg10 gps/北斗 测试代码接口实现。
**  创建日期: 2014.02.28
**
**  当前版本：1.0
**  作者：
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

unsigned char g_recvBuffer[MAX_BUFF_LEN];				        //gps数据接收缓存区
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
** 函数名称： gps_param_init
** 函数功能： gps 串口参数初始化
** 传入参数： param: 串口初始化值
** 传出参数:  
** 返回值： 
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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
** 函数名称： gps_set_param
** 函数功能： gps 串口参数设置
** 传入参数： fd: gps设备描述符，param: 串口初始化值
** 传出参数:  
** 返回值：   0-成功，其它-失败
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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
** 函数名称： gps_check
** 函数功能： gps 设备检测，从设备描述符中读取数据，如果能读到数据，说明真实的
              gps设备存在，否则说明gps设备不存在
** 传入参数： fd: gps设备描述符
** 传出参数:  
** 返回值：   大于0-存在, -1-不存在
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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
** 函数名称： gps_open
** 函数功能： 打开GPS设备，并初始化串口参数
** 传入参数： pDevice: gps设备名
** 传出参数:  
** 返回值：   gps设备文件描述符
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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

    /* 设置串口波特率 */
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
** 函数名称： gps_nmea_parse
** 函数功能： GPS nmea数据解析接口
** 传入参数： 
** 传出参数:  packRMC: nmea解析结果保存，
              posData: 业务需求点信息保存
** 返回值：   
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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

    /* gps数据 */
    if (dataBuffer[2] == NMEA_HEAD_50)
    {
        ret = nmea_parse_gprmc((char *)dataBuffer, dataLen, packRMC);
    }
    else
    {   /* 北斗数据 */
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
** 函数名称： gpsParserThread
** 函数功能： GPS/北斗 nmea数据解析线程
** 传入参数： 
** 传出参数:  
** 返回值：   
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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
** 函数名称： gpsReadThread
** 函数功能： GPS 串口数据读取线程
** 传入参数： 
** 传出参数:  
** 返回值：   
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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
** 函数名称： main
** 函数功能： GPS 测试main函数
** 传入参数： 
** 传出参数:  
** 返回值：   
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
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

