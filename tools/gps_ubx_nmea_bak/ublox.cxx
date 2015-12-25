
#include "nmea.h"
#include "ubx.h"
#include "ublox.h"

#include <iostream>
#include "Logger.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace FrameWork;

Postion pos;
unsigned char g_recvBuffer[MAX_BUFF_LEN];
unsigned char *g_pRecvHead = NULL;
unsigned char *g_pRecvTail = NULL;
int g_gpsHandle = -1;
int dataType = 0;
int resResult = 0;
int gCmdResult = 0;
    
int speed_value[] = 
{
    230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600, 300
};

int speed_param[] = 
{
    B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300
};

static void uart_param_init(uart_param *param)
{
	memset(param, '\0', sizeof(uart_param));
	param->speed = 9600;
	param->databits = 8;
	param->stopbits = 1;
	param->parity	= 'n';
	param->opostflag = 0;	
}

int uart_set_param(int fd, uart_param *param)
{
	int i; 
	int speednum = (int)(sizeof(speed_param)/sizeof(int));
	struct termios options;
	if (tcgetattr(fd, &options) != 0)
	{
		printf("get uart param fail. %s %d\r\n", __FUNCTION__, __LINE__);
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

static int uart_check(int fd)
{
    char readchar = -1;
    int ret = -1;
    int count = 0;
    int result = 0;

    tcflush(fd, TCIOFLUSH);

    while ((ret = read(fd, &readchar, 1)) < 0)
	{			
        printf("ret %d, readchar %d %s %d\r\n", ret, readchar, __FUNCTION__, __LINE__);
        
        count++;
		if (count > 20)
		{
			result = -1;
			break;
		}
        
        usleep(100 * 1000);
	}

    printf("ret %d, readchar %d %s %d\r\n", ret, readchar, __FUNCTION__, __LINE__);

    return result;
}

int uart_open(const char *devicePath)
{
    int ret = -1;
    int flags = O_RDWR | O_NOCTTY | O_NONBLOCK;

    if (devicePath == NULL || *devicePath == '\0')
    {
        return -1;
    }
    
    int fd = open(devicePath, flags);
    
    if (fd < 0)
    {
        printf("open gps device fail. %s %d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* 设置串口波特率 */
	uart_param param;
	uart_param_init(&param);
	if (uart_set_param(fd, &param) != 0)
	{
		printf("Set uart param fail. %s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

    if (uart_check(fd) != 0)
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

int nmea_parse(nmeaGPRMC *packRMC)
{
    int ret = -1;
    int crcRes = -1;
    unsigned char dataBuffer[MAX_BUFF_LEN];
    int dataLen = 0;
    
    memset(dataBuffer, 0, sizeof(dataBuffer));
    ret = nmea_parse_packet(dataBuffer, &dataLen);
    if (ret < 0 || dataLen == 0)
    {
        return -1;
    }

    crcRes = -1;
    ret = nmea_checksum((char *)dataBuffer, dataLen, &crcRes);
    if (ret == 0 || crcRes < 0)
    {
        return -1;
    }

    //dataBuffer[dataLen - 2] = '\0';
    //printf("len %d, %s \r\n", dataLen, dataBuffer);         /* dataLen - 74/68 */
    
    ret = nmea_parse_gprmc((char *)dataBuffer, dataLen, packRMC);
    nmea_parse_result(packRMC);

    return 0;
}

int ubx_posllh_cmd(int fd)
{
    int i, ret = -1;
    int status1 = 1;
    int status2 = 1;
    time_t stime = 0;
    
    for (i = 0; i < 5; i++)
    {
        resResult = 0;
        dataType = 1;
        ret = ubx_msg_cfg(fd, status1, status2, UBX_CLASS_NAV, UBX_CLASS_NAV_POSLLH);
        if (ret < 0)
        {
            LOG(INFO) << "posllh command send fail." ;
            continue;
        }

        stime = time(NULL);
        while (resResult != 1)
        {
            if (time(NULL) - stime > 3)
            {
                LOG(INFO) << "receive posllh response fail, timeout. " << i  ;
                break;
            }
        }

        if (resResult == 1)
        {
            break;
        }
    }

    if (i >= 5)
    {
        return -1;
    }

    return 0;
}

int ubx_status_cmd(int fd)
{
    int i, ret = -1;
    int status1 = 1;
    int status2 = 1;
    time_t stime = 0;
    
    for (i = 0; i < 5; i++)
    {
        resResult = 0;
        ret = ubx_msg_cfg(fd, status1, status2, UBX_CLASS_NAV, UBX_CLASS_NAV_STATUS);
        if (ret < 0)
        {
            LOG(INFO) << "status command send fail." ;
            continue;
        }

        stime = time(NULL);
        while (resResult != 1)
        {
            if (time(NULL) - stime > 3)
            {
                LOG(INFO) << "receive status response timeout. " << i  ;
                break;
            }
        }

        if (resResult == 1)
        {
            break;
        }
    }

    if (i >= 5)
    {
        return -1;
    }

    return 0;
}

int ubx_timeutc_cmd(int fd)
{
    int i, ret = -1;
    int status1 = 1;
    int status2 = 1;
    time_t stime = 0;
    
    for (i = 0; i < 5; i++)
    {
        resResult = 0;
        ret = ubx_msg_cfg(fd, status1, status2, UBX_CLASS_NAV, UBX_CLASS_NAV_TIMEUTC);
        if (ret < 0)
        {
            LOG(INFO) << "timeutc command send fail." ;
            continue;
        }

        stime = time(NULL);
        while (resResult != 1)
        {
            if (time(NULL) - stime > 3)
            {
                LOG(INFO) << "receive timeutc response timeout. ";
                break;
            }
        }

        if (resResult == 1)
        {
            break;
        }
    }

    if (i >= 5)
    {
        return -1;
    }

    return 0;
}

int ubx_velned_cmd(int fd)
{
    int i, ret = -1;
    int status1 = 1;
    int status2 = 1;
    time_t stime = 0;
    
    for (i = 0; i < 5; i++)
    {
        resResult = 0;
        ret = ubx_msg_cfg(fd, status1, status2, UBX_CLASS_NAV, UBX_CLASS_NAV_VELNED);
        if (ret < 0)
        {
            LOG(INFO) << "velned command send fail." ;
            continue;
        }

        stime = time(NULL);
        while (resResult != 1)
        {
            if (time(NULL) - stime > 3)
            {
                LOG(INFO) << "receive velned response timeout. ";
                break;
            }
        }

        if (resResult == 1)
        {
            break;
        }
    }

    if (i >= 5)
    {
        return -1;
    }

    return 0;
}

int ubx_ubx_cfg_cmd(int fd)
{
    int ret = -1;
    
    ret = ubx_posllh_cmd(fd);
    if (ret < 0)
    {
        return -1;
    }

    ret = ubx_status_cmd(fd);
    if (ret < 0)
    {
        return -1;
    }

    ret = ubx_timeutc_cmd(fd);
    if (ret < 0)
    {
        return -1;
    }

    ret = ubx_velned_cmd(fd);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

void ubx_nmea_cfg_cmd(int fd)
{
    ubx_msg_cfg(fd, 0, 0, UBX_CLASS_STD, UBX_CLASS_STD_GPGGA);
    ubx_msg_cfg(fd, 0, 0, UBX_CLASS_STD, UBX_CLASS_STD_GPGGA);
    ubx_msg_cfg(fd, 0, 0, UBX_CLASS_STD, UBX_CLASS_STD_GPGLL);
    ubx_msg_cfg(fd, 0, 0, UBX_CLASS_STD, UBX_CLASS_STD_GPGSA);
    ubx_msg_cfg(fd, 0, 0, UBX_CLASS_STD, UBX_CLASS_STD_GPGSV);       
    ubx_msg_cfg(fd, 0, 0, UBX_CLASS_STD, UBX_CLASS_STD_GPVTG);
}

int ubx_prt_cfg_cmd(int fd, int prtType)
{
    int i, ret = -1;
    time_t stime = 0;
    const char *type = (prtType == 1) ? " ubx" : " nmea";
    
    for (i = 0; i < 5; i++)
    {
        resResult = 0;
        ret = ubx_prt_cfg(fd, prtType);
        if (ret < 0)
        {
            LOG(INFO) << "print command send fail." << i << type;
            continue;
        }

        stime = time(NULL);
        while (resResult != 1)
        {
            if (time(NULL) - stime > 3)
            {
                LOG(INFO) << "receive print response timeout. " << i << type;
                break;
            }
        }

        if (resResult == 1)
        {
            break;
        }
    }

    if (i >= 5)
    {
        return -1;
    }

    return 0;
}

int nmea_parse_ps(nmeaPARSER *parser, nmeaGPRMC *pack)
{
    int ret = 0;
    ret = nmea_packet(parser, pack);
    return ret;
}


void ubx_response_result(unsigned char *dataBuffer)
{
    if (dataBuffer[6] == UBX_CLASS_CFG)
    {
        if (dataBuffer[7] == UBX_CLASS_CFG_MSG)
        {
            if (dataBuffer[3] == 0x01)
            {
                LOG(INFO) << "receive cfg message res ok. ";
                resResult = 1;
            }
            else
            {
                LOG(INFO) << "receive cfg message res fail. ";
                resResult = 0;
            }
        }
        else if (dataBuffer[7] == UBX_CLASS_CFG_PRT)
        {
            if (dataBuffer[3] == 0x01)
            {
                LOG(INFO) << "receive cfg print res ok. ";
                resResult = 1;
            }
            else
            {
                LOG(INFO) << "receive cfg print res fail. ";
                resResult = 0;
            }
        }
    }
}

int ubx_response_parse()
{
    int ret = -1;
    unsigned char dataBuffer[MAX_BUFF_LEN];
    int dataLen = 0;
    
    ret = ubx_cmd_res_pack(dataBuffer, &dataLen);

    //printf("ret %d, dataLen %d %s %d\r\n", ret, dataLen, __FUNCTION__, __LINE__);
    
    if (ret < 0 || dataLen != ACK_DATA_LEN)
    {
        return -1;
    }

    print_hex(dataBuffer, dataLen);
    
    ubx_response_result(dataBuffer);

    return 0;
}

int ubx_data_parse(ubxDATA *data)
{
    int ret = -1;
    unsigned char buffer[512];
    int dataLen = 0;
    static unsigned long tt = 0;
    unsigned long tempTime = 0;
    static int err = 0;
    
    ret = ubx_data_pack(buffer, &dataLen);
    if (ret < 0 || dataLen == 0)
    {
        return -1;
    }
    
    //print_hex(buffer, dataLen);
    ubx_parse_pack(buffer, dataLen, data);

    char date[64] = {0};
    if (data->timeValid)
    {
        tempTime = ubx_utc_local(&(data->time), date);
    }
        
    if (tt != tempTime
        && data->latlonValid 
        && data->timeValid 
        && (data->fixtype == 0x02
        || data->fixtype == 0x03))
    {
        printf("%d, %lf, %lf, %s %s %d\r\n", data->fixtype, data->lat, data->lon, date,
            __FUNCTION__, __LINE__);
        tt = tempTime;
        err = 0;
        memset(&data, 0, sizeof(ubxDATA));
    }
    else
    {
        if (data->latlonValid == 0
            || data->timeValid == 0
            || (data->fixtype != 0x02
            && data->fixtype != 0x03))
        {
            if (data->lat < 0 || data->lon < 0)
            {
                data->lat = 0;
                data->lon = 0;
            }
            
            err++;
            if (err > 3)
            {
                printf("%d, %lf, %lf, %s %s %d \r\n", data->fixtype, data->lat, data->lon, date,
                    __FUNCTION__, __LINE__);
                memset(&data, 0, sizeof(ubxDATA));
                err = 0;
            }
        }
    }
    
    return 0;
}

void *nmea_recv_thread(void *arg)
{
    int hGpshandle = -1;
    int ret = -1;
    int readlen = 0;
    int offset = 0;
    	
    fd_set readfds;
    struct timeval tv;
    
    pthread_detach(pthread_self());
    
	hGpshandle = uart_open("/dev/ttyS2");
    g_gpsHandle = hGpshandle;
    
    printf("----------->hGpshandle: %d %s %d\r\n", hGpshandle, __FUNCTION__, __LINE__);

    if (hGpshandle < 0)
	{
        LOG(INFO) << "open gps deice fail." ;
	    printf("open gps device error. %s %d\r\n", __FUNCTION__, __LINE__);	
	}
    else
    {
        LOG(INFO) << "open gps device ok.";
    }
	    
	memset(g_recvBuffer, 0, sizeof(g_recvBuffer));
	g_pRecvHead = g_recvBuffer;
	g_pRecvTail = g_recvBuffer;
	
    while(hGpshandle >= 0)
    {		
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(hGpshandle, &readfds);

		ret = select(hGpshandle + 1, &readfds, NULL, NULL, &tv);
		if (ret > 0 && FD_ISSET(hGpshandle, &readfds))
		{           
			readlen = read(hGpshandle, g_pRecvHead, MAX_BUFF_LEN - offset);
			offset += readlen;
			if (offset >= MAX_BUFF_LEN)
			{
				offset = 0;
			}

            //printf("%s", g_pRecvHead);
            
			g_pRecvHead = g_recvBuffer + offset;
		}

        usleep(100 * 1000);
	}
	
	if(hGpshandle >= 0) 
	{
		close(hGpshandle);
	}
    
    pthread_exit(NULL);
}

void *nmea_parse_thread(void *arg)
{
    unsigned char *pRecvHeadBak = NULL;
    unsigned char *pRecvTailBak = NULL;
    nmeaGPRMC packet;
    ubxDATA data;
    
    pthread_detach(pthread_self());
    
    memset(&data, 0, sizeof(ubxDATA));
    memset(&packet, 0, sizeof(nmeaGPRMC));
    memset(&pos, 0, sizeof(Postion));
    
    while (1)
    {
        if (g_pRecvHead != g_pRecvTail
            && (pRecvHeadBak != g_pRecvHead 
				|| pRecvTailBak != g_pRecvTail))
        {
            pRecvHeadBak = g_pRecvHead;
			pRecvTailBak = g_pRecvTail;
            
            if (dataType == 0)
            {
                nmea_parse(&packet);
            }
            else if (dataType == 1)
            {
                ubx_response_parse();
            }
            else if (dataType == 2)
            {
                ubx_data_parse(&data);
            }
        }
                
        usleep(100 * 1000);
    }

    pthread_exit(NULL);
}

void *nmea_cfg_thread(void *arg)
{
    int fd = g_gpsHandle;
    time_t stime = 0;
    int oldType = 0;
    
    if (fd >= 0)
    {
        dataType = 1;
        tcflush(fd, TCIFLUSH);
        if (ubx_ubx_cfg_cmd(fd) == 0)
        {
            gCmdResult = 1;
        }

        ubx_nmea_cfg_cmd(fd);

        if (gCmdResult == 1)
        {
            tcflush(fd, TCIFLUSH);
            ubx_prt_cfg_cmd(fd, 0x02);
        }
        dataType = 0;
    }

    stime = time(NULL);
    while (1)
    {
        if (fd >= 0 && gCmdResult == 1 && time(NULL) - stime > 60)
        {
            oldType = dataType;
            dataType = 1;
            if (oldType == 0)
            {
                tcflush(fd, TCIFLUSH);
                if (ubx_prt_cfg_cmd(fd, 0x01) == 0)
                {
                    dataType = 2;
                }
                else
                {
                    dataType = oldType;
                }
            }
            else if (oldType == 2)
            {
                tcflush(fd, TCIFLUSH);
                if (ubx_prt_cfg_cmd(fd, 0x02) == 0)
                {
                    dataType = 0;
                }
                else
                {
                    dataType = oldType;
                }
            }

            stime = time(NULL);
        }
        
        usleep(100 * 1000);
    }
}

int main()
{
    pthread_t recvId;
    pthread_t parseId;
    pthread_t cfgId;

    InitLogging("gps-", INFO, "/root/");
    
    pthread_create(&recvId, NULL, &nmea_recv_thread, NULL);
    pthread_create(&parseId, NULL, &nmea_parse_thread, NULL);
    sleep(2);
    pthread_create(&cfgId, NULL, &nmea_cfg_thread, NULL);
    
    
    while (1)
    {
        usleep(200 * 1000);
    }

    return 0;
}

