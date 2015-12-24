#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <termios.h>
#include <semaphore.h> //?????藕?????头?募?



#define  S3G_DEVICE   "/dev/ttyUSB3"
#define  GPS_DEVICE  "/dev/ttyUSB1"

#define ATCMD_GSP_START_KEY "^GPSSTART" 
#define ATCMD_GSP_STOP_KEY "^GPSSTOP"
#define ATCMD_PRE	"AT" 	
#define MAX_SERIAL_PACK_SIZE_EX	(2048)

#define ATCMD_SIZE		1024
#define AT_GPS_OFF    	0 
#define AT_GPS_ON 		1
#define BUFFER_LEN  512 


typedef struct _AtCmdSt_
{
    pthread_mutex_t     lock;               // ??,???????????AT?????????
    char                req[ATCMD_SIZE];    // AT????????????
    int                 reqSize;            // ????????????С
    char                rsp[ATCMD_SIZE];    // ??3G?????????????
    struct timeval      recvTimeout;        // ??????
    char                preRecvFlag;        // ?????????????(???? \r\n"OK"\r\n) ?????н?????????
} AT_CMD_ST;

ssize_t writen(int fd, char *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) 
	{
		printf("nleft %d \t ptr %s %s %d\r\n", nleft, ptr, __FUNCTION__, __LINE__);
		
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) 
		{
			if (errno == EINTR)
			{
				printf(" %s %d\r\n", __FUNCTION__, __LINE__);
				
				nwritten = 0; /* and call write() again */
			}
			else
			{
				printf(" %s %d\r\n", __FUNCTION__, __LINE__);
				
				return(-1); /* error */
			}
		}
		if (errno == 5)
			return(-1);
		nleft -= nwritten;
		ptr += nwritten;
	}
	return(n);
}



int FiSerialWriteThreeg(int fd,char *buf, int len)
{
	int ret = -1;

	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE_EX && (-1 !=fd))
	{
		ret = writen( fd, buf, len );
	}

	return ret;
}


int AtCmdSend( int fd,AT_CMD_ST *pAtCmd )
{
	if( NULL == pAtCmd )
	{
		printf( "error:NULL == pAtCmd! %s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	return FiSerialWriteThreeg( fd,pAtCmd->req, pAtCmd->reqSize );
}



int FiAtGpsCtl(int fd, int  flag ) 
{ 
	int ret = -1; 
	AT_CMD_ST g_atCmd ;
	AT_CMD_ST *pCmd; 
	
	pCmd = &g_atCmd ; 
	memset(pCmd,0x00,sizeof(AT_CMD_ST));

	if( AT_GPS_OFF == flag ) 
	{ 
	sprintf( pCmd->req, "%s%s\r", ATCMD_PRE, ATCMD_GSP_STOP_KEY ); 
	} 
	else if( AT_GPS_ON == flag ) 
	{ 
	sprintf( pCmd->req, "%s%s\r", ATCMD_PRE, ATCMD_GSP_START_KEY ); 
	} 

	pCmd->reqSize = strlen( pCmd->req ); 

	ret = AtCmdSend(fd, pCmd ); 

	return ret; 
} 

typedef enum BR{
	SB110 =0,
	SB300, 
	SB600, 
	SB1200, 
	SB2400, 
	SB4800, 
	SB9600, 
	SB19200, 
	SB38400, 
	SB57600, 
	SB115200,
}BR;

typedef enum CS_{
	SCS5 = 0, 
	SCS6, 
	SCS7, 
	SCS8,
}CS_t;

/* 
* func : 3G 模块 ttyUsb专用参数设置接口 
*/ 
int FiSerialSetParamThreeg( int handle, BR baudrate, CS_t databits, int stopbits, int parity ) 
{ 
	int ret = -1; 

	if(handle != -1 ) 
	{ 
	struct termios options; 
	int tiocm; 

	const tcflag_t BAUDRATE[] = { B110, B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200 }; 
	const tcflag_t DATABITS[] = { CS5, CS6, CS7, CS8 }; 
	const tcflag_t STOPBITS[] = { 0, CSTOPB }; 

	tcgetattr(handle,&options); 

	cfsetispeed(&options,BAUDRATE[baudrate]); 
	cfsetospeed(&options,BAUDRATE[baudrate]); 
	options.c_cflag &= ~CSIZE; 
	options.c_cflag |= DATABITS[databits]; 
	options.c_cflag |= STOPBITS[stopbits]; 
	options.c_line=0; 

	options.c_iflag = IGNBRK; 
	options.c_lflag = 0; 
	options.c_oflag = 0; 
	options.c_cflag |= CLOCAL | CREAD; 
	options.c_cc[VMIN] = 1; 
	options.c_cc[VTIME] = 5; 
	options.c_iflag &= ~(IXON|IXOFF|IXANY); 
	options.c_cflag &= ~(PARENB | PARODD); 
	options.c_cflag &= ~CSTOPB; 
	tcsetattr(handle,TCSANOW,&options); 
	ioctl(handle,TIOCMGET,&tiocm); 

	tiocm |= TIOCM_DTR|TIOCM_RTS; 
	tiocm &= ~(TIOCM_DSR|TIOCM_CTS); 

	ioctl(handle,TIOCMSET,&tiocm); 
} 

return ret; 
} 




int main(void)
{
	int fd_3g = 0;
	int fd_gps= 0;
	int ret = 0;
	char buffer[BUFFER_LEN] = {0};

	fd_3g = open(S3G_DEVICE,O_RDWR);
	if( 0 > fd_3g )
	{
		printf(" open %s failed ! \n",S3G_DEVICE);
		return -1;
	}

	fd_gps = open(GPS_DEVICE,O_RDONLY|O_NONBLOCK);
	if( 0 > fd_gps )
	{
		printf(" open %s failed ! \n",GPS_DEVICE);
		close(fd_3g);
		return -1;
	}

	//启动的3G的GPS功能
	FiAtGpsCtl(fd_3g,AT_GPS_ON);

	//设置GPS串口属性
	FiSerialSetParamThreeg(fd_gps,SB115200,SCS8,0,0);

	//读取GPS串口信息
	while(1)
	{
		ret = read(fd_gps,buffer,BUFFER_LEN);
		if( 0 < ret )
		{
			printf("get gps message is : %s \n",buffer);
		}
		sleep(1);
		memset(buffer,0x00,BUFFER_LEN);
	}

	close(fd_3g);
	close(fd_gps);
	return 0;
}






