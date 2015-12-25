
#include <stdio.h>		
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>		/* To set non blocking on socket  */
#include <sys/socket.h>		/* Generic socket calls */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/types.h>
#include <net/if.h> 
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/route.h>
                                 /* for if_nametoindex() */
#include "mrj_dhclient.h"
#include "tinyxml.h"

static int g_iptos = 0;
static struct DhclientLeases g_ethxLeases;
static struct DhclientLeases g_raxLeases;

#if 1
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



#if 0

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
#endif


static void dhclient_trace(struct SessionHandle *data, const char *msg, const char *func, int linenum)
{
    if (data->set.verbose)
        TRACE("* %s. %s %d\r\n", msg, func, linenum);
}   


static MRJEcode dhclient_conver_time(long time, char *bufp)
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

static MRJEcode dhclient_check_file(char *pcFileName, int *piFileType, int *piFileLen)
{	
    MRJEcode ret = MRJE_ERR;
    int iFileType = 2;
    int iFileLen = 0;
    struct stat st;

    if(pcFileName == NULL || *pcFileName == '\0') 
    {
        return ret;
    }

    if (stat(pcFileName, &st) == 0) 
    {
        iFileLen = st.st_size;
    }

    if (iFileLen > 0)
    {
        ret = MRJE_OK;
        if(strstr(pcFileName, ".jpg"))
        {
            iFileType = 1;
        }
        else if(strstr(pcFileName, ".bmp")) 
        {
            iFileType = 0;
        }
    }
	
    if (ret == MRJE_OK)
    {
        if (piFileType)
        {
            *piFileType = iFileType;
        }
        
        if (piFileLen)
        {
            *piFileLen = iFileLen;
        }
    }	

    return ret;
}

static void dhclient_init_ethxleases()
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

static void dhclient_init_raxleases()
{
    memset(&g_raxLeases, 0, sizeof(struct DhclientLeases));
    
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

static MRJEcode dhclient_create_ethxleases(char *path)
{
    MRJEcode res = MRJE_ERR;
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
    sprintf(tmpBuf, "%d", g_ethxLeases.lease_time);
    TiXmlText *leaseTimeContent = new TiXmlText(tmpBuf);
    leaseTimeElement->LinkEndChild(leaseTimeContent);

    /* renewal_time */
    TiXmlElement *renewalTimeElement = new TiXmlElement("renewal_time");
    ethxLeasesElement->LinkEndChild(renewalTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_ethxLeases.renewal_time);
    TiXmlText *renewalTimeContent = new TiXmlText(tmpBuf);
    renewalTimeElement->LinkEndChild(renewalTimeContent);

    /* rebinding_time */
    TiXmlElement *rebindingTimeElement = new TiXmlElement("rebinding_time");
    ethxLeasesElement->LinkEndChild(rebindingTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_ethxLeases.rebinding_time);
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
		res = MRJE_OK;
	}
	else
	{
		res = MRJE_ERR;
	}

	ethxLeasesXml->Clear();
	delete ethxLeasesXml;
    
    return res;
}

static MRJEcode dhclient_create_raxleases(char *path)
{
    MRJEcode res = MRJE_ERR;
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
    sprintf(tmpBuf, "%d", g_raxLeases.lease_time);
    TiXmlText *leaseTimeContent = new TiXmlText(tmpBuf);
    leaseTimeElement->LinkEndChild(leaseTimeContent);

    /* renewal_time */
    TiXmlElement *renewalTimeElement = new TiXmlElement("renewal_time");
    raxLeasesElement->LinkEndChild(renewalTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_raxLeases.renewal_time);
    TiXmlText *renewalTimeContent = new TiXmlText(tmpBuf);
    renewalTimeElement->LinkEndChild(renewalTimeContent);

    /* rebinding_time */
    TiXmlElement *rebindingTimeElement = new TiXmlElement("rebinding_time");
    raxLeasesElement->LinkEndChild(rebindingTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", g_raxLeases.rebinding_time);
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
		res = MRJE_OK;
	}
	else
	{
        res = MRJE_ERR;
	}

	raxLeasesXml->Clear();
	delete raxLeasesXml;
    
    return res;
}


static MRJEcode dhclient_set_ethxleases(struct DhclientLeases *pLeases)
{
    MRJEcode res = MRJE_OK;

    if (memcmp(&g_ethxLeases, pLeases, sizeof(*pLeases)) != 0)
    {
        memcpy(&g_ethxLeases, pLeases, sizeof(*pLeases));
        res = dhclient_create_ethxleases((char *)ETHX_LEASES_PATH);
        if (res != MRJE_OK)
        {
            TRACE("> set %s failed. %s %d\r\n", ETHX_LEASES_PATH, MDL);
        }
    }

    return res;
}


static MRJEcode dhclient_set_raxleases(struct DhclientLeases *pLeases)
{
    MRJEcode res = MRJE_OK;

    if (memcmp(&g_raxLeases, pLeases, sizeof(*pLeases)) != 0)
    {
        memcpy(&g_raxLeases, pLeases, sizeof(*pLeases));
        res = dhclient_create_raxleases((char *)RAX_LEASES_PATH);
        if (res != MRJE_OK)
        {
            TRACE("> Set %s failed. %s %d\r\n", RAX_LEASES_PATH, MDL);
        }
    }

    return res;
}


static MRJEcode dhclient_load_ethxleases(char *path, struct DhclientLeases *pLeases)
{   
    MRJEcode res = MRJE_ERR;
    const char * xmlFile = path;
	TiXmlDocument doc;                              

    if (pLeases == NULL)
    {
        TRACE("pLeases is null. %s %d\r\n", MDL);
        return res;
    }
    
	if (doc.LoadFile(xmlFile))
	{
        //doc.Print();
	}
	else
	{
        TRACE("load %s failed. %s %d\r\n", path, MDL);
		return res;
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

    res = MRJE_OK;
    
	return res;
}

static MRJEcode dhclient_load_raxleases(char *path, struct DhclientLeases *pLeases)
{
    MRJEcode res = MRJE_ERR;
    const char * xmlFile = path;
	TiXmlDocument doc;                              

    if (pLeases == NULL)
    {
        TRACE("pLeases is null. %s %d\r\n", MDL);
        return res;
    }
    
	if (doc.LoadFile(xmlFile))
	{
        //doc.Print();
	}
	else
	{
        TRACE("load %s failed. %s %d\r\n", path, MDL);
		return res;
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

    res = MRJE_OK;
    
	return res;
}

MRJEcode dhclient_read_ethxleases()
{
    MRJEcode res = MRJE_ERR;

    res = dhclient_check_file((char *)ETHX_LEASES_PATH, NULL, NULL);
    if (res != MRJE_OK)
    {
        TRACE("> %s not exist %s %d\r\n", ETHX_LEASES_PATH, MDL);
        res = dhclient_create_ethxleases((char *)ETHX_LEASES_PATH);
        if (res != MRJE_OK)
        {
		    TRACE("%s create fail. %s %d\r\n", RAX_LEASES_PATH, MDL);   
        }		
    }
    else
    {
        res = dhclient_load_ethxleases((char *)ETHX_LEASES_PATH, &g_ethxLeases);
        if (res != MRJE_OK)
        {
           TRACE("load %s failed %s %d\r\n", RAX_LEASES_PATH, MDL); 
        }
    }
    
    return res; 
}

static MRJEcode dhclient_read_raxleases()
{
    MRJEcode res = MRJE_ERR;

    res = dhclient_check_file((char *)RAX_LEASES_PATH, NULL, NULL);
    if (res != MRJE_OK)
    {
        TRACE("%s not exist %s %d\r\n", RAX_LEASES_PATH, MDL);
        res = dhclient_create_raxleases((char *)RAX_LEASES_PATH);
        if (res != MRJE_OK)
        {
		    TRACE("%s create fail. %s %d\r\n", RAX_LEASES_PATH, MDL);   
        }		
    }
    else
    {
        res = dhclient_load_raxleases((char *)RAX_LEASES_PATH, &g_raxLeases);
        if (res != MRJE_OK)
        {
           TRACE("load %s failed %s %d\r\n", RAX_LEASES_PATH, MDL); 
        }
    }
    
    return res; 
}

void dhclient_get_ethxleases(struct DhclientLeases *pLeases)
{
    memset(pLeases, 0, sizeof(struct DhclientLeases));
    memcpy(pLeases, &g_ethxLeases, sizeof(struct DhclientLeases));
}


void dhclient_get_raxleases(struct DhclientLeases *pLeases)
{
    memset(pLeases, 0, sizeof(struct DhclientLeases));
    memcpy(pLeases, &g_raxLeases, sizeof(struct DhclientLeases));
}


int dhclient_get_iftype(char *if_name)
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



static MRJEcode dhclient_read_leases()
{
    MRJEcode ret = MRJE_ERR;

    ret = dhclient_read_ethxleases();
    if (ret != MRJE_OK)
    {
        TRACE("read ethx leases failure. %s %d\r\n", MDL);     
    }

    ret = dhclient_read_raxleases();
    if (ret != MRJE_OK)
    {
        TRACE("read rax leases failure. %s %d\r\n", MDL);     
    }

    return ret;
}


static MRJEcode dhclient_init_leases()
{
    MRJEcode res = MRJE_ERR;
    static int s_running_count = 0;

    if (s_running_count++ > 0)
    {
        TRACE("* dhclient leases have been run %d times. %s %d\r\n", s_running_count, MDL);
        return MRJE_OK;
    }
    
    dhclient_init_ethxleases();
    dhclient_init_raxleases();

    mkdir("/var/dhcp", 0777);

    res = dhclient_read_leases();
    if (res != MRJE_OK)
    {
        TRACE("> init leases read leases failure. %s %d\r\n", MDL); 
    }
    
    return res;
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



MRJEcode dhclient_getipstr(char *bufp, size_t bufsize, u_int32_t ip)
{
    if (!bufp)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    struct in_addr src;
	src.s_addr = ip;

    memset(bufp, 0, bufsize);
    
	if (inet_ntop(AF_INET, ((struct sockaddr_in *)&src), bufp, bufsize) == NULL) {
        return MRJE_UNKNOWN;
	}
    
	return MRJE_OK;
}

MRJEcode dhclient_set_result(struct SessionHandle *data)
{
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;
        
    if (set_if_ip(data->resout.fixed_address, data->ifname) != 0)
        return MRJE_UNKNOWN;

    if (set_if_gw(data->resout.router, data->ifname) != 0)
        return MRJE_UNKNOWN;

    if (set_if_mask(data->resout.sub_netmask, data->ifname) != 0)
        return MRJE_UNKNOWN;

    return MRJE_OK;
}

MRJEcode dhclient_update_leases(struct SessionHandle *data)
{   
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    MRJEcode res = MRJE_ERR;
    int type = -1;

    strncpy(data->leases.if_name, data->resout.if_name, sizeof(data->leases.if_name) - 1);
    strncpy(data->leases.fixed_address, data->resout.fixed_address, sizeof(data->leases.fixed_address) - 1);
    strncpy(data->leases.subnet_mask, data->resout.sub_netmask, sizeof(data->leases.subnet_mask) - 1);
    strncpy(data->leases.routers, data->resout.router, sizeof(data->leases.routers) - 1);
    strncpy(data->leases.server_address, data->resout.serv_ident, sizeof(data->leases.server_address) - 1);

    data->leases.lease_time = data->resout.lease_time;                  /* 租约时间 */
    data->leases.renewal_time = data->resout.lease_time / 2;            /* 重新申请时间(1/2租约时间) */
    data->leases.rebinding_time = (data->resout.lease_time * 3) / 4;    /* 再次重新申请时间(3/4租约时间) */

    long timenow = time(NULL);
    long renew_time = timenow + data->leases.renewal_time;
    long rebinding_time = timenow + data->leases.rebinding_time;
    long expire_time = timenow + data->leases.lease_time;               /*  租约到期时的时间 */

    dhclient_conver_time(renew_time, data->leases.renew);
    dhclient_conver_time(rebinding_time, data->leases.rebind);
    dhclient_conver_time(expire_time, data->leases.expire);

    type = dhclient_get_iftype(data->leases.if_name);
    if (0 == type)
    {
        res = dhclient_set_ethxleases(&data->leases);
    }
    else if (1 == type)
    {
        res = dhclient_set_raxleases(&data->leases);
    }
    else
    {
        TRACE("> unkown interface type, %s %s %d\r\n", data->leases.if_name, MDL);
    }
    
    return res;
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
				TRACE("Lease time - %ld Days %ld Hours %ld Minutes\n", \
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
    		TRACE("DHCP Relay agent IP - %s\n", ipstr);
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

static MRJEcode dhclient_select(struct SessionHandle *data, struct timeval *vlp)
{
    int res = -1;
    mrj_socket_t sockfd = data->sockfd;
    fd_set rset, eset;

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    FD_ZERO(&eset);
    FD_SET(sockfd, &eset);

    res = select(sockfd + 1, &rset, NULL, &eset, vlp);
    if (res == 0) {
        dhclient_trace(data, "select time out error", MDL);
        return MRJE_TIME_OUT;
    } else if (res < 0 || FD_ISSET(sockfd, &eset)) {
        dhclient_trace(data, "select error", MDL);
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
        result = dhclient_setstropt(&data->opt.hostname_buff, va_arg(param, char *));
        if (result == MRJE_OK) 
            data->opt.hostname_flag = 1;
        else
            data->opt.hostname_flag = 0;
        break;

    case DHCPOPT_UNICAST_IP:
        data->opt.unicast_ip = inet_addr(va_arg(param, char *));
        if (data->opt.unicast_ip == INADDR_NONE) { 
            data->opt.unicast_ip_flag = 0;
            return MRJE_UNKNOWN;
        }
        else
            data->opt.unicast_ip_flag = 1;
        break;

    case DHCPOPT_SERVER_IP:
        data->opt.server_ident = inet_addr(va_arg(param, char *));
        if (data->opt.server_ident == INADDR_NONE) { 
            data->opt.server_ident_flag = 0;
            return MRJE_UNKNOWN;
        }
        else
            data->opt.server_ident_flag = 1;
        break;
    
    default:
        result = MRJE_UNKNOWN_OPTION;
        break;
    }

    return result;
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
		TRACE("Error: Could not get hardware address of interface '%s'\n", data->ifname);
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

	if ((data->sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) 
    {
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

    if ((data->ifindex = if_nametoindex(data->ifname)) == 0) 
    {
        TRACE("if_nametoindex error(%s). %s %d\r\n", STR_ERROR, MDL);
        return MRJE_UNKNOWN;
    }

    res = dhclient_socketgetifmac(data);
    if (res != MRJE_OK) 
    {
        dhclient_trace(data, "dhclient socketgetifmac error", MDL);
        return res;
    }
    
    res = dhclient_socketopen(data);
    if (res != MRJE_OK) 
    {
        dhclient_trace(data, "dhclient socketopen error", MDL);
        return res;
    }
    
    res = dhclient_socketpromisc(data->sockfd, data->ifname);
    if (res != MRJE_OK) 
    {
        dhclient_trace(data, "dhclient_socketpromisc error", MDL);
        return res;
    }
    
    return res;
}

MRJEcode dhclient_opt53_msgtype(struct SessionHandle *data)
{   
    MRJEcode res = MRJE_OK;
    u_int8_t msgtype;
    u_int8_t msglen;
    u_int8_t msg;
        
    MRJ_DhcpMsg dhcpmsg = data->set.sendmsg;
    
    switch (dhcpmsg) {
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
    
	dhclient_opt_endof(data);

    return res;
}


MRJEcode dhclient_build_request_pack(struct SessionHandle *data)
{
    MRJEcode res = MRJE_OK;
    u_int16_t buf[1024] = {0};
    u_int16_t len = 0;
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
		iph->saddr = data->opt.unicast_ip;
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
		dhpointer->dhcp_cip = data->opt.unicast_ip;
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
    u_int16_t len = 0;
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
		iph->saddr = data->opt.unicast_ip;
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
		dhpointer->dhcp_cip = data->opt.unicast_ip;
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
    return MRJE_OK;
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
        dhclient_trace(data, "dhcp send packet fail", MDL);
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
                                            sizeof(struct udphdr));
        
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
			    dhclient_trace(data, "Check offer packet type error", MDL);
				return MRJE_UNKNOWN;
			}
		} else {
		    dhclient_trace(data, "Check offer packet type error", MDL);
			return MRJE_UNKNOWN;
		}       
    } else {
    
        if (data->eth_hdrp->ether_type == htons(ETHERTYPE_IP) && 
             data->ip_hdrp->protocol == 17 && 
             data->udp_hdrp->source == htons(DHCP_SRC_PORT) && 
             data->udp_hdrp->dest == htons(DHCP_SRC_PORT + 1)) 
        {
			if (*(data->dhcp_optp + 2) == DHCP_MSGOFFER && 
                htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) {
				return MRJE_OK;
			} else {
			    dhclient_trace(data, "Check offer packet type error", MDL);
				return MRJE_UNKNOWN;
			}
		} else {
		    dhclient_trace(data, "Check offer packet type error", MDL);
			return MRJE_UNKNOWN;
		}
    }

    dhclient_trace(data, "Check offer packet type error", MDL);
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
             data->udp_hdrp->dest == htons(DHCP_SRC_PORT + 1)) 
        {
		    if (*(data->dhcp_optp + 2) == DHCP_MSGACK && 
                  htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) {
                  
			    return MRJE_OK;
                
		    } else if (*(data->dhcp_optp + 2) == DHCP_MSGNACK && 
		               htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) {
		               
                TRACE("DHCP nack received\t - Client MAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                        data->srcmac[0], data->srcmac[1], 
                        data->srcmac[2], data->srcmac[3], 
                        data->srcmac[4], data->srcmac[5]);
                
                dhclient_trace(data, "Check ack packet, recv nak packet", MDL);
			    return MRJE_RECV_NAK;
		    } else {
		    
                dhclient_trace(data, "Check ack packet type error", MDL);
			    return MRJE_UNKNOWN;
		    }
		} else {
		
		    dhclient_trace(data, "Check ack packet type error", MDL);
			return MRJE_UNKNOWN;
		}
    } else {
        if (data->eth_hdrp->ether_type == htons(ETHERTYPE_IP) && 
            data->ip_hdrp->protocol == 17 && 
            data->udp_hdrp->source == htons(DHCP_SRC_PORT) && 
            data->udp_hdrp->dest == htons(DHCP_SRC_PORT + 1)) 
        {    
			if (*(data->dhcp_optp + 2) == DHCP_MSGACK && 
                htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) {
                
				return MRJE_OK;
                
			} else if (*(data->dhcp_optp + 2) == DHCP_MSGNACK && 
                        htonl(data->dhcp_hdrp->dhcp_xid) == data->opt.dhcp_xid) {
                        
                TRACE("DHCP nack received\t - Client MAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                            data->srcmac[0], data->srcmac[1], 
                            data->srcmac[2], data->srcmac[3], 
                            data->srcmac[4], data->srcmac[5]);
                dhclient_trace(data, "Check ack packet, recv nak packet", MDL);
				return MRJE_RECV_NAK;
                
			} else {
			
                dhclient_trace(data, "Check ack packet type error", MDL);
				return MRJE_UNKNOWN;
			}
		} else {
		
            dhclient_trace(data, "Check ack packet type error", MDL);
			return MRJE_UNKNOWN;
		}
    }

    return MRJE_UNKNOWN;
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
        dhclient_trace(data, "Message type is not support", MDL);
        return MRJE_UNSUPPORTED_PROTOCOL;
    }

    return res;
}

MRJEcode dhclient_recv_pack(struct SessionHandle *data)
{
    int recvlen = 0;
    MRJEcode res = MRJE_UNKNOWN;
    socklen_t addrlen = sizeof(struct sockaddr_ll);
    struct timeval tv;

    tv.tv_sec = data->set.timeout / 1000L;
    tv.tv_usec = 0;

    while (tv.tv_sec != 0) 
    {
        res = dhclient_select(data, &tv);
        if (res != MRJE_OK)
            return res;
        
        memset(data->recv_buf, 0, sizeof(data->recv_buf));
            
        recvlen = recvfrom(data->sockfd,
                           data->recv_buf,
                           sizeof(data->recv_buf), 
                           0,
                           (struct sockaddr *)&data->sall,
                           &addrlen);

        if (recvlen <= 0) {
            dhclient_trace(data, "dhclient recvfrom error", MDL);
            return MRJE_RECV_ERROR;
        }
        
        data->recv_buf[recvlen] = '\0';

        TRACE("* recvlen %d %s %d\r\n", recvlen, MDL);

        if (recvlen >= 60) {
            res = dhclient_check_pack(data);
            if (res == MRJE_OK) {
                dhclient_trace(data, "dhclient check recv pack error", MDL);
                return res;
            }
        }

        usleep(100);
    }
    
    return res;
}


MRJEcode dhclient_transfer_pack(struct SessionHandle *data) 
{
    MRJEcode res = MRJE_UNKNOWN;

    res = dhclient_option(data);
    if (res != MRJE_OK) {
        dhclient_trace(data, "dhclient option error", MDL);
        return res;
    }
    
    res = dhclient_build_packet(data);
    if (res != MRJE_OK) {
        dhclient_trace(data, "dhclient build packet error", MDL);
        return res;
    }
    
    res = dhclient_send_pack(data);
    if (res != MRJE_OK) {
        dhclient_trace(data, "dhclient send packet error", MDL);
        return res;
    }
    
    res = dhclient_recv_pack(data);
    if (res != MRJE_OK) {
        dhclient_trace(data, "dhclient recv packet error", MDL);
        return res;
    }
    
    return res;
}

MRJEcode dhclient_transfer(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;

    if (!data)
    {
        return MRJE_BAD_FUNCTION_ARGUMENT;
    }
    
    res = dhclient_socketopt(data);
    if (res != MRJE_OK)
    {
        return res;
    }
    
    res = dhclient_transfer_pack(data);
    if (res != MRJE_OK)
    {
        return res;
    }
    
    if (data->set.sendmsg == DHCPMSG_DISCOVER) 
    {
        data->set.sendmsg = DHCPMSG_REQUEST;
        data->set.recvmsg = DHCPMSG_ACK;

        res = dhclient_parse_offer(data);
        if (res != MRJE_OK)
        {
            return res;
        }
        
        res = dhclient_transfer_pack(data);
        if (res != MRJE_OK)
        {
            return res;
        }
    }
    
    res = dhclient_parse_ack(data);
    if (res != MRJE_OK)
    {
        return res;
    }

    res = dhclient_set_result(data);
    if (res != MRJE_OK)
    {
        return res;
    }
    
    res = dhclient_update_leases(data);
    if (res != MRJE_OK)
    {
        return res;
    }
    
    return res;
}

MRJEcode dhclient_get_ifleases(char *if_name, struct DhclientLeases *leases)
{
    MRJEcode res = MRJE_ERR;
    int type = dhclient_get_iftype(if_name);

    switch (type) 
    {
    case 0:
        dhclient_get_ethxleases(leases);
        res = MRJE_OK;
        break;

    case 1:
        dhclient_get_raxleases(leases);
        res = MRJE_OK;
        break;

    default:
        TRACE("Unknow interface type. %s %d\r\n", MDL);
        break;
    }
    
    return res;
}

MRJEcode dhclient_perform_request(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;
    
    data->set.sendmsg = DHCPMSG_REQUEST;
    data->set.recvmsg = DHCPMSG_ACK;

    res = dhclient_transfer(data);

    return res;
}

MRJEcode dhclient_perform_discover(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;
    
    data->set.sendmsg = DHCPMSG_DISCOVER;
    data->set.recvmsg = DHCPMSG_OFFER;

    res = dhclient_transfer(data);

    return res;
}

MRJEcode dhclient_perform(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;
    char now_time_buf[20] = {0};
    struct DhclientLeases leases;
        
    res = dhclient_init_leases();
    if (res != MRJE_OK)
    {
        TRACE("> dhclient init leases failure. %s %d\r\n", MDL);
        return res;
    }
    
    res = dhclient_get_ifleases(data->ifname, &leases);
    if (res != MRJE_OK) 
    {
        TRACE("* dhclient_perform get %s leases failure. %s %d\r\n", data->ifname, MDL);
        return res;
    }
    
    if (strcmp(leases.if_name, data->ifname) != 0) 
    {
        TRACE("* leases record's ifname not equal request ifname, "\
              "send discover packet. %s %d\r\n", MDL);
        res = dhclient_perform_discover(data); 
        return res;
    }

    char now_time_buf[20] = {0};
    dhclient_conver_time(time(NULL), now_time_buf);
    
    if ((strcmp(now_time_buf, leases.expire) > 0 
            && strcmp(now_time_buf, leases.rebind) < 0)
        || (strcmp(now_time_buf, leases.rebind) > 0 
            && strcmp(now_time_buf, leases.expire) < 0))
    {
        TRACE("* dhcp leases don't expire, send request packet. %s %d\r\n", MDL);

        res = dhclient_conver_opt5054(data, leases.fixed_address, leases.server_address);
        if (res != MRJE_OK)
        {
            return res;
        }

        res = dhclient_perform_request(data);
        if (res != MRJE_OK) 
        {
            res = dhclient_perform_discover(data);
        }           
    }
    else
    {
        TRACE("* dhcp leases expire, send discover packet. %s %d\r\n", MDL);
        res = dhclient_perform_discover(data);
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
    MRJ_safefree(data->opt.hostname_buff);        /* strdup */

    free(data);
    
    return MRJE_OK;
}


MRJEcode mrj_dhclient_setopt(MRJDHCLIENT *dhclient, MRJDHCLIENToption tag, ...)
{   
    va_list arg;
    struct SessionHandle *data = (struct SessionHandle *)dhclient;
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


MRJEcode mrj_dhclient_perform(MRJDHCLIENT *data)
{
    return dhclient_perform((struct SessionHandle *)data);
}

void mrj_dhclient_cleanup(MRJDHCLIENT *handle)
{
    struct SessionHandle *data = (struct SessionHandle *)handle;

    if (!data)
        return;

    dhclient_close(data);
}


int mrj_dhclient(char *ifname)
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
        TRACE("MRJ dhclient perform fail. %d %s %d\r\n", (int)res, MDL);
        mrj_dhclient_cleanup(handle);
        return (-1);
    }

    mrj_dhclient_cleanup(handle);

    return (0);
}


int main(int argc, char **argv)
{
    if (argc != 2) {
        TRACE("usage: dhclient <if_name> %s %d\r\n", MDL);
        return (-1);
    }
    
    int ret = mrj_dhclient(argv[1]);
    if (ret == 0) {
        printf("\r\ndhcp get ip address success. \r\n");
    } else {
        printf("\r\ndhcp get ip address failure. \r\n");
    }
    
    return ret;
}

