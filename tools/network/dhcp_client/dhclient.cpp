/*
 * DHCP client simulation tool. For testing pursose only.
 * This program needs to be run with root privileges. 
 * Author - Saravanakumar.G E-mail: saravana815@gmail.com
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <time.h>
#include <stdlib.h>

#include "dhcp.h"
#include "tinyxml.h"

#ifndef ETHX_LEASES_PATH
#define ETHX_LEASES_PATH    "/var/dhcp/ethx.leases"
#endif

#ifndef RAX_LEASES_PATH
#define RAX_LEASES_PATH     "/var/dhcp/rax.leases"
#endif

int sock_packet, iface = 2;	/* Socket descripter & transmit interface index */
struct sockaddr_ll ll = { 0 };	/* Socket address structure */
u_int16_t vlan = 0;		
u_int8_t l3_tos = 0;		
u_int16_t l2_hdr_size = 14;	
u_int16_t l3_hdr_size = 20;	
u_int16_t l4_hdr_size = 8;	

#define IP_TOS          0;

#define IP_HEAD_SIZE    20;         
#define ETHER_HEAD_SIZE 14;
#define UDP_HEAD_SIZE   8;
#define SERVER_IP       "255.255.255.255"


u_int16_t dhcp_hdr_size = sizeof(struct dhcpv4_hdr);

/* All protocheader sizes */

/* DHCP packet, option buffer and size of option buffer */
u_char dhcp_packet_disc[1518] = { 0 };
u_char dhcp_packet_offer[1518] = { 0 };
u_char dhcp_packet_request[1518] = { 0 };
u_char dhcp_packet_ack[1518] = { 0 };
u_char dhcp_packet_release[1518] = { 0 };

u_char dhopt_buff[500] = { 0 };
u_int32_t dhopt_size = { 0 };
u_char dhmac[ETHER_ADDR_LEN] = { 0 };
u_char dmac[ETHER_ADDR_LEN];

char dhmac_fname[20];
char iface_name[30] = { 0 };
char ip_str[128];

u_int32_t server_id = { 0 };
u_int32_t option50_ip = { 0 };
u_int32_t dhcp_xid = 0;  
u_int16_t bcast_flag = 0; /* DHCP broadcast flag */ 
u_int8_t vci_buff[256] = { 0 }; /* VCI buffer*/
u_int16_t vci_flag = 0;
u_int8_t hostname_buff[256] = { 0 }; /* Hostname buffer*/
u_int16_t hostname_flag = 0;
u_int8_t fqdn_buff[256] = { 0 }; /* FQDN buffer*/
u_int16_t fqdn_flag = 0;
u_int16_t fqdn_n = 0;
u_int16_t fqdn_s = 0;
u_int32_t option51_lease_time = 0;
u_int32_t port = 67;
u_int8_t unicast_flag = 0;
u_char *giaddr = (u_char *)"0.0.0.0";
u_char *server_addr = (u_char *)"255.255.255.255";

/* Pointers for all layer data structures */
struct ethernet_hdr *eth_hg = { 0 };
struct vlan_hdr *vlan_hg = { 0 };
struct iphdr *iph_g = { 0 };
struct udphdr *uh_g = { 0 };
struct dhcpv4_hdr *dhcph_g = { 0 };

u_int8_t *dhopt_pointer_g = { 0 };
u_int8_t dhcp_release_flag = 0;
u_int8_t padding_flag = 0;
u_int32_t unicast_ip_address = 0;

static DHCP_LEASES g_ethx_leases;
static DHCP_LEASES g_rax_leases;

int update_leases(DHCP_RESULT *p_result);


static void build_discover_option()
{
    /* option 53: 设置dhcp消息类型 */
	build_option53(DHCP_MSGDISCOVER);	/* Option53 for DHCP discover */

    /* option 12: The DHCP Client Option12 feature specifies the hostname of the client */
    if (hostname_flag) 
    {
		build_option12_hostname();
	}
    
	if (fqdn_flag) 
    {
		build_option81_fqdn();
	}

    /* option 50: dhcp-requested-address */
	if (option50_ip) 
    {
		build_option50();		/* Option50 - req. IP  */
	}

    /* option 51: dhcp-lease-time  */
	if (option51_lease_time) 
    {
		build_option51();               /* Option51 - DHCP lease time requested */
	}
    
	if (vci_flag == 1) 
    {
		build_option60_vci(); 		/* Option60 - VCI  */
	}
    
	build_optioneof();			/* End of option */
}


static void build_request_option()
{
    /* Reset the dhopt buffer to build DHCP request options  */
	reset_dhopt_size();

    build_option53(DHCP_MSGREQUEST); 

    build_option50();       /* option */

    build_option54();

    if (hostname_flag) 
    {
		build_option12_hostname();
	}
    
	if (fqdn_flag) 
    {
		build_option81_fqdn();
	}
    
	if (vci_flag == 1) 
    {
		build_option60_vci();  
	}

    if(option51_lease_time) 
    {
		build_option51();                       /* Option51 - DHCP lease time requested */
	}
    
	build_option55();
    
	build_optioneof();
}


int send_discover()
{
    int receive_state = 0;

    printf("11111 %s %d\r\n", __FUNCTION__, __LINE__);
     
    build_discover_option();

    printf("11111 %s %d\r\n", __FUNCTION__, __LINE__);
    
    build_dhpacket(DHCP_MSGDISCOVER);	/* Build DHCP discover packet */

    printf("11111 %s %d\r\n", __FUNCTION__, __LINE__);
    
    if (send_packet(DHCP_MSGDISCOVER) < 0)
    {
        printf("send discover packet error. %s %d\r\n", __FUNCTION__, __LINE__);
        /* Clear the promiscuous mode */
	    clear_promisc();
        close_socket();
        return (-1);
    }

    printf("11111 %s %d\r\n", __FUNCTION__, __LINE__);
    
    receive_state = recv_packet(DHCP_MSGOFFER);
    if (receive_state != DHCP_OFFR_RCVD)
    {
        printf("receive offer packet error (%s). %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        /* Clear the promiscuous mode */
        clear_promisc();
        close_socket();
        return (-1);
    }

    printf("11111 %s %d\r\n", __FUNCTION__, __LINE__);
    
    set_serv_id_opt50();
    
    printf("DHCP offer received\t - Offerd IP: %s \r\n", get_ip_str(dhcph_g->dhcp_yip));

    parse_dhcp_offer();

    printf("11111 %s %d\r\n", __FUNCTION__, __LINE__);
    
    return (0);
}


int send_request()
{
    int receive_state = 0;
    
    build_request_option();
    
    build_dhpacket(DHCP_MSGREQUEST); 		/* Builds specified packet */

    if (send_packet(DHCP_MSGREQUEST) < 0)
    {
        printf("send request packet error. %s %d\r\n", __FUNCTION__, __LINE__);
        /* Clear the promiscuous mode */
        clear_promisc();
        close_socket();
        return (-1);
    }

    receive_state = recv_packet(DHCP_MSGACK);
    if (receive_state != DHCP_ACK_RCVD) 
    {
        printf("receive ack packet error. %s %d\r\n", __FUNCTION__, __LINE__);
        
        if (receive_state == DHCP_NAK_RCVD) 
        {
            printf("DHCP nack received\t - Client MAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                dhmac[0], dhmac[1], dhmac[2], dhmac[3], dhmac[4], dhmac[5]);
        }

        /* Clear the promiscuous mode */
        clear_promisc();
        close_socket();
        return (-1);
    }

    return (0);
}

int set_interface(char *if_name)
{
    if (if_name == NULL) 
    {
        printf("Interface: set interface name param is NULL. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }
    
    iface = if_nametoindex(if_name);
	if(iface == 0) 
    {
		printf("Interface: %s doesnot exist. %s %d \r\n", if_name, __FUNCTION__, __LINE__);
		return (-1);
	}
    
	strncpy(iface_name, if_name, sizeof(iface_name) - 1);
    if (get_if_mac(iface_name, (char *)dhmac, sizeof(dhmac)) != 0)
    {   
        printf("Interface: get %s mac address error. %s %d \r\n", iface_name, __FUNCTION__, __LINE__);
        return (-1);
    }

    strncpy((char *)hostname_buff, "MEIRENJI_XZ", sizeof(hostname_buff) - 1);

    hostname_flag = 1;

    return (0);
}


int set_socket()
{
    /* Opens the PF_PACKET socket */
	if(open_socket() < 0) 
    {				
        return (-1);
	}

	/* Sets the promiscuous mode */
	if (set_promisc() < 0)
	{
        close_socket();
        return (-1);
	}

    return (0);
}

int dhclient_request(char *if_name)
{   
    if (set_interface(if_name) < 0) 
    {
        return (-1);
    }
       
	if (set_socket() < 0)
	{
        return (-1);
	}

    /* Sets a random DHCP xid */
	set_rand_dhcp_xid(); 

    if (send_request() < 0) 
    {
        return (-1);
    }   

    struct dhcp_result result;
    
    if (parse_dhcp_ack(&result, if_name) < 0)
    {
        return (-1);
    }

    printf("DHCP ack received\t-\tAcquired IP: %s\r\n", result.fixed_address);

    update_leases(&result);
    
    /* Clear the promiscuous mode */
	clear_promisc();

    /* Close the socket */
	close_socket();

    return (0);
}


int dhclient_discover(char *if_name)
{   
    if (set_interface(if_name) < 0) 
    {
        return (-1);
    }

    printf("..... %s %d\r\n", __FUNCTION__, __LINE__);
       
	if (set_socket() < 0)
	{
        return (-1);
	}

    printf("..... %s %d\r\n", __FUNCTION__, __LINE__);

    /* Sets a random DHCP xid */
	set_rand_dhcp_xid(); 

    printf("..... %s %d\r\n", __FUNCTION__, __LINE__);
    
    if (send_discover() < 0)
    {
        return (-1);
    }

    printf("..... %s %d\r\n", __FUNCTION__, __LINE__);
    
    if (send_request() < 0) 
    {
        return (-1);
    }       

    printf("..... %s %d\r\n", __FUNCTION__, __LINE__);
    
    DHCP_RESULT result;
    if (parse_dhcp_ack(&result, if_name) < 0)
    {
        return (-1);
    }

    printf("DHCP ack received\t-\tAcquired IP: %s\r\n", result.fixed_address);

    update_leases(&result);
    
	/* Clear the promiscuous mode */
	clear_promisc();

    /* Close the socket */
	close_socket();

    return (0);
}


int CheckFile(char *pcFileName, int *piFileType, int *piFileLen)
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


void TimeToDatetime(unsigned long time, char *pDatetime)
{
	if (pDatetime) 
    {
		struct tm strTm;
		struct tm *pTm = &strTm;
		time_t ti = time;

		localtime_r(&ti, pTm);
		if (pTm) 
        {
			sprintf(pDatetime, "%d-%02d-%02d %02d:%02d:%02d", 
				pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
		}
	}

	return;
}

void init_ethx_default_leases()
{
    memset(&g_ethx_leases, 0, sizeof(DHCP_LEASES));
    
    strncpy(g_ethx_leases.if_name, "eth0", sizeof(g_ethx_leases.if_name) - 1);
    strncpy(g_ethx_leases.fixed_address, "192.168.0.230", sizeof(g_ethx_leases.fixed_address) - 1);
    strncpy(g_ethx_leases.subnet_mask, "255.255.255.0", sizeof(g_ethx_leases.subnet_mask) - 1);
    strncpy(g_ethx_leases.routers, "192.168.0.1", sizeof(g_ethx_leases.routers) - 1);
    strncpy(g_ethx_leases.server_address, "192.168.0.1", sizeof(g_ethx_leases.server_address) - 1);
    strncpy(g_ethx_leases.broadcast_address, "192.168.0.255", sizeof(g_ethx_leases.broadcast_address) - 1);
    
    g_ethx_leases.lease_time = 43200;
    g_ethx_leases.renewal_time = 21600;
    g_ethx_leases.rebinding_time = 37800;

    time_t cur_time = time(NULL);
    TimeToDatetime(cur_time, g_ethx_leases.renew);
    TimeToDatetime(cur_time, g_ethx_leases.rebind);
    TimeToDatetime(cur_time, g_ethx_leases.expire);
}

void init_rax_default_leases()
{
    memset(&g_rax_leases, 0, sizeof(DHCP_LEASES));
    
    strncpy(g_rax_leases.if_name, "ra0", sizeof(g_ethx_leases.if_name) - 1);
    strncpy(g_rax_leases.fixed_address, "192.168.0.230", sizeof(g_ethx_leases.fixed_address) - 1);
    strncpy(g_rax_leases.subnet_mask, "255.255.255.0", sizeof(g_ethx_leases.subnet_mask) - 1);
    strncpy(g_rax_leases.routers, "192.168.0.1", sizeof(g_ethx_leases.routers) - 1);
    strncpy(g_rax_leases.server_address, "192.168.0.1", sizeof(g_ethx_leases.server_address) - 1);
    strncpy(g_rax_leases.broadcast_address, "192.168.0.255", sizeof(g_ethx_leases.broadcast_address) - 1);
    
    g_rax_leases.lease_time = 43200;
    g_rax_leases.renewal_time = 21600;
    g_rax_leases.rebinding_time = 37800;

    time_t cur_time = time(NULL);
    TimeToDatetime(cur_time, g_rax_leases.renew);
    TimeToDatetime(cur_time, g_rax_leases.rebind);
    TimeToDatetime(cur_time, g_rax_leases.expire);
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
    TiXmlText *ifNameContent = new TiXmlText(g_ethx_leases.if_name);
    ifNameElement->LinkEndChild(ifNameContent);

    /* fixed_address */
    TiXmlElement *fixedAddressElement = new TiXmlElement("fixed_address");
    ethxLeasesElement->LinkEndChild(fixedAddressElement);
    TiXmlText *fixedAddressContent = new TiXmlText(g_ethx_leases.fixed_address);
    fixedAddressElement->LinkEndChild(fixedAddressContent);

    /* subnet_mask */
    TiXmlElement *subnetMaskElement = new TiXmlElement("subnet_mask");
    ethxLeasesElement->LinkEndChild(subnetMaskElement);
    TiXmlText *subnetMaskContent = new TiXmlText(g_ethx_leases.subnet_mask);
    subnetMaskElement->LinkEndChild(subnetMaskContent);

    /* routers */
    TiXmlElement *routersElement = new TiXmlElement("routers");
    ethxLeasesElement->LinkEndChild(routersElement);
    TiXmlText *routersContent = new TiXmlText(g_ethx_leases.routers);
    routersElement->LinkEndChild(routersContent);

    /* server_address */
    TiXmlElement *serverAddressElement = new TiXmlElement("server_address");
    ethxLeasesElement->LinkEndChild(serverAddressElement);
    TiXmlText *serverAddressContent = new TiXmlText(g_ethx_leases.server_address);
    serverAddressElement->LinkEndChild(serverAddressContent);

    /* broadcast_address */
    TiXmlElement *broadcastAddressElement = new TiXmlElement("broadcast_address");
    ethxLeasesElement->LinkEndChild(broadcastAddressElement);
    TiXmlText *broadcastAddressContent = new TiXmlText(g_ethx_leases.broadcast_address);
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
    TiXmlText *renewContent = new TiXmlText(g_ethx_leases.renew);
    renewElement->LinkEndChild(renewContent);

    /* rebind */
    TiXmlElement *rebindElement = new TiXmlElement("rebind");
    ethxLeasesElement->LinkEndChild(rebindElement);
    TiXmlText *rebindContent = new TiXmlText(g_ethx_leases.rebind);
    rebindElement->LinkEndChild(rebindContent);

    /* expire */
    TiXmlElement *expireElement = new TiXmlElement("expire");
    ethxLeasesElement->LinkEndChild(expireElement);
    TiXmlText *expireContent = new TiXmlText(g_ethx_leases.expire);
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
    TiXmlText *ifNameContent = new TiXmlText(g_rax_leases.if_name);
    ifNameElement->LinkEndChild(ifNameContent);

    /* fixed_address */
    TiXmlElement *fixedAddressElement = new TiXmlElement("fixed_address");
    raxLeasesElement->LinkEndChild(fixedAddressElement);
    TiXmlText *fixedAddressContent = new TiXmlText(g_rax_leases.fixed_address);
    fixedAddressElement->LinkEndChild(fixedAddressContent);

    /* subnet_mask */
    TiXmlElement *subnetMaskElement = new TiXmlElement("subnet_mask");
    raxLeasesElement->LinkEndChild(subnetMaskElement);
    TiXmlText *subnetMaskContent = new TiXmlText(g_rax_leases.subnet_mask);
    subnetMaskElement->LinkEndChild(subnetMaskContent);

    /* routers */
    TiXmlElement *routersElement = new TiXmlElement("routers");
    raxLeasesElement->LinkEndChild(routersElement);
    TiXmlText *routersContent = new TiXmlText(g_rax_leases.routers);
    routersElement->LinkEndChild(routersContent);

    /* server_address */
    TiXmlElement *serverAddressElement = new TiXmlElement("server_address");
    raxLeasesElement->LinkEndChild(serverAddressElement);
    TiXmlText *serverAddressContent = new TiXmlText(g_rax_leases.server_address);
    serverAddressElement->LinkEndChild(serverAddressContent);

    /* broadcast_address */
    TiXmlElement *broadcastAddressElement = new TiXmlElement("broadcast_address");
    raxLeasesElement->LinkEndChild(broadcastAddressElement);
    TiXmlText *broadcastAddressContent = new TiXmlText(g_rax_leases.broadcast_address);
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
    TiXmlText *renewContent = new TiXmlText(g_rax_leases.renew);
    renewElement->LinkEndChild(renewContent);

    /* rebind */
    TiXmlElement *rebindElement = new TiXmlElement("rebind");
    raxLeasesElement->LinkEndChild(rebindElement);
    TiXmlText *rebindContent = new TiXmlText(g_rax_leases.rebind);
    rebindElement->LinkEndChild(rebindContent);

    /* expire */
    TiXmlElement *expireElement = new TiXmlElement("expire");
    raxLeasesElement->LinkEndChild(expireElement);
    TiXmlText *expireContent = new TiXmlText(g_rax_leases.expire);
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


void set_ethx_leases(DHCP_LEASES *pLeases)
{
    int ret = -1;

    if (memcmp(&g_ethx_leases, pLeases, sizeof(DHCP_LEASES)))
    {
        memcpy(&g_ethx_leases, pLeases, sizeof(DHCP_LEASES));
        ret = create_ethx_leases((char *)ETHX_LEASES_PATH);
        if(ret)
        {
            printf("set %s failed. %s %d\r\n", ETHX_LEASES_PATH, __FUNCTION__, __LINE__);
        }
    }
}


void set_rax_leases(DHCP_LEASES *pLeases)
{
    int ret = -1;

    if (memcmp(&g_rax_leases, pLeases, sizeof(DHCP_LEASES)))
    {
        memcpy(&g_rax_leases, pLeases, sizeof(DHCP_LEASES));
        ret = create_rax_leases((char *)RAX_LEASES_PATH);
        if(ret)
        {
            printf("set %s failed. %s %d\r\n", RAX_LEASES_PATH, __FUNCTION__, __LINE__);
        }
    }
}


int load_ethx_leases(char *path, DHCP_LEASES *pLeases)
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

int load_rax_leases(char *path, DHCP_LEASES *pLeases)
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

    ret = CheckFile((char *)ETHX_LEASES_PATH, NULL, NULL);
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
        ret = load_ethx_leases((char *)ETHX_LEASES_PATH, &g_ethx_leases);
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

    ret = CheckFile((char *)RAX_LEASES_PATH, NULL, NULL);
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
        ret = load_rax_leases((char *)RAX_LEASES_PATH, &g_rax_leases);
        if (ret)
        {
           printf("load %s failed %s %d\r\n", RAX_LEASES_PATH, __FUNCTION__, __LINE__); 
        }
    }
    
    return ret; 
}

void get_ethx_leases(DHCP_LEASES *pLeases)
{
    memset(pLeases, 0, sizeof(DHCP_LEASES));
    memcpy(pLeases, &g_ethx_leases, sizeof(DHCP_LEASES));
}


void get_rax_leases(DHCP_LEASES *pLeases)
{
    memset(pLeases, 0, sizeof(DHCP_LEASES));
    memcpy(pLeases, &g_rax_leases, sizeof(DHCP_LEASES));
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


int get_interface_leases(char *if_name, DHCP_LEASES *leases)
{
    int ret = get_interface_type(if_name);

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

void init_default_leases()
{
    init_ethx_default_leases();
    init_rax_default_leases();
}

int read_leases()
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


int init_dhclient_leases()
{
    int ret = -1;

    init_default_leases();

    mkdir("/var/dhcp", 0777);

    ret = read_leases();
    if (ret)
    {
        printf("init leases read leases failure. %s %d\r\n", __FUNCTION__, __LINE__); 
    }
    
    return ret;
}

int update_leases(DHCP_RESULT *p_result)
{
    int ret = -1;
    DHCP_LEASES leases;

    memset(&leases, 0, sizeof(leases));

    strncpy(leases.if_name, p_result->if_name, sizeof(leases.if_name) - 1);
    strncpy(leases.fixed_address, p_result->fixed_address, sizeof(leases.fixed_address) - 1);
    strncpy(leases.subnet_mask, p_result->sub_netmask, sizeof(leases.subnet_mask) - 1);
    strncpy(leases.routers, p_result->router, sizeof(leases.routers) - 1);
    strncpy(leases.server_address, p_result->serv_ident, sizeof(leases.server_address) - 1);

    leases.lease_time = p_result->lease_time;               /* 租约时间 */
    leases.renewal_time = leases.lease_time / 2;            /* 重新申请时间(1/2租约时间) */
    leases.rebinding_time = (leases.lease_time * 3) / 4;    /* 再次重新申请时间(3/4租约时间) */

    time_t cur_time = time(NULL);
    time_t renew_time = cur_time + leases.renewal_time;
    time_t rebinding_time = cur_time + leases.rebinding_time;
    time_t expire_time = cur_time + leases.lease_time;          /*  租约到期时的时间 */

    TimeToDatetime(renew_time, leases.renew);
    TimeToDatetime(rebinding_time, leases.rebind);
    TimeToDatetime(expire_time, leases.expire);

    ret = get_interface_type(p_result->if_name);
    if (0 == ret)
    {
        set_ethx_leases(&leases);
    }
    else if (1 == ret)
    {
        set_rax_leases(&leases);
    }
    else
    {
        printf("unkown interface type, %s %s %d\r\n", p_result->if_name, __FUNCTION__, __LINE__);
    }
    
    return 0;
}


int do_dhclient(char *if_name)
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

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: dhtest <interface> \r\n");
        return (-1);
    }

    int ret = do_dhclient(argv[1]);
    if (ret == 0) {
        printf("\r\ndhcp get ip address success. \r\n");
    } else {
        printf("\r\ndhcp get ip address failure. \r\n");
    }
    
    return ret;
}


