
#ifndef DHCP_CLIENT_H
#define DHCP_CLIENT_H

#ifndef TRACE
#define TRACE printf
#define MDL __FUNCTION__, __LINE__
#endif

#define DHCP_CLIENT_PORT    68
#define DHCP_SERVER_PORT    67
#define TIME_OUT            3000
#define IF_NAME_LEN         32
#define ETHER_ADDR_LEN      0x6	        /* Ethernet address len: 6 bytes */
#define DATA_BUF_LEN        2048
#define DHCP_OFFR_RCVD 		1
#define DHCP_DISC_RESEND 	2
#define UNKNOWN_PACKET 		3
#define DHCP_ACK_RCVD 		4
#define DHCP_REQ_RESEND 	5
#define DHCP_NAK_RCVD 		6
#define HOSTNAME_BUF_LEN    32
#define SSID_BUF_LEN        64
#define IP_BUF_LEN          16
#define DHCPOPT_BUF_LEN     512

#define ARPHRD_ETHER        1   /* Ethernet 10Mbps */
#define DHCP_GIADDR         "0.0.0.0"

typedef struct dhcp_option 
{
    u_int32_t dhcpXid;
    u_int32_t option50RequestIp;
    u_int32_t servId;
    u_int32_t option51LeaseTime;
    u_int16_t bcastFlag;           /* DHCP broadcast flag */
    u_int16_t unicastFlag;
    u_int32_t unicastIpAddress;
    u_char hostname[HOSTNAME_BUF_LEN];
    u_int32_t hostnameFlag;
    u_char optBuff[DHCPOPT_BUF_LEN];
    u_int32_t optSize;
    
} DHCP_OPTION;

typedef struct dhcp_result
{
    char ifname[IF_NAME_LEN];
    char fixedAddress[IP_BUF_LEN];
    char subnetMask[IP_BUF_LEN];
    char router[IP_BUF_LEN];
    char servIdent[IP_BUF_LEN];
    char dnsServer[IP_BUF_LEN];
    int leaseTime;
    char ssid[SSID_BUF_LEN];
    
} DHCP_RESULT;

typedef struct dhcp_client 
{
    int sockfd;
    int timeout;
    struct sockaddr_in broadcastAddr;
    char ifname[IF_NAME_LEN];
    unsigned char srcMac[ETHER_ADDR_LEN];
    unsigned char dstMac[ETHER_ADDR_LEN];
    size_t recvLen;
    char recvBuff[DATA_BUF_LEN];
    size_t sendLen;
    char sendBuff[DATA_BUF_LEN];
    char ssid[SSID_BUF_LEN];
    
    struct dhcpv4_header *dhcpHeader;
    u_int8_t *dhcpOption;
    
    struct dhcp_option opt;
    struct dhcp_result result;
    
} DHCP_CLIENT;

struct dhcpv4_header
{
	u_int8_t dhcp_opcode;     /* opcode */
#define DHCP_REQUEST 0x1
#define DHCP_REPLY   0x2
	u_int8_t dhcp_htype;      /* hardware address type */
	u_int8_t dhcp_hlen;       /* hardware address length */
	u_int8_t dhcp_hopcount;   /* used by proxy servers */
	u_int32_t dhcp_xid;        /* transaction ID */
	u_int16_t dhcp_secs;      /* number of seconds since trying to bootstrap */
	u_int16_t dhcp_flags;     /* flags for DHCP, unused for BOOTP */
	u_int32_t dhcp_cip;        /* client's IP */
	u_int32_t dhcp_yip;        /* your IP */
	u_int32_t dhcp_sip;        /* server's IP */
	u_int32_t dhcp_gip;        /* gateway IP */
	u_int8_t dhcp_chaddr[16]; /* client hardware address */
	u_int8_t dhcp_sname[64];  /* server host name */
	u_int8_t dhcp_file[128];  /* boot file name */
	u_int32_t dhcp_magic;      /* BOOTP magic header */
#define DHCP_MAGIC                  0x63825363
#define BOOTP_MIN_LEN        0x12c
#define DHCP_PAD             0x00
#define DHCP_SUBNETMASK      0x01
#define DHCP_TIMEOFFSET      0x02
#define DHCP_ROUTER          0x03
#define DHCP_TIMESERVER      0x04
#define DHCP_NAMESERVER      0x05
#define DHCP_DNS             0x06
#define DHCP_LOGSERV         0x07
#define DHCP_COOKIESERV      0x08
#define DHCP_LPRSERV         0x09
#define DHCP_IMPSERV         0x0a
#define DHCP_RESSERV         0x0b
#define DHCP_HOSTNAME        0x0c
#define DHCP_BOOTFILESIZE    0x0d
#define DHCP_DUMPFILE        0x0e
#define DHCP_DOMAINNAME      0x0f
#define DHCP_SWAPSERV        0x10
#define DHCP_ROOTPATH        0x11
#define DHCP_EXTENPATH       0x12
#define DHCP_IPFORWARD       0x13
#define DHCP_SRCROUTE        0x14
#define DHCP_POLICYFILTER    0x15
#define DHCP_MAXASMSIZE      0x16
#define DHCP_IPTTL           0x17
#define DHCP_MTUTIMEOUT      0x18
#define DHCP_MTUTABLE        0x19
#define DHCP_MTUSIZE         0x1a
#define DHCP_LOCALSUBNETS    0x1b
#define DHCP_BROADCASTADDR   0x1c
#define DHCP_DOMASKDISCOV    0x1d
#define DHCP_MASKSUPPLY      0x1e
#define DHCP_DOROUTEDISC     0x1f
#define DHCP_ROUTERSOLICIT   0x20
#define DHCP_STATICROUTE     0x21
#define DHCP_TRAILERENCAP    0x22
#define DHCP_ARPTIMEOUT      0x23
#define DHCP_ETHERENCAP      0x24
#define DHCP_TCPTTL          0x25
#define DHCP_TCPKEEPALIVE    0x26
#define DHCP_TCPALIVEGARBAGE 0x27
#define DHCP_NISDOMAIN       0x28
#define DHCP_NISSERVERS      0x29
#define DHCP_NISTIMESERV     0x2a
#define DHCP_VENDSPECIFIC    0x2b
#define DHCP_NBNS            0x2c
#define DHCP_NBDD            0x2d
#define DHCP_NBTCPIP         0x2e
#define DHCP_NBTCPSCOPE      0x2f
#define DHCP_XFONT           0x30
#define DHCP_XDISPLAYMGR     0x31
#define DHCP_REQUESTEDIP     0x32
#define DHCP_LEASETIME       0x33
#define DHCP_OPTIONOVERLOAD  0x34
#define DHCP_MESSAGETYPE     0x35
#define DHCP_SERVIDENT       0x36
#define DHCP_PARAMREQUEST    0x37
#define DHCP_MESSAGE         0x38
#define DHCP_MAXMSGSIZE      0x39
#define DHCP_RENEWTIME       0x3a
#define DHCP_REBINDTIME      0x3b
#define DHCP_CLASSSID        0x3c
#define DHCP_CLIENTID        0x3d
#define DHCP_NISPLUSDOMAIN   0x40
#define DHCP_NISPLUSSERVERS  0x41
#define DHCP_MOBILEIPAGENT   0x44
#define DHCP_SMTPSERVER      0x45
#define DHCP_POP3SERVER      0x46
#define DHCP_NNTPSERVER      0x47
#define DHCP_WWWSERVER       0x48
#define DHCP_FINGERSERVER    0x49
#define DHCP_IRCSERVER       0x4a
#define DHCP_STSERVER        0x4b
#define DHCP_STDASERVER      0x4c
#define DHCP_FQDN            0x51
#define DHCP_END             0xff

#define DHCP_MSGDISCOVER     0x01
#define DHCP_MSGOFFER        0x02
#define DHCP_MSGREQUEST      0x03
#define DHCP_MSGDECLINE      0x04
#define DHCP_MSGACK          0x05
#define DHCP_MSGNACK         0x06
#define DHCP_MSGRELEASE      0x07
#define DHCP_MSGINFORM       0x08
};

#ifdef __cplusplus
extern "C" {
#endif

int DHClientInit(struct dhcp_client *client, const char *ifname, const char *ssid, const char *hostname);

void DHClientDeinit(struct dhcp_client *client);

void DHClientSetDhcpXid(struct dhcp_option *option);

int DHClientSendDiscoverPakcet(struct dhcp_client *client);

int DHClientParseOfferResponse(struct dhcp_client *client);

int DHClientSendRequestPacket(struct dhcp_client *client);

int DHClientParseAckResponse(struct dhcp_client *client);

#ifdef __cplusplus
}
#endif

#endif /* DHCP_CLIENT_H */

