#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>
#define MAX_WAIT_TIME   1
#define MAX_NO_PACKETS  1
#define ICMP_HEADSIZE 8 
#define PACKET_SIZE     4096
struct timeval tvsend,tvrecv;	
struct sockaddr_in dest_addr,recv_addr;
int sockfd;
pid_t pid;
char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

//函数定义
void timeout(int signo);
unsigned short cal_chksum(unsigned short *addr,int len);
int pack(int pkt_no,char *sendpacket);
int send_packet(int pkt_no,char *sendpacket);
int recv_packet(int pkt_no,char *recvpacket);
int unpack(int cur_seq,char *buf,int len);
void tv_sub(struct timeval *out,struct timeval *in);
void _CloseSocket();

//#define _USE_DNS

int NetIsOk(char *name)
{     
	double rtt;
	struct hostent *host;
	struct protoent *protocol;
	int i,recv_status;

#ifdef _USE_DNS //如果定义该宏，则可以使用域名进行判断网络连接，例如www.baidu.com
	/* 设置目的地址信息 */
	char hostname[32];
	sprintf(hostname, "%s", name);
	bzero(&dest_addr, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;	

	if ((host = gethostbyname(hostname)) == NULL)
	{
		printf("[NetStatus]  error : Can't get serverhost info!\n");
		return -1;
	}

	bcopy((char*)host->h_addr,(char*)&dest_addr.sin_addr,host->h_length);
#else //如果不使用域名，则只能用ip地址直接发送icmp包，例如谷歌的地址：8.8.8.8
	//dest_addr.sin_addr.s_addr = inet_addr("203.195.154.44");
    dest_addr.sin_addr.s_addr = inet_addr("192.168.0.222");
#endif
	
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{	/* 创建原始ICMP套接字 */
		printf("[NetStatus]  error : socket");
		return -1;
	}

	int iFlag;
	if(iFlag = fcntl(sockfd, F_GETFL,0)<0)
	{
		printf("[NetStatus]  error : fcntl(sockfd,F_GETFL,0)");
		_CloseSocket();
		return -1;
	}
    
	iFlag |= O_NONBLOCK;
	if(iFlag = fcntl(sockfd,F_SETFL,iFlag)<0)
	{
		printf("[NetStatus]  error : fcntl(sockfd,F_SETFL,iFlag )");
		_CloseSocket();
		return -1;
	}

	pid=getpid();
	for(i=0;i<MAX_NO_PACKETS;i++)
	{		
	
		if(send_packet(i,sendpacket)<0)
		{
			printf("[NetStatus]  error : send_packet");
			_CloseSocket();
			return -1;
		}	

		if(recv_packet(i,recvpacket)>0)
		{
			_CloseSocket();
			return 0;
		}
		
	} 
	_CloseSocket();     	
	return -1;
}


int send_packet(int pkt_no,char *sendpacket)
{    
	int packetsize;       
	packetsize=pack(pkt_no,sendpacket); 
	gettimeofday(&tvsend,NULL);    
	if(sendto(sockfd,sendpacket,packetsize,0,(struct sockaddr *)&dest_addr,sizeof(dest_addr) )<0)
	{      
		printf("[NetStatus]  error : sendto error");
		return -1;
	}
	return 1;
}


int pack(int pkt_no,char*sendpacket)
{       
	int i,packsize;
	struct icmp *icmp;
	struct timeval *tval;
	icmp=(struct icmp*)sendpacket;
	icmp->icmp_type=ICMP_ECHO;   //设置类型为ICMP请求报文
	icmp->icmp_code=0;
	icmp->icmp_cksum=0;
	icmp->icmp_seq=pkt_no;
	icmp->icmp_id=pid;			//设置当前进程ID为ICMP标示符
	packsize=ICMP_HEADSIZE+sizeof(struct timeval);
	tval= (struct timeval *)icmp->icmp_data;
	gettimeofday(tval,NULL);
	icmp->icmp_cksum=cal_chksum( (unsigned short *)icmp,packsize); 
	return packsize;
}


unsigned short cal_chksum(unsigned short *addr,int len)
{       
	int nleft=len;
	int sum=0;
	unsigned short *w=addr;
	unsigned short answer=0;
	while(nleft>1)		//把ICMP报头二进制数据以2字节为单位累加起来
	{       
		sum+=*w++;
		nleft-=2;
	}
	if( nleft==1)		//若ICMP报头为奇数个字节,会剩下最后一字节.把最后一个字节视为一个2字节数据的高字节,这个2字节数据的低字节为0,继续累加
	{
		*(unsigned char *)(&answer)=*(unsigned char *)w;
		sum+=answer;
	}
	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);
	answer=~sum;
	return answer;
}


int recv_packet(int pkt_no, char *recvpacket)
{       	
	int n,fromlen;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);
	signal(SIGALRM, timeout);
	fromlen=sizeof(recv_addr);
	alarm(MAX_WAIT_TIME);
	while (1)
	{
		select(sockfd+1, &rfds, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rfds))
		{  
			if ((n = recvfrom(sockfd,recvpacket,PACKET_SIZE,0,(struct sockaddr *)&recv_addr,&fromlen)) <0)
    		{   
			    if(errno==EINTR)
			    {
				    return -1;
			    }
                else
                {
				    perror("recvfrom error");
				    return -2;
                }
      		}
		}
        
		gettimeofday(&tvrecv, NULL); 
		if (unpack(pkt_no, recvpacket, n) == -1)
		{
			continue;
		}
        
		return 1;
	}
}

int unpack(int cur_seq,char *buf,int len)
{    
	int iphdrlen;
	struct ip *ip;
	struct icmp *icmp;
	ip=(struct ip *)buf;
	iphdrlen=ip->ip_hl<<2;		//求ip报头长度,即ip报头的长度标志乘4
	icmp=(struct icmp *)(buf+iphdrlen);		//越过ip报头,指向ICMP报头
	len-=iphdrlen;		//ICMP报头及ICMP数据报的总长度
	if (len < 8)
		return -1;       
	if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid) && (icmp->icmp_seq == cur_seq))
		return 0;	
	else 
        return -1;
}


void timeout(int signo)
{
	printf("Request Timed Out\n");
}

void tv_sub(struct timeval *out,struct timeval *in)
{       
	if( (out->tv_usec-=in->tv_usec)<0)
	{       
		--out->tv_sec;
		out->tv_usec+=1000000;
	}
	out->tv_sec-=in->tv_sec;
}

void _CloseSocket()
{
	close(sockfd);
	sockfd = 0;
}


int main(char argc, char *argv[])
{
    int ret = NetIsOk(argv[1]);
    if (ret == 0)
    {
        printf("net is ok! \r\n");
    }
    else
    {
        printf("net is not ok! \r\n");
    }

    return 0;
}
