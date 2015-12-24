
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "dhcp.h"

#define DHCLIENT_DEBUG 1

static char s_ipBuff[32] = {0};

char *DHClientGetIpString(u_int32_t ip)
{
	struct in_addr src;
    memset(&src, '\0', sizeof(src));
    memset(s_ipBuff, '\0', sizeof(s_ipBuff));

    src.s_addr = ip;
    
	inet_ntop(AF_INET, ((struct sockaddr_in *)&src), s_ipBuff, sizeof(s_ipBuff));
    
	return s_ipBuff;
}

int DHClientInitMacAddress(const char *ifname, unsigned char *macBuff, int buffSize)
{
    int sock = -1;
	struct ifreq ifr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        TRACE("> socket error. %s %s %d\r\n", strerror(errno), MDL);
        return -1;
    }
    
	strncpy((char *)&ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
    {
		TRACE("> get hardware address of interface(%s) fail. \n", ifname);
        close(sock);
		return -1;
    }

    memset(macBuff, 0, buffSize);
    memcpy(&macBuff[0], &ifr.ifr_hwaddr.sa_data, 6);

    close(sock);
    
	return 0;
}

int DHClientInitBroadcastAddr(struct dhcp_client *client)
{
    /* send the DHCPDISCOVER packet to broadcast address */
    client->broadcastAddr.sin_family        = AF_INET;
    client->broadcastAddr.sin_port          = htons(DHCP_SERVER_PORT);
    client->broadcastAddr.sin_addr.s_addr   = INADDR_BROADCAST;
	bzero(&client->broadcastAddr.sin_zero, sizeof(client->broadcastAddr.sin_zero));

    return 0;
}

int DHClientInitSocket(struct dhcp_client *client, const char *ifname)
{
    struct sockaddr_in sockaddr;
	struct ifreq ifr;
    int flag = 1;

    /* Set up the address we're going to bind to. */
	bzero(&sockaddr, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(DHCP_CLIENT_PORT);
    sockaddr.sin_addr.s_addr = INADDR_ANY;                 /* listen on any address */
    bzero(&sockaddr.sin_zero, sizeof(sockaddr.sin_zero));
    
    /* create a socket for DHCP communications */
	client->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client->sockfd < 0)
    {
	    TRACE("> ERROR, could not create socket. \n");
	    return -1;
    }

    /* set the reuse address flag so we don't get errors when restarting */
    flag = 1;
    if (setsockopt(client->sockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&flag, sizeof(flag)) < 0)
    {
	    TRACE("> ERROR, could not set reuse address option on DHCP socket. \n");
	    return -1;
    }

    /* set the broadcast option - we need this to listen to DHCP broadcast messages */
    if (setsockopt(client->sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&flag, sizeof(flag)) < 0)
    {
	    TRACE("> ERROR, could not set broadcast option on DHCP socket. %s %d\r\n", MDL);
	    return -1;
    }

	/* bind socket to interface */
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	if (setsockopt(client->sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0)
    {
		TRACE("> ERROR, could not bind socket to interface %s. %s %d\r\n", ifname, MDL);
		return -1;
	}

    /* bind the socket */
    if (bind(client->sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
	    TRACE("> ERROR, could not bind to DHCP socket (port %d). %s %d\r\n", DHCP_CLIENT_PORT, MDL);
	    return -1;
    }

    return 0;
}

int DHClientInit(struct dhcp_client *client, const char *ifname, const char *ssid, const char *hostname)
{
    int ret = -1;
    
    if (!client || !ifname)
    {
        return -1;
    }

    memset(client, 0, sizeof(struct dhcp_client));
    client->sockfd = -1;
    client->timeout = TIME_OUT;

    /* interface name */
    strncpy(client->ifname, ifname, sizeof(client->ifname) - 1);

    /* hostname */
    if (hostname != NULL && hostname[0] != '\0')
    {
        strncpy((char *)client->opt.hostname, hostname, sizeof(client->opt.hostname) - 1);
        client->opt.hostname[sizeof(client->opt.hostname) - 1] = '\0';
        client->opt.hostnameFlag = 1;
    }

    /* wifi ssid */
    if (ssid != NULL && ssid[0] != '\0')
    {
        strncpy(client->ssid, ssid, sizeof(client->ssid) - 1);
        client->ssid[sizeof(client->ssid) - 1] = '\0';
    }
    
    ret = DHClientInitMacAddress(ifname, client->srcMac, ETHER_ADDR_LEN);
    if (0 != ret)
    {
        TRACE("> ERROR, get local mac address fail. \n");
        return -1;
    }
    
    DHClientInitBroadcastAddr(client);

    ret = DHClientInitSocket(client, ifname);
    if (0 != ret)
    {
        TRACE("> ERROR, create socket fail. \n");    
        return -1;
    }
    
    return 0;
}

void DHClientDeinit(struct dhcp_client *client)
{
    if (client->sockfd >= 0)
    {
        close(client->sockfd);
        client->sockfd = -1;
    }
}

void DHClientSetDhcpXid(struct dhcp_option *opt)
{
	if (opt->dhcpXid == 0) 
    {
		srand(time(NULL) ^ (getpid() << 16));
		opt->dhcpXid = rand() % 0xffffffff;
	}
}

void DHClientBuildOption53MessageType(u_int8_t dhcpMsgType, struct dhcp_option *option)
{
    u_int8_t msgtype = DHCP_MESSAGETYPE;
	u_int8_t msglen  = 1;
	u_int8_t msg     = dhcpMsgType;

	memcpy(option->optBuff, &msgtype, 1);
    strncpy((char *)(option->optBuff + 1), (char *)&msglen, 1);
    strncpy((char *)(option->optBuff + 2), (char *)&msg, 1);
	option->optSize = option->optSize + 3;
}

void DHClientBuildOption50RequestIp(struct dhcp_option *opt)
{   
    if (opt->option50RequestIp)
    {
        TRACE("--------option50_ip: %u %s %d\r\n", opt->option50RequestIp, MDL);
        TRACE("------- option50_ip: %s \n", DHClientGetIpString(opt->option50RequestIp));
        
        u_int8_t msgtype = DHCP_REQUESTEDIP;
    	u_int8_t msglen = 4;
    	u_int32_t msg = opt->option50RequestIp; 

    	memcpy((opt->optBuff + opt->optSize), &msgtype, 1);
    	memcpy((opt->optBuff + opt->optSize + 1), &msglen, 1);
    	memcpy((opt->optBuff + opt->optSize + 2), &msg, 4);
    	opt->optSize = opt->optSize + 6; 
    }
}

void DHClientBuildOption51LeaseTime(struct dhcp_option *opt)
{
    if (opt->option51LeaseTime > 0)
    {
    	u_int8_t msgtype = DHCP_LEASETIME;
    	u_int8_t msglen = 4;
    	u_int32_t msg = htonl(opt->option51LeaseTime); 

    	memcpy((opt->optBuff + opt->optSize), &msgtype, 1);
    	memcpy((opt->optBuff + opt->optSize + 1), &msglen, 1);
    	memcpy((opt->optBuff + opt->optSize + 2), &msg, 4);
    	opt->optSize = opt->optSize + 6; 
    }
}

void DHClientBuildOption54ServIdent(struct dhcp_option *opt)
{
    if (opt->servId)
    {
    	u_int8_t msgtype = DHCP_SERVIDENT;
    	u_int8_t msglen = 4;
    	u_int32_t msg = opt->servId;

#if DHCLIENT_DEBUG
        TRACE("------- server_id: %s \n", DHClientGetIpString(opt->servId));
#endif
        
    	memcpy((opt->optBuff + opt->optSize), &msgtype, 1);
    	memcpy((opt->optBuff + opt->optSize + 1), &msglen, 1);
    	memcpy((opt->optBuff + opt->optSize + 2), &msg, 4);
    	opt->optSize = opt->optSize + 6; 
    }
}


void DHClientBuildOption12Hostname(struct dhcp_option *opt)
{
    if (opt->hostnameFlag == 1)
    {
    	u_int32_t msgtype = DHCP_HOSTNAME;
    	u_int32_t msglen = strlen((char *)opt->hostname);
        
    	memcpy((opt->optBuff + opt->optSize), &msgtype, 1);
    	memcpy((opt->optBuff + opt->optSize + 1), &msglen, 1);
    	memcpy((opt->optBuff + opt->optSize + 2), opt->hostname, msglen);

    	opt->optSize = opt->optSize + 2 + msglen;
    }
}

void DHClientBuildOption55Extra(struct dhcp_option *opt) 
{
	u_int32_t msgtype = DHCP_PARAMREQUEST;
	u_int32_t msglen = 4;
	u_int8_t msg[4] = { 0 };
    
	msg[0] = DHCP_SUBNETMASK;
	msg[1] = DHCP_ROUTER;
	msg[2] = DHCP_DOMAINNAME;
	msg[3] = DHCP_DNS;
	/* msg[4] = DHCP_LOGSERV; */

	memcpy((opt->optBuff + opt->optSize), &msgtype, 1);
	memcpy((opt->optBuff + opt->optSize + 1), &msglen, 1);
	memcpy((opt->optBuff + opt->optSize + 2), msg, 4);
	opt->optSize = opt->optSize + 6; 
}

void DHClientBuildOptionEof(struct dhcp_option *opt)
{
	u_int8_t eof = 0xff;
    
	memcpy((opt->optBuff + opt->optSize), &eof, 1);
	opt->optSize = opt->optSize + 1; 
}

void DHClientResetDhcpOptionBuff(struct dhcp_option *opt)
{
    memset(opt->optBuff, 0, sizeof(opt->optBuff));
	opt->optSize = 0;
}

void DHClientBuildRequestOption(struct dhcp_option *option)
{
    DHClientResetDhcpOptionBuff(option);
    DHClientBuildOption53MessageType(DHCP_MSGREQUEST, option);
    DHClientBuildOption50RequestIp(option);
    DHClientBuildOption54ServIdent(option);
    DHClientBuildOption12Hostname(option);
    DHClientBuildOption51LeaseTime(option);
    DHClientBuildOption55Extra(option);
    DHClientBuildOptionEof(option);
}

int DHClientBuildRequestPacket(struct dhcp_client *client)
{   
    memset(client->sendBuff, 0, sizeof(client->sendBuff));

    struct dhcpv4_header dhcpHeader;
    memset(&dhcpHeader, 0, sizeof(dhcpHeader));
    struct dhcpv4_header *dhhdr = &dhcpHeader;

    dhhdr->dhcp_opcode = DHCP_REQUEST;
	dhhdr->dhcp_htype = ARPHRD_ETHER;
	dhhdr->dhcp_hlen = ETHER_ADDR_LEN;
	dhhdr->dhcp_hopcount = 0;
	dhhdr->dhcp_xid = htonl(client->opt.dhcpXid);
	dhhdr->dhcp_secs = 0;
	dhhdr->dhcp_flags = client->opt.bcastFlag;
	if (client->opt.unicastFlag)
		dhhdr->dhcp_cip = client->opt.unicastIpAddress;
	else
		dhhdr->dhcp_cip = 0;
	dhhdr->dhcp_yip = 0;
	dhhdr->dhcp_sip = 0;
	dhhdr->dhcp_gip = inet_addr(DHCP_GIADDR);
	memcpy(dhhdr->dhcp_chaddr, client->srcMac, ETHER_ADDR_LEN);
	/*dhpointer->dhcp_sname 
	  dhpointer->dhcp_file*/
	dhhdr->dhcp_magic = htonl(DHCP_MAGIC);
    
    memcpy(client->sendBuff, (char *)dhhdr, sizeof(struct dhcpv4_header)); 
    
	u_char *dhopt_pointer = (u_char *)(client->sendBuff + sizeof(struct dhcpv4_header));
	memcpy(dhopt_pointer, client->opt.optBuff, client->opt.optSize);
    
    client->sendLen = sizeof(struct dhcpv4_header) + client->opt.optSize;

    return 0;
}

void DHClientBuildDiscoverOption(struct dhcp_option *option)
{
    DHClientBuildOption53MessageType(DHCP_MSGDISCOVER, option);
    DHClientBuildOption12Hostname(option);
    DHClientBuildOption50RequestIp(option);
    DHClientBuildOption51LeaseTime(option);
    DHClientBuildOptionEof(option);
}

int DHClientBuildDiscoverPacket(struct dhcp_client *client)
{
    memset(client->sendBuff, 0, sizeof(client->sendBuff));

    struct dhcpv4_header dhcpHeader;
    memset(&dhcpHeader, 0, sizeof(dhcpHeader));
    struct dhcpv4_header *dhpointer = &dhcpHeader;

    dhpointer->dhcp_opcode = DHCP_REQUEST;
	dhpointer->dhcp_htype = ARPHRD_ETHER;
	dhpointer->dhcp_hlen = ETHER_ADDR_LEN;
	dhpointer->dhcp_hopcount = 0;
	dhpointer->dhcp_xid = htonl(client->opt.dhcpXid);
	dhpointer->dhcp_secs = 0;
	dhpointer->dhcp_flags = client->opt.bcastFlag;
	if (client->opt.unicastFlag)
		dhpointer->dhcp_cip = client->opt.unicastIpAddress;
	else
		dhpointer->dhcp_cip = 0;
	dhpointer->dhcp_yip = 0;
	dhpointer->dhcp_sip = 0;
	dhpointer->dhcp_gip = inet_addr(DHCP_GIADDR);
	memcpy(dhpointer->dhcp_chaddr, client->srcMac, ETHER_ADDR_LEN);
	/*dhpointer->dhcp_sname 
	  dhpointer->dhcp_file*/
	dhpointer->dhcp_magic = htonl(DHCP_MAGIC);

    memcpy(client->sendBuff, (char *)dhpointer, sizeof(struct dhcpv4_header)); 
    u_char *dhopt_pointer = (u_char *)(client->sendBuff + sizeof(struct dhcpv4_header));
    memcpy(dhopt_pointer, client->opt.optBuff, client->opt.optSize);
    client->sendLen = sizeof(struct dhcpv4_header) + client->opt.optSize;

    return 0;
}

void DHClientSetServidOpt50RequestIp(struct dhcp_client *client)
{    
	client->opt.option50RequestIp = client->dhcpHeader->dhcp_yip;        /* offerered ip */
    u_int8_t *tmp_dhopt = client->dhcpOption;
    
	while (*(tmp_dhopt) != DHCP_END) 
    {        
        u_int32_t tmp = 0;
        
		if (*(tmp_dhopt) == DHCP_SERVIDENT) 
        {   
            memcpy(&tmp, tmp_dhopt + 2, sizeof(tmp));
            client->opt.servId = tmp;
#if DHCLIENT_DEBUG       
            TRACE("* server_id: (%u)%s \n", client->opt.servId, DHClientGetIpString(client->opt.servId));
#endif
		}
        
		tmp_dhopt = tmp_dhopt + *(tmp_dhopt + 1) + 2;
	}
}

int DHClientParseOfferResponse(struct dhcp_client *client)
{
    u_int16_t tmp;

#if DHCLIENT_DEBUG
    TRACE("\n* DHCP offer details\n");
	TRACE("* ----------------------------------------------------------\n");
	TRACE("* DHCP offered IP from server - %s\n", DHClientGetIpString(client->dhcpHeader->dhcp_yip));
	TRACE("* Next server IP(Probably TFTP server) - %s\n", DHClientGetIpString(client->dhcpHeader->dhcp_sip));

    if (client->dhcpHeader->dhcp_gip)
    {
		TRACE("* DHCP Relay agent IP - %s\n", DHClientGetIpString(client->dhcpHeader->dhcp_gip));
	}
#endif

    memset(&client->result, 0, sizeof(struct dhcp_result));
    if (client->ifname[0] != '\0')
    {
        strncpy(client->result.ifname, client->ifname, sizeof(client->result.ifname) - 1);
        client->result.ifname[sizeof(client->result.ifname) - 1] = '\0';
    }

    if (client->ssid[0] != '\0')
    {
        strncpy(client->result.ssid, client->ssid, sizeof(client->result.ssid) - 1);
        client->result.ssid[sizeof(client->result.ssid) - 1] = '\0';
    }

    strncpy(client->result.fixedAddress, DHClientGetIpString(client->dhcpHeader->dhcp_yip), 
                sizeof(client->result.fixedAddress) - 1);
    client->result.fixedAddress[sizeof(client->result.fixedAddress) - 1] = '\0';

    u_int8_t *tmp_dhopt = client->dhcpOption;
    
    while (*(tmp_dhopt) != DHCP_END)
    {   
        u_int32_t tmp_data = 0;
        
		switch (*(tmp_dhopt))
        {
			case DHCP_SERVIDENT:
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 2, sizeof(tmp_data));
                strncpy(client->result.servIdent, DHClientGetIpString(tmp_data), sizeof(client->result.servIdent) - 1);
                client->result.servIdent[sizeof(client->result.servIdent) - 1] = '\0';
#if DHCLIENT_DEBUG
                TRACE("* DHCP server  - %s\n", client->result.servIdent);
#endif
				break;

			case DHCP_LEASETIME:
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 2, sizeof(tmp_data));
                client->result.leaseTime = ntohl(tmp_data);
#if DHCLIENT_DEBUG
				TRACE("* Lease time - %d Days %d Hours %d Minutes\n", \
						(client->result.leaseTime) / (3600 * 24), \
						((client->result.leaseTime) % (3600 * 24)) / 3600, \
						(((client->result.leaseTime) % (3600 * 24)) % 3600) / 60); 
#endif
				break;

			case DHCP_SUBNETMASK:
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 2, sizeof(tmp_data));
                strncpy(client->result.subnetMask, DHClientGetIpString(tmp_data), sizeof(client->result.subnetMask) - 1);
                client->result.subnetMask[sizeof(client->result.subnetMask) - 1] = '\0';
#if DHCLIENT_DEBUG
				TRACE("* Subnet mask - %s\n", client->result.subnetMask);
#endif
				break;

			case DHCP_ROUTER:
				for(tmp = 0; tmp < (*(tmp_dhopt + 1) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy((char *)&tmp_data, (tmp_dhopt + 2 + (tmp * 4)), sizeof(tmp_data));
                    strncpy(client->result.router, DHClientGetIpString(tmp_data), sizeof(client->result.router) - 1);
                    client->result.router[sizeof(client->result.router) - 1] = '\0';
#if DHCLIENT_DEBUG
					TRACE("* Router/gateway - %s\n", client->result.router);
#endif
				}
				break;

			case DHCP_DNS:
				for(tmp = 0; tmp < ((*(tmp_dhopt + 1)) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy(&tmp_data, (tmp_dhopt + 2 + (tmp * 4)), sizeof(tmp_data));
                    strncpy(client->result.dnsServer, DHClientGetIpString(tmp_data), sizeof(client->result.dnsServer) - 1);
                    client->result.dnsServer[sizeof(client->result.dnsServer) - 1] = '\0';
#if DHCLIENT_DEBUG
					TRACE("* DNS server - %s\n", client->result.dnsServer);
#endif
				}
				break;

			case DHCP_FQDN:
			{
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 1, sizeof(tmp_data));
                
				/* Minus 3 beacause 3 bytes are used to flags, rcode1 and rcode2 */
				u_int32_t size = tmp_data - 3;
                
				/* Plus 2 to add string terminator */
				u_char fqdn_client_name[size + 1];

				/* Plus 5 to reach the beginning of the string */
				memcpy(fqdn_client_name, tmp_dhopt + 5, size);
				fqdn_client_name[size] = '\0';
                
#if DHCLIENT_DEBUG
				TRACE("* FQDN Client name - %s\n", fqdn_client_name);
#endif
			}
		}

		tmp_dhopt = tmp_dhopt + *(tmp_dhopt + 1) + 2;
	}

    TRACE("* ---------------------------------------------------------- %s %d\r\n", MDL);
    TRACE("* dhcp offer details: \n");
    TRACE("* ip                - %s \r\n", client->result.fixedAddress);
    TRACE("* subnetmask        - %s \r\n", client->result.subnetMask);
    TRACE("* gateway           - %s \r\n", client->result.router);
    TRACE("* dnsserver         - %s \r\n", client->result.dnsServer);
    TRACE("* leasetime         - %d \r\n", client->result.leaseTime);
    TRACE("* ---------------------------------------------------------- %s %d\r\n", MDL);
    
	return 0;
}

int DHClientParseAckResponse(struct dhcp_client *client)
{
    u_int16_t tmp;

#if DHCLIENT_DEBUG
	TRACE("\n* DHCP ack details\n");
	TRACE("* ----------------------------------------------------------\n");
	TRACE("* DHCP offered IP from server - %s\n", DHClientGetIpString(client->dhcpHeader->dhcp_yip));
	TRACE("* Next server IP(Probably TFTP server) - %s\n", DHClientGetIpString(client->dhcpHeader->dhcp_sip));

    if (client->dhcpHeader->dhcp_gip) 
    {
		TRACE("* DHCP Relay agent IP - %s\n", DHClientGetIpString(client->dhcpHeader->dhcp_gip));
	}
#endif

    memset(&client->result, 0, sizeof(struct dhcp_result));
    if (client->ifname[0] != '\0')
    {
        strncpy(client->result.ifname, client->ifname, sizeof(client->result.ifname) - 1);
        client->result.ifname[sizeof(client->result.ifname) - 1] = '\0';
    }

    if (client->ssid[0] != '\0')
    {
        strncpy(client->result.ssid, client->ssid, sizeof(client->result.ssid) - 1);
        client->result.ssid[sizeof(client->result.ssid) - 1] = '\0';
    }

    strncpy(client->result.fixedAddress, DHClientGetIpString(client->dhcpHeader->dhcp_yip), 
                sizeof(client->result.fixedAddress) - 1);
    client->result.fixedAddress[sizeof(client->result.fixedAddress) - 1] = '\0';
    
    u_int8_t *tmp_dhopt = client->dhcpOption;
    
    while (*(tmp_dhopt) != DHCP_END)
    {   
        u_int32_t tmp_data = 0;
        
		switch (*(tmp_dhopt))
        {
			case DHCP_SERVIDENT:
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 2, sizeof(tmp_data));
                strncpy(client->result.servIdent, DHClientGetIpString(tmp_data), sizeof(client->result.servIdent) - 1);
                client->result.servIdent[sizeof(client->result.servIdent) - 1] = '\0';
#if DHCLIENT_DEBUG
                TRACE("* DHCP server  - %s\n", client->result.servIdent);
#endif
				break;

			case DHCP_LEASETIME:
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 2, sizeof(tmp_data));
                client->result.leaseTime = ntohl(tmp_data);
                
#if DHCLIENT_DEBUG
				TRACE("* Lease time - %d Days %d Hours %d Minutes\n", \
						(client->result.leaseTime) / (3600 * 24), \
						((client->result.leaseTime) % (3600 * 24)) / 3600, \
						(((client->result.leaseTime) % (3600 * 24)) % 3600) / 60); 
#endif   
				break;

			case DHCP_SUBNETMASK:
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 2, sizeof(tmp_data));
                strncpy(client->result.subnetMask, DHClientGetIpString(tmp_data), sizeof(client->result.subnetMask) - 1);
                client->result.subnetMask[sizeof(client->result.subnetMask) - 1] = '\0';
#if DHCLIENT_DEBUG
				TRACE("* Subnet mask - %s\n", client->result.subnetMask);
#endif
				break;

			case DHCP_ROUTER:
				for(tmp = 0; tmp < (*(tmp_dhopt + 1) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy((char *)&tmp_data, (tmp_dhopt + 2 + (tmp * 4)), sizeof(tmp_data));
                    strncpy(client->result.router, DHClientGetIpString(tmp_data), sizeof(client->result.router) - 1);
                    client->result.router[sizeof(client->result.router) - 1] = '\0';
#if DHCLIENT_DEBUG
					TRACE("* Router/gateway - %s\n", client->result.router);
#endif
				}
				break;

			case DHCP_DNS:
				for(tmp = 0; tmp < ((*(tmp_dhopt + 1)) / 4); tmp++) 
                {
                    tmp_data = 0;
                    memcpy(&tmp_data, (tmp_dhopt + 2 + (tmp * 4)), sizeof(tmp_data));
                    strncpy(client->result.dnsServer, DHClientGetIpString(tmp_data), sizeof(client->result.dnsServer) - 1);
                    client->result.dnsServer[sizeof(client->result.dnsServer) - 1] = '\0';
#if DHCLIENT_DEBUG
					TRACE("* DNS server - %s\n", client->result.dnsServer);
#endif
				}
				break;

			case DHCP_FQDN:
			{
                tmp_data = 0;
                memcpy((char *)&tmp_data, tmp_dhopt + 1, sizeof(tmp_data));
                
				/* Minus 3 beacause 3 bytes are used to flags, rcode1 and rcode2 */
				u_int32_t size = tmp_data - 3;
                
				/* Plus 2 to add string terminator */
				u_char fqdn_client_name[size + 1];

				/* Plus 5 to reach the beginning of the string */
				memcpy(fqdn_client_name, tmp_dhopt + 5, size);
				fqdn_client_name[size] = '\0';

#if DHCLIENT_DEBUG
				TRACE("* FQDN Client name - %s\n", fqdn_client_name);
#endif
			}
		}

		tmp_dhopt = tmp_dhopt + *(tmp_dhopt + 1) + 2;
	}

	TRACE("* ---------------------------------------------------------- %s %d\r\n", MDL);
    TRACE("* dhcp ack details: \n");
    TRACE("* ip                - %s \r\n", client->result.fixedAddress);
    TRACE("* subnetmask        - %s \r\n", client->result.subnetMask);
    TRACE("* gateway           - %s \r\n", client->result.router);
    TRACE("* dnsserver         - %s \r\n", client->result.dnsServer);
    TRACE("* leasetime         - %d \r\n", client->result.leaseTime);
    TRACE("* ---------------------------------------------------------- %s %d\r\n", MDL);

#if 0
    if (client->result.leaseTime <= 0 || client->result.leaseTime > 86400)
    {
        client->result.leaseTime = 86400;          
    }
    
    if (client->result.fixedAddress[0] != '\0' &&     /* ip */
        IsIP(client->result.fixedAddress) == 0 &&     
        result->router[0] != '\0' &&            /* router */
        IsIP(result->router) == 0 &&            
        result->sub_netmask[0] != '\0')        /* netmask */
    {
        TRACE("* parse dhcp ack success. %s %d\r\n", MDL);
        return 0;    
    }

    TRACE("> parse dhcp ack fail. %s %d\r\n", MDL);
#endif

    return 0;
}

int DHClientCheckPacket(int packType, struct dhcp_client *client)
{
    client->dhcpHeader = (struct dhcpv4_header *)(client->recvBuff);
	client->dhcpOption = (u_int8_t *)(client->recvBuff + sizeof(struct dhcpv4_header));

    u_int8_t *tmpDhcpOpt = client->dhcpOption;
    
    if (packType == DHCP_MSGOFFER)
    {   
        if (*(tmpDhcpOpt + 2) == DHCP_MSGOFFER && 
            htonl(client->dhcpHeader->dhcp_xid) == client->opt.dhcpXid) 
        {
			return DHCP_OFFR_RCVD;
		} 
        else 
        {
			return UNKNOWN_PACKET;
		}
    }
    else if (packType == DHCP_MSGACK)
    {
        if (*(tmpDhcpOpt + 2) == DHCP_MSGACK && 
            htonl(client->dhcpHeader->dhcp_xid) == client->opt.dhcpXid) 
        {
			return DHCP_ACK_RCVD;
		} 
        else if (*(tmpDhcpOpt + 2) == DHCP_MSGNACK && 
            htonl(client->dhcpHeader->dhcp_xid) == client->opt.dhcpXid) 
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

    return UNKNOWN_PACKET;  
}

int DHClientSendPacket(struct dhcp_client *client)
{
    int ret = -1;
    int retry = 0;
    int len = client->sendLen;
    socklen_t addrlen = sizeof(client->broadcastAddr);
    
    do {
        ret = sendto(client->sockfd, client->sendBuff, len, 0, 
            (struct sockaddr *)&client->broadcastAddr, addrlen);
        
    } while (ret < 0 &&
		 (errno == EHOSTUNREACH ||
		  errno == ECONNREFUSED) &&
		 retry++ < 10);
    
    if (ret != len)
    {
        TRACE("> send_packet error(%d:%s). %s %d\r\n", errno, strerror(errno), MDL);
		if (errno == ENETUNREACH)
		{
			TRACE("> send_packet: regarding broadcast address. %s %d\r\n", MDL);
		}

        return -1;
    }
    
	return 0;
}

int DHClientRecvPacket(int packType, struct dhcp_client *client)
{
    int ret = 0;
    struct sockaddr_in sockaddr;
    socklen_t sockLen = 0;
    int retval = -1;
    int retPackType = -1;
	fd_set readfds; 
	struct timeval tval;
    
	tval.tv_sec = client->timeout / 1000L; 
	tval.tv_usec = 0;

	while (tval.tv_sec != 0)
    {   
        usleep(1000);
        
		FD_ZERO(&readfds);
		FD_SET(client->sockfd, &readfds);
        
		retval = select(client->sockfd + 1, &readfds, NULL, NULL, &tval);
		if (retval == 0) 
        {
			TRACE("> dhcp recv packet select timeout, continue. %s %d\r\n", MDL);
            continue;
		} 
        else if (retval > 0 && FD_ISSET(client->sockfd, &readfds)) 
        {
			memset(client->recvBuff, 0, sizeof(client->recvBuff));
			sockLen = sizeof(sockaddr);
			ret = recvfrom(client->sockfd, client->recvBuff, sizeof(client->recvBuff), 0,
					    (struct sockaddr *)&sockaddr, &sockLen);
		}
        
		if (ret >= 60) 
        {
            if (packType == DHCP_MSGOFFER)
            {
				retPackType = DHClientCheckPacket(DHCP_MSGOFFER, client);
				if(retPackType == DHCP_OFFR_RCVD) 
                {
					return DHCP_OFFR_RCVD;
				}
            }
            else if (packType == DHCP_MSGACK)
            {
                retPackType = DHClientCheckPacket(DHCP_MSGACK, client);
                if(retPackType == DHCP_ACK_RCVD) 
                {
					return DHCP_ACK_RCVD;
				}
                else if(retPackType == DHCP_NAK_RCVD) 
                {
					return DHCP_NAK_RCVD;
				}
            }
		} 
	}
    
	return DHCP_DISC_RESEND;    
}



int DHClientSendDiscoverPakcet(struct dhcp_client *client)
{
    int ret = -1;

    DHClientBuildDiscoverOption(&client->opt);

    DHClientBuildDiscoverPacket(client);

    ret = DHClientSendPacket(client);
    if (0 != ret)
    {   
        TRACE("> ERROR, dhcp send discover packet fail. %s %d\r\n", MDL);
        return -1;
    }

    ret = DHClientRecvPacket(DHCP_MSGOFFER, client);
    if (ret != DHCP_OFFR_RCVD)
    {
        TRACE("> recv dhcp offer packet fail. %s %d\r\n", MDL);
        return -1;
    }

    DHClientSetServidOpt50RequestIp(client);

    return 0;
}

int DHClientSendRequestPacket(struct dhcp_client *client)
{
    int ret = -1;

    DHClientBuildRequestOption(&client->opt);

    DHClientBuildRequestPacket(client);

    ret = DHClientSendPacket(client);
    if (0 != ret)
    {   
        TRACE("> ERROR, dhcp send request packet fail. %s %d\r\n", MDL);
        return -1;
    }

    ret = DHClientRecvPacket(DHCP_MSGACK, client);
    if (ret != DHCP_ACK_RCVD)
    {
        TRACE("> ERROR, recv dhcp ack packet fail. %s %d\r\n", MDL);
        if (ret == DHCP_NAK_RCVD) 
        {   
            TRACE("> dhcp nack received. %s %d\r\n", MDL);
            return 1;           /* return NAK */
        }
        
        return -1;
    }

    return 0;
}

int DHClientDiscover(struct dhcp_client *client)
{
    int ret = -1;

    if (client->sockfd < 0)
        return -1;

    /* set dhcp xid */
    DHClientSetDhcpXid(&client->opt);

    /* send discover packet */
    ret = DHClientSendDiscoverPakcet(client);
    if (0 != ret)
    {
        return -1;
    }

    /* parse discover packet response */
    DHClientParseOfferResponse(client);

    /* send request packet */
    ret = DHClientSendRequestPacket(client);
    if (0 != ret)
    {
        return -1;
    }

    /* parse request packet response */
    DHClientParseAckResponse(client);

    return 0;
}

int DHClientRequest(struct dhcp_client *client)
{   
    int ret = -1;
    
    if (client->sockfd < 0)
        return -1;

    DHClientSetDhcpXid(&client->opt);

    ret = DHClientSendRequestPacket(client);
    if (0 != ret)
    {   
        TRACE("> ERROR, send dhcp request packet fail, ret = %d. %s %d\r\n", ret, MDL);
        if (ret == 1)
        {
            return 1;           /* return NAK */
        }
        
        return -1;
    }

    ret = DHClientParseAckResponse(client);
    if (0 != ret)
    {   
        TRACE("> ERROR, parser ack packet fail. %s %d\r\n", MDL);
        return -1;
    }

    return 0;
}

#if 1
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        TRACE("> usage dhcp_client <ifname> \n");
        return -1;
    }

    int ret = -1;
    struct dhcp_client client;

    ret = DHClientInit(&client, argv[1], NULL, "DHCP_TEST");
    if (0 != ret)
    {
        TRACE("> ERROR, init dhcp client fail. %s %d\r\n", MDL);
        return -1;
    }
    
    ret = DHClientDiscover(&client);

    DHClientDeinit(&client);

    return ret;
}
#endif

