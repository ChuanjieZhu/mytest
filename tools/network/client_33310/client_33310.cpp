
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include "client_33310.h"

#define SERV_IP "192.168.58.1"
#define SERV_PORT 33310
/* 重连服务器超时时间 */
#define RECONNECT_SERVER_TIME 8

static int g_exitFlag = 0;
static int g_reConnectFlag = 0;

int Rand()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );
	unsigned int seed = tv.tv_sec ^ tv.tv_usec;
	return rand_r(&seed);
}

int TcpRecvMsg( int socketID, int length, void *pContent )
{
	// 处理非法参数
	if( socketID < 0 || pContent == NULL || length <= 0 )
		return -1;

	int retVal = 0;
	int rc = 0;
	int offset = 0;
	
	int bytes	= 0;
	int nTimeOut = 0;
	int selectVal = 0;
	fd_set rd, ex;	
	struct timeval tv;
	
	while ( length > 0 )
	{
		// start: 设置等待超时时间
		FD_ZERO( &rd ); 
		FD_SET( socketID, &rd );
		
		FD_ZERO( &ex ); 
		FD_SET( socketID, &ex );

		tv.tv_sec	= 1;	
		tv.tv_usec	= 0;
		selectVal = select( socketID+1, &rd, NULL, &ex, &tv );
		if ( selectVal == 0 )	
		{
			nTimeOut++;
			if ( nTimeOut > 60 )
			{
				retVal = -2;	/* 超时退出 */
				printf("Wait message request timeout, return (%d) ...\r\n", retVal );
				break;
			}
			continue;
		} 
		else if ( selectVal < 0 || FD_ISSET(socketID, &ex) )	
		{	
			retVal = -3;
			break;
		}
		else
		{
			nTimeOut = 0;
		}
		
		if ( ioctl( socketID, FIONREAD, &bytes) < 0 || bytes <= 0 ) 
		{
			retVal = -4;
			break;
		}
		// end: 设置等待超时时间
		
		rc = recv( socketID, (char *)pContent+offset, length, 0 );
		if( rc <= 0 )			/* 出错或对端退出 */
		{
			retVal = -1;
			break;
		}
		else					/* 继续接收 */
		{
			length -= rc;
			offset += rc;
		}
	}

	return retVal;
}

/*
**  函数: TcpSendMsg
**  输入: 客户端 socketID, 数据长度 length, 数据内容 pContent
**  输出: 无
**  功能: 函数从服务器端发送一条指定长度的数据
**  返回: 正常退出返回0，出错退出返回负数
*/

int TcpSendMsg( int socketID, int length, void *pContent )
{
    	// 处理非法参数
	if ( socketID < 0 || pContent == NULL || length <= 0 )
		return -2;
		
	int rc = 0, sendLen = 0, offset = 0;

	// 对于大于16K的数据进行分包发送
	int subPackFlag = 0;
	if ( length > 16 * 1024 ) subPackFlag = 1;
#if _COMM_DEBUG
	printf( "length = %d\r\n", length );
#endif

	do
	{
		sendLen = length;
		if ( subPackFlag && sendLen > 1460 ) sendLen = 1460;		

//		rc = send( socketID, (char *)pContent+offset, sendLen, MSG_DONTWAIT|MSG_NOSIGNAL );
		rc = send( socketID, (char *)pContent+offset, sendLen, 0 );

#if _COMM_DEBUG
		printf( "rc = %d\r\n", rc );
#endif

		if ( rc < 0 )
		{
#if _COMM_DEBUG
			printf( "Send error : %d -> %s\r\n", errno, strerror( errno ) );
#endif
			if ( errno == EWOULDBLOCK )
			{
				printf("Error(EWOULDBLOCK): Tcp Send Msg return %d\r\n", rc);
				rc = 0;	
				usleep( 20*1000 );
			}
			else
			{
				printf("Error: Tcp Send Msg return %d\r\n", rc);
				return -1;
			}
		}

		length -= rc;
		offset += rc;

	} while ( length > 0 );

	return 0;
}

/*
**  函数: TcpRecvPack
**  输入: 客户端 socketID
**  输出: 数据包 pack
**  功能: 函数从客户端接收一个完整的数据包
**  返回: 正常返回0，出错返回-1，对方断开返回-2
**  注意: 接收数据包的最大长度为< DATA_MAX_LEN >
*/
int TcpRecvPack( int			socketID,	/* 客户端ID    */
				LPPROTOCOL_PACK	pack )		/* 接收数据包 */
{
	int retVal = -1;
	retVal = TcpRecvMsg( socketID, sizeof( pack->head ), &pack->head );
 	if ( retVal < 0 ) return -2;

	pack->head.flag		= ntohs( pack->head.flag );
	pack->head.index	= ntohs( pack->head.index );
	pack->head.dataLen	= ntohl( pack->head.dataLen );

	if ( pack->head.flag != PROTOCOL_HEAD_FLAG ) return -1;
	if ( pack->head.dataLen > DATA_MAX_LEN ) return -1;

	if ( pack->head.dataLen > 0 )
	{
		retVal = TcpRecvMsg( socketID, pack->head.dataLen, pack->data);
	 	if ( retVal < 0 ) 
	 	{
            return -2;
	 	}
	}
    
	return 0;
}

/*
**  函数: TcpRecvPack
**  输入: 略
**  输出: 无
**  功能: 服务器发送数据包到客户端
**  返回: 正常退出返回0，出错退出返回负数
*/
int TcpSendPack( int			socketID,	 /* 客户端ID */
					  unsigned short	flag,		 /* 包标识    */
					  unsigned short	index,		 /* 序号       */
					  char			packType,	 /* 包类型    */
					  char			subType,	 /* 子类型    */
					  unsigned int		dataLen,       	 /* 数据长度 */
					  void * 			data )		 /* 数据内容 */
{
	int retVal = 0;
	LPPROTOCOL_PACK pData = ( LPPROTOCOL_PACK )malloc( sizeof(PROTOCOL_HEAD) + dataLen );

	if ( pData )
	{
		pData->head.flag		= htons( flag );
		pData->head.index		= htons( index );
		pData->head.packType	= packType;
		pData->head.subType	= subType;
		pData->head.dataLen		= htonl( dataLen );
        if (data && dataLen > 0)
        {
		    memcpy( pData->data, data, dataLen );
        }
        
		retVal = TcpSendMsg( socketID, sizeof(PROTOCOL_HEAD) + dataLen, pData );
		free( pData );
	}

	return retVal;
}

int CreateTcpClient(int *pSocketFd)
{
    struct sockaddr_in addr;
    int sockId;
    int ret = -1;

    if (pSocketFd == NULL)
    {
        return -1;
    }
    
    sockId = socket(AF_INET, SOCK_STREAM, 0);
    if (sockId == -1)
    {
        printf("< create socket failed. %s %d\r\n", MDL);
        return -1;
    }

    struct timeval tv = {3, 0};
    setsockopt(sockId, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr = inet_addr(SERV_IP);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERV_PORT);
    
    ret = connect(sockId, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        printf("< connect server fail(%s). %s %d\r\n", strerror(errno), MDL);
        close(sockId);
        sockId = -1;
        return -1;
    }
    
    *pSocketFd = sockId;
    
    return ret;
}


int CreateHead(PROTOCOL_HEAD *pstHead, char packType, char subType, int dataLen)
{
    if (pstHead == NULL)
    {
        return -1;
    }

    memset(pstHead, 0, sizeof(PROTOCOL_HEAD));
    pstHead->flag = htons(PROTOCOL_HEAD_FLAG);
    pstHead->index = htons(Rand() % 65535);
    pstHead->packType = packType;
    pstHead->subType = subType;
    pstHead->dataLen = htonl(dataLen);

    return 0;
}


int DealGetLoggerRequest(int iSocketFd, LPPROTOCOL_PACK pRecv)
{
    int iRet = -1;
    unsigned short index = Rand() % 65535;
    
    iRet = TcpSendPack(iSocketFd, 
                       PROTOCOL_HEAD_FLAG, 
                       index, 
                       GET,
                       GET_DEV_LOG, 
                       0, 
                       NULL);
    if (iRet != 0)
    {
        printf("< Tcp send get logger packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    sleep(1);
    
    iRet = TcpRecvPack(iSocketFd, pRecv);

     printf( "TcpRecvPack : nIndex=%d, Pack Flag = %X, Pack Type = %d, Sub Type = %d, Pack Data Length = %d, Return Value = %d\r\n", 
		pRecv->head.index, pRecv->head.flag, pRecv->head.packType, pRecv->head.subType, pRecv->head.dataLen, iRet );
    
    if (iRet != 0)
    {
        printf("< Tcp recv get logger response packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    printf("* pRecv->head.index: %d, index: %d %s %d\r\n", pRecv->head.index, index, MDL);
    
    if (pRecv->head.index != index
        || pRecv->head.packType != RESPONSE
        || pRecv->head.subType != ACK)
    {
        printf("< Tcp recv get logger response packet error. %s %d\r\n", MDL);    
        return -1;
    }

    if (pRecv->head.dataLen < sizeof(GET_LOGGER))
    {
        printf("< Tcp recv get logger response size error. %s %d\r\n", MDL); 
        return -1;
    }
    
    GET_LOGGER stGetLogger;
    memset(&stGetLogger, 0, sizeof(GET_LOGGER));
    memcpy(&stGetLogger, pRecv->data, sizeof(GET_LOGGER));
    printf("* version: %s \r\n", stGetLogger.version);
    printf("* sn: %s \r\n", stGetLogger.sn);
    printf("* time: %s \r\n", stGetLogger.time);
    stGetLogger.loggerLen = ntohl(stGetLogger.loggerLen);
    printf("* logger len: %d \r\n", stGetLogger.loggerLen);

    if (stGetLogger.loggerLen > 0)
    {
        char *data = (char *)malloc(stGetLogger.loggerLen);
        if (data == NULL)
        {
            return -1;
        }

        memset(data, 0, stGetLogger.loggerLen);
        memcpy(data, pRecv->data + sizeof(GET_LOGGER), stGetLogger.loggerLen);
        printf("* %s \r\n", data);
        free(data);
        data = NULL;
    }

    return 0;
}

int DealGetVersion(int iSocketFd, LPPROTOCOL_PACK pRecv)
{
    int iRet = -1;
    unsigned short index = Rand() % 65535;
    
    iRet = TcpSendPack(iSocketFd, 
                       PROTOCOL_HEAD_FLAG, 
                       index, 
                       GET,
                       SYS_VERSION, 
                       0, 
                       NULL);
    if (iRet != 0)
    {
        printf("< Tcp send get version packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    sleep(1);
    
    iRet = TcpRecvPack(iSocketFd, pRecv);

     printf( "TcpRecvPack : nIndex=%d, Pack Flag = %X, Pack Type = %d, Sub Type = %d, Pack Data Length = %d, Return Value = %d\r\n", 
		pRecv->head.index, pRecv->head.flag, pRecv->head.packType, pRecv->head.subType, pRecv->head.dataLen, iRet );
    
    if (iRet != 0)
    {
        printf("< Tcp recv get version response packet failed. %s %d\r\n", MDL);
        return iRet;
    }
    
    if (pRecv->head.index != index
        || pRecv->head.packType != RESPONSE
        || pRecv->head.subType != ACK)
    {
        printf("< Tcp recv get logger response packet error. %s %d\r\n", MDL);    
        return -1;
    }

    if (pRecv->head.dataLen < sizeof(GET_VERSION))
    {
        printf("< Tcp recv get version response size error. %s %d\r\n", MDL); 
        return -1;
    }
    
    GET_VERSION stVersion;
    memset(&stVersion, 0, sizeof(GET_VERSION));
    memcpy(&stVersion, pRecv->data, sizeof(GET_VERSION));
    printf("* version: %s \r\n", stVersion.version);

    return 0;    
}

int DealGetSN(int iSocketFd, LPPROTOCOL_PACK pRecv)
{
    int iRet = -1;
    char SN[16] = {0};
    unsigned short index = Rand() % 65535;
    
    iRet = TcpSendPack(iSocketFd, 
                       PROTOCOL_HEAD_FLAG, 
                       index, 
                       GET,
                       SYS_DEV_INFO, 
                       0, 
                       NULL);
    if (iRet != 0)
    {
        printf("< Tcp send get SN packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    sleep(1);
    
    iRet = TcpRecvPack(iSocketFd, pRecv);

     printf( "TcpRecvPack : nIndex=%d, Pack Flag = %X, Pack Type = %d, Sub Type = %d, Pack Data Length = %d, Return Value = %d\r\n", 
		pRecv->head.index, pRecv->head.flag, pRecv->head.packType, pRecv->head.subType, pRecv->head.dataLen, iRet );
    
    if (iRet != 0)
    {
        printf("< Tcp recv get SN response packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    printf("* pRecv->head.index: %d, index: %d %s %d\r\n", pRecv->head.index, index, MDL);
    
    if (pRecv->head.index != index
        || pRecv->head.packType != RESPONSE
        || pRecv->head.subType != ACK)
    {
        printf("< Tcp recv get SN response packet error. %s %d\r\n", MDL);    
        return -1;
    }

    if (pRecv->head.dataLen < sizeof(SN))
    {
        printf("< Tcp recv get SN response size error. %s %d\r\n", MDL); 
        return -1;
    }
    
    memset(SN, 0, sizeof(SN));
    strncpy(SN, pRecv->data, sizeof(SN) - 1);
    printf("* SN: %s \r\n", SN);

    return 0;    
}

int DealSetDNS(int iSocketFd, LPPROTOCOL_PACK pRecv)
{
    int iRet = -1;
    SET_DNS stSetDns;
    unsigned short index = Rand() % 65535;
    
    memset(&stSetDns, 0, sizeof(SET_DNS));
    strncpy(stSetDns.dns1, "202.96.134.133", sizeof(stSetDns.dns1) - 1);
    strncpy(stSetDns.dns2, "202.96.134.133", sizeof(stSetDns.dns2) - 1);
    
    iRet = TcpSendPack(iSocketFd, 
                       PROTOCOL_HEAD_FLAG, 
                       index, 
                       PUT,
                       SYS_NET_DNS,  
                       sizeof(stSetDns),
                       &stSetDns);
    if (iRet != 0)
    {
        printf("< Tcp send set DNS packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    sleep(1);
    
    iRet = TcpRecvPack(iSocketFd, pRecv);

     printf( "TcpRecvPack : nIndex=%d, Pack Flag = %X, Pack Type = %d, Sub Type = %d, Pack Data Length = %d, Return Value = %d\r\n", 
		pRecv->head.index, pRecv->head.flag, pRecv->head.packType, pRecv->head.subType, pRecv->head.dataLen, iRet );
    
    if (iRet != 0)
    {
        printf("< Tcp recv set DNS response packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    printf("* pRecv->head.index: %d, index: %d %s %d\r\n", pRecv->head.index, index, MDL);
    
    if (pRecv->head.index != index
        || pRecv->head.packType != RESPONSE
        || pRecv->head.subType != ACK)
    {
        printf("< Tcp recv set DNS response packet error. %s %d\r\n", MDL);    
        return -1;
    }

    return 0;     
}

int DealAuthServIpAndPort(int iSocketFd, LPPROTOCOL_PACK pRecv)
{
    int iRet = -1;
    SET_AUTH_SERVER_PORT stAuthIpPort;
    unsigned short index = Rand() % 65535;
    
    memset(&stAuthIpPort, 0, sizeof(SET_AUTH_SERVER_PORT));
    strncpy(stAuthIpPort.ip, "120.24.240.158", sizeof(stAuthIpPort.ip) - 1);
    stAuthIpPort.port = htonl(8888);
    
    iRet = TcpSendPack(iSocketFd, 
                       PROTOCOL_HEAD_FLAG, 
                       index, 
                       PUT,
                       SYS_SERVER_IP_PORT,  
                       sizeof(stAuthIpPort),
                       &stAuthIpPort);
    if (iRet != 0)
    {
        printf("< Tcp send set AUTH IP PORT packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    sleep(1);
    
    iRet = TcpRecvPack(iSocketFd, pRecv);

     printf( "TcpRecvPack : nIndex=%d, Pack Flag = %X, Pack Type = %d, Sub Type = %d, Pack Data Length = %d, Return Value = %d\r\n", 
		pRecv->head.index, pRecv->head.flag, pRecv->head.packType, pRecv->head.subType, pRecv->head.dataLen, iRet );
    
    if (iRet != 0)
    {
        printf("< Tcp recv set AUTH IP PORT response packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    printf("* pRecv->head.index: %d, index: %d %s %d\r\n", pRecv->head.index, index, MDL);
    
    if (pRecv->head.index != index
        || pRecv->head.packType != RESPONSE
        || pRecv->head.subType != ACK)
    {
        printf("< Tcp recv set AUTH IP PORT response packet error. %s %d\r\n", MDL);    
        return -1;
    }

    return 0;         
}

int DealHeartBeat(int iSocketFd, LPPROTOCOL_PACK pRecv)
{
    int iRet = -1;
    unsigned short index = Rand() % 65535;
    
    iRet = TcpSendPack(iSocketFd, 
                       PROTOCOL_HEAD_FLAG,
                       index, 
                       HEARTBEAT,
                       HEART_BEAT, 
                       0, 
                       NULL);
    if (iRet != 0)
    {
        printf("< Tcp send HEARTBEAT packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    sleep(1);
    
    iRet = TcpRecvPack(iSocketFd, pRecv);

     printf( "TcpRecvPack : nIndex=%d, Pack Flag = %X, Pack Type = %d, Sub Type = %d, Pack Data Length = %d, Return Value = %d\r\n", 
		pRecv->head.index, pRecv->head.flag, pRecv->head.packType, pRecv->head.subType, pRecv->head.dataLen, iRet );
    
    if (iRet != 0)
    {
        printf("< Tcp recv HEARTBEAT response packet failed. %s %d\r\n", MDL);
        return iRet;
    }

    printf("* pRecv->head.index: %d, index: %d %s %d\r\n", pRecv->head.index, index, MDL);
    
    if (pRecv->head.index != index
        || pRecv->head.packType != RESPONSE
        || pRecv->head.subType != ACK)
    {
        printf("< Tcp recv HEARTBEAT response packet error. %s %d\r\n", MDL);    
        return -1;
    }

    return 0;            
}

void *ClientThread(void *pArgs)
{
    int iConnectErrCnt = 0;
    int iConnectOkCnt = 0;
    int iSuccssCnt = 0;
    int iErrorCnt = 0;
    int iRet = -1;
    int iClientFd = -1;

    LPPROTOCOL_PACK pRecv = ( LPPROTOCOL_PACK )malloc( sizeof(PROTOCOL_HEAD) + DATA_MAX_LEN );
	if ( pRecv == NULL ) 
	{
		printf("Process client request out of memory, thread exit ...\r\n");
		return NULL;
	}
    
    while (1)
    {
        if (g_reConnectFlag == 1)
        {
            printf("* reconnect flag set. %s %d\r\n", MDL);
            break;    
        }
        
        if (g_exitFlag == 1)
        {
            printf("* exit client thread. %s %d\r\n", MDL);
            break;
        }

        iRet = CreateTcpClient(&iClientFd);
        if (iRet != 0)
        {   
            iConnectErrCnt++;
            printf("* 33310 connect server success(%d), failed(%d). %s %d\r\n", iConnectOkCnt, iConnectErrCnt, MDL);
            sleep(RECONNECT_SERVER_TIME);
            continue;
        }
        else
        {
            iConnectOkCnt++;
            printf("* 33310 connect server success(%d), failed(%d). %s %d\r\n", iConnectOkCnt, iConnectErrCnt, MDL);
        }

        while (1)
        {
            if (g_reConnectFlag == 1)
            {
                printf("* reconnect flag set. %s %d\r\n", MDL);
                break;    
            }
            
            if (g_exitFlag == 1)
            {
                printf("* exit client thread. %s %d\r\n", MDL);
                break;
            }

            iRet = DealHeartBeat(iClientFd, pRecv);
            if (iRet != 0)
            {
                printf("< Deal HEARTBEAT failed. %s %d\r\n", MDL);     
            }
            else
            {
                printf("* Deal HEARTBEAT success. %s %d\r\n", MDL); 
            }

            usleep(500 * 1000);
                
            if (iRet == 0)
            {
                iRet = DealGetLoggerRequest(iClientFd, pRecv);
                if (iRet != 0)
                {
                    printf("< Deal get logger request failed. %s %d\r\n", MDL);                      
                }
                else
                {
                    printf("* Deal get LOGGER request success. %s %d\r\n", MDL);
                }

                usleep(500 * 1000);
            } 

            if (iRet == 0)
            {
                iRet = DealGetVersion(iClientFd, pRecv);
                if (iRet != 0)
                {
                    printf("< Deal get VERSION request failed. %s %d\r\n", MDL);    
                }
                else
                {
                    printf("* Deal get VERSION request success. %s %d\r\n", MDL);   
                }

                usleep(500 * 1000);
            }

            if (iRet == 0)
            {
                iRet = DealHeartBeat(iClientFd, pRecv);
                if (iRet != 0)
                {
                    printf("< Deal HEARTBEAT failed. %s %d\r\n", MDL);     
                }
                else
                {
                    printf("* Deal HEARTBEAT success. %s %d\r\n", MDL); 
                }
                
                usleep(500 * 1000);
            }

            if (iRet == 0)
            {
                iRet = DealAuthServIpAndPort(iClientFd, pRecv);
                if (iRet != 0)
                {
                    printf("< Deal set AUTH SERVER PORT  request failed. %s %d\r\n", MDL);    
                }
                else
                {
                    printf("* Deal set AUTH SERVER PORT request success. %s %d\r\n", MDL);   
                }

                usleep(500 * 1000);
            }

            if (iRet == 0)
            {
                iRet = DealGetSN(iClientFd, pRecv);
                if (iRet != 0)
                {
                    printf("< Deal get SN request failed. %s %d\r\n", MDL);    
                }
                else
                {
                    printf("* Deal get SN request success. %s %d\r\n", MDL);   
                }
                     
                usleep(500 * 1000);
            }

            if (iRet == 0)
            {
                iRet = DealSetDNS(iClientFd, pRecv);
                if (iRet != 0)
                {
                    printf("< Deal SET DNS request failed. %s %d\r\n", MDL);    
                }
                else
                {
                    printf("* Deal SET DNS request success. %s %d\r\n", MDL);   
                }

                usleep(500 * 1000);
            }

            if (iRet == 0)
            {
                iRet = DealHeartBeat(iClientFd, pRecv);
                if (iRet != 0)
                {
                    printf("< Deal HEARTBEAT failed. %s %d\r\n", MDL);     
                }
                else
                {
                    printf("* Deal HEARTBEAT success. %s %d\r\n", MDL); 
                }

                usleep(500 * 1000);
            }

            if (iRet == 0)
            {
                iSuccssCnt++;
                printf("* 33310 test success(%d), failed(%d). wait 5s, reconnect.... %s %d\r\n", iSuccssCnt, iErrorCnt, MDL);
                if (iClientFd >= 0)
                {
                    close(iClientFd);
            	    iClientFd = -1;
                }

                sleep(6);
                break;
            }
            else
            {
                iErrorCnt++;
                printf("* 33310 test success(%d), failed(%d). wait 5s, reconnect.... %s %d\r\n", iSuccssCnt, iErrorCnt, MDL);
                if (iClientFd >= 0)
                {
                    close(iClientFd);
            	    iClientFd = -1;
                }

                sleep(6);
                break;
            }
            
            usleep(100 * 1000);
        }
   
        usleep(100 * 1000);
    }

    if (iClientFd >= 0)
    {
        close(iClientFd);
	    iClientFd = -1;
    }
    
    if (pRecv)
	{
		free(pRecv);
		pRecv = NULL;
	}

    printf("* %s will exit. %s %d\r\n", __FUNCTION__, MDL);
    
    pthread_exit(NULL);

    return NULL;
}

void Quit(int signo)
{
    printf("* receive exit message by CTRL+C. %s %d\r\n", MDL);
    g_exitFlag = 1; 
}


int main()
{
    int iRet = -1;
    pthread_t ClientThreadId = 0;

    signal(SIGINT, Quit);
    
    iRet = pthread_create(&ClientThreadId, NULL, ClientThread, NULL);
    if (iRet != 0) {
        printf("< create net wifi thread failed. %s %d\r\n", MDL);
        return -1;
    }

    while (1)
    {
        usleep(100 * 1000);

        if (g_exitFlag == 1)
        {
            printf("* platform test program exit. %s %d\r\n", MDL);
            break;
        }
    }

    if (g_exitFlag == 1) 
    {
        pthread_join(ClientThreadId, NULL);
    }

    return 0;
}
