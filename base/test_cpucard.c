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
#include <semaphore.h>

#define  BUFFER_LEN   1024
#define  CPU_CARD_DEVICE    "/dev/ttyS1"

/* 圆志科信读头的命令 */ 
#define FRAME_HEAD 0x02 
#define FRAME_END 0x03 
#define SET_SERIAL_BAUD_RATE 0x15 
#define SEARCH_CARD 0x46 
#define ESCAPE_CONFLICT 0x47 
#define CHOOSE_CARD 0x48 
#define CHECK_SECRET_KEY 0x4A 
#define READ_BLOCK 0x4B 
#define WRITE_BLOCK 0x4C 

/* 圆志科信读头CPU卡命令 */ 
#define SET_TYPE_A_MODE 0x3A /* 设置读卡模块工作于ISO14443 TYPE A工作模式 */ 
#define RESET_CPU_CARD 0x53 /* 复位卡指令 */ 
#define CPU_CARD_COS 0x54 /* 发送COS指令 */ 

#define COS_EXTERNAL_AUTH 0X82 /* 保密模块验证 */ 
#define COS_GET_CHALLENGE 0x84 /* 获取随机数 */ 
#define COS_WRITE_KEY 0xF0 /* 创建、修改或重新激活密钥 */ 
#define COS_CREATE_FILE 0xE0 /* 华虹专有COS指令，创建文件 */ 
#define COS_SELECT_FILE 0xA4 /* COS指令，选择文件 */ 
#define COS_UPDATE_FILE 0xD6 /* COS指令，写或修改文件 */ 
#define COS_READ_BINARY 0xB0 /* COS指令，读文件 */ 
#define COS_ERASE_DF 0x0E /* 删除DF/MF */ 


#define uint8   unsigned char


int speed_value[] = 
{ 
	230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600, 300 
};
   
int speed_param[] = 
{ 
	B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300 
}; 

#define TTY_SPEED   B19200 


 static int PortInit(int fd)
 {
   struct termios tty;

   if(tcgetattr(fd,&tty))
	   return -1;

   if(cfsetospeed(&tty,(speed_t)TTY_SPEED))
	   return -1;
   if(cfsetispeed(&tty,(speed_t)TTY_SPEED))
	   return -1;

   tty.c_iflag = IGNBRK;
   //tty.c_iflag &= ~ (IXON | IXOFF | IXANY);

   tty.c_lflag = 0;
   tty.c_oflag = 0;
   tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
   tty.c_cflag |= CLOCAL | CREAD;
   tty.c_cflag |= CLOCAL | CREAD;
   tty.c_cflag &= ~CRTSCTS;


   tty.c_cc[VMIN] = 4;
   tty.c_cc[VTIME] = 0;//5

   if(tcsetattr(fd,TCSANOW,&tty))
	   return -1;

   return 0;
 }


void set_serial_param(int fd, int speed, int databits, int stopbits, int parity, int opostflag) 
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
		options.c_cflag &= ~PARENB; /* Clear parity enable */ 
		options.c_iflag &= ~INPCK; /* CLEAR parity checking */ 
		break; 
		case 'o': 
		case 'O': 
		options.c_cflag |= (PARODD | PARENB); 
		options.c_iflag |= INPCK; /* ENABLE parity checking */ 
		break; 
		case 'e': 
		case 'E': 
		options.c_cflag |= PARENB; /* Enable parity */ 
		options.c_cflag &= ~PARODD; 
		options.c_iflag |= INPCK; /* ENABLE parity checking */ 
		break; 
		case 'S': 
		case 's': 
		/*as no parity*/ 
		options.c_cflag &= ~PARENB; 
		options.c_cflag &= ~CSTOPB; 
		options.c_iflag &= ~INPCK; /* Disnable parity checking */ 
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
	options.c_cc[VTIME] = 1; /* 0: 15 seconds */ 
	options.c_cc[VMIN] = 1; /* 1 */ 

	tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */ 
	tcsetattr(fd, TCSANOW, &options); 
}




void print_hex(const char *tip, unsigned char *buff, short bytes, const char *pFuntion, int iLineNum) 
{ 
	unsigned char ucindex = 0; 

	printf("%s: \r\n", tip); 

	for(ucindex = 0; ucindex < bytes; ucindex++) 
	{ 
	printf("%02X ", buff[ucindex]); 
	} 

	if (pFuntion != NULL && iLineNum != 0) 
	{ 
		printf("%s %d\r\n", pFuntion, iLineNum); 
	} 
}

int CpuCardSetCmd(int hCard, unsigned char * Cmd, int DataLen, void* pArg)
{
	unsigned char buf[512];
	unsigned char send_buf[512];
	int j = 0;
	int i = 0;
	int ret = -1;
	int index = 0;
	
	memset(buf, 0, sizeof(buf));
	memset(send_buf, 0, sizeof(send_buf));
	
	/* 帧头 */
	buf[index++] = FRAME_HEAD;		/* 帧头 0x02 */

	/* 模块地址 */
	buf[index++] = 0x00;
	buf[index++] = 0x00;

	/* 长度字 */
	buf[index++] = (char)DataLen;

	/* 命令 + 数据 */
	for (j = 0; j < (DataLen - 2); j++)
	{
		buf[index++] = Cmd[j];
	}
	
	/* 校验字-从模块地址到数据域最后一个字节的逐字节累加 */
	for (j = 1; j < index; j++)
	{
		buf[index] += buf[j];
	}
	
	index++;
	buf[index] = FRAME_END;
	i = 0;
	j = 0;
	send_buf[i++] = buf[j];
	
	/* 如果数据包数据为0x02或0x03或0x10，则增加辨识字符0x10 */
	for (j = 1; j < index; j++)
	{
		if ((buf[j] == 0x02) || (buf[j] == 0x03) || (buf[j] == 0x10))
		{
			send_buf[i++] = 0x10;
			send_buf[i++] = buf[j];
		}
		else
		{
			send_buf[i++] = buf[j];
		}
	}	
	
	send_buf[i++] = buf[j];
	index = i;

	if (send_buf[4] != 0x46)
	{
		print_hex("send buf", send_buf, index, __FUNCTION__, __LINE__);
	}

	ret = write(hCard, send_buf, index);
	if (ret == index)
	{		
		ret = CpuCardRead(hCard, Cmd, pArg);	 /* Cmd[0]为指令命令 */
	}
	else
	{
		ret = -1;
	}

	return ret;
}



int start_cpucard(int hCard)
{
	int  i  = 0;
	int  ret = -1;
	unsigned char cmd[32];
	memset(cmd, 0, sizeof(cmd)); 
	cmd[0] = SET_SERIAL_BAUD_RATE; 
	cmd[1] = 0x03; 

	for(i = 0; i < 3; i++) 
	{ 
		ret = CpuCardSetCmd(hCard, cmd, 4, NULL); 
		if(ret >= 0) 
		{ 
		break; 
		} 
		sleep(1); 
	}
}

/* 读头寻卡 */ 
int CpuSearchCard(int g_hCard) 
{ 
	int iret = -1; 
	int icmdlen = 0; 
	unsigned char cmd[64]; 

	memset(cmd, 0, sizeof(cmd)); 

	cmd[0] = SEARCH_CARD; 
	cmd[1] = 0x52; 

	icmdlen = 4; 

	iret = CpuCardSetCmd(g_hCard, cmd, icmdlen, NULL); 
	return iret; 
}




int CpuCardRead(int hCard, unsigned char *cmd, void* pArg)
{
	unsigned char buf[512];
	unsigned char tmp_buf[512];
	int ret = -1;
	int rtn = -1;
	int j = 0;
	int i = 0;
	int nCardLen = 0;
	
	fd_set readfds;
	struct timeval tv;
    
	unsigned int nCardNo = 0;
	uint8 ucReadLen = 0;      /* 每次读取文件的长度 */
	
	memset(buf, 0, sizeof(buf));
	memset(tmp_buf, 0, sizeof(tmp_buf));

	for (i = 0; i < 100; i++)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 200 * 1000;
		
		FD_ZERO(&readfds);
		FD_SET(hCard, &readfds);

		ret = select(hCard + 1, &readfds, NULL, NULL, &tv);
		if(ret > 0 && FD_ISSET(hCard, &readfds))
		{
			ret = read(hCard, tmp_buf + nCardLen, 128);
			if (ret >= 0)
			{
				nCardLen += ret;
				
				if (((tmp_buf[0] == 0x02 && tmp_buf[nCardLen - 1] == 0x03)|| (tmp_buf[0] == 0x13 && tmp_buf[nCardLen - 1] == 0x03))
						&& (tmp_buf[nCardLen - 2] != 0x10 || (tmp_buf[nCardLen - 2] == 0x10 && tmp_buf[nCardLen - 3] == 0x10)))
				{	
					break;
				}
			}
			else
			{
				continue;
			}
		}
	}
	
	if(tmp_buf[0] == 0x13)
	{
		i = 1;
	}
	else
	{
		i = 0;
	}
	
	if (tmp_buf[i] == FRAME_HEAD && tmp_buf[nCardLen - 1] == FRAME_END)
	{
		j = 0;
		buf[j++] = tmp_buf[i++];
		
		for(; i < nCardLen;)
		{
			/* 去掉0x10 辨识字符 */
			if(tmp_buf[i] == 0x10)
			{
				i++;
				buf[j++] = tmp_buf[i++];
			}
			else
			{
				buf[j++] = tmp_buf[i++];
			}
		}
		
		if (buf[4] != SEARCH_CARD)
		{
			print_hex("read buf", buf, j, __FUNCTION__, __LINE__);
		}
				
		if(buf[4] == cmd[0])                        /* response cmd is same the send cmd */
		{	    
			if(buf[5] == 0x00)                      /* cmd excuted success */
			{	
				if (buf[4] == SEARCH_CARD)          /* M1 card search card cmd response */
				{	
					if ((buf[6] == 0x04) || (buf[6] == 0x02)) /* is M1 card */
					{
						rtn = 1;
						printf(" card is M1 card \n ");
					}
					
					else                                    /* is cpu card */ 
					{
						rtn = 2;
						
						printf(" card is cpu card  \n ");
					}
				}
			}
			else
			{
				rtn = -10;
			}
		}
		else
		{
			printf("Respond Error: Receive cmd 0x%02X, Need cmd 0x%02X\r\n", buf[4], cmd[0]);
			rtn = -11;
		}
	}
	
	return rtn;
}



int main(void)
{
	int fd_cpucard = -1;
	int  ret = 0;
	unsigned char buffer[BUFFER_LEN]={0};

	fd_cpucard = open(CPU_CARD_DEVICE,O_RDWR);
	if( 0 >  fd_cpucard )
	{
		printf(" open %s failed ! \n",CPU_CARD_DEVICE);
		return -1;
	}
	printf("fd_cpucard = %d \n",fd_cpucard);
	//设置串口参数
	set_serial_param(fd_cpucard, 19200, 8, 1, 'n', 0);
	//set_serial_param(fd_cpucard, 115200, 8, 1, 'n', 0);
#if 0
	while(1)
	{
		ret = write(fd_cpucard,"yuemalin",strlen("yuemalin"));
		if( 0 > ret )
		{
			printf(" write fd failed ! \n");
			close(fd_cpucard);
			return -1;
		}
		ret = read(fd_cpucard,buffer,BUFFER_LEN);
		if( 0 > ret )
		{
			printf("read failed !\n");
			close(fd_cpucard);
			return -1;
		}
		printf("i have read buf: %s \n",buffer);
		sleep(1);
	}

#endif	

	//启动CPU card
	start_cpucard(fd_cpucard);

	/* 读头寻卡 */ 
	while(1)
	{
		CpuSearchCard(fd_cpucard);
	}
	//读取CPU card
	while(1)
	{
		memset(buffer,0x00,sizeof(buffer));
		ret = read(fd_cpucard,buffer,sizeof(buffer));
		if( 0 > ret )
		{
			printf(" read %s failed ! \n",CPU_CARD_DEVICE);
			close(fd_cpucard);
			return -1;
		}
		else
		{
			printf(" read card information is: %s \n",buffer);
		}
		sleep(1);
	}

	return 0;
}




