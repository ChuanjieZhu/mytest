
#include <stdio.h>		
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>		/* To set non blocking on socket  */
#include <sys/socket.h>		/* Generic socket calls */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <net/route.h>
#include "dhcp.h"

//Defined in dhtest.c
extern int sock_packet;
extern struct sockaddr_ll ll;
extern int iface;
extern u_int16_t vlan;
extern u_int8_t l3_tos;

extern u_int16_t l2_hdr_size;
extern u_int16_t l3_hdr_size;
extern u_int16_t l4_hdr_size;
extern u_int16_t dhcp_hdr_size;
extern u_int16_t fqdn_n;
extern u_int16_t fqdn_s;

extern u_char dhcp_packet_disc[1514];
extern u_char dhcp_packet_offer[1514];
extern u_char dhcp_packet_request[1514];
extern u_char dhcp_packet_ack[1514];
extern u_char dhcp_packet_release[1514];

extern u_int8_t dhopt_buff[500];
extern u_int32_t dhopt_size;
extern u_int32_t dhcp_xid;
extern u_int32_t bcast_flag;
extern u_int8_t padding_flag;
extern u_int8_t vci_buff[256];
extern u_int8_t hostname_buff[256];
extern u_int8_t fqdn_buff[256];
extern u_int32_t option51_lease_time;
extern u_int32_t port;
extern u_int8_t unicast_flag;
extern u_char *giaddr;
extern u_char *server_addr;

extern struct ethernet_hdr *eth_hg;
extern struct vlan_hdr *vlan_hg; 
extern struct iphdr *iph_g;
extern struct udphdr *uh_g;
extern struct dhcpv4_hdr *dhcph_g;
extern u_int8_t *dhopt_pointer_g;

extern u_char dhmac[ETHER_ADDR_LEN];
extern u_char dmac[ETHER_ADDR_LEN];

extern char dhmac_fname[20];
extern char iface_name[30];
extern char ip_str[128];
extern u_int32_t server_id;
extern u_int32_t option50_ip;
extern u_int8_t dhcp_release_flag;
extern u_int32_t unicast_ip_address;

#if 0
int set_if_ip(char *ip, char *ifname)
{       
    int sock = -1;
    struct ifreq ifr;
	struct sockaddr_in addr;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    /*set ip address, and take affect instant.*/
	if (sock < 0)
	{
		return (-1);
	}
	
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	bzero(&addr, sizeof(struct sockaddr_in));
    
	if (inet_pton(AF_INET, ip, &addr.sin_addr) < 0)
	{		
		close(sock);
        return (-1);
	}
	
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    memcpy(&ifr.ifr_addr, &addr, sizeof(struct sockaddr));
    if (0 > ioctl(sock, SIOCSIFADDR, &ifr))
	{		
		close(sock);
       	return (-1);
	}
	
	close(sock);
    
	return (0);
}   


int del_default_gw()
{
	static char s_acDevName[16][256];
	struct rtentry tagRouteEntry[16];

	char acBuf[1024], *pcBuf = acBuf, acTmpBuf[256];
	int i = 0;
	int j = 0;
	int iNum = 0;
	int iLen = 0;
	int iColDev = 0;
	int iColDst = 1;
	int iColGw = 2;
	int iColMask = 7;
	int iColFlag = 3;
	
	int iFd = open("/proc/net/route", O_RDONLY);
	if(-1 == iFd) 
	{
		return -2;
	}
	
	iLen = read(iFd, acBuf, 1024-1);
	acBuf[iLen] = 0;
	close(iFd);

	memset(tagRouteEntry, 0, sizeof(struct rtentry)*16);

	i = 0;
	while(*pcBuf) 
	{
		while(!((*pcBuf >= 'A' && *pcBuf <= 'Z') || 
                (*pcBuf >= 'a' && *pcBuf <= 'z') || 
                *pcBuf == '\r' || 
                *pcBuf == '\n'))
		{
			pcBuf++;
		}
		
		if(*pcBuf == 0 || *pcBuf == '\r' || *pcBuf == '\n')
		{
			break;
		}
		
		if(strncasecmp(pcBuf, "iface", 5) == 0)
		{
			iColDev = i;
		}
		else if(strncasecmp(pcBuf, "destination", 11) == 0)
		{
			iColDst = i;
		}
		else if(strncasecmp(pcBuf, "gateway", 7) == 0)
		{
			iColGw = i;
		}
		else if(strncasecmp(pcBuf, "mask", 4) == 0)
		{
			iColMask = i;
		}
		else if(strncasecmp(pcBuf, "flags", 5) == 0)
		{
			iColFlag = i;
		}
		
		while((*pcBuf>='A' && *pcBuf<='Z') || 
              (*pcBuf>='a' && *pcBuf<='z'))
		{
			pcBuf++;
		}
		
		i++;
	}
	
	while(*pcBuf == '\r' || *pcBuf == '\n')
	{
		pcBuf++;
	}
	
	i = 0;
	iNum = 0;
	while(*pcBuf) 
	{
		j = 0;
		while(*pcBuf == ' ' || *pcBuf == '\t')
		{
			pcBuf++;
		}
		
		while(*pcBuf && (*pcBuf != ' ' && *pcBuf != '\t' && *pcBuf != '\r' && *pcBuf != '\n'))
		{
			acTmpBuf[j++] = *pcBuf++;
		}
		
		acTmpBuf[j] = 0;
		if(j) 
		{
			if(i == iColDev) 
			{
				strcpy(s_acDevName[iNum], acTmpBuf);
				tagRouteEntry[iNum].rt_dev = s_acDevName[iNum];
			} 
			else 
			{
				unsigned long ulVal = strtoul(acTmpBuf, NULL, 16);
				if(i == iColDst) 
				{
					((struct sockaddr_in*)&tagRouteEntry[iNum].rt_dst)->sin_family = PF_INET;
					((struct sockaddr_in*)&tagRouteEntry[iNum].rt_dst)->sin_addr.s_addr = ulVal;
				} 
				else if(i == iColGw) 
				{
					((struct sockaddr_in*)&tagRouteEntry[iNum].rt_gateway)->sin_family = PF_INET;
					((struct sockaddr_in*)&tagRouteEntry[iNum].rt_gateway)->sin_addr.s_addr = ulVal;
				} 
				else if(i == iColMask) 
				{
					((struct sockaddr_in*)&tagRouteEntry[iNum].rt_genmask)->sin_family = PF_INET;
					((struct sockaddr_in*)&tagRouteEntry[iNum].rt_genmask)->sin_addr.s_addr = ulVal;
				} 
				else if(i == iColFlag)
				{
					tagRouteEntry[iNum].rt_flags = (short)ulVal;
				}
			}
		}
		
		i++;
		if(*pcBuf == 0 || *pcBuf == '\r' || *pcBuf == '\n') 
		{
			if( (tagRouteEntry[iNum].rt_flags & RTF_UP) && ((struct sockaddr_in*)&tagRouteEntry[iNum].rt_dst)->sin_addr.s_addr == 0 ) 
			{
				iNum++;
				if(iNum >= 16)
				{
					break;
				}
			}
			
			while(*pcBuf == '\r' || *pcBuf == '\n')
			{
				pcBuf++;
			}
			
			i = 0;
		}
		
		if(*pcBuf == 0) 
		{
			break;
		}
	}
	
	if(iNum) 
	{
		int iSock = socket(AF_INET, SOCK_DGRAM, 0);
		if(iSock != -1) 
		{
			for(i = 0; i < iNum; i++) 
			{
				struct in_addr tagInAddr;
				tagInAddr.s_addr = ((struct sockaddr_in*)&tagRouteEntry[i].rt_gateway)->sin_addr.s_addr;
				if(ioctl(iSock, SIOCDELRT, &tagRouteEntry[i]) < 0) ;
					//Printf("ioctl SIOCDELRT(%s): %s\r\n",inet_ntoa(tagInAddr),STRERROR_ERRNO);
			}
			close(iSock);
		}
	}

	return 0;
}


int set_if_default_gw(char *gw, char *ifname)
{
	int iFd = -1;
	char *pcIfName = ifname;

	struct ifreq tagIfr;
	struct sockaddr_in tagAddr;	

    struct rtentry tagRouteEntery;   
    struct sockaddr_in *ptagAddr = NULL;  
	
	if (0 > (iFd = socket(AF_INET, SOCK_DGRAM, 0)))
	{
		return -4;
	}
	
	strncpy(tagIfr.ifr_name, pcIfName, IFNAMSIZ);
	tagIfr.ifr_name[IFNAMSIZ - 1] = '\0';
	bzero(&tagAddr, sizeof(struct sockaddr_in));
	
	if (0 > inet_pton(AF_INET, gw, &tagAddr.sin_addr))
	{		
		close(iFd);
		//Printf("inet_pton() error:%s!\r\n", STRERROR_ERRNO);
        return -5;
	}

    //SetDefaultRoute(iFd, pcIfName, &tagAddr.sin_addr.s_addr); 

    memset(&tagRouteEntery, 0, sizeof(struct rtentry));
    ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_dst;
    ptagAddr->sin_family = AF_INET;
    ptagAddr->sin_addr.s_addr = 0;
    ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_gateway;
    ptagAddr->sin_family = AF_INET;
    ptagAddr->sin_addr.s_addr = tagAddr.sin_addr.s_addr;
    ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_genmask;
    ptagAddr->sin_family = AF_INET;
    ptagAddr->sin_addr.s_addr = 0;
   
    tagRouteEntery.rt_dev = pcIfName;
   
    tagRouteEntery.rt_metric = 1;
    tagRouteEntery.rt_window = 0;
    tagRouteEntery.rt_flags = RTF_UP | RTF_GATEWAY;
    if (-1 == ioctl(iFd, SIOCADDRT, &tagRouteEntery))
    {
        if (errno == ENETUNREACH)    /* possibly gateway is over the bridge */
        { 
            /* try adding a route to gateway first */   
            memset(&tagRouteEntery, 0, sizeof(struct rtentry));
            ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_dst;
            ptagAddr->sin_family = AF_INET;
            ptagAddr->sin_addr.s_addr = tagAddr.sin_addr.s_addr;
            ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_gateway; 
            ptagAddr->sin_family = AF_INET;
            ptagAddr->sin_addr.s_addr = 0;
            ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_genmask;
            ptagAddr->sin_family = AF_INET;
            ptagAddr->sin_addr.s_addr = 0xffffffff;

            tagRouteEntery.rt_dev = pcIfName;
            tagRouteEntery.rt_metric = 0;
            tagRouteEntery.rt_flags = RTF_UP | RTF_HOST;
            if (0 == ioctl(iFd, SIOCADDRT, &tagRouteEntery))
            {
                memset(&tagRouteEntery, 0, sizeof(struct rtentry));
                ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_dst;
                ptagAddr->sin_family = AF_INET;
                ptagAddr->sin_addr.s_addr = 0;
                ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_gateway;
                ptagAddr->sin_family = AF_INET;
                ptagAddr->sin_addr.s_addr = tagAddr.sin_addr.s_addr;
                ptagAddr = (struct sockaddr_in *)&tagRouteEntery.rt_genmask;
                ptagAddr->sin_family = AF_INET;
                ptagAddr->sin_addr.s_addr = 0;

                tagRouteEntery.rt_dev = pcIfName;
                tagRouteEntery.rt_metric = 1; 
                tagRouteEntery.rt_window = 0;
                tagRouteEntery.rt_flags = RTF_UP | RTF_GATEWAY;
                if (-1 == ioctl(iFd, SIOCADDRT, &tagRouteEntery))
                {
                    //Printf("ioctl SIOCADDRT error: %s\n", STRERROR_ERRNO);
                    close(iFd);
                    return -8;
                }
            }
			else
			{
				close(iFd);
				return -7;
			}
        }
        else
        {
            //Printf("ioctl SIOCADDRT: error: %s\n", STRERROR_ERRNO);
            close(iFd);
            return -6;   
        }
    }
	
	close(iFd);
	return 0;
}


int set_if_gw(char *gw, char *ifname)
{   
    if (del_default_gw() != 0)
    {
        return (-1);
    }

    if (set_if_default_gw(gw, ifname) != 0)
	{
		return -2;
	}
        
	return 0;
}


int set_if_mask(char *mask, char *ifname)
{
    int iFd = -1;
	struct ifreq tagIfr;
	struct sockaddr_in tagAddr;	

	
	/*set netMask, and take affect instant.*/
	if (0 > (iFd = socket (AF_INET, SOCK_DGRAM, 0)))
	{
		return -4;
	}
	
	strncpy(tagIfr.ifr_name, ifname, IFNAMSIZ);
	tagIfr.ifr_name[IFNAMSIZ - 1] = '\0';
	bzero(&tagAddr, sizeof(struct sockaddr_in));
	
	if(0 > inet_pton(AF_INET, mask, &tagAddr.sin_addr))
	{		
		close(iFd);
        return -5;
	}
    tagAddr.sin_family = AF_INET;
    tagAddr.sin_port = 0;
    memcpy(&tagIfr.ifr_addr, &tagAddr, sizeof(struct sockaddr));
	
    if (0 > ioctl(iFd, SIOCSIFNETMASK, &tagIfr))
	{
		close(iFd);
        return -6;
	}
	
	close(iFd);
	return 0;
}
#endif


int get_if_mac(const char *if_name, char *mac, int length)
{
    char temp[24];
    int sock = -1;
	struct ifreq ifr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        fprintf(stderr, "socket error. %s\r\n", strerror(errno));
        return (-1);
    }
    
	strncpy((char *)&ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
	/* try and grab hardware address of requested interface */
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
    {
		fprintf(stderr, "Error: Could not get hardware address of interface '%s'\n", if_name);
        close(sock);
		return (-1);
    }

    memset(mac, 0, length);
    memcpy(&mac[0], &ifr.ifr_hwaddr.sa_data, 6);

    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X",
					(unsigned char)ifr.ifr_hwaddr.sa_data[0],
					(unsigned char)ifr.ifr_hwaddr.sa_data[1],
					(unsigned char)ifr.ifr_hwaddr.sa_data[2],
					(unsigned char)ifr.ifr_hwaddr.sa_data[3],
					(unsigned char)ifr.ifr_hwaddr.sa_data[4],
					(unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    close(sock);
    
	return (0);
}

/*
 * Opens PF_PACKET socket and return error if socket
 * opens fails
 */

int open_socket()
{
    sock_packet = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sock_packet < 0) 
    {
		printf("Error on creating the socket(%s) %s %d\r\n", 
                    strerror(errno), __FUNCTION__, __LINE__);

        return (-1);
	} 
    
	/* Set link layer parameters */
	ll.sll_family = AF_PACKET;
	ll.sll_protocol = htons(ETH_P_ALL);
	ll.sll_ifindex = iface; 
	ll.sll_hatype = ARPHRD_ETHER;
	ll.sll_pkttype = PACKET_OTHERHOST;
	ll.sll_halen = 6;

	if (bind(sock_packet, (struct sockaddr *)&ll, sizeof(struct sockaddr_ll)) < 0)
	{
        printf("bind sock_packet error, %s, %s %d\r\n", strerror(errno),
            __FUNCTION__, __LINE__);
        
        close(sock_packet);
        return (-1);
	}
    
	return (0);
}

/*
 * Closes PF_PACKET socket
 */
int close_socket()
{
	close(sock_packet);
	return 0;
}

/*
 * Sets the promiscous mode on the interface
 */
int set_promisc() 
{
	int status;
	struct ifreq ifr;

    if (!strlen((const char *)iface_name)) 
    {
		strcpy(iface_name, "eth0");
	}

    strcpy(ifr.ifr_name, iface_name);
	ifr.ifr_flags = (IFF_PROMISC | IFF_UP);
	status = ioctl(sock_packet, SIOCSIFFLAGS, &ifr);

    if(status < 0)
    {
	    printf("Error on setting promisc, %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        return (-1);
	}

    return (0);
}

int clear_promisc() 
{
	int status;
	struct ifreq ifr;

    strcpy(ifr.ifr_name, iface_name);
	ifr.ifr_flags = IFF_UP;

    status = ioctl(sock_packet, SIOCSIFFLAGS, &ifr);
	if(status < 0) 
    {
	    printf("Error on disabling promisc. %s %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        return (-1);
	}
    
	return (0);
}

/*
 * Get address from the interface
 */
u_int32_t get_interface_address()
{
	int status;
	struct ifreq ifr;

	if (!strlen((const char *) iface_name)) 
    {
		strcpy(iface_name, "eth0");
	}
    
	strcpy(ifr.ifr_name, iface_name);
	ifr.ifr_addr.sa_family = AF_INET;
	status = ioctl(sock_packet, SIOCGIFADDR, &ifr);

	if(status < 0)
    {
		printf("Error getting interface address. %s %s %d\r\n", strerror(errno), 
                    __FUNCTION__, __LINE__);
        return (-1);
	}
    
	return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}

/*
 * Sends DHCP packet on the socket. Packet type 
 * is passed as argument. Extended to send ARP and ICMP packets
 */
int send_packet(int pkt_type)
{
	int ret  = -1;

    if (pkt_type == DHCP_MSGDISCOVER) 
    {
		ret = sendto(sock_packet,\
				dhcp_packet_disc,\
				(l2_hdr_size + l3_hdr_size + l4_hdr_size + dhcp_hdr_size + dhopt_size),\
				0,\
				(struct sockaddr *) &ll,\
				sizeof(ll));
	}
    else if (pkt_type == DHCP_MSGREQUEST) 
    {
		ret = sendto(sock_packet,\
				dhcp_packet_request,\
				(l2_hdr_size + l3_hdr_size + l4_hdr_size + dhcp_hdr_size + dhopt_size),\
				0,\
				(struct sockaddr *) &ll,\
				sizeof(ll));
	} 
    else if (pkt_type == DHCP_MSGRELEASE)
    {
		ret = sendto(sock_packet,\
				dhcp_packet_release,\
				(l2_hdr_size + l3_hdr_size + l4_hdr_size + dhcp_hdr_size + dhopt_size),\
				0,\
				(struct sockaddr *) &ll,\
				sizeof(ll));
	} 

	if (ret < 0) 
    {
        printf("Packet send failure (%s), %s %d\r\n", 
                strerror(errno), __FUNCTION__, __LINE__);        
        
		return (-1);
	}  
    
	return (0);
}

/*
 * Receives DHCP packet. Packet type is passed as argument
 * Extended to recv ARP and ICMP packets
 */
int recv_packet(int pkt_type) 
{
	int ret = 0;
    int sock_len = 0;
    int retval = -1;
    int chk_pkt_state = -1;
	fd_set read_fd;
	struct timeval tval;
    
	tval.tv_sec = 5; 
	tval.tv_usec = 0;

	if (pkt_type == DHCP_MSGOFFER) 
    {
		while(tval.tv_sec != 0) 
        {
			FD_ZERO(&read_fd);
			FD_SET(sock_packet, &read_fd);
            
			retval = select(sock_packet + 1, &read_fd, NULL, NULL, &tval);
			if(retval == 0) 
            {
				return DHCP_DISC_RESEND;
			} 
            else if (retval > 0 && FD_ISSET(sock_packet, &read_fd)) 
            {
				bzero(dhcp_packet_offer, sizeof(dhcp_packet_offer));
				sock_len = sizeof(ll);
				ret = recvfrom(sock_packet,\
						    dhcp_packet_offer,\
						    sizeof(dhcp_packet_offer),\
						    0,\
						    (struct sockaddr *)&ll,\
						    (socklen_t *)&sock_len);
			}
            
			if (ret >= 60) 
            {
				chk_pkt_state = check_packet(DHCP_MSGOFFER);
				if(chk_pkt_state == DHCP_OFFR_RCVD) 
                {
					return DHCP_OFFR_RCVD;
				}
			} 
		}
        
		return DHCP_DISC_RESEND;
	}
    else if (pkt_type == DHCP_MSGACK) 
    {
		while (tval.tv_sec != 0) 
        {
			FD_ZERO(&read_fd);
			FD_SET(sock_packet, &read_fd);

            retval = select(sock_packet + 1, &read_fd, NULL, NULL, &tval);
            if (retval == 0) 
            {
				return DHCP_REQ_RESEND;
			} 
            else if (retval > 0 && FD_ISSET(sock_packet, &read_fd))
            {
				bzero(dhcp_packet_ack, sizeof(dhcp_packet_ack));
				sock_len = sizeof(ll);
				ret = recvfrom(sock_packet,\
						dhcp_packet_ack,\
						sizeof(dhcp_packet_ack),\
						0,\
						(struct sockaddr *)&ll,
                        (socklen_t *)&sock_len);
			}
            
			if (ret >= 60) 
            {
				chk_pkt_state = check_packet(DHCP_MSGACK);
                
                if(chk_pkt_state == DHCP_ACK_RCVD) 
                {
					return DHCP_ACK_RCVD;
				}
                else if(chk_pkt_state == DHCP_NAK_RCVD) 
                {
					return DHCP_NAK_RCVD;
				}
			} 
		}
        
		return DHCP_REQ_RESEND;
	}

    return DHCP_DISC_RESEND;
}

/* Debug function - Prints the buffer on HEX format */
int print_buff(u_int8_t *buff, int size)
{
	int tmp;

    fprintf(stdout, "\n---------Buffer data-------\n");

    for(tmp = 0; tmp < size; tmp++) 
    {
		fprintf(stdout, "%02X ", buff[tmp]);
		if((tmp % 16) == 0 && tmp != 0) 
        {
			fprintf(stdout, "\n");
		}
	}
    
	fprintf(stdout, "\n");

    return 0;
}

/* Reset the DHCP option buffer to zero and dhopt_size to zero */
int reset_dhopt_size()
{
	bzero(dhopt_buff, sizeof(dhopt_buff));
	dhopt_size = 0;
	return 0;
}

/*
 * Sets a random DHCP xid
 */
int set_rand_dhcp_xid()
{
	if (dhcp_xid == 0) 
    {
		srand(time(NULL) ^ (getpid() << 16));
		dhcp_xid = rand() % 0xffffffff;
	}
    
	return 0;
}

/*
 * IP checksum function - Calculates the IP checksum
 */
u_int16_t ipchksum(u_int16_t *buff, int words) 
{
	unsigned int sum;
    int i;
	sum = 0;
	for(i = 0;i < words; i++){
		sum = sum + *(buff + i);
	}
	sum = (sum >> 16) + sum;
	return (u_int16_t)~sum;
}


/*
 * TCP/UDP checksum function
 */
u_int16_t l4_sum(u_int16_t *buff, int words, u_int32_t *srcaddr, u_int32_t *dstaddr, u_int16_t proto, u_int16_t len) 
{
	unsigned int sum, last_word = 0;
    int i;

    /* covert u_int32_t to u_int16_t */
    union cv cv_src;
    cv_src.i32_value = *srcaddr;
    u_int16_t *t_srcaddr = &cv_src.i16_value[0];

    union cv cv_dst;
    cv_dst.i32_value = *dstaddr;
    u_int16_t *t_dstaddr = &cv_dst.i16_value[0];
    
	/* Checksum enhancement - Support for odd byte packets */
	if ((htons(len) % 2) == 1) 
    {
		last_word = *((u_int8_t *)buff + ntohs(len) - 1);
		last_word = (htons(last_word) << 8);
		sum = 0;
		for(i = 0;i < words; i++){
			sum = sum + *(buff + i);
		}
		sum = sum + last_word;
		//sum = sum + *(srcaddr) + *(srcaddr + 1) + *(dstaddr) + *(dstaddr + 1) + proto + len;
        sum = sum + *(t_srcaddr) + *(t_srcaddr + 1) + *(t_dstaddr) + *(t_dstaddr + 1) + proto + len;
        sum = (sum >> 16) + sum;
		return ~sum;
	} 
    else
    {
		/* Original checksum function */
		sum = 0;
		for(i = 0;i < words; i++){
			sum = sum + *(buff + i);
		}

		//sum = sum + *(srcaddr) + *(srcaddr + 1) + *(dstaddr) + *(dstaddr + 1) + proto + len;
		sum = sum + *(t_srcaddr) + *(t_srcaddr + 1) + *(t_dstaddr) + *(t_dstaddr + 1) + proto + len;
		sum = (sum >> 16) + sum;
		return ~sum;
	}
}

/*
 * Builds DHCP option53 on dhopt_buff，设置DHCP消息类型。
 */
int build_option53(int msg_type)
{
	if (msg_type == DHCP_MSGDISCOVER) 
    {
		u_int8_t msgtype = DHCP_MESSAGETYPE;
		u_int8_t msglen = 1;
		u_int8_t msg = DHCP_MSGDISCOVER;

		memcpy(dhopt_buff, &msgtype, 1);
        strncpy((char *)(dhopt_buff + 1), (char *)&msglen, 1);
        strncpy((char *)(dhopt_buff + 2), (char *)&msg, 1);
		dhopt_size = dhopt_size + 3; 
	} 
    else if (msg_type == DHCP_MSGREQUEST) 
    {
		u_int8_t msgtype = DHCP_MESSAGETYPE;
		u_int8_t msglen = 1;
		u_int8_t msg = DHCP_MSGREQUEST;

		memcpy(dhopt_buff, &msgtype, 1);
        strncpy((char *) (dhopt_buff + 1), (char *) &msglen, 1);
        strncpy((char *) (dhopt_buff + 2), (char *) &msg, 1);
		dhopt_size = dhopt_size + 3; 
	} 
    else if (msg_type == DHCP_MSGRELEASE) 
    {
		u_int8_t msgtype = DHCP_MESSAGETYPE;
		u_int8_t msglen = 1;
		u_int8_t msg = DHCP_MSGRELEASE;

		memcpy(dhopt_buff, &msgtype, 1);
                strncpy((char *) (dhopt_buff + 1), (char *) &msglen, 1);
                strncpy((char *) (dhopt_buff + 2), (char *) &msg, 1);
		dhopt_size = dhopt_size + 3; 
	}
    
	return 0;
}

/*
 * Builds DHCP option50 on dhopt_buff
 */
int build_option50()
{
	u_int8_t msgtype = DHCP_REQUESTEDIP;
	u_int8_t msglen = 4;
	u_int32_t msg = option50_ip; 

	memcpy((dhopt_buff + dhopt_size), &msgtype, 1);
	memcpy((dhopt_buff + dhopt_size + 1), &msglen, 1);
	memcpy((dhopt_buff + dhopt_size + 2), &msg, 4);
	dhopt_size = dhopt_size + 6; 
	return 0;
}

/*
 * Builds DHCP option51 on dhopt_buff - DHCP lease time requested
 */
int build_option51()
{
	u_int8_t msgtype = DHCP_LEASETIME;
	u_int8_t msglen = 4;
	u_int32_t msg = htonl(option51_lease_time); 

	memcpy((dhopt_buff + dhopt_size), &msgtype, 1);
	memcpy((dhopt_buff + dhopt_size + 1), &msglen, 1);
	memcpy((dhopt_buff + dhopt_size + 2), &msg, 4);
	dhopt_size = dhopt_size + 6; 
	return 0;
}
/*
 * Builds DHCP option54 on dhopt_buff
 */
int build_option54()
{
	u_int8_t msgtype = DHCP_SERVIDENT;
	u_int8_t msglen = 4;
	u_int32_t msg = server_id;

	memcpy((dhopt_buff + dhopt_size), &msgtype, 1);
	memcpy((dhopt_buff + dhopt_size + 1), &msglen, 1);
	memcpy((dhopt_buff + dhopt_size + 2), &msg, 4);
	dhopt_size = dhopt_size + 6; 

    return 0;
}

/*
 * Builds DHCP option55 on dhopt_buff
 */
int build_option55() 
{
	u_int32_t msgtype = DHCP_PARAMREQUEST;
	u_int32_t msglen = 4;
	u_int8_t msg[4] = { 0 };
    
	msg[0] = DHCP_SUBNETMASK;
	msg[1] = DHCP_ROUTER;
	msg[2] = DHCP_DOMAINNAME;
	msg[3] = DHCP_DNS;
	/* msg[4] = DHCP_LOGSERV; */

	memcpy((dhopt_buff + dhopt_size), &msgtype, 1);
	memcpy((dhopt_buff + dhopt_size + 1), &msglen, 1);
	memcpy((dhopt_buff + dhopt_size + 2), msg, 4);
	dhopt_size = dhopt_size + 6; 
    
	return 0;
}

/*
 * Builds DHCP option60 on dhopt_buff
 */
int build_option60_vci()
{
	u_int32_t msgtype = DHCP_CLASSSID;
	u_int32_t msglen = strlen((const char *) vci_buff);

	memcpy((dhopt_buff + dhopt_size), &msgtype, 1);
	memcpy((dhopt_buff + dhopt_size + 1), &msglen, 1);
	memcpy((dhopt_buff + dhopt_size + 2), vci_buff, strlen((const char *) vci_buff));

	dhopt_size = dhopt_size + 2 + strlen((const char *) vci_buff);
    
	return 0;
}

/*
 * Builds DHCP option 12, hostname, on dhopt_buff
 * The DHCP Client Option12 feature specifies the hostname of the client
 */
int build_option12_hostname()
{
	u_int32_t msgtype = DHCP_HOSTNAME;
	u_int32_t msglen = strlen((const char *) hostname_buff);

	memcpy((dhopt_buff + dhopt_size), &msgtype, 1);
	memcpy((dhopt_buff + dhopt_size + 1), &msglen, 1);
	memcpy((dhopt_buff + dhopt_size + 2), hostname_buff, strlen((const char *) hostname_buff));

	dhopt_size = dhopt_size + 2 + strlen((const char *) hostname_buff);
    
	return 0;
}


/*
 * Builds DHCP option 81, fqdn, on dhopt_buff
 */
int build_option81_fqdn()
{
	u_int32_t msgtype = DHCP_FQDN;
	u_int8_t flags = 0;
	u_int8_t rcode1 = 0;
	u_int8_t rcode2 = 0;
	u_int32_t msglen = strlen((const char *) fqdn_buff) + 3;

	if (fqdn_n)
		flags |= FQDN_N_FLAG;
	if (fqdn_s)
		flags |= FQDN_S_FLAG;

	memcpy((dhopt_buff + dhopt_size), &msgtype, 1);
	memcpy((dhopt_buff + dhopt_size + 1), &msglen, 1);
	memcpy((dhopt_buff + dhopt_size + 2), &flags, 1);
	memcpy((dhopt_buff + dhopt_size + 3), &rcode1, 1);
	memcpy((dhopt_buff + dhopt_size + 4), &rcode2, 1);
	memcpy((dhopt_buff + dhopt_size + 5), fqdn_buff, strlen((const char *) fqdn_buff));

	dhopt_size = dhopt_size + 2 + msglen;
    
	return 0;
}

/*
 * Builds DHCP end of option on dhopt_buff
 */
int build_optioneof()
{
	u_int8_t eof = 0xff;
    
	memcpy((dhopt_buff + dhopt_size), &eof, 1);
	dhopt_size = dhopt_size + 1; 
    
	return 0;
}

/*
 * Build DHCP packet. Packet type is passed as argument
 */
int build_dhpacket(int pkt_type)
{
    u_int16_t buf[1024] = {0};
	u_int32_t dhcp_packet_size = dhcp_hdr_size + dhopt_size;

    if (!dhcp_release_flag)
    {
		u_char dmac_tmp[ETHER_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
		memcpy(dmac, dmac_tmp, ETHER_ADDR_LEN);
	}
    
	if (pkt_type == DHCP_MSGDISCOVER) 
    {
		if (vlan == 0) 
        {       
            struct ethernet_hdr t_ethhdr;
            memset(&t_ethhdr, 0, sizeof(struct ethernet_hdr));
            struct ethernet_hdr *ethhdr = &t_ethhdr;

			memcpy(ethhdr->ether_dhost, dmac, ETHER_ADDR_LEN);      /* 目的mac地址 */
            memcpy(ethhdr->ether_shost, dhmac, ETHER_ADDR_LEN);     /* 源mac地址 */
            ethhdr->ether_type = htons(ETHERTYPE_IP);               /* 类型 */

            memcpy(dhcp_packet_disc, (char *)ethhdr, sizeof(struct ethernet_hdr));
		}
        else
        {
            struct vlan_hdr t_vhdr;
            memset(&t_vhdr, 0, sizeof(struct vlan_hdr));
            struct vlan_hdr *vhdr = &t_vhdr;

			memcpy(vhdr->vlan_dhost, dmac, ETHER_ADDR_LEN);
			memcpy(vhdr->vlan_shost, dhmac, ETHER_ADDR_LEN);
			vhdr->vlan_tpi = htons(ETHERTYPE_VLAN);
			vhdr->vlan_priority_c_vid = htons(vlan);
			vhdr->vlan_len = htons(ETHERTYPE_IP);
            
            memcpy(dhcp_packet_disc, (char *)vhdr, sizeof(struct vlan_hdr));  
		}

		if (padding_flag && dhcp_packet_size < MINIMUM_PACKET_SIZE) 
        {
			memset(dhopt_buff + dhopt_size, 0, MINIMUM_PACKET_SIZE - dhcp_packet_size);
			dhopt_size += MINIMUM_PACKET_SIZE - dhcp_packet_size;
		}

        /* 填充ip头部  */
        struct iphdr t_iphdr;
        memset(&t_iphdr, 0, sizeof(t_iphdr));
        struct iphdr *iph = &t_iphdr;

        iph->version = 4;
		iph->ihl = 5;
		iph->tos = l3_tos;
        /* ip数据报总长度 = ip头20字节 + udp头8字节 + dhcp头 + dhcp选项数据 */
		iph->tot_len = htons(l3_hdr_size +  l4_hdr_size + dhcp_hdr_size + dhopt_size);  
		iph->id = 0;
		iph->frag_off = 0;
		iph->ttl = 64;
		iph->protocol = 17;
		iph->check = 0; // Filled later;
		if (unicast_flag)
			iph->saddr = unicast_ip_address;
		else
			iph->saddr = inet_addr("0.0.0.0");
        
		iph->daddr = inet_addr((const char *)server_addr);

        memset(buf, 0, sizeof(buf));
        memcpy(buf, iph, sizeof(struct iphdr));
        iph->check = ipchksum((u_int16_t *)buf, iph->ihl << 1);
        
        memcpy(dhcp_packet_disc + l2_hdr_size, (char *)iph, sizeof(struct iphdr));

        /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

        /* 填充udp头部 */
        struct udphdr t_uh;
        memset(&t_uh, 0, sizeof(struct udphdr));
        struct udphdr *uh = &t_uh;

        uh->source = htons(port + 1);
		uh->dest = htons(port);
		u_int16_t l4_proto = 17;
		u_int16_t l4_len = (l4_hdr_size + dhcp_hdr_size + dhopt_size);
		uh->len = htons(l4_len);
		uh->check = 0; /* UDP checksum will be done after dhcp header*/

        memcpy(dhcp_packet_disc + l2_hdr_size + l3_hdr_size, (char *)uh, sizeof(struct udphdr));
        
        /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

        /* 填充dhcp头部 */
		struct dhcpv4_hdr t_dhpointer;
        memset(&t_dhpointer, 0, sizeof(t_dhpointer));
        struct dhcpv4_hdr *dhpointer = &t_dhpointer;

        dhpointer->dhcp_opcode = DHCP_REQUEST;
		dhpointer->dhcp_htype = ARPHRD_ETHER;
		dhpointer->dhcp_hlen = ETHER_ADDR_LEN;
		dhpointer->dhcp_hopcount = 0;
		dhpointer->dhcp_xid = htonl(dhcp_xid);
		dhpointer->dhcp_secs = 0;
		dhpointer->dhcp_flags = bcast_flag;
		if (unicast_flag)
			dhpointer->dhcp_cip = unicast_ip_address;
		else
			dhpointer->dhcp_cip = 0;
		dhpointer->dhcp_yip = 0;
		dhpointer->dhcp_sip = 0;
		dhpointer->dhcp_gip = inet_addr((const char *) giaddr);
		memcpy(dhpointer->dhcp_chaddr, dhmac, ETHER_ADDR_LEN);
		/*dhpointer->dhcp_sname 
		  dhpointer->dhcp_file*/
		dhpointer->dhcp_magic = htonl(DHCP_MAGIC);

        memcpy(dhcp_packet_disc + l2_hdr_size + l3_hdr_size + l4_hdr_size, (char *)dhpointer, 
                sizeof(struct dhcpv4_hdr));      
        
		/* DHCP option buffer is copied here to DHCP packet */
		u_char *dhopt_pointer = (u_char *)(dhcp_packet_disc + l2_hdr_size + l3_hdr_size + l4_hdr_size + dhcp_hdr_size);
		memcpy(dhopt_pointer, dhopt_buff, dhopt_size);    

		/* UDP checksum is done here */
        memset(buf, 0, sizeof(buf));
        memcpy(buf, dhcp_packet_disc + l2_hdr_size + l3_hdr_size, 
               dhcp_hdr_size + dhopt_size + l4_hdr_size);
        
        uh->check = l4_sum((u_int16_t *)buf, 
                    ((dhcp_hdr_size + dhopt_size + l4_hdr_size) / 2), 
                    (u_int32_t *)&iph->saddr, 
                    (u_int32_t *)&iph->daddr,
                    htons(l4_proto), 
                    htons(l4_len)); 
    }

    /* ----------------------------------------------------------------------------------- */
    
	if(pkt_type == DHCP_MSGREQUEST) 
    {   
		if (vlan == 0) 
        {      
            struct ethernet_hdr t_ethhdr;
            memset(&t_ethhdr, 0, sizeof(struct ethernet_hdr));
            struct ethernet_hdr *ethhdr = &t_ethhdr;

			memcpy(ethhdr->ether_dhost, dmac, ETHER_ADDR_LEN);
			memcpy(ethhdr->ether_shost, dhmac, ETHER_ADDR_LEN);
			ethhdr->ether_type = htons(ETHERTYPE_IP);
            
            memcpy(dhcp_packet_request, (char *)ethhdr, sizeof(struct ethernet_hdr));
		} 
        else 
        {
            struct vlan_hdr t_vhdr;
            memset(&t_vhdr, 0, sizeof(struct vlan_hdr));
            struct vlan_hdr *vhdr = &t_vhdr;

			memcpy(vhdr->vlan_dhost, dmac, ETHER_ADDR_LEN);
			memcpy(vhdr->vlan_shost, dhmac, ETHER_ADDR_LEN);
			vhdr->vlan_tpi = htons(ETHERTYPE_VLAN);
			vhdr->vlan_priority_c_vid = htons(vlan);
			vhdr->vlan_len = htons(ETHERTYPE_IP);
            
            memcpy(dhcp_packet_request, (char *)vhdr, sizeof(struct vlan_hdr));
		}

		if (padding_flag && dhcp_packet_size < MINIMUM_PACKET_SIZE) 
        {
			memset(dhopt_buff + dhopt_size, 0, MINIMUM_PACKET_SIZE - dhcp_packet_size);
			dhopt_size += MINIMUM_PACKET_SIZE - dhcp_packet_size;
		}

        /* ----------------------------------------------------------------------------------- */
        
        struct iphdr t_iph;
        memset(&t_iph, 0, sizeof(t_iph));
        struct iphdr *iph = &t_iph;

        iph->version = 4;
		iph->ihl = 5;
		iph->tos = l3_tos;
		iph->tot_len = htons(l3_hdr_size +  l4_hdr_size + dhcp_hdr_size + dhopt_size);  
		iph->id = 0;
		iph->frag_off = 0;
		iph->ttl = 64;
		iph->protocol = 17;
		iph->check = 0; // Filled later;
		if (unicast_flag)
			iph->saddr = unicast_ip_address;
		else
			iph->saddr = inet_addr("0.0.0.0");
		iph->daddr = inet_addr((const char *) server_addr);

        memset(buf, 0, sizeof(buf));
        memcpy(buf, iph, sizeof(struct iphdr));
        iph->check = ipchksum((u_int16_t *)buf, iph->ihl << 1);
        memcpy(dhcp_packet_request + l2_hdr_size, (char *)iph, sizeof(struct iphdr));

        /* ----------------------------------------------------------------------------------- */

        struct udphdr t_uh;
        memset(&t_uh, 0, sizeof(t_uh));
        struct udphdr *uh = &t_uh;

		uh->source = htons(port + 1);
		uh->dest = htons(port);
		u_int16_t l4_proto = 17;
		u_int16_t l4_len = (l4_hdr_size + dhcp_hdr_size + dhopt_size);
		uh->len = htons(l4_len);
		uh->check = 0; /* UDP checksum will be done after building dhcp header*/

        memcpy(dhcp_packet_request + l2_hdr_size + l3_hdr_size, (char *)uh, sizeof(struct udphdr));

        /* ----------------------------------------------------------------------------------- */

        struct dhcpv4_hdr t_dhpointer;
        memset(&t_dhpointer, 0, sizeof(struct dhcpv4_hdr));
        struct dhcpv4_hdr *dhpointer = &t_dhpointer;

        dhpointer->dhcp_opcode = DHCP_REQUEST;
		dhpointer->dhcp_htype = ARPHRD_ETHER;
		dhpointer->dhcp_hlen = ETHER_ADDR_LEN;
		dhpointer->dhcp_hopcount = 0;
		dhpointer->dhcp_xid = htonl(dhcp_xid);
		dhpointer->dhcp_secs = 0;
		dhpointer->dhcp_flags = bcast_flag;
		if (unicast_flag)
			dhpointer->dhcp_cip = unicast_ip_address;
		else
			dhpointer->dhcp_cip = 0;
		dhpointer->dhcp_yip = 0;
		dhpointer->dhcp_sip = 0;
		dhpointer->dhcp_gip = inet_addr((const char *) giaddr);
		memcpy(dhpointer->dhcp_chaddr, dhmac, ETHER_ADDR_LEN);
		/*dhpointer->dhcp_sname 
		  dhpointer->dhcp_file*/
		dhpointer->dhcp_magic = htonl(DHCP_MAGIC);

        memcpy(dhcp_packet_request + l2_hdr_size + l3_hdr_size + l4_hdr_size, (char *)dhpointer, 
                sizeof(struct dhcpv4_hdr)); 
        
		/* DHCP option buffer is copied here to DHCP packet */
		u_char *dhopt_pointer = (u_char *)(dhcp_packet_request + l2_hdr_size + l3_hdr_size + l4_hdr_size + dhcp_hdr_size);
		memcpy(dhopt_pointer, dhopt_buff, dhopt_size);

        /* ----------------------------------------------------------------------------------- */
    
        memset(buf, 0, sizeof(buf));
        memcpy(buf, dhcp_packet_request + l2_hdr_size + l3_hdr_size, 
                dhcp_hdr_size + dhopt_size + l4_hdr_size);
        
        uh->check = l4_sum((u_int16_t *)buf, 
                    ((dhcp_hdr_size + dhopt_size + l4_hdr_size) / 2), 
                    (u_int32_t *)&iph->saddr, 
                    (u_int32_t *)&iph->daddr, 
                    htons(l4_proto), 
                    htons(l4_len));         
	}

#if 0    
	if (pkt_type == DHCP_MSGRELEASE)
    {
		if(vlan == 0) 
        {
			struct ethernet_hdr *ethhdr = (struct ethernet_hdr *)dhcp_packet_release;
			memcpy(ethhdr->ether_dhost, dmac, ETHER_ADDR_LEN);
			memcpy(ethhdr->ether_shost, dhmac, ETHER_ADDR_LEN);
			ethhdr->ether_type = htons(ETHERTYPE_IP);
		}
        else
        {
			struct vlan_hdr *vhdr = (struct vlan_hdr *)dhcp_packet_release;
			memcpy(vhdr->vlan_dhost, dmac, ETHER_ADDR_LEN);
			memcpy(vhdr->vlan_shost, dhmac, ETHER_ADDR_LEN);
			vhdr->vlan_tpi = htons(ETHERTYPE_VLAN);
			vhdr->vlan_priority_c_vid = htons(vlan);
			vhdr->vlan_len = htons(ETHERTYPE_IP);
		}
        
		//print_buff(dhcp_packet_disc, sizeof(struct ethernet_hdr));

		if (padding_flag && dhcp_packet_size < MINIMUM_PACKET_SIZE) {
			memset(dhopt_buff + dhopt_size, 0, MINIMUM_PACKET_SIZE - dhcp_packet_size);
			dhopt_size += MINIMUM_PACKET_SIZE - dhcp_packet_size;
		}

		struct iphdr *iph = (struct iphdr *)(dhcp_packet_release + l2_hdr_size);
		iph->version = 4;
		iph->ihl = 5;
		iph->tos = l3_tos;
		iph->tot_len = htons(l3_hdr_size +  l4_hdr_size + dhcp_hdr_size + dhopt_size);  
		iph->id = 0;
		iph->frag_off = 0;
		iph->ttl = 64;
		iph->protocol = 17;
		iph->check = 0; // Filled later;
		iph->saddr = option50_ip; //inet_addr("0.0.0.0");
		iph->daddr = server_id; //inet_addr("255.255.255.255");
		iph->check = ipchksum((u_int16_t *)(dhcp_packet_release + l2_hdr_size), iph->ihl << 1);

		struct udphdr *uh = (struct udphdr *) (dhcp_packet_release + l2_hdr_size + l3_hdr_size);
		uh->source = htons(port + 1);
		uh->dest = htons(port);
		u_int16_t l4_proto = 17;
		u_int16_t l4_len = (l4_hdr_size + dhcp_hdr_size + dhopt_size);
		uh->len = htons(l4_len);
		uh->check = 0; /* UDP checksum will be done after dhcp header*/

		struct dhcpv4_hdr *dhpointer = (struct dhcpv4_hdr *)(dhcp_packet_release + l2_hdr_size + l3_hdr_size + l4_hdr_size);
		dhpointer->dhcp_opcode = DHCP_REQUEST;
		dhpointer->dhcp_htype = ARPHRD_ETHER;
		dhpointer->dhcp_hlen = ETHER_ADDR_LEN;
		dhpointer->dhcp_hopcount = 0;
		dhpointer->dhcp_xid = htonl(dhcp_xid);
		dhpointer->dhcp_secs = 0;
		dhpointer->dhcp_flags = bcast_flag;
		dhpointer->dhcp_cip = option50_ip;
		dhpointer->dhcp_yip = 0;
		dhpointer->dhcp_sip = 0;
		dhpointer->dhcp_gip = inet_addr((const char *) giaddr);
		memcpy(dhpointer->dhcp_chaddr, dhmac, ETHER_ADDR_LEN);
		/*dhpointer->dhcp_sname 
		  dhpointer->dhcp_file*/
		dhpointer->dhcp_magic = htonl(DHCP_MAGIC);

		/* DHCP option buffer is copied here to DHCP packet */
		u_char *dhopt_pointer = (u_char *)(dhcp_packet_release + l2_hdr_size + l3_hdr_size + l4_hdr_size + dhcp_hdr_size);
		memcpy(dhopt_pointer, dhopt_buff, dhopt_size);

		/* UDP checksum is done here */
		uh->check = l4_sum((u_int16_t *) (dhcp_packet_release + l2_hdr_size + l3_hdr_size), ((dhcp_hdr_size + dhopt_size + l4_hdr_size) / 2), (u_int16_t *)&iph->saddr, (u_int16_t *)&iph->daddr, htons(l4_proto), htons(l4_len)); 
	}
#endif
    
    return 0;
}

/*
 * Checks whether received packet is DHCP offer/ACK/NACK/ARP/ICMP
 * and retunrs the received packet type
 */
int check_packet(int pkt_type) 
{
	if(pkt_type == DHCP_MSGOFFER && vlan != 0) 
    {
		map_all_layer_ptr(DHCP_MSGOFFER);
		if((ntohs(vlan_hg->vlan_priority_c_vid) & VLAN_VIDMASK) == vlan && ntohs(vlan_hg->vlan_tpi) == ETHERTYPE_VLAN && iph_g->protocol == 17 && uh_g->source == htons(port) && uh_g->dest == htons(port + 1)) {
			if(*(dhopt_pointer_g + 2) == DHCP_MSGOFFER && htonl(dhcph_g->dhcp_xid) == dhcp_xid) {
				return DHCP_OFFR_RCVD;
			} else {
				return UNKNOWN_PACKET;
			}
		} else {
			return UNKNOWN_PACKET;
		}
	} else if (pkt_type == DHCP_MSGACK && vlan != 0){
		map_all_layer_ptr(DHCP_MSGACK);
		if((ntohs(vlan_hg->vlan_priority_c_vid) & VLAN_VIDMASK) == vlan && ntohs(vlan_hg->vlan_tpi) == ETHERTYPE_VLAN && iph_g->protocol == 17 && uh_g->source == htons(port) && uh_g->dest == htons(port + 1)) {
			if(*(dhopt_pointer_g + 2) == DHCP_MSGACK && htonl(dhcph_g->dhcp_xid) == dhcp_xid) {
				return DHCP_ACK_RCVD;
			} else if(*(dhopt_pointer_g + 2) == DHCP_MSGNACK && htonl(dhcph_g->dhcp_xid) == dhcp_xid){
				return DHCP_NAK_RCVD;
			} else {
				return UNKNOWN_PACKET;
			}

		} else {
			return UNKNOWN_PACKET;
		}
	}
    else if (pkt_type == DHCP_MSGOFFER) 
    {   
		map_all_layer_ptr(DHCP_MSGOFFER);

        if(eth_hg->ether_type == htons(ETHERTYPE_IP) && iph_g->protocol == 17 && uh_g->source == htons(port) && uh_g->dest == htons(port + 1)) 
        {
			if(*(dhopt_pointer_g + 2) == DHCP_MSGOFFER && htonl(dhcph_g->dhcp_xid) == dhcp_xid) 
            {
				return DHCP_OFFR_RCVD;
			} 
            else 
            {
				return UNKNOWN_PACKET;
			}
		} 
        else 
        {
			return UNKNOWN_PACKET;
		}

	}
    else if (pkt_type == DHCP_MSGACK)
    {
		map_all_layer_ptr(DHCP_MSGACK);
        
		if (eth_hg->ether_type == htons(ETHERTYPE_IP) && 
            iph_g->protocol == 17 && 
            uh_g->source == htons(port) && 
            uh_g->dest == htons(port + 1)) 
        {
			if(*(dhopt_pointer_g + 2) == DHCP_MSGACK && htonl(dhcph_g->dhcp_xid) == dhcp_xid) 
            {
				return DHCP_ACK_RCVD;
			} 
            else if(*(dhopt_pointer_g + 2) == DHCP_MSGNACK && htonl(dhcph_g->dhcp_xid) == dhcp_xid) 
            {
				return DHCP_NAK_RCVD;
			} 
            else 
            {
				return UNKNOWN_PACKET;
			}
		} 
        else 
        {
			return UNKNOWN_PACKET;
		}
	}

    return UNKNOWN_PACKET;
}

/*
 * Sets the server ip and offerered ip on serv_id, option50_ip
 * from the DHCP offer packet
 */
int set_serv_id_opt50()
{    
	map_all_layer_ptr(DHCP_MSGOFFER);

	option50_ip = dhcph_g->dhcp_yip;        /* offerered ip */

	while(*(dhopt_pointer_g) != DHCP_END) 
    {
        u_int32_t tmp = 0;
        
		if(*(dhopt_pointer_g) == DHCP_SERVIDENT) 
        {       
            memcpy(&tmp, dhopt_pointer_g + 2, sizeof(tmp));
			memcpy(&server_id, &tmp, 4);        /* server ip */
		}
        
		dhopt_pointer_g = dhopt_pointer_g + *(dhopt_pointer_g + 1) + 2;
	}
    
	return 0;
}


int set_option54(char *ip_str)
{
    if (ip_str == NULL)
    {
        return (-1);
    }
    
    struct in_addr dst;

    memset(&dst, 0, sizeof(dst));
    inet_pton(AF_INET, ip_str, (void *)&dst);
    server_id = dst.s_addr;

    return (0);
}

int set_option50(char *ip_str)
{
    if (ip_str == NULL)
    {
        return (-1);
    }
    
    struct in_addr dst;

    memset(&dst, 0, sizeof(dst));
    inet_pton(AF_INET, ip_str, (void *)&dst);
    option50_ip = dst.s_addr;

    return (0);
}


int parse_dhcp_offer()
{
    u_int16_t tmp;
    
    map_all_layer_ptr(DHCP_MSGOFFER);

    fprintf(stdout, "\nDHCP offer details\n");
	fprintf(stdout, "----------------------------------------------------------\n");
	fprintf(stdout, "DHCP offered IP from server - %s\n", get_ip_str(dhcph_g->dhcp_yip));
	fprintf(stdout, "Next server IP(Probably TFTP server) - %s\n", get_ip_str(dhcph_g->dhcp_sip));

    if (dhcph_g->dhcp_gip) 
    {
		fprintf(stdout, "DHCP Relay agent IP - %s\n", get_ip_str(dhcph_g->dhcp_gip));
	}

    while (*(dhopt_pointer_g) != DHCP_END)
    {   
        u_int32_t tmp_data = 0;
        
		switch (*(dhopt_pointer_g))
        {
			case DHCP_SERVIDENT:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
                fprintf(stdout, "DHCP server  - %s\n", get_ip_str(tmp_data));
				break;

			case DHCP_LEASETIME:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
				fprintf(stdout, "Lease time - %d Days %d Hours %d Minutes\n", \
						(ntohl(tmp_data)) / (3600 * 24), \
						((ntohl(tmp_data)) % (3600 * 24)) / 3600, \
						(((ntohl(tmp_data)) % (3600 * 24)) % 3600) / 60); 
   
				break;

			case DHCP_SUBNETMASK:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
				fprintf(stdout, "Subnet mask - %s\n", get_ip_str(tmp_data));
				break;

			case DHCP_ROUTER:
				for(tmp = 0; tmp < (*(dhopt_pointer_g + 1) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy((char *)&tmp_data, (dhopt_pointer_g + 2 + (tmp * 4)), sizeof(tmp_data));
					fprintf(stdout, "Router/gateway - %s\n", get_ip_str(tmp_data));
				}
				break;

			case DHCP_DNS:
				for(tmp = 0; tmp < ((*(dhopt_pointer_g + 1)) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy(&tmp_data, (dhopt_pointer_g + 2 + (tmp * 4)), sizeof(tmp_data));
					fprintf(stdout, "DNS server - %s\n", get_ip_str(tmp_data));
				}
				break;

			case DHCP_FQDN:
			{
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 1, sizeof(tmp_data));
                
				/* Minus 3 beacause 3 bytes are used to flags, rcode1 and rcode2 */
				u_int32_t size = tmp_data - 3;
                
				/* Plus 2 to add string terminator */
				u_char fqdn_client_name[size + 1];

				/* Plus 5 to reach the beginning of the string */
				memcpy(fqdn_client_name, dhopt_pointer_g + 5, size);
				fqdn_client_name[size] = '\0';

				fprintf(stdout, "FQDN Client name - %s\n", fqdn_client_name);
			}
		}

		dhopt_pointer_g = dhopt_pointer_g + *(dhopt_pointer_g + 1) + 2;
	}

	fprintf(stdout, "----------------------------------------------------------\n\n");
    
	return 0;
}


int parse_dhcp_ack(DHCP_RESULT *p_result, char *if_name)
{
    u_int16_t tmp;

    memset(p_result, 0, sizeof(DHCP_RESULT));
    
    map_all_layer_ptr(DHCP_MSGACK);

	fprintf(stdout, "\nDHCP ack details\n");
	fprintf(stdout, "----------------------------------------------------------\n");
	fprintf(stdout, "DHCP offered IP from server - %s\n", get_ip_str(dhcph_g->dhcp_yip));
	fprintf(stdout, "Next server IP(Probably TFTP server) - %s\n", get_ip_str(dhcph_g->dhcp_sip));

    if (dhcph_g->dhcp_gip) 
    {
		fprintf(stdout, "DHCP Relay agent IP - %s\n", get_ip_str(dhcph_g->dhcp_gip));
	}

    strncpy(p_result->if_name, if_name, sizeof(p_result->if_name) - 1);
    strncpy(p_result->fixed_address, get_ip_str(dhcph_g->dhcp_yip), sizeof(p_result->fixed_address) - 1);
    
    while (*(dhopt_pointer_g) != DHCP_END)
    {   
        u_int32_t tmp_data = 0;
        
		switch (*(dhopt_pointer_g))
        {
			case DHCP_SERVIDENT:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
                strncpy(p_result->serv_ident, get_ip_str(tmp_data), sizeof(p_result->serv_ident) - 1);
                fprintf(stdout, "DHCP server  - %s\n", get_ip_str(tmp_data));
				break;

			case DHCP_LEASETIME:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
                p_result->lease_time = ntohl(tmp_data);
				fprintf(stdout, "Lease time - %d Days %d Hours %d Minutes\n", \
						(ntohl(tmp_data)) / (3600 * 24), \
						((ntohl(tmp_data)) % (3600 * 24)) / 3600, \
						(((ntohl(tmp_data)) % (3600 * 24)) % 3600) / 60); 
   
				break;

			case DHCP_SUBNETMASK:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
                strncpy(p_result->sub_netmask, get_ip_str(tmp_data), sizeof(p_result->sub_netmask) - 1);
				fprintf(stdout, "Subnet mask - %s\n", get_ip_str(tmp_data));
				break;

			case DHCP_ROUTER:
				for(tmp = 0; tmp < (*(dhopt_pointer_g + 1) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy((char *)&tmp_data, (dhopt_pointer_g + 2 + (tmp * 4)), sizeof(tmp_data));
                    strncpy(p_result->router, get_ip_str(tmp_data), sizeof(p_result->router) - 1);
					fprintf(stdout, "Router/gateway - %s\n", get_ip_str(tmp_data));
				}
				break;

			case DHCP_DNS:
				for(tmp = 0; tmp < ((*(dhopt_pointer_g + 1)) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy(&tmp_data, (dhopt_pointer_g + 2 + (tmp * 4)), sizeof(tmp_data));
					fprintf(stdout, "DNS server - %s\n", get_ip_str(tmp_data));
				}
				break;

			case DHCP_FQDN:
			{
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 1, sizeof(tmp_data));
                
				/* Minus 3 beacause 3 bytes are used to flags, rcode1 and rcode2 */
				u_int32_t size = tmp_data - 3;
                
				/* Plus 2 to add string terminator */
				u_char fqdn_client_name[size + 1];

				/* Plus 5 to reach the beginning of the string */
				memcpy(fqdn_client_name, dhopt_pointer_g + 5, size);
				fqdn_client_name[size] = '\0';

				fprintf(stdout, "FQDN Client name - %s\n", fqdn_client_name);
			}
		}

		dhopt_pointer_g = dhopt_pointer_g + *(dhopt_pointer_g + 1) + 2;
	}

	fprintf(stdout, "----------------------------------------------------------\n\n");

    printf("fixed_address: %s \r\n", p_result->fixed_address);
    printf("serv_ident: %s \r\n", p_result->serv_ident);
    printf("router: %s \r\n", p_result->router);
    printf("sub_netmask: %s \r\n", p_result->sub_netmask);
    printf("lease_time: %d \r\n", p_result->lease_time);
    
    if (p_result->fixed_address[0] != '\0' &&
        p_result->serv_ident[0] != '\0' &&
        p_result->router[0] != '\0' &&
        p_result->sub_netmask[0] != '\0' &&
        p_result->lease_time > 0) 
    {
        printf("parse dhcp ack success. %s %d\r\n", __FUNCTION__, __LINE__);
        return (0);    
    }

    printf("parse dhcp ack fail. %s %d\r\n", __FUNCTION__, __LINE__);

    clear_promisc();

    close_socket();
        
    return (-1);
}


/*
 * Prints the DHCP offer/ack info
 */
int print_dhinfo(int pkt_type) 
{
	u_int16_t tmp;
    
	if(pkt_type == DHCP_MSGOFFER) 
    {
		map_all_layer_ptr(DHCP_MSGOFFER);

		fprintf(stdout, "\nDHCP offer details\n");
		fprintf(stdout, "----------------------------------------------------------\n");
		fprintf(stdout, "DHCP offered IP from server - %s\n", get_ip_str(dhcph_g->dhcp_yip));
		fprintf(stdout, "Next server IP(Probably TFTP server) - %s\n", get_ip_str(dhcph_g->dhcp_sip));

        if(dhcph_g->dhcp_gip) 
        {
			fprintf(stdout, "DHCP Relay agent IP - %s\n", get_ip_str(dhcph_g->dhcp_gip));
		}
	}
    else if( pkt_type == DHCP_MSGACK) 
    {
		map_all_layer_ptr(DHCP_MSGACK);

		fprintf(stdout, "\nDHCP ack details\n");
		fprintf(stdout, "----------------------------------------------------------\n");
		fprintf(stdout, "DHCP offered IP from server - %s\n", get_ip_str(dhcph_g->dhcp_yip));
		fprintf(stdout, "Next server IP(Probably TFTP server) - %s\n", get_ip_str(dhcph_g->dhcp_sip));

        if(dhcph_g->dhcp_gip) {
			fprintf(stdout, "DHCP Relay agent IP - %s\n", get_ip_str(dhcph_g->dhcp_gip));
		}
    }
    
	while(*(dhopt_pointer_g) != DHCP_END)
    {   
        u_int32_t tmp_data = 0;
        
		switch(*(dhopt_pointer_g))
        {
			case DHCP_SERVIDENT:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
                fprintf(stdout, "DHCP server  - %s\n", get_ip_str(tmp_data));
				break;

			case DHCP_LEASETIME:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
                //printf("lease time: %d %s %d\r\n", ntohl(tmp_data), __FUNCTION__, __LINE__);
				fprintf(stdout, "Lease time - %d Days %d Hours %d Minutes\n", \
						(ntohl(tmp_data)) / (3600 * 24), \
						((ntohl(tmp_data)) % (3600 * 24)) / 3600, \
						(((ntohl(tmp_data)) % (3600 * 24)) % 3600) / 60); 
   
				break;

			case DHCP_SUBNETMASK:
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 2, sizeof(tmp_data));
				fprintf(stdout, "Subnet mask - %s\n", get_ip_str(tmp_data));
				break;

			case DHCP_ROUTER:
				for(tmp = 0; tmp < (*(dhopt_pointer_g + 1) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy((char *)&tmp_data, (dhopt_pointer_g + 2 + (tmp * 4)), sizeof(tmp_data));
					fprintf(stdout, "Router/gateway - %s\n", get_ip_str(tmp_data));
				}
				break;

			case DHCP_DNS:
				for(tmp = 0; tmp < ((*(dhopt_pointer_g + 1)) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy(&tmp_data, (dhopt_pointer_g + 2 + (tmp * 4)), sizeof(tmp_data));
					fprintf(stdout, "DNS server - %s\n", get_ip_str(tmp_data));
				}
				break;

			case DHCP_FQDN:
			{
                tmp_data = 0;
                memcpy((char *)&tmp_data, dhopt_pointer_g + 1, sizeof(tmp_data));
                
				/* Minus 3 beacause 3 bytes are used to flags, rcode1 and rcode2 */
				u_int32_t size = tmp_data - 3;
                
				/* Plus 2 to add string terminator */
				u_char fqdn_client_name[size + 1];

				/* Plus 5 to reach the beginning of the string */
				memcpy(fqdn_client_name, dhopt_pointer_g + 5, size);
				fqdn_client_name[size] = '\0';

				fprintf(stdout, "FQDN Client name - %s\n", fqdn_client_name);
			}
		}

		dhopt_pointer_g = dhopt_pointer_g + *(dhopt_pointer_g + 1) + 2;
	}

	fprintf(stdout, "----------------------------------------------------------\n\n");
	return 0;
}

/*
 * Function maps all pointers on OFFER/ACK/ARP/ICMP packet
 */
int map_all_layer_ptr(int pkt_type)
{
	if(pkt_type == DHCP_MSGOFFER && vlan != 0) 
    {
		vlan_hg = (struct vlan_hdr *)dhcp_packet_offer; 
		iph_g = (struct iphdr *)(dhcp_packet_offer + l2_hdr_size);
		uh_g = (struct udphdr *)(dhcp_packet_offer + l2_hdr_size + l3_hdr_size);
		dhcph_g = (struct dhcpv4_hdr *)(dhcp_packet_offer + l2_hdr_size + l3_hdr_size + l4_hdr_size);
		dhopt_pointer_g = (u_int8_t *)(dhcp_packet_offer + l2_hdr_size + l3_hdr_size + l4_hdr_size + sizeof(struct dhcpv4_hdr));
	} 
    else if(pkt_type == DHCP_MSGOFFER && vlan == 0) 
    {
		eth_hg = (struct ethernet_hdr *)dhcp_packet_offer;
		iph_g = (struct iphdr *)(dhcp_packet_offer + l2_hdr_size);
		uh_g = (struct udphdr *)(dhcp_packet_offer + l2_hdr_size + l3_hdr_size);
		dhcph_g = (struct dhcpv4_hdr *)(dhcp_packet_offer + l2_hdr_size + l3_hdr_size + l4_hdr_size);
		dhopt_pointer_g = (u_int8_t *)(dhcp_packet_offer + l2_hdr_size + l3_hdr_size + l4_hdr_size + sizeof(struct dhcpv4_hdr));
	} 
    else if(pkt_type == DHCP_MSGACK && vlan != 0) 
    {
		vlan_hg = (struct vlan_hdr *)dhcp_packet_ack; 
		iph_g = (struct iphdr *)(dhcp_packet_ack + l2_hdr_size);
		uh_g = (struct udphdr *)(dhcp_packet_ack + l2_hdr_size + l3_hdr_size);
		dhcph_g = (struct dhcpv4_hdr *)(dhcp_packet_ack + l2_hdr_size + l3_hdr_size + l4_hdr_size);
		dhopt_pointer_g = (u_int8_t *)(dhcp_packet_ack + l2_hdr_size + l3_hdr_size + l4_hdr_size + sizeof(struct dhcpv4_hdr));
	}
    else if(pkt_type == DHCP_MSGACK && vlan == 0) 
	{
        eth_hg = (struct ethernet_hdr *)dhcp_packet_ack;
		iph_g = (struct iphdr *)(dhcp_packet_ack + l2_hdr_size);
		uh_g = (struct udphdr *)(dhcp_packet_ack + l2_hdr_size + l3_hdr_size);
		dhcph_g = (struct dhcpv4_hdr *)(dhcp_packet_ack + l2_hdr_size + l3_hdr_size + l4_hdr_size);
		dhopt_pointer_g = (u_int8_t *)(dhcp_packet_ack + l2_hdr_size + l3_hdr_size + l4_hdr_size + sizeof(struct dhcpv4_hdr));
	}
    
	return 0;
}


char *get_ip_str(u_int32_t ip)
{
	struct in_addr src;
	src.s_addr = ip;
    
    memset(ip_str, 0, sizeof(ip_str));
    
	inet_ntop(AF_INET, ((struct sockaddr_in *)&src), ip_str, sizeof(ip_str));
    
	return ip_str;
}


int g_iptos = IP_TOS;
int g_ipheadsize = IP_HEAD_SIZE;
int g_udpheadsize = UDP_HEAD_SIZE;
int g_etherheadsize = ETHER_HEAD_SIZE;
char *g_serverip = SERVER_IP;

static struct DhclientLeases g_ethxLeases;
static struct DhclientLeases g_raxLeases;

int dhclient_check_file(char *pcFileName, int *piFileType, int *piFileLen)
{	
    int ret = -1;
    int iFileType = 2;
    int iFileLen = 0;
    struct stat st;

    if(pcFileName == NULL || *pcFileName == '\0') 
    {
        ret = -1;
    }
    else 
    {
        if(stat(pcFileName, &st) == 0) 
        {
            iFileLen = st.st_size;
        }

        if(iFileLen)
        {
            ret = 0;
            if(strstr(pcFileName, ".jpg"))
            {
                iFileType = 1;
            }
            else if(strstr(pcFileName, ".bmp")) 
            {
                iFileType = 0;
            }
        }
    }
	
    if(ret == 0)
    {
        if(piFileType)
        {
            *piFileType = iFileType;
        }
        if(piFileLen)
        {
            *piFileLen = iFileLen;
        }
    }	

    return ret;
    
}

void dhclient_initethxdefleases()
{
    memset(&g_ethxLeases, 0, sizeof(struct DhclientLeases));
    
    strncpy(g_ethxLeases.if_name, "eth0", sizeof(g_ethxLeases.if_name) - 1);
    strncpy(g_ethxLeases.fixed_address, "192.168.0.230", sizeof(g_ethxLeases.fixed_address) - 1);
    strncpy(g_ethxLeases.subnet_mask, "255.255.255.0", sizeof(g_ethxLeases.subnet_mask) - 1);
    strncpy(g_ethxLeases.routers, "192.168.0.1", sizeof(g_ethxLeases.routers) - 1);
    strncpy(g_ethxLeases.server_address, "192.168.0.1", sizeof(g_ethxLeases.server_address) - 1);
    strncpy(g_ethxLeases.broadcast_address, "192.168.0.255", sizeof(g_ethxLeases.broadcast_address) - 1);
    
    g_ethxLeases.lease_time = 43200;
    g_ethxLeases.renewal_time = 21600;
    g_ethxLeases.rebinding_time = 37800;

    long timenow = time(NULL);
    dhclient_conver_time(timenow, g_ethxLeases.renew);
    dhclient_conver_time(timenow, g_ethxLeases.rebind);
    dhclient_conver_time(timenow, g_ethxLeases.expire);
}

void dhclient_initraxdefleases()
{
    memset(&g_raxLeases, 0, sizeof(DHCP_LEASES));
    
    strncpy(g_raxLeases.if_name, "ra0", sizeof(g_raxLeases.if_name) - 1);
    strncpy(g_raxLeases.fixed_address, "192.168.0.230", sizeof(g_raxLeases.fixed_address) - 1);
    strncpy(g_raxLeases.subnet_mask, "255.255.255.0", sizeof(g_raxLeases.subnet_mask) - 1);
    strncpy(g_raxLeases.routers, "192.168.0.1", sizeof(g_raxLeases.routers) - 1);
    strncpy(g_raxLeases.server_address, "192.168.0.1", sizeof(g_raxLeases.server_address) - 1);
    strncpy(g_raxLeases.broadcast_address, "192.168.0.255", sizeof(g_raxLeases.broadcast_address) - 1);
    
    g_raxLeases.lease_time = 43200;
    g_raxLeases.renewal_time = 21600;
    g_raxLeases.rebinding_time = 37800;

    long timenow = time(NULL);
    dhclient_conver_time(timenow, g_raxLeases.renew);
    dhclient_conver_time(timenow, g_raxLeases.rebind);
    dhclient_conver_time(timenow, g_raxLeases.expire);
}


int create_ethx_leases(char *path)
{
    int iRet = 0;
    char tmpBuf[256] = {0};

    TiXmlDocument *ethxLeasesXml = new TiXmlDocument();
    TiXmlDeclaration declaration("1.0", "gb18030", "no");
    ethxLeasesXml->InsertEndChild(declaration);

    TiXmlElement *rootElement = new TiXmlElement("HHProtocol");
    ethxLeasesXml->LinkEndChild(rootElement);

    TiXmlElement *ethxLeasesElement = new TiXmlElement("ethx_leases");
    rootElement->LinkEndChild(ethxLeasesElement);

    /* if_name */
    TiXmlElement *ifNameElement = new TiXmlElement("if_name");
    ethxLeasesElement->LinkEndChild(ifNameElement);
    TiXmlText *ifNameContent = new TiXmlText(g_ethxLeases.if_name);
    ifNameElement->LinkEndChild(ifNameContent);

    /* fixed_address */
    TiXmlElement *fixedAddressElement = new TiXmlElement("fixed_address");
    ethxLeasesElement->LinkEndChild(fixedAddressElement);
    TiXmlText *fixedAddressContent = new TiXmlText(g_ethxLeases.fixed_address);
    fixedAddressElement->LinkEndChild(fixedAddressContent);

    /* subnet_mask */
    TiXmlElement *subnetMaskElement = new TiXmlElement("subnet_mask");
    ethxLeasesElement->LinkEndChild(subnetMaskElement);
    TiXmlText *subnetMaskContent = new TiXmlText(g_ethxLeases.subnet_mask);
    subnetMaskElement->LinkEndChild(subnetMaskContent);

    /* routers */
    TiXmlElement *routersElement = new TiXmlElement("routers");
    ethxLeasesElement->LinkEndChild(routersElement);
    TiXmlText *routersContent = new TiXmlText(g_ethxLeases.routers);
    routersElement->LinkEndChild(routersContent);

    /* server_address */
    TiXmlElement *serverAddressElement = new TiXmlElement("server_address");
    ethxLeasesElement->LinkEndChild(serverAddressElement);
    TiXmlText *serverAddressContent = new TiXmlText(g_ethxLeases.server_address);
    serverAddressElement->LinkEndChild(serverAddressContent);

    /* broadcast_address */
    TiXmlElement *broadcastAddressElement = new TiXmlElement("broadcast_address");
    ethxLeasesElement->LinkEndChild(broadcastAddressElement);
    TiXmlText *broadcastAddressContent = new TiXmlText(g_ethxLeases.broadcast_address);
    broadcastAddressElement->LinkEndChild(broadcastAddressContent);

    /* lease_time */
    TiXmlElement *leaseTimeElement = new TiXmlElement("lease_time");
    ethxLeasesElement->LinkEndChild(leaseTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_ethx_leases.lease_time);
    TiXmlText *leaseTimeContent = new TiXmlText(tmpBuf);
    leaseTimeElement->LinkEndChild(leaseTimeContent);

    /* renewal_time */
    TiXmlElement *renewalTimeElement = new TiXmlElement("renewal_time");
    ethxLeasesElement->LinkEndChild(renewalTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_ethx_leases.renewal_time);
    TiXmlText *renewalTimeContent = new TiXmlText(tmpBuf);
    renewalTimeElement->LinkEndChild(renewalTimeContent);

    /* rebinding_time */
    TiXmlElement *rebindingTimeElement = new TiXmlElement("rebinding_time");
    ethxLeasesElement->LinkEndChild(rebindingTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_ethx_leases.rebinding_time);
    TiXmlText *rebindingTimeContent = new TiXmlText(tmpBuf);
    rebindingTimeElement->LinkEndChild(rebindingTimeContent);

    /* renew */
    TiXmlElement *renewElement = new TiXmlElement("renew");
    ethxLeasesElement->LinkEndChild(renewElement);
    TiXmlText *renewContent = new TiXmlText(g_ethxLeases.renew);
    renewElement->LinkEndChild(renewContent);

    /* rebind */
    TiXmlElement *rebindElement = new TiXmlElement("rebind");
    ethxLeasesElement->LinkEndChild(rebindElement);
    TiXmlText *rebindContent = new TiXmlText(g_ethxLeases.rebind);
    rebindElement->LinkEndChild(rebindContent);

    /* expire */
    TiXmlElement *expireElement = new TiXmlElement("expire");
    ethxLeasesElement->LinkEndChild(expireElement);
    TiXmlText *expireContent = new TiXmlText(g_ethxLeases.expire);
    expireElement->LinkEndChild(expireContent);

    if (ethxLeasesXml->SaveFile(path))
    {
		iRet = 0;
	}
	else
	{
		iRet = -1;
	}

	ethxLeasesXml->Clear();
	delete ethxLeasesXml;
    
    return iRet;
}

int create_rax_leases(char *path)
{
    int iRet = 0;
    char tmpBuf[256] = {0};

    TiXmlDocument *raxLeasesXml = new TiXmlDocument();
    TiXmlDeclaration declaration("1.0", "gb18030", "no");
    raxLeasesXml->InsertEndChild(declaration);

    TiXmlElement *rootElement = new TiXmlElement("HHProtocol");
    raxLeasesXml->LinkEndChild(rootElement);

    TiXmlElement *raxLeasesElement = new TiXmlElement("rax_leases");
    rootElement->LinkEndChild(raxLeasesElement);

    /* if_name */
    TiXmlElement *ifNameElement = new TiXmlElement("if_name");
    raxLeasesElement->LinkEndChild(ifNameElement);
    TiXmlText *ifNameContent = new TiXmlText(g_raxLeases.if_name);
    ifNameElement->LinkEndChild(ifNameContent);

    /* fixed_address */
    TiXmlElement *fixedAddressElement = new TiXmlElement("fixed_address");
    raxLeasesElement->LinkEndChild(fixedAddressElement);
    TiXmlText *fixedAddressContent = new TiXmlText(g_raxLeases.fixed_address);
    fixedAddressElement->LinkEndChild(fixedAddressContent);

    /* subnet_mask */
    TiXmlElement *subnetMaskElement = new TiXmlElement("subnet_mask");
    raxLeasesElement->LinkEndChild(subnetMaskElement);
    TiXmlText *subnetMaskContent = new TiXmlText(g_raxLeases.subnet_mask);
    subnetMaskElement->LinkEndChild(subnetMaskContent);

    /* routers */
    TiXmlElement *routersElement = new TiXmlElement("routers");
    raxLeasesElement->LinkEndChild(routersElement);
    TiXmlText *routersContent = new TiXmlText(g_raxLeases.routers);
    routersElement->LinkEndChild(routersContent);

    /* server_address */
    TiXmlElement *serverAddressElement = new TiXmlElement("server_address");
    raxLeasesElement->LinkEndChild(serverAddressElement);
    TiXmlText *serverAddressContent = new TiXmlText(g_raxLeases.server_address);
    serverAddressElement->LinkEndChild(serverAddressContent);

    /* broadcast_address */
    TiXmlElement *broadcastAddressElement = new TiXmlElement("broadcast_address");
    raxLeasesElement->LinkEndChild(broadcastAddressElement);
    TiXmlText *broadcastAddressContent = new TiXmlText(g_raxLeases.broadcast_address);
    broadcastAddressElement->LinkEndChild(broadcastAddressContent);

    /* lease_time */
    TiXmlElement *leaseTimeElement = new TiXmlElement("lease_time");
    raxLeasesElement->LinkEndChild(leaseTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_rax_leases.lease_time);
    TiXmlText *leaseTimeContent = new TiXmlText(tmpBuf);
    leaseTimeElement->LinkEndChild(leaseTimeContent);

    /* renewal_time */
    TiXmlElement *renewalTimeElement = new TiXmlElement("renewal_time");
    raxLeasesElement->LinkEndChild(renewalTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_rax_leases.renewal_time);
    TiXmlText *renewalTimeContent = new TiXmlText(tmpBuf);
    renewalTimeElement->LinkEndChild(renewalTimeContent);

    /* rebinding_time */
    TiXmlElement *rebindingTimeElement = new TiXmlElement("rebinding_time");
    raxLeasesElement->LinkEndChild(rebindingTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_rax_leases.rebinding_time);
    TiXmlText *rebindingTimeContent = new TiXmlText(tmpBuf);
    rebindingTimeElement->LinkEndChild(rebindingTimeContent);

    /* renew */
    TiXmlElement *renewElement = new TiXmlElement("renew");
    raxLeasesElement->LinkEndChild(renewElement);
    TiXmlText *renewContent = new TiXmlText(g_raxLeases.renew);
    renewElement->LinkEndChild(renewContent);

    /* rebind */
    TiXmlElement *rebindElement = new TiXmlElement("rebind");
    raxLeasesElement->LinkEndChild(rebindElement);
    TiXmlText *rebindContent = new TiXmlText(g_raxLeases.rebind);
    rebindElement->LinkEndChild(rebindContent);

    /* expire */
    TiXmlElement *expireElement = new TiXmlElement("expire");
    raxLeasesElement->LinkEndChild(expireElement);
    TiXmlText *expireContent = new TiXmlText(g_raxLeases.expire);
    expireElement->LinkEndChild(expireContent);

    if (raxLeasesXml->SaveFile(path))
    {
		iRet = 0;
	}
	else
	{
		iRet = -1;
	}

	raxLeasesXml->Clear();
	delete raxLeasesXml;
    
    return iRet;
}


void set_ethx_leases(struct DhclientLeases *pLeases)
{
    int ret = -1;

    if (memcmp(&g_ethxLeases, pLeases, sizeof(DHCP_LEASES)))
    {
        memcpy(&g_ethxLeases, pLeases, sizeof(DHCP_LEASES));
        ret = create_ethx_leases((char *)ETHX_LEASES_PATH);
        if(ret)
        {
            printf("set %s failed. %s %d\r\n", ETHX_LEASES_PATH, __FUNCTION__, __LINE__);
        }
    }
}


void set_rax_leases(struct DhclientLeases *pLeases)
{
    int ret = -1;

    if (memcmp(&g_raxLeases, pLeases, sizeof(DHCP_LEASES)))
    {
        memcpy(&g_raxLeases, pLeases, sizeof(DHCP_LEASES));
        ret = create_rax_leases((char *)RAX_LEASES_PATH);
        if(ret)
        {
            printf("set %s failed. %s %d\r\n", RAX_LEASES_PATH, __FUNCTION__, __LINE__);
        }
    }
}


int load_ethx_leases(char *path, struct DhclientLeases *pLeases)
{
    const char * xmlFile = path;
	TiXmlDocument doc;                              

    if (pLeases == NULL)
    {
        printf("pLeases is null. %s %d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
	if (doc.LoadFile(xmlFile))
	{
        //doc.Print();
	}
	else
	{
        printf("load %s failed. %s %d\r\n", path, __FUNCTION__, __LINE__);
		return -1;
	}
    
	TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* EthxLeasesDataElement = rootElement->FirstChildElement();

	for(; EthxLeasesDataElement != NULL; EthxLeasesDataElement = EthxLeasesDataElement->NextSiblingElement())
	{
		TiXmlElement* contactElement = EthxLeasesDataElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "if_name", strlen("if_name")) == 0)
					{
						strncpy(pLeases->if_name, contactValue, sizeof(pLeases->if_name) - 1);
					}
                    else if(strncmp(contactType, "fixed_address", strlen("fixed_address")) == 0)
                    {
                        strncpy(pLeases->fixed_address, contactValue, sizeof(pLeases->fixed_address) - 1);
                    }
					else if(strncmp(contactType, "subnet_mask", strlen("subnet_mask")) == 0)
					{
						strncpy(pLeases->subnet_mask, contactValue, sizeof(pLeases->subnet_mask) - 1);
					}
					else if(strncmp(contactType, "routers", strlen("routers")) == 0)
					{
						strncpy(pLeases->routers, contactValue, sizeof(pLeases->routers) - 1);
					}
					else if(strncmp(contactType, "server_address", strlen("server_address")) == 0)
					{
						strncpy(pLeases->server_address, contactValue, sizeof(pLeases->server_address) - 1);
					}
					else if(strncmp(contactType, "broadcast_address", strlen("broadcast_address")) == 0)
					{
						strncpy(pLeases->broadcast_address, contactValue, sizeof(pLeases->broadcast_address) - 1);
					}
					else if(strncmp(contactType, "lease_time", strlen("lease_time")) == 0)
					{
                        pLeases->lease_time = atoi(contactValue);
					}
					else if(strncmp(contactType, "renewal_time", strlen("renewal_time")) == 0)
					{
                        pLeases->renewal_time = atoi(contactValue);		
                    }
					else if(strncmp(contactType, "rebinding_time", strlen("rebinding_time")) == 0)
					{
						pLeases->rebinding_time = atoi(contactValue);
					}
                    else if(strncmp(contactType, "renew", strlen("renew")) == 0)
                    {
                        strncpy(pLeases->renew, contactValue, sizeof(pLeases->renew) - 1);
                    }
                    else if(strncmp(contactType, "rebind", strlen("rebind")) == 0)
					{
                        strncpy(pLeases->rebind, contactValue, sizeof(pLeases->rebind) - 1);
					}
                    else if(strncmp(contactType, "expire", strlen("expire")) == 0)
					{
                        strncpy(pLeases->expire, contactValue, sizeof(pLeases->expire) - 1);
					}
				}
			}
		}
	} 

	return 0;
}

int load_rax_leases(char *path, struct DhclientLeases *pLeases)
{
    const char * xmlFile = path;
	TiXmlDocument doc;                              

    if (pLeases == NULL)
    {
        printf("pLeases is null. %s %d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
	if (doc.LoadFile(xmlFile))
	{
        //doc.Print();
	}
	else
	{
        printf("load %s failed. %s %d\r\n", path, __FUNCTION__, __LINE__);
		return -1;
	}
    
	TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* EthxLeasesDataElement = rootElement->FirstChildElement();

	for(; EthxLeasesDataElement != NULL; EthxLeasesDataElement = EthxLeasesDataElement->NextSiblingElement())
	{
		TiXmlElement* contactElement = EthxLeasesDataElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "if_name", strlen("if_name")) == 0)
					{
						strncpy(pLeases->if_name, contactValue, sizeof(pLeases->if_name) - 1);
					}
                    else if(strncmp(contactType, "fixed_address", strlen("fixed_address")) == 0)
                    {
                        strncpy(pLeases->fixed_address, contactValue, sizeof(pLeases->fixed_address) - 1);
                    }
					else if(strncmp(contactType, "subnet_mask", strlen("subnet_mask")) == 0)
					{
						strncpy(pLeases->subnet_mask, contactValue, sizeof(pLeases->subnet_mask) - 1);
					}
					else if(strncmp(contactType, "routers", strlen("routers")) == 0)
					{
						strncpy(pLeases->routers, contactValue, sizeof(pLeases->routers) - 1);
					}
					else if(strncmp(contactType, "server_address", strlen("server_address")) == 0)
					{
						strncpy(pLeases->server_address, contactValue, sizeof(pLeases->server_address) - 1);
					}
					else if(strncmp(contactType, "broadcast_address", strlen("broadcast_address")) == 0)
					{
						strncpy(pLeases->broadcast_address, contactValue, sizeof(pLeases->broadcast_address) - 1);
					}
					else if(strncmp(contactType, "lease_time", strlen("lease_time")) == 0)
					{
                        pLeases->lease_time = atoi(contactValue);
					}
					else if(strncmp(contactType, "renewal_time", strlen("renewal_time")) == 0)
					{
                        pLeases->renewal_time = atoi(contactValue);		
                    }
					else if(strncmp(contactType, "rebinding_time", strlen("rebinding_time")) == 0)
					{
						pLeases->rebinding_time = atoi(contactValue);
					}
                    else if(strncmp(contactType, "renew", strlen("renew")) == 0)
                    {
                        strncpy(pLeases->renew, contactValue, sizeof(pLeases->renew) - 1);
                    }
                    else if(strncmp(contactType, "rebind", strlen("rebind")) == 0)
					{
                        strncpy(pLeases->rebind, contactValue, sizeof(pLeases->rebind) - 1);
					}
                    else if(strncmp(contactType, "expire", strlen("expire")) == 0)
					{
                        strncpy(pLeases->expire, contactValue, sizeof(pLeases->expire) - 1);
					}
				}
			}
		}
	} 

	return 0;
}

int read_ethx_leases()
{
    int ret = -1;

    ret = dhclient_check_file((char *)ETHX_LEASES_PATH, NULL, NULL);
    if (ret)
    {
        printf("%s not exist %s %d\r\n", ETHX_LEASES_PATH, __FUNCTION__, __LINE__);
        ret = create_ethx_leases((char *)ETHX_LEASES_PATH);
        if (ret)
        {
		    printf("%s create fail. %s %d\r\n", ETHX_LEASES_PATH, __FUNCTION__, __LINE__);   
        }		
    }
    else
    {
        ret = load_ethx_leases((char *)ETHX_LEASES_PATH, &g_ethxLeases);
        if (ret)
        {
           printf("load %s failed %s %d\r\n", ETHX_LEASES_PATH, __FUNCTION__, __LINE__); 
        }
    }
    
    return ret; 
}

int read_rax_leases()
{
    int ret = -1;

    ret = dhclient_check_file((char *)RAX_LEASES_PATH, NULL, NULL);
    if (ret)
    {
        printf("%s not exist %s %d\r\n", RAX_LEASES_PATH, __FUNCTION__, __LINE__);
        ret = create_rax_leases((char *)RAX_LEASES_PATH);
        if (ret)
        {
		    printf("%s create fail. %s %d\r\n", RAX_LEASES_PATH, __FUNCTION__, __LINE__);   
        }		
    }
    else
    {
        ret = load_rax_leases((char *)RAX_LEASES_PATH, &g_raxLeases);
        if (ret)
        {
           printf("load %s failed %s %d\r\n", RAX_LEASES_PATH, __FUNCTION__, __LINE__); 
        }
    }
    
    return ret; 
}

void get_ethx_leases(struct DhclientLeases *pLeases)
{
    memset(pLeases, 0, sizeof(struct DhclientLeases));
    memcpy(pLeases, &g_ethxLeases, sizeof(struct DhclientLeases));
}


void get_rax_leases(struct DhclientLeases *pLeases)
{
    memset(pLeases, 0, sizeof(struct DhclientLeases));
    memcpy(pLeases, &g_raxLeases, sizeof(struct DhclientLeases));
}


int get_interface_type(char *if_name)
{
    if (strstr(if_name, "eth") != NULL) 
    {
        return 0;
    }

    if (strstr(if_name, "ra") != NULL 
        || strstr(if_name, "wlan") != NULL)
    {
        return 1;
    }

    return -1;
}

int dhclient_getifacetype(char *if_name)
{
    if (strstr(if_name, "eth") != NULL) 
    {
        return 0;
    }

    if (strstr(if_name, "ra") != NULL 
        || strstr(if_name, "wlan") != NULL)
    {
        return 1;
    }

    return -1;
}

int dhclient_get_ifleases(char *if_name, struct DhclientLeases *leases)
{
    int ret = dhclient_getifacetype(if_name);

    /* ethx */
    if (ret == 0) 
    {
        get_ethx_leases(leases);
        return 0;
    }

    /* rax or wlanx */
    if (ret == 1)
    {
        get_rax_leases(leases);
        return 0;
    }

    return (-1);
}

int dhclient_read_leases()
{
    int ret = -1;

    ret = read_ethx_leases();
    if (ret)
    {
        printf("read ethx leases failure. %s %d\r\n", __FUNCTION__, __LINE__);     
    }

    ret = read_rax_leases();
    if (ret)
    {
        printf("read rax leases failure. %s %d\r\n", __FUNCTION__, __LINE__);     
    }

    return ret;
}


int dhclient_init_leases()
{
    int ret = -1;

    dhclient_initethxdefleases();
    dhclient_initraxdefleases();

    mkdir("/var/dhcp", 0777);

    ret = read_leases();
    if (ret)
    {
        printf("init leases read leases failure. %s %d\r\n", __FUNCTION__, __LINE__); 
    }
    
    return ret;
}

int dhclient(char *if_name)
{
    int ret = -1;
    char time_buf[20] = {0};
    struct dhcp_leases leases;
    
    init_dhclient_leases();
   
    TimeToDatetime(time(NULL), time_buf);
    
    ret = get_interface_leases(if_name, &leases);
    if (ret)
    {
        printf("get interface %s leases failure. %s %d\r\n", if_name, __FUNCTION__, __LINE__);
        return (-1);
    }
    
    if (strcmp(leases.if_name, if_name) != 0)
    {
        printf("leases record's interface name is not equal the request interface name.  \r\n");
        ret = dhclient_discover(if_name);
    }
    else
    {
        if (strcmp(time_buf, leases.expire) < 0) 
        {
            printf("dhcp leases don't expire, send request packet. %s %d\r\n", __FUNCTION__, __LINE__);
            set_option50(leases.fixed_address);
            set_option54(leases.server_address);
            ret = dhclient_request(if_name);
            if (ret)
            {
                printf("send request packet failure, send discover packet. %s %d \r\n", __FUNCTION__, __LINE__);
                ret = dhclient_discover(if_name);
            }
        }
        else 
        {
            printf("dhcp leases expire, send discover packet. %s %d\r\n", __FUNCTION__, __LINE__);
            ret = dhclient_discover(if_name);
        }
    }
    
    return ret;
}

MRJEcode dhclient_conver_opt5054(struct SessionHandle *data, char *requestip, char *serverip)
{
    if (!data || !requestip || !serverip)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    struct in_addr addr;

    memset(&addr, 0, sizeof(struct in_addr));
    
    if (inet_pton(AF_INET, requestip, (void *)&addr) != 1)
        return MRJE_UNKNOWN;

    data->opt.option50_ip = addr.s_addr;
    data->opt.option50_ip_flag = 1;

    memset(&addr, 0, sizeof(struct in_addr));
    if (inet_pton(AF_INET, serverip, (void *)&addr) != 1)
        return MRJE_UNKNOWN;

    data->opt.server_ident = addr.s_addr;
    data->opt.server_ident_flag = 1;
    
    return MRJE_OK;
}

MRJEcode dhclient_conver_time(long time, char *bufp)
{   
    if (!bufp || time <= 0)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
	struct tm strTm;
	struct tm *pTm = &strTm;

	if (localtime_r(&time, pTm) == NULL) {
        return MRJE_UNKNOWN;
	}       
    
	if (pTm) {
		sprintf(bufp, "%d-%02d-%02d %02d:%02d:%02d", 
			pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday, 
			pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	}
    
    return MRJE_OK;
}

MRJEcode dhclient_getipstr(char *bufp, size_t bufsize, u_int32_t ip)
{
    if (!bufp)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    struct in_addr src;
	src.s_addr = ip; //data->dhcp_hdrp->dhcp_yip;

    memset(bufp, 0, bufsize);
	if (inet_ntop(AF_INET, ((struct sockaddr_in *)&src), bufp, bufsize) == NULL) {
        return MRJE_UNKNOWN;
	}
    
	return MRJE_OK;
}

MRJEcode dhclient_update_leases(struct SessionHandle *data)
{   
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    int type = -1;

    strncpy(data->leases.if_name, data->resout.if_name, sizeof(data->leases.if_name) - 1);
    strncpy(data->leases.fixed_address, data->resout.fixed_address, sizeof(data->leases.fixed_address) - 1);
    strncpy(data->leases.subnet_mask, data->resout.sub_netmask, sizeof(data->leases.subnet_mask) - 1);
    strncpy(data->leases.routers, data->resout.router, sizeof(data->leases.routers) - 1);
    strncpy(data->leases.server_address, data->resout.serv_ident, sizeof(data->leases.server_address) - 1);

    data->leases.lease_time = data->resout.lease_time;               /* 租约时间 */
    data->leases.renewal_time = data->resout.lease_time / 2;            /* 重新申请时间(1/2租约时间) */
    data->leases.rebinding_time = (data->resout.lease_time * 3) / 4;    /* 再次重新申请时间(3/4租约时间) */

    long timenow = time(NULL);
    long renew_time = timenow + data->leases.renewal_time;
    long rebinding_time = timenow + data->leases.rebinding_time;
    long expire_time = timenow + data->leases.lease_time;          /*  租约到期时的时间 */

    dhclient_conver_time(renew_time, data->leases.renew);
    dhclient_conver_time(rebinding_time, data->leases.rebind);
    dhclient_conver_time(expire_time, data->leases.expire);

    type = dhclient_getifacetype(data->leases.if_name);
    if (0 == type)
        set_ethx_leases(&data->leases);
    else if (1 == type)
        set_rax_leases(&data->leases);
    else
        TRACE("unkown interface type, %s %s %d\r\n", data->leases.if_name, MDL);
    
    return MRJE_OK;
}


MRJEcode dhclient_parse_offer(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    data->opt.option50_ip_flag = 1;
    data->opt.option50_ip = data->dhcp_hdrp->dhcp_yip;

    u_int8_t *optbufp = data->dhcp_optp;
      
    while (*(optbufp) != DHCP_END) 
    {
        u_int32_t tmp = 0;
		if (*(optbufp) == DHCP_SERVIDENT) 
        {       
            memcpy(&tmp, optbufp + 2, sizeof(tmp));
			memcpy(&data->opt.server_ident, &tmp, 4);
            data->opt.server_ident_flag = 1;
		}
        
		optbufp = optbufp + *(optbufp + 1) + 2;
	}

    optbufp = NULL;
    
    if (data->set.verbose != 1) {
        return MRJE_OK;
    }

    long leasetime = 0;
    u_int16_t tmp;
    char ipstr[32] = {0};
    
    TRACE("\nDHCP offer details\n");
	TRACE("----------------------------------------------------------\n");

    dhclient_getipstr(ipstr, sizeof(ipstr), data->dhcp_hdrp->dhcp_yip);
	TRACE("DHCP offered IP from server - %s\n", ipstr);
    
    dhclient_getipstr(ipstr, sizeof(ipstr), data->dhcp_hdrp->dhcp_sip);
	TRACE("Next server IP(Probably TFTP server) - %s\n", ipstr);

    if (data->dhcp_hdrp->dhcp_gip) 
    {
        dhclient_getipstr(ipstr, sizeof(ipstr), data->dhcp_hdrp->dhcp_gip);
		TRACE("DHCP Relay agent IP - %s\n", ipstr);
	}

    /* 再次指向opt buf的头部 */
    optbufp = data->dhcp_optp;
    
    while (*(optbufp) != DHCP_END)
    {   
        u_int32_t tmp_data = 0;
        
		switch (*(optbufp))
        {
			case DHCP_SERVIDENT:
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 2, sizeof(tmp_data));
                if ((res = dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data)) != MRJE_OK)
                    return res; 
                   
                TRACE("DHCP server  - %s\n", ipstr);
				break;

			case DHCP_LEASETIME:
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 2, sizeof(tmp_data));
                leasetime = ntohl(tmp_data);
				TRACE("Lease time - %d Days %d Hours %d Minutes\n", \
						(leasetime) / (3600 * 24),    \
						((leasetime) % (3600 * 24)) / 3600, \
						(((leasetime) % (3600 * 24)) % 3600) / 60); 
				break;

			case DHCP_SUBNETMASK:
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 2, sizeof(tmp_data));
                if ((res = dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data)) != MRJE_OK)
                    return res; 
				TRACE("Subnet mask - %s\n", ipstr);
				break;

			case DHCP_ROUTER:
				for(tmp = 0; tmp < (*(optbufp + 1) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy((char *)&tmp_data, (optbufp + 2 + (tmp * 4)), sizeof(tmp_data));
                    dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data);
					TRACE("Router/gateway - %s\n", ipstr);
				}
				break;

			case DHCP_DNS:
				for(tmp = 0; tmp < ((*(optbufp + 1)) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy(&tmp_data, (optbufp + 2 + (tmp * 4)), sizeof(tmp_data));
                    dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data);
					TRACE("DNS server - %s\n", ipstr);
				}
				break;

			case DHCP_FQDN:
			{
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 1, sizeof(tmp_data));
                
				/* Minus 3 beacause 3 bytes are used to flags, rcode1 and rcode2 */
				u_int32_t size = tmp_data - 3;
                
				/* Plus 2 to add string terminator */
				u_char fqdn_client_name[size + 1];

				/* Plus 5 to reach the beginning of the string */
				memcpy(fqdn_client_name, optbufp + 5, size);
				fqdn_client_name[size] = '\0';
                
				TRACE("FQDN Client name - %s\n", fqdn_client_name);
			}
		}

		optbufp = optbufp + *(optbufp + 1) + 2;
	}

	TRACE("----------------------------------------------------------\n\n");

    return MRJE_OK;    
}


MRJEcode dhclient_parse_ack(struct SessionHandle *data)
{
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    u_int16_t tmp;
    char ipstr[32] = {0};
    MRJEcode res = MRJE_UNKNOWN;
    u_int8_t *optbufp = NULL;
       
    if ((res = dhclient_getipstr(ipstr, sizeof(ipstr), data->dhcp_hdrp->dhcp_yip)) != MRJE_OK)
        return res;
        
    strncpy(data->resout.fixed_address, ipstr, sizeof(data->resout.fixed_address) - 1);
    strncpy(data->resout.if_name, data->ifname, sizeof(data->resout.if_name) - 1);

    if (data->set.verbose == 1) {
        TRACE("\nDHCP ack details\n");
    	TRACE("----------------------------------------------------------\n");
        TRACE("DHCP offered IP from server - %s\n", ipstr);

        dhclient_getipstr(ipstr, sizeof(ipstr), data->dhcp_hdrp->dhcp_sip);
    	TRACE("Next server IP(Probably TFTP server) - %s\n", ipstr);

        if (data->dhcp_hdrp->dhcp_gip) 
        {
            dhclient_getipstr(ipstr, sizeof(ipstr), data->dhcp_hdrp->dhcp_gip);
    		TRACE("DHCP Relay agent IP - %s\n", ipstr));
    	}
    }

    optbufp = data->dhcp_optp;
    
    while (*(optbufp) != DHCP_END)
    {   
        u_int32_t tmp_data = 0;
        
		switch (*(optbufp))
        {
			case DHCP_SERVIDENT:
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 2, sizeof(tmp_data));
                if ((res = dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data)) != MRJE_OK)
                    return res; 
                strncpy(data->resout.serv_ident, ipstr, sizeof(data->resout.serv_ident) - 1);
                if (data->set.verbose == 1)
                    TRACE("DHCP server  - %s\n", ipstr);
				break;

			case DHCP_LEASETIME:
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 2, sizeof(tmp_data));
                data->resout.lease_time = ntohl(tmp_data);
                if (data->set.verbose == 1)
				    TRACE("Lease time - %d Days %d Hours %d Minutes\n", \
						(data->resout.lease_time) / (3600 * 24),    \
						((data->resout.lease_time) % (3600 * 24)) / 3600, \
						(((data->resout.lease_time) % (3600 * 24)) % 3600) / 60); 
   
				break;

			case DHCP_SUBNETMASK:
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 2, sizeof(tmp_data));
                if ((res = dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data)) != MRJE_OK)
                    return res; 
                strncpy(data->resout.sub_netmask, ipstr, sizeof(data->resout.sub_netmask) - 1);
                if (data->set.verbose == 1)
				    TRACE("Subnet mask - %s\n", ipstr);
				break;

			case DHCP_ROUTER:
				for (tmp = 0; tmp < (*(optbufp + 1) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy((char *)&tmp_data, (optbufp + 2 + (tmp * 4)), sizeof(tmp_data));
                    if ((res = dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data)) != MRJE_OK)
                        return res; 
                    
                    strncpy(data->resout.router, ipstr, sizeof(data->resout.router) - 1);
                    if (data->set.verbose == 1)
					    TRACE("Router/gateway - %s\n", ipstr);
				}
				break;

			case DHCP_DNS:
				for(tmp = 0; tmp < ((*(optbufp + 1)) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy(&tmp_data, (optbufp + 2 + (tmp * 4)), sizeof(tmp_data));
                    dhclient_getipstr(ipstr, sizeof(ipstr), tmp_data);
                    if (data->set.verbose == 1)
					    TRACE("DNS server - %s\n", ipstr);
				}
				break;

			case DHCP_FQDN:
			{
                tmp_data = 0;
                memcpy((char *)&tmp_data, optbufp + 1, sizeof(tmp_data));
                
				/* Minus 3 beacause 3 bytes are used to flags, rcode1 and rcode2 */
				u_int32_t size = tmp_data - 3;
                
				/* Plus 2 to add string terminator */
				u_char fqdn_client_name[size + 1];

				/* Plus 5 to reach the beginning of the string */
				memcpy(fqdn_client_name, optbufp + 5, size);
				fqdn_client_name[size] = '\0';

                if (data->set.verbose == 1)
				    TRACE("FQDN Client name - %s\n", fqdn_client_name);
			}
		}

		optbufp = optbufp + *(optbufp + 1) + 2;
	}

    if (data->set.verbose == 1) {    
	    TRACE("----------------------------------------------------------\n\n");
        TRACE("fixed_address: %s \r\n",    data->resout.fixed_address);
        TRACE("serv_ident: %s \r\n",       data->resout.serv_ident);
        TRACE("router: %s \r\n",           data->resout.router);
        TRACE("sub_netmask: %s \r\n",      data->resout.sub_netmask);
        TRACE("lease_time: %d \r\n",       data->resout.lease_time);
    }
    
    if (data->resout.fixed_address[0] != '\0' &&
        data->resout.serv_ident[0] != '\0' &&
        data->resout.router[0] != '\0' &&
        data->resout.sub_netmask[0] != '\0' &&
        data->resout.lease_time > 0) 
    {
        TRACE("parse dhcp ack success. %s %d\r\n", __FUNCTION__, __LINE__);
        return MRJE_OK;    
    }

    TRACE("parse dhcp ack fail. %s %d\r\n", __FUNCTION__, __LINE__);
        
    return MRJE_UNKNOWN;   
}

static MRJEcode dhclient_select(int sockfd, long timeout)
{
    int res = -1;
    fd_set rset, eset;
    struct timeval tv = {0};

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    FD_ZERO(&eset);
    FD_SET(sockfd, &eset);
    tv.tv_sec = (timeout / 1000L);
	tv.tv_usec = 0;

    res = select(sockfd + 1, &rset, NULL, &eset, &tv);
    if (res == 0) {
        TRACE("select time out, return. %s %d\r\n", MDL);
        return MRJE_TIME_OUT;
    } else if (res < 0 || FD_ISSET(sockfd, &eset)) {
        TRACE("select error, return. %s %d\r\n", MDL);
        return MRJE_SELECTFAL;
    } else {
        if (FD_ISSET(sockfd, &rset) > 0) {
            return MRJE_OK;
        }
    }

    return MRJE_SELECTFAL;
}


MRJDHCLIENT *dhclient_init()
{
    struct SessionHandle *data;

    data = (struct SessionHandle *)malloc(sizeof(struct SessionHandle));
    if (!data) {
        TRACE("Malloc SessionHandle fail. %s %d\r\n", MDL);
        return NULL;
    }

    memset(data, 0, sizeof(struct SessionHandle));
    data->sockfd = -1;

    return data;
}


MRJEcode dhclient_setstropt(char **charp, char *s)
{
    MRJ_safefree(*charp);

    if(s) {
        s = strdup(s);

        if(!s)
            return MRJE_OUT_OF_MEMORY;

        *charp = s;
    }
    
    return MRJE_OK;
}


MRJEcode dhclient_setopt(struct SessionHandle *data, MRJDHCLIENToption option, 
                         va_list param)
{
    char *argptr;
    MRJEcode result = MRJE_OK;
    long arg;

    switch (option) {
    case DHCPOPT_VERBOSE:
        data->set.verbose = (0 != va_arg(param, long)) ? TRUE : FALSE;
        break;

    case DHCPOPT_TIMEOUT:
        data->set.timeout = va_arg(param, long) * 1000L;
        break;
        
    case DHCPOPT_IFNAME:
        result = dhclient_setstropt(&data->ifname, va_arg(param, char *));      /* set ifname */
        break;

    case DHCPOPT_HOSTNAME:
        result = dhclient_setstropt(&data->opt.hostname_buff, va_arg(param, const char *));
        if (result == MRJE_OK) 
            data->opt.hostname_flag = 1;
        else
            data->opt.hostname_flag = 0;
        break;

    case DHCPOPT_UNICAST_IP:
        data->opt.unicast_ip_address = inet_addr(va_arg(param, char *));
        if (data->opt.unicast_ip_address == -1) { 
            data->opt.unicast_ip_flag = 0;
            return MRJE_UNKNOWN;
        }
        else
            data->opt.unicast_ip_flag = 1;
        break;

    case DHCPOPT_SERVER_IP:
        data->opt.server_ident = inet_addr(va_arg(param, char *));
        if (data->opt.server_ident == -1) { 
            data->opt.server_ident = 0;
            return MRJE_UNKNOWN;
        }
        else
            data->opt.server_ip_flag = 1;
        break;
    
    default:
        result = MRJE_UNKNOWN_OPTION;
        break;
    }

    return res;
}


MRJEcode dhclient_socketgetifmac(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;
    int sock = -1;
	struct ifreq ifr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        TRACE("getifmac socket error. %s %s %d\r\n", STR_ERROR, MDL);
        return res;
    }
    
	strncpy((char *)&ifr.ifr_name, data->ifname, sizeof(ifr.ifr_name));
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		fprintf(stderr, "Error: Could not get hardware address of interface '%s'\n", data->ifname);
        close(sock);
		return MRJE_IOCTLFAL;
    }

    memset(data->srcmac, 0, sizeof(data->srcmac));
    memcpy(&data->srcmac[0], &ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);

    if (data->set.verbose) {
        char temp[24];
        memset(temp, 0, sizeof(temp));
        snprintf(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X",
    					(unsigned char)ifr.ifr_hwaddr.sa_data[0],
    					(unsigned char)ifr.ifr_hwaddr.sa_data[1],
    					(unsigned char)ifr.ifr_hwaddr.sa_data[2],
    					(unsigned char)ifr.ifr_hwaddr.sa_data[3],
    					(unsigned char)ifr.ifr_hwaddr.sa_data[4],
    					(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
        
        TRACE("* MAC - %s %s %d\r\n", temp, MDL);
    }

    close(sock);
    
	return MRJE_OK;
}



MRJEcode dhclient_socketpromisc(mrj_socket_t sockfd, char *ifname) 
{
	struct ifreq ifr;

    if (!ifname)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_flags = (IFF_PROMISC | IFF_UP);

    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
	    TRACE("Error on setting promisc, %s %s %d\r\n", STR_ERROR, MDL);
        return MRJE_IOCTLFAL;
	}

    return MRJE_OK;
}


MRJEcode dhclient_socketopen(struct SessionHandle *data)
{
    if (!data) 
        return MRJE_BAD_FUNCTION_ARGUMENT;

	if ((data->sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
		TRACE("Error on creating the socket(%s) %s %d\r\n", STR_ERROR, MDL);
        return MRJE_UNKNOWN;
	} 
    
	/* Set link layer parameters */
	data->sall.sll_family   = AF_PACKET;
	data->sall.sll_protocol = htons(ETH_P_ALL);
	data->sall.sll_ifindex  = data->ifindex; 
	data->sall.sll_hatype   = ARPHRD_ETHER;
	data->sall.sll_pkttype  = PACKET_OTHERHOST;
	data->sall.sll_halen    = 6;

	if (bind(data->sockfd, (struct sockaddr *)&data->sall, sizeof(struct sockaddr_ll)) < 0)
	{
        TRACE("bind sock_packet error, %s, %s %d\r\n", STR_ERROR, MDL);
        return MRJE_UNKNOWN;
	}
    
	return MRJE_OK;
}

MRJEcode dhclient_socketopt(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;

    if ((data->ifindex = if_nametoindex(data->ifname)) == 0) {
        TRACE("if_nametoindex error(%s). %s %d\r\n", STR_ERROR, MDL);
        return MRJE_UNKNOWN;
    }

    res = dhclient_socketgetifmac(data);
    if (res != MRJE_OK) 
        return res;
    
    res = dhclient_socketopen(data);
    if (res != MRJE_OK)
        return res;

    res = dhclient_socketpromisc(data->sockfd, data->ifname);
    if (res != MRJE_OK)
        return res;

    return res;
}

MRJEcode dhclient_opt53_msgtype(struct SessionHandle *data)
{   
    MRJEcode res = MRJE_OK;
    u_int8_t msgtype;
    u_int8_t msglen;
    u_int8_t msg;
        
    MRJ_DhcpMsg msg = data->set.sendmsg;
    
    switch (msg) {
    case DHCPMSG_DISCOVER:
        msgtype = DHCP_MESSAGETYPE;
        msglen = 1;
        msg = DHCP_MSGDISCOVER;
        break;

    case DHCPMSG_REQUEST:
        msgtype = DHCP_MESSAGETYPE;
        msglen = 1;
        msg = DHCP_MSGREQUEST;
        break;
        
    case DHCPMSG_RELEASE:
        msgtype = DHCP_MESSAGETYPE;
        msglen = 1;
        msg = DHCP_MSGRELEASE;
        break;

     default:
        TRACE("dhcp message type not support. %s %d\r\n", MDL);
        return MRJE_UNSUPPORTED_PROTOCOL;
    }

    memcpy(data->opt.opt_buff, &msgtype,  1);
    memcpy(data->opt.opt_buff + 1, (char *)&msglen, 1);
    memcpy(data->opt.opt_buff + 2, (char *)&msg, 1);
    data->opt.opt_size = data->opt.opt_size + 3;
    
    return res;
}


void dhclient_opt50_requestip(struct SessionHandle *data)
{
    if (data->opt.option50_ip_flag == 0) {
        return;
    }
    
    u_int8_t msgtype = DHCP_REQUESTEDIP;
	u_int8_t msglen = 4;
	u_int32_t msg = data->opt.option50_ip;

    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;

	memcpy(bufp, &msgtype, 1);
	memcpy(bufp + 1, &msglen, 1);
	memcpy(bufp + 2, &msg, 4);
	data->opt.opt_size = data->opt.opt_size + 6; 
}

void dhclient_opt12_hostname(struct SessionHandle *data)
{
    if (data->opt.hostname_flag == 0) {
        return;
    }
    
    u_int32_t msgtype = DHCP_HOSTNAME;
	u_int32_t msglen = strlen((const char *)data->opt.hostname_buff);

    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;
    
	memcpy(bufp, &msgtype, 1);
	memcpy((bufp + 1), &msglen, 1);
	memcpy((bufp + 2), data->opt.hostname_buff, msglen);

	data->opt.opt_size = data->opt.opt_size + 2 + msglen;
}

void dhclient_opt54_servident(struct SessionHandle *data) 
{
    if (data->opt.server_ident_flag == 0) {
        return;    
    }
    
    u_int8_t msgtype = DHCP_SERVIDENT;
	u_int8_t msglen = 4;
	u_int32_t msg = data->opt.server_ident;

    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;

	memcpy(bufp, &msgtype, 1);
	memcpy(bufp + 1, &msglen, 1);
	memcpy(bufp + 2, &msg, 4);
	data->opt.opt_size = data->opt.opt_size + 6; 
}

void dhclient_opt81_fqdn(struct SessionHandle *data)
{
    if (data->opt.fqdn_flag == 0) {
        return;
    }
    
	u_int32_t msgtype = DHCP_FQDN;
	u_int8_t flags = 0;
	u_int8_t rcode1 = 0;
	u_int8_t rcode2 = 0;
	u_int32_t msglen = strlen((const char *)data->opt.fqdn_buff) + 3;

    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;
    
	if (data->opt.fqdn_n)
		flags |= FQDN_N_FLAG;
	if (data->opt.fqdn_s)
		flags |= FQDN_S_FLAG;

	memcpy(bufp, &msgtype, 1);
	memcpy(bufp + 1, &msglen, 1);
	memcpy(bufp + 2, &flags, 1);
	memcpy(bufp + 3, &rcode1, 1);
	memcpy(bufp + 4, &rcode2, 1);
	memcpy(bufp + 5, data->opt.fqdn_buff, strlen((const char *)data->opt.fqdn_buff));

	data->opt.opt_size = data->opt.opt_size + 2 + msglen;
}

void dhclient_opt60_vci(struct SessionHandle *data)
{
    if (data->opt.vci_flag == 0) {
        return;
    }
    
	u_int32_t msgtype = DHCP_CLASSSID;
	u_int32_t msglen = strlen((const char *)data->opt.vci_buff);

    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;
    
	memcpy(bufp, &msgtype, 1);
	memcpy(bufp + 1, &msglen, 1);
	memcpy(bufp + 2, data->opt.vci_buff, msglen);

	data->opt.opt_size = data->opt.opt_size + 2 + msglen;
}

void dhclient_opt51_leasetime(struct SessionHandle *data)
{
    if (data->opt.option51_flag == 0) {
        return;
    }
    
	u_int8_t msgtype = DHCP_LEASETIME;
	u_int8_t msglen = 4;
	u_int32_t msg = htonl(data->opt.option51_lease_time); 

    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;
    
	memcpy(bufp, &msgtype, 1);
	memcpy(bufp + 1, &msglen, 1);
	memcpy(bufp + 2, &msg, 4);
    
	data->opt.opt_size = data->opt.opt_size + 6;
}

void dhclient_opt55_paramrequest(struct SessionHandle *data)
{
	u_int32_t msgtype = DHCP_PARAMREQUEST;
	u_int32_t msglen = 4;
	u_int8_t msg[4] = { 0 };
    
	msg[0] = DHCP_SUBNETMASK;
	msg[1] = DHCP_ROUTER;
	msg[2] = DHCP_DOMAINNAME;
	msg[3] = DHCP_DNS;
	/* msg[4] = DHCP_LOGSERV; */

    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;
    
	memcpy(bufp, &msgtype, 1);
	memcpy(bufp + 1, &msglen, 1);
	memcpy(bufp + 2, msg, 4);
    
	data->opt.opt_size = data->opt.opt_size + 6; 
}


void dhclient_opt_endof(struct SessionHandle *data)
{
	u_int8_t eof = 0xff;
    u_char *bufp = data->opt.opt_buff + data->opt.opt_size;
    
	memcpy(bufp, &eof, 1);
	data->opt.opt_size = data->opt.opt_size + 1; 
}


MRJEcode dhclient_option(struct SessionHandle *data)
{   
    MRJEcode res = MRJE_OK;
    
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    if (data->opt.dhcp_xid == 0) {
        srand(time(NULL) ^ (getpid() << 16));
		data->opt.dhcp_xid = rand() % 0xffffffff;
    }

    memset(data->opt.opt_buff, 0, sizeof(data->opt.opt_buff));
    data->opt.opt_size = 0;

    res = dhclient_opt53_msgtype(data);
    if (res != MRJE_OK)
        return res;

    if (data->set.sendmsg == DHCPMSG_REQUEST) {
        dhclient_opt50_requestip(data);
        dhclient_opt54_servident(data);
        dhclient_opt55_paramrequest(data);
    }

    dhclient_opt12_hostname(data);
    
	dhclient_opt81_fqdn(data);

	dhclient_opt60_vci(data);

    dhclient_opt51_leasetime(data);
    
	dhclient_opt_endof();

    return res;
}


MRJEcode dhclient_build_request_pack(struct SessionHandle *data)
{
    MRJEcode res = MRJE_OK;
    u_int16_t buf[1024] = {0};
    u_int16_6 len = 0;
    u_char *bufp = NULL;
    
	u_int32_t packsize = sizeof(struct dhcpv4_hdr) + data->opt.opt_size;

    if (data->opt.release_flag == 0) {
        char tmpmac[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        memcpy(data->dstmac, tmpmac, ETHER_ADDR_LEN);
    }

    if (data->opt.vlan == 0) {
        
        struct ethernet_hdr t_ethhdr;
        memset(&t_ethhdr, 0, sizeof(struct ethernet_hdr));
        struct ethernet_hdr *ethhdr = &t_ethhdr;

		memcpy(ethhdr->ether_dhost, data->dstmac, ETHER_ADDR_LEN);
		memcpy(ethhdr->ether_shost, data->srcmac, ETHER_ADDR_LEN);
		ethhdr->ether_type = htons(ETHERTYPE_IP);
        
        memcpy(data->send_buf, (char *)ethhdr, sizeof(struct ethernet_hdr));
        
	} else {
        struct vlan_hdr t_vhdr;
        memset(&t_vhdr, 0, sizeof(struct vlan_hdr));
        struct vlan_hdr *vhdr = &t_vhdr;

		memcpy(vhdr->vlan_dhost, data->dstmac, ETHER_ADDR_LEN);
		memcpy(vhdr->vlan_shost, data->srcmac, ETHER_ADDR_LEN);
		vhdr->vlan_tpi = htons(ETHERTYPE_VLAN);
		vhdr->vlan_priority_c_vid = htons(data->opt.vlan);
		vhdr->vlan_len = htons(ETHERTYPE_IP);
        
        memcpy(data->send_buf, (char *)vhdr, sizeof(struct vlan_hdr));
	}

    /* packet padding */
    if (data->opt.padding_flag && packsize < MINIMUM_PACKET_SIZE) {
        memset(data->opt.opt_buff + data->opt.opt_size, 0, MINIMUM_PACKET_SIZE - packsize);
        data->opt.opt_size += MINIMUM_PACKET_SIZE - packsize;
    }
	
    /* ----------------------------------------------------------------------------------- */
    
    struct iphdr t_iph;
    memset(&t_iph, 0, sizeof(t_iph));
    struct iphdr *iph = &t_iph;

    len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr) + data->opt.opt_size;
    
    iph->version = 4;
	iph->ihl = 5;
	iph->tos = g_iptos;
	iph->tot_len = htons(len);
	iph->id = 0;
	iph->frag_off = 0;
	iph->ttl = 64;
	iph->protocol = 17;
	iph->check = 0; // Filled later;
	if (data->opt.unicast_ip_flag)
		iph->saddr = inet_addr(data->opt.unicast_ip);
	else
		iph->saddr = inet_addr("0.0.0.0");
	iph->daddr = inet_addr("255.255.255.255");

    memset(buf, 0, sizeof(buf));
    memcpy(buf, iph, sizeof(struct iphdr));
    iph->check = ipchksum((u_int16_t *)buf, iph->ihl << 1);

    bufp = data->send_buf + sizeof(struct ethernet_hdr);
    
    memcpy(bufp, (char *)iph, sizeof(struct iphdr));

    /* ----------------------------------------------------------------------------------- */

    struct udphdr t_uh;
    memset(&t_uh, 0, sizeof(t_uh));
    struct udphdr *uh = &t_uh;

	uh->source = htons(DHCP_SRC_PORT + 1);
	uh->dest = htons(DHCP_SRC_PORT);
	u_int16_t l4_proto = 17;
	u_int16_t l4_len = (sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr) + data->opt.opt_size);
	uh->len = htons(l4_len);
	uh->check = 0;                      /* UDP checksum will be done after building dhcp header*/

    bufp = data->send_buf + sizeof(struct ethernet_hdr) + sizeof(struct iphdr);
    
    memcpy(bufp, (char *)uh, sizeof(struct udphdr));

    /* ----------------------------------------------------------------------------------- */

    struct dhcpv4_hdr t_dhpointer;
    memset(&t_dhpointer, 0, sizeof(struct dhcpv4_hdr));
    struct dhcpv4_hdr *dhpointer = &t_dhpointer;

    dhpointer->dhcp_opcode = DHCP_REQUEST;
	dhpointer->dhcp_htype = ARPHRD_ETHER;
	dhpointer->dhcp_hlen = ETHER_ADDR_LEN;
	dhpointer->dhcp_hopcount = 0;
	dhpointer->dhcp_xid = htonl(data->opt.dhcp_xid);
	dhpointer->dhcp_secs = 0;
	dhpointer->dhcp_flags = data->opt.bcast_flag;
	if (data->opt.unicast_ip_flag)
		dhpointer->dhcp_cip = inet_addr(data->opt.unicast_ip);
	else
		dhpointer->dhcp_cip = 0;
	dhpointer->dhcp_yip = 0;
	dhpointer->dhcp_sip = 0;
	dhpointer->dhcp_gip = inet_addr("0.0.0.0");
	memcpy(dhpointer->dhcp_chaddr, data->srcmac, ETHER_ADDR_LEN);

    /*dhpointer->dhcp_sname 
	  dhpointer->dhcp_file*/

    dhpointer->dhcp_magic = htonl(DHCP_MAGIC);

    /* dhcpv4 header */
    bufp = data->send_buf + sizeof(struct ethernet_hdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(bufp, (char *)dhpointer, sizeof(struct dhcpv4_hdr)); 

    /* dhcp packet data */
    bufp = data->send_buf + sizeof(struct ethernet_hdr) + sizeof(struct iphdr) + 
                            sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr);
	memcpy(bufp, data->opt.opt_buff, data->opt.opt_size);

    /* ----------------------------------------------------------------------------------- */

    memset(buf, 0, sizeof(buf));
    bufp = data->send_buf + sizeof(struct ethernet_hdr) + sizeof(struct iphdr);
    len = sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr) + data->opt.opt_size;
        
    memcpy(buf, bufp, len); 
    
    uh->check = l4_sum((u_int16_t *)buf, 
                (len / 2), 
                (u_int32_t *)&iph->saddr, 
                (u_int32_t *)&iph->daddr, 
                htons(l4_proto), 
                htons(l4_len));

    bufp = NULL;

    return res;
}


MRJEcode dhclient_build_discover_pack(struct SessionHandle *data)
{   
    MRJEcode res = MRJE_OK;
    u_int16_t buf[1024] = {0};
    u_int16_6 len = 0;
    u_char *bufp = NULL;
    
	u_int32_t dhcppacksize = sizeof(struct dhcpv4_hdr) + data->opt.opt_size;

    if (data->opt.release_flag == 0) {
        char tmpmac[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        memcpy(data->dstmac, tmpmac, ETHER_ADDR_LEN);
    }
    
    if (data->opt.vlan == 0) 
    {       
        struct ethernet_hdr t_ethhdr;
        memset(&t_ethhdr, 0, sizeof(struct ethernet_hdr));
        struct ethernet_hdr *ethhdr = &t_ethhdr;

		memcpy(ethhdr->ether_dhost, data->dstmac, ETHER_ADDR_LEN);      /* 目的mac地址 */
        memcpy(ethhdr->ether_shost, data->srcmac, ETHER_ADDR_LEN);     /* 源mac地址 */
        ethhdr->ether_type = htons(ETHERTYPE_IP);               /* 类型 */

        memcpy(data->send_buf, (char *)ethhdr, sizeof(struct ethernet_hdr));
	}
    else
    {
        struct vlan_hdr t_vhdr;
        memset(&t_vhdr, 0, sizeof(struct vlan_hdr));
        struct vlan_hdr *vhdr = &t_vhdr;

		memcpy(vhdr->vlan_dhost, data->dstmac, ETHER_ADDR_LEN);
		memcpy(vhdr->vlan_shost, data->srcmac, ETHER_ADDR_LEN);
		vhdr->vlan_tpi = htons(ETHERTYPE_VLAN);
		vhdr->vlan_priority_c_vid = htons(data->opt.vlan);
		vhdr->vlan_len = htons(ETHERTYPE_IP);
        
        memcpy(data->send_buf, (char *)vhdr, sizeof(struct vlan_hdr));  
	}

	if (data->opt.padding_flag && dhcppacksize < MINIMUM_PACKET_SIZE) 
    {
		memset(data->opt.opt_buff + data->opt.opt_size, 0, MINIMUM_PACKET_SIZE - dhcppacksize);
		data->opt.opt_size += MINIMUM_PACKET_SIZE - dhcppacksize;
	}

    /* 填充ip头部  */
    struct iphdr t_iphdr;
    memset(&t_iphdr, 0, sizeof(t_iphdr));
    struct iphdr *iph = &t_iphdr;

    /* ip数据报总长度 = ip头20字节 + udp头8字节 + dhcp头 + dhcp选项数据 */
    len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr) + data->opt.opt_size
    
    iph->version = 4;
	iph->ihl = 5;
	iph->tos = g_iptos;
    
	iph->tot_len = htons(len);
	iph->id = 0;
	iph->frag_off = 0;
	iph->ttl = 64;
	iph->protocol = 17;
	iph->check = 0; // Filled later;
	if (data->opt.unicast_ip_flag)
		iph->saddr = inet_addr(data->opt.unicast_ip);
	else
		iph->saddr = inet_addr("0.0.0.0");
	iph->daddr = inet_addr("255.255.255.255");

    memset(buf, 0, sizeof(buf));
    memcpy(buf, iph, sizeof(struct iphdr));
    iph->check = ipchksum((u_int16_t *)buf, iph->ihl << 1);

    /* copy ip header to the end of ethernet header */
    bufp = data->send_buf + sizeof(struct ethernet_hdr);
    memcpy(bufp, (char *)iph, sizeof(struct iphdr));

    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    /* 填充udp头部 */
    struct udphdr t_uh;
    memset(&t_uh, 0, sizeof(struct udphdr));
    struct udphdr *uh = &t_uh;

    uh->source = htons(DHCP_SRC_PORT + 1);
	uh->dest = htons(DHCP_SRC_PORT);
	u_int16_t l4_proto = 17;
	u_int16_t l4_len = (sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr) + data->opt.opt_size);
	uh->len = htons(l4_len);
	uh->check = 0;              /* UDP checksum will be done after dhcp header*/

    bufp = data->send_buf + sizeof(struct ethernet_hdr) + sizeof(struct iphdr);
    memcpy(bufp, (char *)uh, sizeof(struct udphdr));
    
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    /* 填充dhcp头部和数据*/
	struct dhcpv4_hdr t_dhpointer;
    memset(&t_dhpointer, 0, sizeof(t_dhpointer));
    struct dhcpv4_hdr *dhpointer = &t_dhpointer;

    dhpointer->dhcp_opcode = DHCP_REQUEST;
	dhpointer->dhcp_htype = ARPHRD_ETHER;
	dhpointer->dhcp_hlen = ETHER_ADDR_LEN;
	dhpointer->dhcp_hopcount = 0;
	dhpointer->dhcp_xid = htonl(data->opt.dhcp_xid);
	dhpointer->dhcp_secs = 0;
	dhpointer->dhcp_flags = data->opt.bcast_flag;
	if (data->opt.unicast_ip_flag)
		dhpointer->dhcp_cip = inet_addr(data->opt.unicast_ip);
	else
		dhpointer->dhcp_cip = 0;
	dhpointer->dhcp_yip = 0;
	dhpointer->dhcp_sip = 0;
	dhpointer->dhcp_gip = inet_addr("0.0.0.0");
	memcpy(dhpointer->dhcp_chaddr, data->srcmac, ETHER_ADDR_LEN);
	/*dhpointer->dhcp_sname 
	  dhpointer->dhcp_file*/
	dhpointer->dhcp_magic = htonl(DHCP_MAGIC);

    bufp = data->send_buf + sizeof(struct ethernet_hdr) + 
                            sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(bufp, (char *)dhpointer, sizeof(struct dhcpv4_hdr));      

    bufp = data->send_buf + sizeof(struct ethernet_hdr) + sizeof(struct iphdr) + 
                            sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr);
	memcpy(bufp, data->opt.opt_buff, data->opt.opt_size);    

	/* UDP checksum is done here */
    memset(buf, 0, sizeof(buf));
    bufp = data->send_buf + sizeof(struct ethernet_hdr) + sizeof(struct iphdr);
    len  = sizeof(struct udphdr) + sizeof(struct dhcpv4_hdr) + data->opt.opt_size;
    
    memcpy(buf, bufp, len);
    
    uh->check = l4_sum((u_int16_t *)buf, 
                (len / 2), 
                (u_int32_t *)&iph->saddr, 
                (u_int32_t *)&iph->daddr,
                htons(l4_proto), 
                htons(l4_len));

    return res;
}

MRJEcode dhclient_build_release_pack(struct SessionHandle *data)
{
    
}


MRJEcode dhclient_build_packet(struct SessionHandle *data)
{
    MRJEcode res = MRJE_OK;
    MRJ_DhcpMsg msgtype = data->set.sendmsg;
    
    switch (msgtype) {
    case DHCPMSG_REQUEST:
        res = dhclient_build_request_pack(data);
        break;
        
    case DHCPMSG_DISCOVER:
        res = dhclient_build_discover_pack(data);        
        break;
    
    case DHCPMSG_RELEASE:
        res = dhclient_build_release_pack(data);
        break;
    
    default:
        TRACE("Message type is not support. %s %d\r\n", MDL);
        return MRJE_UNSUPPORTED_PROTOCOL;
    }
    
    return res;
}


MRJEcode dhclient_send_pack(struct SessionHandle *data)
{
    int result = -1;
    int len = 0;
    MRJEcode res = MRJE_OK;
    socklen_t addrlen = sizeof(struct sockaddr_ll);
    
    len = sizeof(struct ethernet_hdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + 
              sizeof(struct dhcpv4_hdr) + data->opt.opt_size;
    
    result = sendto(data->sockfd, data->send_buf, len, 0, (struct sockaddr *)&data->sall, addrlen); 
    if (result == -1) {
        TRACE("dhcp send %d packet fail(%s). %s %d\r\n", (int)data->set.sendmsg, STR_ERROR, MDL);
        return MRJE_SEND_ERROR;
    }
    
    return res;
}

MRJEcode dhclient_check_offer_pack(struct SessionHandle *data)
{
    if (data->opt.vlan != 0) {
        data->vlan_hdrp = (struct vlan_hdr *)(data->recv_buf);
    } else {
        data->eth_hdrp = (struct ethernet_hdr *)(data->recv_buf);
    }

    data->ip_hdrp = (struct iphdr *)(data->recv_buf + 
                                     sizeof(struct ethernet_hdr));
        
    data->udp_hdrp = (struct udphdr *)(data->recv_buf + 
                                       sizeof(struct ethernet_hdr) + 
                                       sizeof(struct iphdr));
        
    data->dhcp_hdrp = (struct dhcpv4_hdr *)(data->recv_buf + 
                                            sizeof(struct ethernet_hdr) + 
                                            sizeof(struct iphdr) + 
                                            sizeof(struct updhdr));
        
	data->dhcp_optp = (u_int8_t *)(data->recv_buf + 
                                   sizeof(struct ethernet_hdr) + 
                                   sizeof(struct iphdr) + 
                                   sizeof(struct updhdr) + 
                                   sizeof(struct dhcpv4_hdr));

    if (data->opt.vlan != 0) {
        
        if ((ntohs(data->vlan_hdrp->vlan_priority_c_vid) & VLAN_VIDMASK) == data->opt.vlan && 
             ntohs(data->vlan_hdrp->vlan_tpi) == ETHERTYPE_VLAN && 
             data->ip_hdrp->protocol == 17 && 
             data->udp_hdrp->source == htons(DHCP_SRC_PORT) && 
             data->udp_hdrp->dest == htons(DHCP_SRC_PORT + 1)) 
        {
			if (*(data->dhcp_optp + 2) == DHCP_MSGOFFER && 
                htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) {
				return MRJE_OK;
			} else {
				return MRJE_UNKNOWN;
			}
		} else {
			return MRJE_UNKNOWN;
		}       
    } else {
    
        if (data->eth_hdrp->ether_type == htons(ETHERTYPE_IP) && 
             data->ip_hdrp->protocol == 17 && 
             data->udp_hdrp->source == htons(DHCP_SRC_PORT) && 
             data->udp_hdrp->dest == htons(DHCP_SRC_PORT + 1)) 
        {
			if (*(data->dhcp_optp + 2) == DHCP_MSGOFFER && 
                htonl( data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) {
				return MRJE_OK;
			} else {
				return MRJE_UNKNOWN;
			}
		} else {
			return MRJE_UNKNOWN;
		}
    }

    return MRJE_UNKNOWN;
}

MRJEcode dhclient_check_ack_pack(struct SessionHandle *data)
{
    if (data->opt.vlan != 0) {
        data->vlan_hdrp = (struct vlan_hdr *)(data->recv_buf);
    } else {
        data->eth_hdrp = (struct ethernet_hdr *)(data->recv_buf);
    }

    data->ip_hdrp = (struct iphdr *)(data->recv_buf + 
                                     sizeof(struct ethernet_hdr));
        
    data->udp_hdrp = (struct udphdr *)(data->recv_buf + 
                                       sizeof(struct ethernet_hdr) + 
                                       sizeof(struct iphdr));
        
    data->dhcp_hdrp = (struct dhcpv4_hdr *)(data->recv_buf + 
                                            sizeof(struct ethernet_hdr) + 
                                            sizeof(struct iphdr) + 
                                            sizeof(struct updhdr));
        
	data->dhcp_optp = (u_int8_t *)(data->recv_buf + 
                                   sizeof(struct ethernet_hdr) + 
                                   sizeof(struct iphdr) + 
                                   sizeof(struct updhdr) + 
                                   sizeof(struct dhcpv4_hdr));

    if (data->opt.vlan != 0) {
        if ((ntohs(data->vlan_hdrp->vlan_priority_c_vid) & VLAN_VIDMASK) == data->opt.vlan && 
             ntohs(data->vlan_hdrp->vlan_tpi) == ETHERTYPE_VLAN && 
             data->ip_hdrp->protocol == 17 && 
             data->udp_hdrp->source == htons(DHCP_SRC_PORT) && 
             data->udp_hdrp->dest == htons(DHCP_SRC_PORT + 1)) {
             
			    if(*(data->dhcp_optp + 2) == DHCP_MSGACK && 
                    htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) 
                {
				    return MRJE_OK;
			    } 
                else if(*(data->dhcp_optp + 2) == DHCP_MSGNACK && 
			           htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid)
			    { 
                    TRACE("DHCP nack received\t - Client MAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                            data->srcmac[0], data->srcmac[1], 
                            data->srcmac[2], data->srcmac[3], 
                            data->srcmac[4], data->srcmac[5]);
				    return MRJE_RECV_NAK;
			    } 
                else
                {
				    return MRJE_UNKNOWN;
			    }

		} else {
			return MRJE_UNKNOWN;
		}
    } else {
        if (data->eth_hdrp->ether_type == htons(ETHERTYPE_IP) && 
            data->ip_hdrp->protocol == 17 && 
            data->udp_hdrp->source == htons(DHCP_SRC_PORT) && 
            data->udp_hdrp->dest == htons(DHCP_SRC_PORT + 1)) 
        {
			if (*(data->dhcp_optp + 2) == DHCP_MSGACK && 
                htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) 
            {
				return MRJE_OK;
			} 
            else if(*(data->dhcp_optp + 2) == DHCP_MSGNACK && 
                htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) 
            {
                TRACE("DHCP nack received\t - Client MAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                            data->srcmac[0], data->srcmac[1], 
                            data->srcmac[2], data->srcmac[3], 
                            data->srcmac[4], data->srcmac[5]);
				return MRJE_RECV_NAK;
			} 
            else 
            {
				return MRJE_UNKNOWN;
			}
		} 
        else 
        {
			return MRJE_UNKNOWN;
		}
    }

    return MRJE_UNKNOWN
}


MRJEcode dhclient_check_pack(struct SessionHandle *data)
{   
    MRJEcode res = MRJE_UNKNOWN;
    MRJ_DhcpMsg msgtype = data->set.recvmsg;
        
    if (msgtype == DHCPMSG_OFFER) {
        res = dhclient_check_offer_pack(data);
    } else if (msgtype == DHCPMSG_ACK) {
        res = dhclient_check_ack_pack(data);
    } else {
        TRACE("Message type is not support. %s %d\r\n", MDL);
        return MRJE_UNSUPPORTED_PROTOCOL;
    }

    return res;
}

MRJEcode dhclient_recv_pack(struct SessionHandle *data)
{
    int res = -1;
    int requestlen = 60;
    int recvlen = 0;
    MRJEcode res = MRJE_OK;
    sockaddr_t addrlen = sizeof(struct sockaddr_ll);
    
    res = dhclient_select(data->sockfd, data->set.timeout);
    if (res != MRJE_OK) {
        TRACE("select timeout. %s %d\r\n", MDL);
        return res;
    }
        
    recvlen = recvfrom(data->sockfd, \
                       data->recv_buf, \
                       sizeof(data->recv_buf),\ 
                       0, \
                       (struct sockaddr *)&data->sall, \
                       &addrlen);

    if (recvlen <= 0) {
        TRACE("Recv dhcp packet error(%s). %s %d\r\n", STR_ERROR, MDL);
        return MRJE_RECV_ERROR;
    }
    
    data->recv_buf[recvlen] = '\0';
    
    res = dhclient_check_pack(data);

    return res;
}


MRJEcode dhclient_transfer_request(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;

    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    res = dhclient_send_pack(data);
    if (res != MRJE_OK)
        return res;

    res = dhclient_recv_pack(data);
    if (res != MRJE_OK) 
        return res;

    
    return res;
}

MRJEcode dhclient_transfer_discover(struct SessionHandle *data) 
{
    MRJEcode res = MRJE_UNKNOWN;
    return res;
}

MRJEcode dhclient_transfer(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;

    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    res = dhclient_socketopt(data);
    if (res != MRJE_OK)
        return res;

    res = dhclient_option(data);
    if (res != MRJE_OK)
        return res;
    
    res = dhclient_build_packet(data);
    if (res != MRJE_OK)
        return res;
    
    res = dhclient_send_pack(data);
    if (res != MRJE_OK)
        return res;

    res = dhclient_recv_pack(data);
    if (res != MRJE_OK) 
        return res;

    if (data->set.sendmsg == DHCPMSG_DISCOVER) {
        data->set.sendmsg = DHCPMSG_REQUEST;
        data->set.recvmsg = DHCPMSG_ACK;

        res = dhclient_parse_offer(data);
        if (res != MRJE_OK)
            return res;

        res = dhclient_option(data);
        if (res != MRJE_OK)
            return res;
        
        res = dhclient_build_packet(data);
        if (res != MRJE_OK)
            return res;
        
        res = dhclient_send_pack(data);
        if (res != MRJE_OK)
            return res;

        res = dhclient_recv_pack(data);
        if (res != MRJE_OK)
            return res;
    }
    
    res = dhclient_parse_ack(data);
    if (res != MRJE_OK)
        return res;

    res = dhclient_update_leases(data);
    if (res != MRJE_OK)
        return res;
    
    return MRJE_OK;
}

MRJEcode dhclient_perform(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;
    char tmpbuf[20] = {0};
    int res = -1;
    struct DhclientLeases leases;
        
    dhclient_init_leases();
    dhclient_conver_time(time(NULL), tmpbuf);
    
    res = dhclient_get_ifleases(data->ifname, &leases);
    if (res == -1) {
        TRACE("dhclient_perform get %s leases failure. %s %d\r\n", data->ifname, MDL);
        return MRJE_UNKNOWN;
    }
    
    if (strcmp(leases.if_name, data->ifname) != 0) {
        TRACE("leases record's interface name is not equal the request interface name.  \r\n");
        data->set.sendmsg = DHCPMSG_DISCOVER;
        data->set.recvmsg = DHCPMSG_OFFER;
        res = dhclient_transfer(data);
        if (res != MRJE_OK)
            return res;
    } else {

        if (strcmp(tmpbuf, leases.expire) < 0) {

            TRACE("dhcp leases don't expire, send request packet. %s %d\r\n", MDL);
            res = dhclient_conver_opt5054(data);
            if (res != MRJE_OK)
                return res;

            data->set.sendmsg = DHCPMSG_REQUEST;
            data->set.recvmsg = DHCPMSG_ACK;
            res = dhclient_transfer(data);
            if (res != MRJE_OK) {
                data->set.sendmsg = DHCPMSG_DISCOVER;
                data->set.recvmsg = DHCPMSG_OFFER;
                res = dhclient_transfer(data);
                if (res != MRJE_OK)
                    return res;
            }
        } else {
            TRACE("dhcp leases expire, send discover packet. %s %d\r\n", MDL);
            data->set.sendmsg = DHCPMSG_DISCOVER;
            data->set.recvmsg = DHCPMSG_OFFER;
            res = dhclient_transfer(data);
            if (res != MRJE_OK)
                return res;
        }
    }
    
    return res;
}


MRJEcode dhclient_cleanup_promisc(mrj_socket_t sockfd, char *ifname) 
{
    if (!ifname)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
	struct ifreq ifr;

    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
	ifr.ifr_flags = IFF_UP;

	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
	    TRACE("Error on disabling promisc. %s %s %d\r\n", STR_ERROR, MDL);
        return MRJE_IOCTLFAL;
	}
    
	return MRJE_OK;
}


MRJEcode dhclient_close(struct SessionHandle *data)
{
    dhclient_cleanup_promisc(data->sockfd, data->ifname);
    
    if (data->sockfd != -1) {
        close(data->sockfd);
        data->sockfd = -1;
    }

    MRJ_safefree(data->ifname);                 /* strdup */
    MRJ_safefree(data->opt.hostname_buff)        /* strdup */
    //MRJ_safefree(data->set.recv_data);

    free(data);
    
    return MRJE_OK;
}


MRJEcode mrj_dhclient_setopt(MRJDHCLIENT *dhclient, MRJDHCLIENToption tag, ...)
{   
    va_list arg;
    struct SessionHandle *data = dhclient;
    MRJEcode res;
     
    if (!data) 
        return MRJE_BAD_FUNCTION_ARGUMENT;

    va_start(arg, tag);
    
    res = dhclient_setopt(data, tag, arg);

    va_end(arg);
        
    return res;
}


MRJDHCLIENT *mrj_dhclient_init(void)
{   
    return dhclient_init();
}


MRJEcode mrj_dhclient_perform(struct SessionHandle *data)
{
    return dhclient_perform(data);
}

void mrj_dhclient_cleanup(MRJDHCLIENT *handle)
{
    struct SessionHandle *data = (struct SessionHandle *)handle;

    if (!data)
        return;

    dhclient_close(data);
}


int mrj_dhclient()
{
    MRJEcode res = MRJE_OK;
    MRJDHCLIENT *handle = NULL;
    const char *hostnamep = "MRJ_M1";
    
    handle = mrj_dhclient_init();
    if (!handle) {
        TRACE("mrj dhclient init fail. %s %d\r\n", MDL);
        return (-1);
    }

    mrj_dhclient_setopt(handle, DHCPOPT_IFNAME, ifname);
    mrj_dhclient_setopt(handle, DHCPOPT_HOSTNAME, hostnamep);
    mrj_dhclient_setopt(handle, DHCPOPT_TIMEOUT, 10L);
    mrj_dhclient_setopt(handle, DHCPOPT_VERBOSE, 1L);

    res = mrj_dhclient_perform(handle);
    if (res != MRJE_OK) {
        TRACE("MRJ dhclient perform fail. %s %d\r\n", MDL);
        mrj_dhclient_cleanup(handle);
        return (-1);
    }

    mrj_dhclient_cleanup(handle);

    return (0);
}


int main(int argc, char **argv)
{
    int ret = mrj_dhclient();
    if (ret == 0) {
        printf("\r\ndhcp get ip address success. \r\n");
    } else {
        printf("\r\ndhcp get ip address failure. \r\n");
    }
    
    return ret;
}

