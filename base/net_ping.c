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

//��������
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

#ifdef _USE_DNS //�������ú꣬�����ʹ�����������ж��������ӣ�����www.baidu.com
	/* ����Ŀ�ĵ�ַ��Ϣ */
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
#else //�����ʹ����������ֻ����ip��ֱַ�ӷ���icmp��������ȸ�ĵ�ַ��8.8.8.8
	//dest_addr.sin_addr.s_addr = inet_addr("203.195.154.44");
    dest_addr.sin_addr.s_addr = inet_addr("192.168.0.222");
#endif
	
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{	/* ����ԭʼICMP�׽��� */
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
	icmp->icmp_type=ICMP_ECHO;   //��������ΪICMP������
	icmp->icmp_code=0;
	icmp->icmp_cksum=0;
	icmp->icmp_seq=pkt_no;
	icmp->icmp_id=pid;			//���õ�ǰ����IDΪICMP��ʾ��
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
	while(nleft>1)		//��ICMP��ͷ������������2�ֽ�Ϊ��λ�ۼ�����
	{       
		sum+=*w++;
		nleft-=2;
	}
	if( nleft==1)		//��ICMP��ͷΪ�������ֽ�,��ʣ�����һ�ֽ�.�����һ���ֽ���Ϊһ��2�ֽ����ݵĸ��ֽ�,���2�ֽ����ݵĵ��ֽ�Ϊ0,�����ۼ�
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
	iphdrlen=ip->ip_hl<<2;		//��ip��ͷ����,��ip��ͷ�ĳ��ȱ�־��4
	icmp=(struct icmp *)(buf+iphdrlen);		//Խ��ip��ͷ,ָ��ICMP��ͷ
	len-=iphdrlen;		//ICMP��ͷ��ICMP���ݱ����ܳ���
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
