#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/keyboard.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <linux/watchdog.h>
#include <termios.h>
#include <errno.h>

/* 和嘉485请求包 */
typedef struct HeJia_Request
{
	unsigned char head[2];
	unsigned char len[2];
	unsigned char deviceType;
	unsigned char mainCmd;
	unsigned char subCmd;
}HeJia_Request, *LPHeJia_Request;

/* 和嘉485应答包 */
typedef struct HeJia_Response
{
	unsigned char head[2];
	unsigned char len[2];
	unsigned char deviceType;
	unsigned char mainCmd;
	unsigned char subCmd;
}HeJia_Response, *LPHeJia_Response;

#define MAX_ID_LEN 			8
#define MAX_NAME_LEN		32
/* 最大学员人脸特征文件大小为(11 * 1024)字节 */
#define MAX_FACE_BUFFER_LEN	(11 * 1024)
#define MAX_PHOTO_BUFFER_LEN	(20 * 1024)
/* 和嘉485命令字 */
/* 协议请求数据头 */
#define HEJIA_REQ_START_DATA0  		0x23
#define HEJIA_REQ_START_DATA1  		0x23
/* 协议应答数据头 */
#define HEJIA_RES_START_DATA0  		0x26
#define HEJIA_RES_START_DATA1  		0x26
/* 协议结束符 */
#define HEJIA_END_DATA0  			0x0d
#define HEJIA_END_DATA1  			0x0a

/* 外设类型，固定为0x0b */
#define HEJIA_DEVICE_TYPE  			0x0b

/* 主命令字，固定为0x40 */
#define HEJIA_MAIN_CMD  			0x40

/* 从命令字，握手信息 */
#define HEJIA_SUB_CMD_AUTH  		0x01
/* 从命令字，比对溢出时间设定 */
#define HEJIA_SUB_CMD_OVER_TIME  	0x02
/* 从命令字，下发特征文件 */
#define HEJIA_SUB_CMD_SEND_FEATURE  0x03
/* 从命令字，开始比对 */
#define HEJIA_SUB_CMD_START_VERIFY  0x04
/* 从命令字，查询比对结果 */
#define HEJIA_SUB_CMD_GET_RESULT  	0x05
/* 从命令字，读取识别图片 */
#define HEJIA_SUB_CMD_GET_PIC  		0x06
/* 从命令字 保存登记照片 */   
#define HEJIA_SAVE_REG_GET_PIC  	0x07
/* 从命令字 1:N识别 */   
#define SUB_CMD_ONE_TO_ONE_VERIFY  	0x08
/* 从命令字 获取用户列表 */   
#define SUB_CMD_GET_USER_LIST		0x09
/* 从命令字 获取用户特征 */
#define SUB_GET_USER_FEATURE		0x0A
/* 从命令字 获取用户登记照 */
#define SUB_GET_USER_REG_PHOTO		0x0B

/* 定义从485接口接收最大数据长度 */
#define MAX_RECV_BUF_SIZE 			1024

/* 定义文件名全名的长度 */
#define MAX_FILE_PATH_LEN			512

/* 定义一个包的大最长度*/
#define MAX_PACKET_SIZE				128	

/* 定义一张图片最大长度 */			
#define MAX_PIC_SIZE				6*1024

int speed_value[] = 
{
    230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600, 300
};

int speed_param[] = 
{
    B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300
};

/* 学员人脸特征文件内容 */
char g_faceBuf[MAX_FACE_BUFFER_LEN] = {0};	
char g_RegPhotoBuf[MAX_PHOTO_BUFFER_LEN] = {0};	
unsigned int g_faceBufOffset = 0;

char g_facePicName[MAX_PACKET_SIZE] = {0};
char g_facePicBuf[MAX_PIC_SIZE] = {0};

/* 握手是否通过  1 为通过 0为失败*/
char g_AuthFlag = 0;

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

unsigned short getXorResult(unsigned char * buf, unsigned short len)
{
	unsigned char tmpXor = 0;
	unsigned short ret;
	
	if (buf == NULL || len <= 0)
	{
		return 0;
	}
	
	while (len != 0)
	{		
		tmpXor = tmpXor ^ (*buf);
		buf++;
		len--;
	}

	ret = (tmpXor & 0xFF);
	
	return ret;
}

#if 0
{
	unsigned short tmpXor = 0;

	printf("len %d %s %d\r\n", len, __FUNCTION__, __LINE__);
	
	if (buf == NULL || len <= 0)
	{
		return 0;
	}
	
	while (len != 0)
	{		
		tmpXor = tmpXor ^ ((unsigned short)*buf);
		buf++;
		len--;
	}

	printf("tmpXor 0x%x %s %d\r\n", tmpXor, __FUNCTION__, __LINE__);
	
	return tmpXor;
}
#endif

//返回实际读取的数据长度，-1出错
int ReadFeature(char* pBuf, int nBufLen, char* pFileName)
{
	int handle = -1;
	int nFileLen = 0;
	int nReadLen = 0;
	struct stat st;
	
	if (pBuf == NULL || pFileName == NULL)
	{
		return -1;
	}

	handle = open(pFileName, O_RDONLY);
	if (handle < 0)
	{
		printf("open %s error\n", pFileName);
		
		return -1;
	}

	if ( stat( pFileName, &st) == 0 )
	{
		nFileLen = st.st_size;
	}

	if(nFileLen > nBufLen)
	{
		nFileLen = nBufLen;
		
		printf("nFileLen > nBufLen\n");
	}

	nReadLen = read(handle, pBuf, nFileLen);

	close(handle);

	return nReadLen;
}

int WriteData(int fd, unsigned char *pWriteBuf, int nWriteSize)
{
	int writelen = 0;
	int nleft = 0;

	if (fd < 0)
	{
		return -1;
	}
	
	unsigned char *ptr = NULL;

	ptr = pWriteBuf;
	nleft = nWriteSize;

	while(nleft > 0)
	{
		if((writelen = write(fd, ptr, nleft)) <= 0)
		{
			if (writelen < 0 && errno == EINTR)
			{
				writelen = 0;
			}
			else
			{
				return -1;
			}
		}

		nleft -= writelen;
		ptr += writelen;
	}
	
	return nWriteSize;
}

int CheckPackContent(unsigned char * pPack, int len, int * validLen)
{
	int i = 0;
	int OffSet = -1;
	unsigned char lenLow;
	unsigned char lenHigh;
	unsigned short dataLen;
		
	for (i = 0; i < len; i++)
	{
		if ((pPack[i] == HEJIA_RES_START_DATA0)
			&& (pPack[i + 1] == HEJIA_RES_START_DATA0))
		{
			OffSet = i;
			
			lenLow = pPack[OffSet + 2];
			lenHigh = pPack[OffSet + 3];

			dataLen = ((unsigned short)lenHigh << 8) + lenLow;
			
			if (dataLen < (MAX_PACKET_SIZE + 32)
				&& (pPack[OffSet + 4] == HEJIA_DEVICE_TYPE)
				&& (pPack[OffSet + dataLen - 2] == HEJIA_END_DATA0)
				&& (pPack[OffSet + dataLen - 1] == HEJIA_END_DATA1))
			{
				* validLen = dataLen;
				
				return OffSet;
			}			
		}
	}

	return -1;	
}

/************************************
函数名称：ReadPacket
函数功能: 读一个包的数据
入口参数：
		Handle			: RS485设备句柄
		pValidBuf		: 存放读取的数据
		pValidLen		: 有效长度
返回值  ：
		数据在pValidBuf中的偏移
************************************/
int ReadPacket(int Handle, unsigned char * pValidBuf, int * pValidLen)
{
	int ret= 0;
	int readBufLen = 0;
	int OffSet = -1;
	int validLen = 0;
	
	for (;;)
	{
		ret = read(Handle, (pValidBuf + readBufLen), MAX_RECV_BUF_SIZE);
		if(ret >= 0)
		{
#if 0
			for (i = 0; i < readBufLen + ret; i++)
			{
				if (i % 8 == 0)
				{
					printf("\r\n");
				}
				
				printf("0x%02x \t", pValidBuf[i]);
			}
			printf("ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
#endif
			readBufLen += ret;

			printf("%s %d\r\n", __FUNCTION__, __LINE__);
			
			OffSet = CheckPackContent(pValidBuf, readBufLen, &validLen);

			printf("%s %d\r\n", __FUNCTION__, __LINE__);
			
			if (OffSet != -1)
			{
				*pValidLen = validLen;
				
				break;
			}
		}
		else
		{
			printf("errno %d %s %d\r\n", errno, __FUNCTION__, __LINE__);

			break;
		}
	}

	return OffSet;
}

int dealCmd(int handle, char cmd)
{
	int ret = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int i;
	int iOffset = 0;
	int iValidLen = 0;
	
	switch(cmd)
	{
		/* 握手 */
		case '1':
		{
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			memset(sendBuf, 0, sizeof(sendBuf));
			
			sendBuf[0] = HEJIA_REQ_START_DATA0;
			sendBuf[1] = HEJIA_REQ_START_DATA1;
			sendBuf[2] = 0x0b;
			sendBuf[3] = 0x00;
			sendBuf[4] = HEJIA_DEVICE_TYPE;
			sendBuf[5] = HEJIA_MAIN_CMD;
			sendBuf[6] = HEJIA_SUB_CMD_AUTH;

			sendXor = getXorResult(sendBuf, 7);
			printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
			
			sendBuf[7] = sendXor & 0xFF;
			sendBuf[8] = (sendXor >> 8) & 0xFF;

			sendBuf[9] = HEJIA_END_DATA0;
			sendBuf[10] = HEJIA_END_DATA1;
			
			write(handle, sendBuf, 11);

#if 1
			printf("---------------------------SendBuf----------------------\r\n");
			for(i = 0; i < sendBuf[2]; i++)
			{
				if(i % 6 == 0)
				{
					printf("\r\n");
				}
				printf("buf[%d] 0x%02X\t", i, sendBuf[i]);
			}
			printf("\r\n");			
#endif

			sleep(1);

			FD_ZERO(&readfds);
			FD_SET(handle, &readfds);
			
			tv.tv_sec = 5;
			tv.tv_usec = 0;

			memset(recvBuf, 0, sizeof(recvBuf));
			readBufLen = 0;
			
			ret = select(handle + 1, &readfds, NULL, NULL, &tv);
			
			if (ret > 0 && FD_ISSET(handle, &readfds)) 
			{
				iOffset = ReadPacket(handle, validBuf, &iValidLen);

				memcpy(recvBuf, validBuf + iOffset, iValidLen);
				readBufLen = iValidLen;
					
				printf("---------------------------RecvBuf----------------------\r\n");
				printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
				for (i = 0; i < readBufLen; i++)
				{
					if (i % 6 == 0)
					{
						printf("\r\n");
					}
					
					printf("buf[%d] 0x%02X\t", i, recvBuf[i]);
				}
				printf("\r\n");
			}

			printf("%s %d\r\n", __FUNCTION__, __LINE__);
			
			break;
		}
		/* 比对溢出时间设定 */
		case '2':
		{
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			memset(sendBuf, 0, sizeof(sendBuf));
			
			sendBuf[0] = HEJIA_REQ_START_DATA0;
			sendBuf[1] = HEJIA_REQ_START_DATA1;
			sendBuf[2] = 0x0c;
			sendBuf[3] = 0x00;
			sendBuf[4] = HEJIA_DEVICE_TYPE;
			sendBuf[5] = HEJIA_MAIN_CMD;
			sendBuf[6] = HEJIA_SUB_CMD_OVER_TIME;
			sendBuf[7] = 0x01;

			sendXor = getXorResult(sendBuf, 8);
			
			sendBuf[8] = sendXor & 0xFF;
			sendBuf[9] = (sendXor >> 8) & 0xFF;

			sendBuf[10] = HEJIA_END_DATA0;
			sendBuf[11] = HEJIA_END_DATA1;

#if 1
			printf("---------------------------SendBuf----------------------\r\n");
			for(i = 0; i < sendBuf[2]; i++)
			{
				if(i % 6 == 0)
				{
					printf("\r\n");
				}
				printf("buf[%d] 0x%02X\t", i, sendBuf[i]);
			}
			printf("\r\n");			
#endif

			write(handle, sendBuf, 12);

			sleep(1);

			FD_ZERO(&readfds);
			FD_SET(handle, &readfds);
			
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			memset(recvBuf, 0, sizeof(recvBuf));
			readBufLen = 0;
			
			ret = select(handle + 1, &readfds, NULL, NULL, &tv);
			if(ret > 0 && FD_ISSET(handle, &readfds)) 
			{
				iOffset = ReadPacket(handle, validBuf, &iValidLen);
				
				memcpy(recvBuf, validBuf + iOffset, iValidLen);
				readBufLen = iValidLen;
					
				printf("---------------------------RecvBuf----------------------\r\n");
				printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
				for (i = 0; i < readBufLen; i++)
				{
					if (i % 6 == 0)
					{
						printf("\r\n");
					}
					
					printf("buf[%d] 0x%02X\t", i, recvBuf[i]);
				}
				printf("\r\n");
				printf("---------------------------RecvBuf----------------------\r\n");
			}
			
			break;
		}
		/* 下载特征文件 */
		case '3':
		{
			char FeaturePath[MAX_FILE_PATH_LEN];
			int nFeatureLen = 0;
			int nPacketTotal = 0;
			unsigned char nPacketCur = 0;
			int nPacketDataLen = 0;
			int ret = 0;
			char caStuId[8 + 1] = {0};
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);
			
			memcpy(caStuId, "10000000", MAX_ID_LEN);

			sprintf(FeaturePath, "FeatureTest.dat");

			nFeatureLen = ReadFeature(g_faceBuf, MAX_FACE_BUFFER_LEN, FeaturePath);
			g_faceBufOffset = 0;
			
			nPacketTotal = nFeatureLen / MAX_PACKET_SIZE;
			
			if((nFeatureLen % MAX_PACKET_SIZE) != 0)
			{
				nPacketTotal++;
			}

			printf("Packet Total is %d\n", nPacketTotal);
			for(nPacketCur = 0; nPacketCur < nPacketTotal; nPacketCur++)
			{
				printf("Cur packet is %d\n", nPacketCur);
				
				memset(sendBuf, 0, sizeof(sendBuf));
				
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;
		
				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = HEJIA_SUB_CMD_SEND_FEATURE;
				memcpy(sendBuf + 7, caStuId, 8);
				sendBuf[15] = nPacketTotal;
				sendBuf[16] = nPacketCur;
				
				/* 拷备Feature 数据 */
				if((nFeatureLen - g_faceBufOffset) >= MAX_PACKET_SIZE)
				{
					nPacketDataLen = MAX_PACKET_SIZE;
				}
				else
				{
					nPacketDataLen = nFeatureLen - g_faceBufOffset;
				}

				printf("nPacketCur %d g_faceBufOffset %d nPacketDataLen %d %s %d\r\n", 
					nPacketCur, g_faceBufOffset, nPacketDataLen, __FUNCTION__, __LINE__);
				
				memcpy(sendBuf + 17, g_faceBuf + g_faceBufOffset, nPacketDataLen);
				g_faceBufOffset += nPacketDataLen;

				sendBuf[2] = (21 + nPacketDataLen) & 0xff;
				sendBuf[3] = ((21 + nPacketDataLen) >> 8) & 0xff;

				sendXor = getXorResult(sendBuf, 17 + nPacketDataLen);
				
				sendBuf[17 + nPacketDataLen] = sendXor & 0xFF;
				sendBuf[17 + nPacketDataLen + 1] = (sendXor >> 8) & 0xFF;

				sendBuf[17 + nPacketDataLen + 2] = HEJIA_END_DATA0;
				sendBuf[17 + nPacketDataLen + 3] = HEJIA_END_DATA1;

#if 0
				printf("datalen %d \r\n", sendBuf[2]);

				for (i = 0; i < 21 + nPacketDataLen; i++)
				{
					if (i % 8 == 0)
					{
						printf("\r\n");
					}
					
					printf("0x%02X \t", sendBuf[i]);
				}
				printf("%s %d\r\n", __FUNCTION__, __LINE__);
#endif
				printf("nPacketDataLen %d %s %d\r\n", nPacketDataLen + 17 + 4, __FUNCTION__, __LINE__);

				ret = WriteData(handle, sendBuf, nPacketDataLen + 17 + 4);
					
				//ret = write(handle, sendBuf, nPacketDataLen + 17 + 4);

				printf("handle %d ret %d %s %d\r\n", handle, ret, __FUNCTION__, __LINE__);

				usleep(10 * 1000);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 15;
				tv.tv_usec = 0;

				memset(validBuf, 0, sizeof(validBuf));
				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				iValidLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);

				printf("ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
				
				if(ret > 0 && FD_ISSET(handle, &readfds)) 
				{
					printf("%s %d\r\n", __FUNCTION__, __LINE__);
					
					iOffset = ReadPacket(handle, validBuf, &iValidLen);
				
					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;
					
					printf("---------------------------RecvBuf----------------------\r\n");	
					printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
					memset(caStuId, 0 ,sizeof(caStuId));
					memcpy(caStuId, &recvBuf[7], 8);

					printf("StuId %s, Packet total Num is %d, Packet CurNo is %d, packet flag %d\r\n",
						caStuId, recvBuf[15], recvBuf[16], recvBuf[17]);
				        
                    if (recvBuf[17] != 0x01)
                    {
    					printf("Send Pack index %u Error! %s %d\r\n", recvBuf[16], __FUNCTION__, __LINE__);
                        break;
                    }

					for (i = 0; i < readBufLen; i++)
					{
						if (i % 8 == 0)
						{
							printf("\r\n");
						}
						
						printf("0x%02X \t", recvBuf[i]);
					}
					printf("\r\n");
					printf("---------------------------RecvBuf----------------------\r\n");
				}
				else
				{
					printf("Recv Respone Timeout, Send Pack index %u Error! %s %d\r\n", recvBuf[16], __FUNCTION__, __LINE__);
					break;
				}
				printf(" %s %d\r\n", __FUNCTION__, __LINE__);
			}

			printf("All feature data have send\n");
			
			break;
		}
		/* 开始比对 */
		case '4':
		{
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			memset(sendBuf, 0, sizeof(sendBuf));
			
			sendBuf[0] = HEJIA_REQ_START_DATA0;
			sendBuf[1] = HEJIA_REQ_START_DATA1;
			sendBuf[2] = 0x0b;
			sendBuf[3] = 0x00;
			sendBuf[4] = HEJIA_DEVICE_TYPE;
			sendBuf[5] = HEJIA_MAIN_CMD;
			sendBuf[6] = HEJIA_SUB_CMD_START_VERIFY;

			sendXor = getXorResult(sendBuf, 7);
			printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
			
			sendBuf[7] = sendXor & 0xFF;
			sendBuf[8] = (sendXor >> 8) & 0xFF;

			sendBuf[9] = HEJIA_END_DATA0;
			sendBuf[10] = HEJIA_END_DATA1;
			
			write(handle, sendBuf, 11);

			sleep(1);

			FD_ZERO(&readfds);
			FD_SET(handle, &readfds);
			
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			memset(recvBuf, 0, sizeof(recvBuf));
			readBufLen = 0;
			
			ret = select(handle + 1, &readfds, NULL, NULL, &tv);
			if(ret > 0 && FD_ISSET(handle, &readfds)) 
			{
				iOffset = ReadPacket(handle, validBuf, &iValidLen);
			
				memcpy(recvBuf, validBuf + iOffset, iValidLen);
				readBufLen = iValidLen;
				
				printf("---------------------------RecvBuf----------------------\r\n");	
				printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
				for (i = 0; i < readBufLen; i++)
				{
					if (i % 8 == 0)
					{
						printf("\r\n");
					}
					
					printf("buf[%d] 0x%02X \t", i, recvBuf[i]);
				}
				printf("\r\n");
				printf("---------------------------RecvBuf----------------------\r\n");	
			}
			
			break;
		}
		/* 1:1 */
		case '8':
		{
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);
			memset(sendBuf, 0, sizeof(sendBuf));
			
			sendBuf[0] = HEJIA_REQ_START_DATA0;
			sendBuf[1] = HEJIA_REQ_START_DATA1;
			sendBuf[2] = 0x13;
			sendBuf[3] = 0x00;
			sendBuf[4] = HEJIA_DEVICE_TYPE;
			sendBuf[5] = HEJIA_MAIN_CMD;
			sendBuf[6] = SUB_CMD_ONE_TO_ONE_VERIFY;

			memcpy(sendBuf + 7, "10000000", MAX_ID_LEN);
			
			sendXor = getXorResult(sendBuf, 15);
			printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
			
			sendBuf[15] = sendXor & 0xFF;
			sendBuf[16] = (sendXor >> 8) & 0xFF;

			sendBuf[17] = HEJIA_END_DATA0;
			sendBuf[18] = HEJIA_END_DATA1;
			
			write(handle, sendBuf, 19);

			sleep(1);

			FD_ZERO(&readfds);
			FD_SET(handle, &readfds);
			
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			memset(recvBuf, 0, sizeof(recvBuf));
			readBufLen = 0;
			
			ret = select(handle + 1, &readfds, NULL, NULL, &tv);
			if(ret > 0 && FD_ISSET(handle, &readfds)) 
			{
				iOffset = ReadPacket(handle, validBuf, &iValidLen);
			
				memcpy(recvBuf, validBuf + iOffset, iValidLen);
				readBufLen = iValidLen;
				
				printf("---------------------------RecvBuf----------------------\r\n");	
				printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
				for (i = 0; i < readBufLen; i++)
				{
					if (i % 8 == 0)
					{
						printf("\r\n");
					}
					
					printf("buf[%d] 0x%02X \t", i, recvBuf[i]);
				}
				printf("\r\n");
				printf("1:1 verify %s \r\n", (recvBuf[7] == 0x01) ? "Ok" : "Fail");
				printf("---------------------------RecvBuf----------------------\r\n");	
			}

			break;
		}
		/* 查询比对结果 */
		case '5':
		{
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			memset(sendBuf, 0, sizeof(sendBuf));
			
			sendBuf[0] = HEJIA_REQ_START_DATA0;
			sendBuf[1] = HEJIA_REQ_START_DATA1;
			sendBuf[2] = 0x0b;
			sendBuf[3] = 0x00;
			sendBuf[4] = HEJIA_DEVICE_TYPE;
			sendBuf[5] = HEJIA_MAIN_CMD;
			sendBuf[6] = HEJIA_SUB_CMD_GET_RESULT;

			sendXor = getXorResult(sendBuf, 7);
			
			sendBuf[7] = sendXor & 0xFF;
			sendBuf[8] = (sendXor >> 8) & 0xFF;

			sendBuf[9] = HEJIA_END_DATA0;
			sendBuf[10] = HEJIA_END_DATA1;
			
			write(handle, sendBuf, 11);

			sleep(1);

			FD_ZERO(&readfds);
			FD_SET(handle, &readfds);
			
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			memset(recvBuf, 0, sizeof(recvBuf));
			readBufLen = 0;
			
			ret = select(handle + 1, &readfds, NULL, NULL, &tv);
			if(ret > 0 && FD_ISSET(handle, &readfds)) 
			{
				iOffset = ReadPacket(handle, validBuf, &iValidLen);
			
				memcpy(recvBuf, validBuf + iOffset, iValidLen);
				readBufLen = iValidLen;

				printf("---------------------------RecvBuf----------------------\r\n");
				printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
				printf("Result is %s\r\n", recvBuf[7] == 0x01 ? "ok" : "bad");
				
				memset(g_facePicName, 0, sizeof(g_facePicName));
				memcpy(g_facePicName, recvBuf + 8, readBufLen - 12);
				
				printf("g_facePicName %s\r\n", g_facePicName);
				
				for (i = 0; i < readBufLen; i++)
				{
					if (i % 8 == 0)
					{
						printf("\r\n");
					}
					
					printf("buf[%d] 0x%02X \t", i, recvBuf[i]);
				}
				printf("\r\n");
				printf("---------------------------RecvBuf----------------------\r\n");
			}
			
			break;
		}
		/* 读取比对图片 */
		case '6':
		{
			int nPacketTotal = 1;
			int nPacketCur = 0;
			int nFacePicOffset = 0;
			int hWrite = -1;
//			int nPicNameLen = 0;
			char PicPath[MAX_FILE_PATH_LEN];
			
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			for(nPacketCur = 0; nPacketCur < nPacketTotal; nPacketCur++)
			{
				memset(sendBuf, 0, sizeof(sendBuf));
				
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;
				sendBuf[2] = 0x0b + 65;
				sendBuf[3] = 0x00;
				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = HEJIA_SUB_CMD_GET_PIC;

				//nPicNameLen = strlen(g_facePicName);
			
				memcpy(sendBuf+7, g_facePicName, 64);
				sendBuf[7 + 64] = nPacketCur;
				
				sendXor = getXorResult(sendBuf, 7 + 65);
				printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
				
				sendBuf[7 + 64 + 1] = sendXor & 0xFF;
				sendBuf[7 + 64 + 2] = (sendXor >> 8) & 0xFF;

				sendBuf[7 + 64 + 3] = HEJIA_END_DATA0;
				sendBuf[7 + 64 + 4] = HEJIA_END_DATA1;
				
				write(handle, sendBuf, 11 + 65);
				
				sleep(1);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 10;
				tv.tv_usec = 0;

				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);

				if(ret > 0 && FD_ISSET(handle, &readfds)) 
				{
					iOffset = ReadPacket(handle, validBuf, &iValidLen);
				
					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;

					/* 根据收到的数据包，更新 总包数*/
					nPacketTotal = recvBuf[7];

					printf("nPacketTotal %d %s %d\n", nPacketTotal, __FUNCTION__, __LINE__);

					memcpy(g_facePicBuf + nFacePicOffset, recvBuf + 9, readBufLen - 13);
					nFacePicOffset += readBufLen - 13;
					
					printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);

					printf("Packet total Num is %d, Packet CurNo is %d\n", recvBuf[7], recvBuf[8]);
					
				}
			
			}

			sprintf(PicPath, "PicTest.jpg");

			hWrite = open(PicPath, O_WRONLY | O_CREAT);

			if(hWrite < 0)
			{
				printf("open file %s error\n", PicPath);
				break;
			}

			printf("nFacePicOffset %d\n", nFacePicOffset);
			
			write(hWrite, g_facePicBuf, nFacePicOffset);

			printf("Write ok %s %d\n", __FUNCTION__, __LINE__);
			close(hWrite);

			g_AuthFlag = 0;
			break;
		}	
		case '7':
		{
			char caRegPhotoPath[MAX_FILE_PATH_LEN];
			int iRegPhotoLen = 0;
			int nPacketTotal = 0;
			unsigned char nPacketCur = 0;
			int nPacketDataLen = 0;
			int ret = 0;
			char caStuId[8 + 1] = {0};
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);
			
			memcpy(caStuId, "10000000", 8);

			sprintf(caRegPhotoPath, "RegPhoto.jpg");

			iRegPhotoLen = ReadFeature(g_RegPhotoBuf, MAX_PHOTO_BUFFER_LEN, caRegPhotoPath);
			g_faceBufOffset = 0;
			
			nPacketTotal = iRegPhotoLen / MAX_PACKET_SIZE;
			
			if((iRegPhotoLen % MAX_PACKET_SIZE) != 0)
			{
				nPacketTotal++;
			}

			printf("Packet Total is %d\n", nPacketTotal);
			
			for(nPacketCur = 0; nPacketCur < nPacketTotal; nPacketCur++)
			{
				printf("Cur packet is %d\n", nPacketCur);
				
				memset(sendBuf, 0, sizeof(sendBuf));
				
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;
				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = HEJIA_SAVE_REG_GET_PIC;
				memcpy(sendBuf + 7, caStuId, 8);
				sendBuf[15] = nPacketTotal;
				sendBuf[16] = nPacketCur;
				
				/* 拷备Feature 数据 */
				if((iRegPhotoLen - g_faceBufOffset) >= MAX_PACKET_SIZE)
				{
					nPacketDataLen = MAX_PACKET_SIZE;
				}
				else
				{
					nPacketDataLen = iRegPhotoLen - g_faceBufOffset;
				}

				printf("nPacketCur %d g_faceBufOffset %d nPacketDataLen %d %s %d\r\n", 
					nPacketCur, g_faceBufOffset, nPacketDataLen, __FUNCTION__, __LINE__);
				
				memcpy(sendBuf + 17, g_RegPhotoBuf + g_faceBufOffset, nPacketDataLen);
				g_faceBufOffset += nPacketDataLen;

				sendBuf[2] = (21 + nPacketDataLen) & 0xff;
				sendBuf[3] = ((21 + nPacketDataLen) >> 8) & 0xff;

				sendXor = getXorResult(sendBuf, 17 + nPacketDataLen);
				
				sendBuf[17 + nPacketDataLen] = sendXor & 0xFF;
				sendBuf[17 + nPacketDataLen + 1] = (sendXor >> 8) & 0xFF;

				sendBuf[17 + nPacketDataLen + 2] = HEJIA_END_DATA0;
				sendBuf[17 + nPacketDataLen + 3] = HEJIA_END_DATA1;

#if 1
				printf("datalen %d \r\n", sendBuf[2]);

				for (i = 0; i < 21 + nPacketDataLen; i++)
				{
					if (i % 8 == 0)
					{
						printf("\r\n");
					}
					
					printf("0x%02X \t", sendBuf[i]);
				}
				printf("%s %d\r\n", __FUNCTION__, __LINE__);
#endif
				printf("nPacketDataLen %d %s %d\r\n", nPacketDataLen + 17 + 4, __FUNCTION__, __LINE__);

				ret = write(handle, sendBuf, nPacketDataLen + 17 + 4);

				printf("handle %d ret %d %s %d\r\n", handle, ret, __FUNCTION__, __LINE__);

				usleep(10 * 1000);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 10;
				tv.tv_usec = 0;

				memset(validBuf, 0, sizeof(validBuf));
				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				iValidLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);

				printf("ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
				
				if(ret > 0 && FD_ISSET(handle, &readfds)) 
				{
					iOffset = ReadPacket(handle, validBuf, &iValidLen);
				
					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;
					
					printf("---------------------------RecvBuf----------------------\r\n");	
					printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
					memset(caStuId, 0 ,sizeof(caStuId));
					memcpy(caStuId, &recvBuf[7], 8);

					printf("StuId %s, Packet total Num is %d, Packet CurNo is %d, packet flag %d\r\n",
						caStuId, recvBuf[15], recvBuf[16], recvBuf[17]);
				        
                    if (recvBuf[17] != 0x01)
                    {
		   	 			printf("Send Pack index %u Error! %s %d\r\n", recvBuf[16], __FUNCTION__, __LINE__);
                        break;
                    }

					for (i = 0; i < readBufLen; i++)
					{
						if (i % 8 == 0)
						{
							printf("\r\n");
						}
						
						printf("0x%02X \t", recvBuf[i]);
					}
					printf("\r\n");
					printf("---------------------------RecvBuf----------------------\r\n");
				}	
				else
				{
					printf("Recv Response Timeout, Send Pack index %u Error! %s %d\r\n", recvBuf[16], __FUNCTION__, __LINE__);
					break;
				}
			}

			printf("All Photo data have send\n");
			
			break;
		}
		case '9':
		{
			short sTotalUserNum = 1;
			short sIndex = 0;
			unsigned char ucaUserNo[MAX_ID_LEN + 1] = {0};
			unsigned char ucaUserName[MAX_NAME_LEN + 1] = {0};
			char cUserType = 0;
			char cPhotoType = 0;
			
			int iLen = 0;
			
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			for(sIndex = 0; sIndex < sTotalUserNum; sIndex++)
			{
				memset(sendBuf, 0, sizeof(sendBuf));
				
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;

				sendBuf[2] = 0x0D;
				sendBuf[3] = 0x00;

				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = SUB_CMD_GET_USER_LIST;

				sendBuf[7] = sIndex & 0xFF;
				sendBuf[8] = (sIndex >> 8) & 0xFF;
				
				sendXor = getXorResult(sendBuf, 9);

				printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
				
				sendBuf[9] = sendXor & 0xFF;
				sendBuf[10] = (sendXor >> 8) & 0xFF;

				sendBuf[11] = HEJIA_END_DATA0;
				sendBuf[12] = HEJIA_END_DATA1;
				
				write(handle, sendBuf, 13);
				
				sleep(1);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 10;
				tv.tv_usec = 0;

				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);

				if(ret > 0 && FD_ISSET(handle, &readfds)) 
				{
					iOffset = ReadPacket(handle, validBuf, &iValidLen);
				
					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;

					if (iValidLen == 0x0B)
					{
						printf("No User Exist! %s %d \r\n", __FUNCTION__, __LINE__);
					}
					else
					{
						/* 更新总用户数 */
						sTotalUserNum = (recvBuf[8] << 8) | recvBuf[7];
						printf("totalUserNum %d %s %d \r\n", sTotalUserNum, __FUNCTION__, __LINE__);

						sIndex = (recvBuf[10] << 8) | recvBuf[9];
						printf("sIndex %d %s %d \r\n", sIndex, __FUNCTION__, __LINE__);

						iLen = 11;
						
						memset(ucaUserNo, '\0', sizeof(ucaUserNo));
						memcpy(ucaUserNo, recvBuf + iLen, MAX_ID_LEN);
						iLen += MAX_ID_LEN;
						printf("ucaUserNo %s %s %d \r\n", ucaUserNo, __FUNCTION__, __LINE__);
							
						memset(ucaUserName, '\0', sizeof(ucaUserName));
						memcpy(ucaUserName, recvBuf + iLen, MAX_NAME_LEN);
						iLen += MAX_NAME_LEN;
						printf("ucaUserName %s %s %d \r\n", ucaUserName, __FUNCTION__, __LINE__);

						cUserType = recvBuf[iLen];
						iLen += 1;
						printf("UserType %s %s %d \r\n", (cUserType == 0) ? "Normal" : "Admin", __FUNCTION__, __LINE__);
						
						cPhotoType = recvBuf[iLen];
						printf("PhotoType %s %s %d \r\n", (cPhotoType == 0) ? "BMP" : "JPG", __FUNCTION__, __LINE__);	
					}						
				}
			}

			break;
		}
		case 'a':
		{
			int iTotalPackNum = 1;
			char cIndex = 0;
			unsigned char ucaUserNo[MAX_ID_LEN + 1] = {0};
			char caFeaturePath[128] = {0};
			
			int iOffset = 0;
			
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			for(cIndex = 0; cIndex < iTotalPackNum; cIndex++)
			{
				memset(sendBuf, 0, sizeof(sendBuf));
				
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;
				sendBuf[2] = 0x14;
				sendBuf[3] = 0x00;
				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = SUB_GET_USER_FEATURE;
			
				memcpy(sendBuf + 7, "10000000", MAX_ID_LEN);
				
				sendBuf[15] = cIndex;
				
				sendXor = getXorResult(sendBuf, 16);

				printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
				
				sendBuf[16] = sendXor & 0xFF;
				sendBuf[17] = (sendXor >> 8) & 0xFF;

				sendBuf[18] = HEJIA_END_DATA0;
				sendBuf[19] = HEJIA_END_DATA1;
				
				write(handle, sendBuf, 20);
				
				sleep(1);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 10;
				tv.tv_usec = 0;

				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);

				if(ret > 0 && FD_ISSET(handle, &readfds)) 
				{
					iOffset = ReadPacket(handle, validBuf, &iValidLen);
				
					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;

					memset(ucaUserNo, '\0', sizeof(ucaUserNo));
					memcpy(ucaUserNo, recvBuf + 7, MAX_ID_LEN);

					printf("UserId: %s %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);
					
					if (recvBuf[15] == 0x00)
					{
						printf("User %s Not Exist! %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);
					}
					else if (recvBuf[15] == 0x01)
					{
						printf("User %s Exist, But No Feature! %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);
					}
					else
					{
						/* 根据收到的数据包，更新 总包数*/
						printf("User %s Exist, Feature Exist! %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);

						iTotalPackNum = recvBuf[15];
						
						printf("Packet total Num is %d, Packet CurNo is %d\n", recvBuf[15], recvBuf[16]);
						
						memcpy(g_faceBuf + iOffset, recvBuf + 17, readBufLen - 17 - 4);

						iOffset += (readBufLen - 17 - 4);
						
						printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
					}
				}
			}

			if (cIndex != 0 && cIndex == iTotalPackNum - 1)
			{
				int hWrite = -1;
				snprintf(caFeaturePath, sizeof(caFeaturePath), "%s_Feature.dat", ucaUserNo);
				
				hWrite = open(caFeaturePath, O_WRONLY | O_CREAT);

				if(hWrite < 0)
				{
					printf("open file %s error\n", caFeaturePath);
					break;
				}

				printf("iOffset %d %s %d\n", iOffset, __FUNCTION__, __LINE__);
				
				write(hWrite, g_faceBuf, iOffset);

				printf("Get User %s Feature Success! %s %d\n", ucaUserNo, __FUNCTION__, __LINE__);

				close(hWrite);
			}

			g_AuthFlag = 0;

			break;
		}
		case 'b':
		{
			int iTotalPackNum = 1;
			char cIndex = 0;
			unsigned char ucaUserNo[MAX_ID_LEN + 1] = {0};
			char caRegPhotoPath[128] = {0};
			
			int iOffset = 0;
			
			printf(" %s %d\r\n", __FUNCTION__, __LINE__);

			for(cIndex = 0; cIndex < iTotalPackNum; cIndex++)
			{
				memset(sendBuf, 0, sizeof(sendBuf));
				
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;
				sendBuf[2] = 0x14;
				sendBuf[3] = 0x00;
				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = SUB_GET_USER_REG_PHOTO;
			
				memcpy(sendBuf + 7, "10000000", MAX_ID_LEN);
				
				sendBuf[15] = cIndex;
				
				sendXor = getXorResult(sendBuf, 16);

				printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
				
				sendBuf[16] = sendXor & 0xFF;
				sendBuf[17] = (sendXor >> 8) & 0xFF;

				sendBuf[18] = HEJIA_END_DATA0;
				sendBuf[19] = HEJIA_END_DATA1;
				
				write(handle, sendBuf, 20);
				
				sleep(1);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 10;
				tv.tv_usec = 0;

				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);

				if(ret > 0 && FD_ISSET(handle, &readfds)) 
				{
					iOffset = ReadPacket(handle, validBuf, &iValidLen);
				
					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;

					memset(ucaUserNo, '\0', sizeof(ucaUserNo));
					memcpy(ucaUserNo, recvBuf + 7, MAX_ID_LEN);

					printf("UserId: %s %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);
					
					if (recvBuf[15] == 0x00)
					{
						printf("User %s Not Exist! %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);
					}
					else if (recvBuf[15] == 0x01)
					{
						printf("User %s Exist, But No Regist Photo! %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);
					}
					else
					{
						/* 根据收到的数据包，更新 总包数*/
						printf("User %s Exist, Regist Photo Exist! %s %d\r\n", ucaUserNo, __FUNCTION__, __LINE__);

						iTotalPackNum = recvBuf[15];
						
						printf("Packet total Num is %d, Packet CurNo is %d\n", recvBuf[15], recvBuf[16]);
						
						memcpy(g_RegPhotoBuf + iOffset, recvBuf + 17, readBufLen - 17 - 4);

						iOffset += (readBufLen - 17 - 4);
						
						printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
					}
				}
			}

			if (cIndex != 0 && cIndex == iTotalPackNum - 1)
			{
				int hWrite = -1;
				snprintf(caRegPhotoPath, sizeof(caRegPhotoPath), "%s_4.jpg", ucaUserNo);
				
				hWrite = open(caRegPhotoPath, O_WRONLY | O_CREAT);

				if(hWrite < 0)
				{
					printf("open file %s error\n", caRegPhotoPath);
					break;
				}

				printf("iOffset %d %s %d\n", iOffset, __FUNCTION__, __LINE__);
				
				write(hWrite, g_RegPhotoBuf, iOffset);

				printf("Get User %s Reg Photo Success! %s %d\n", ucaUserNo, __FUNCTION__, __LINE__);

				close(hWrite);
			}

			g_AuthFlag = 0;

			break;
		}
		default:
		{
			printf("\r\n");
			printf("1: auth\r\n");
			printf("2: set over time\r\n");
			printf("3: download feature file\r\n");

			printf("4: start 1:N verify\r\n");
			printf("8: start 1:1 verify\r\n");
			
			printf("5: get verify result\r\n");
			printf("6: read verify picture\r\n");
			printf("7: download reg picture\r\n");
			
			printf("9: get total user list\r\n");
			printf("a: get user feature\r\n");
			printf("b: get user register photo\r\n");
			
			break;
		}
	}

	return ret;
}

int main(int argc, char* argv[])
{
	int handle = -1;
	char devPath[64];
	char cmd = 0;
	int ret = 0;
	
	sprintf(devPath, "/dev/ttyS%s", argv[1]);

	printf("devPath %s %s %d\r\n", devPath, __FUNCTION__, __LINE__);
	
	handle = open(devPath, O_RDWR);

	printf("handle %d %s %d\r\n", handle, __FUNCTION__, __LINE__);

	if(handle >= 0)
	{
		//set_serial_param(handle, 57600, 8, 1, 'n', 0);
		set_serial_param(handle, 57600, 8, 1, 'n', 0);
	}
	else
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	while(1)
	{
		printf("\r\nPlease input cmd:");
		cmd = getchar();
		
		if(((cmd >= '0') && (cmd <= '9')) || ((cmd >= 'a') && (cmd <= 'z')))
		{
			while ((getchar()) != '\n')
			{
			}
		}

		if ((cmd == 'q') || (cmd == 'Q'))
		{
			break;
		}

		/*if(g_AuthFlag == 0 && (cmd != '1' && cmd != 0x0a))
		{
			printf("cmd is invalid\n");
			continue;
		}*/
		
		ret = dealCmd(handle, cmd);
		
		if(ret < 0)
		{
			break;
		}
	}
	
	close(handle);
	
	return 0;
}

