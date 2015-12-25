/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  文件说明: afg10 ic cpu卡读写测试
**  创建日期: 2014.02.26
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ic_test.h"
#include "cpu.h"

static int speed_value[] = 
{
    230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600, 300
};

static int speed_param[] = 
{
    B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300
};

static int ic_open(const char *pName)
{
    if (pName == NULL)
    {
        return -1;
    }

    int fd = -1;
    fd = open(pName, O_RDWR);
    
    if (fd < 0)
    {
        perror("ic_open ");
        return -1;
    }

    return fd;
}

static void ic_set_serial(int fd, int speed, int databits, int stopbits, int parity, int opostflag)
{
    int i, speednum = (int)(sizeof(speed_param)/sizeof(int));
	struct termios options;
	tcgetattr(fd, &options);
	for (i = 0; i < speednum; i++)
	{
		if (speed == speed_value[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&options, speed_param[i]);
			cfsetospeed(&options, speed_param[i]);
			tcsetattr(fd, TCSANOW, &options);
		} 
	}

	options.c_cflag &= ~CSIZE;
	switch (databits)
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

	switch (stopbits)
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

	switch (parity)
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

	if(opostflag) options.c_oflag |= OPOST;
	else options.c_oflag &= ~OPOST;
	options.c_oflag &= ~(ONLCR | OCRNL | ONOCR | ONLRET);

	options.c_cflag |= CLOCAL | CREAD;
	options.c_cc[VTIME] = 1; /*  0: 15   seconds   */
	options.c_cc[VMIN] = 1;  /* 1 */

	tcflush(fd, TCIFLUSH); /*   Update   the   options   and   do   it   NOW   */
	tcsetattr(fd, TCSANOW, &options);
}

int ic_init(const char *pName)
{
    int fd = -1;
    int i, ret;
    unsigned char buffer[32] = {0};
    
    fd = ic_open(pName);
    if (fd < 0)
    {
        printf("ic_open fail. %s %d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ic_set_serial(fd, 19200, 8, 1, 'n', 0);

    memset(buffer, 0, sizeof(buffer));
    buffer[0] = SET_SERIAL_BAUD_RATE;
    buffer[1] = 0x03;

    for (i = 0; i < 3; i++)
    {
        ret = CpuCardSetCmd(fd, buffer, 4);
        if (ret >= 0)
        {
            break;
        }
    }

    usleep(20 * 1000);

    if (ret < 0)
    {
        printf("ic_init set serial fail. %s %d\r\n", __FUNCTION__, __LINE__);
        close(fd);
        return -1;
    }

    return fd;
}

int ic_read(int fd)
{
    int ret = -1;
    ret = CosReadFile(fd);
    return ret;
}

int ic_write(int fd)
{
    int ret = -1;
    ret = CosWriteFile(fd);
    return ret;
}


void *ic_thread(void *arg)
{
    unsigned char cmd[32];
    int fd = -1;
    int ret = -1;
     
    if (pthread_detach(pthread_self()) != 0)
    {
        printf("pthread_detach fail. %s %d\r\n", __FUNCTION__, __LINE__);
    }
    
    fd = ic_init((const char *)IC_DEVICE_NAME);

    while (fd >= 0)
    {
       /* M1卡寻卡 */
		memset(cmd, 0, sizeof(cmd));
		cmd[0] = SEARCH_CARD;
		cmd[1] = 0x52;
		
		ret = CpuCardSetCmd(fd, &cmd[0], 4);

        if (ret >= 0)
        {
            /* M1卡防冲突 */
			memset(cmd, 0, sizeof(cmd));
			cmd[0] = ESCAPE_CONFLICT;
			cmd[1] = 0x04;
			
			ret = CpuCardSetCmd(fd, &cmd[0], 4);
        }
        
        usleep(100 * 1000);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t pIcThreadId = 0;
    pthread_create(&pIcThreadId, NULL, ic_thread, NULL);

    while (1)
    {
        usleep(100 * 1000);
    }

    return 0;
}

