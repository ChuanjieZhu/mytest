

#ifndef _MRJ_DHCLIENT_H_
#define _MRJ_DHCLIENT_H_

#include <sys/types.h>
#include <netinet/in.h>

#pragma   pack(1)

#define MDL __FUNCTION__, __LINE__
#define STR_ERROR   (strerror(errno))
#define TRACE printf
#define WHILE_FALSE  while(0)
#define MRJ_safefree(ptr) \
  do {if((ptr)) {free((ptr)); (ptr) = NULL;}} WHILE_FALSE
    
typedef long Bool;
typedef int	mrj_socket_t;
typedef void MRJDHCLIENT;

#define DHCP_SRC_PORT  67

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef ETHX_LEASES_PATH
#define ETHX_LEASES_PATH    "/var/dhcp/ethx.leases"
#endif

#ifndef RAX_LEASES_PATH
#define RAX_LEASES_PATH     "/var/dhcp/rax.leases"
#endif

#if defined(__STDC__) || defined(_MSC_VER) || defined(__cplusplus) || \
  defined(__HP_aCC) || defined(__BORLANDC__) || defined(__LCC__) || \
  defined(__POCC__) || defined(__SALFORDC__) || defined(__HIGHC__) || \
  defined(__ILEC400__)
  /* This compiler is believed to have an ISO compatible preprocessor */
#define DHCLIENT_ISOCPP
#else
  /* This compiler is believed NOT to have an ISO compatible preprocessor */
#undef DHCLIENT_ISOCPP
#endif

/* long may be 32 or 64 bits, but we should never depend on anything else
   but 32 */
#define DHCPOPTTYPE_LONG          0
#define DHCPOPTTYPE_OBJECTPOINT   10000
#define DHCPOPTTYPE_FUNCTIONPOINT 20000
#define DHCPOPTTYPE_OFF_T         30000

#ifdef DHCPINIT
#undef DHCPINIT
#endif

#ifdef DHCLIENT_ISOCPP
#define DHCPINIT(na,t,nu) DHCPOPT_ ## na = DHCPOPTTYPE_ ## t + nu
#else
/* The macro "##" is ISO C, we assume pre-ISO C doesn't support it. */
#define LONG          DHCPOPTTYPE_LONG
#define OBJECTPOINT   DHCPOPTTYPE_OBJECTPOINT
#define FUNCTIONPOINT DHCPOPTTYPE_FUNCTIONPOINT
#define OFF_T         DHCPOPTTYPE_OFF_T
#define DHCPINIT(name,type,number) DHCPOPT_/**/name = type + number
#endif

typedef enum {
    DHCPINIT(IFNAME, OBJECTPOINT, 1),
    DHCPINIT(MSG_REQ, OBJECTPOINT, 2),
    DHCPINIT(WRITEFUNCTION, FUNCTIONPOINT, 3),
    DHCPINIT(POSTFIELDS, OBJECTPOINT, 4),
    DHCPINIT(MSG_REQUEST, LONG, 5),          /* HTTP PUT */
    DHCPINIT(MSG_DISCOVER, LONG, 6),          /* HTTP PUT */
    DHCPINIT(MSG_ACK, LONG, 7),
    DHCPINIT(MSG_OFFER, LONG, 8),
    DHCPINIT(MSG_RELEASE, LONG, 9),
    DHCPINIT(TIMEOUT, LONG, 10),
    DHCPINIT(VERBOSE, LONG, 11),
    DHCPINIT(HOSTNAME, OBJECTPOINT, 12),
    DHCPINIT(UNICAST_IP, OBJECTPOINT, 13),
    DHCPINIT(SERVER_IP, OBJECTPOINT, 14)
} MRJDHCLIENToption;

/* 
 * Libnet defines header sizes for every builder function exported.
 */

#define ETHER_H		0x10	/* Ethernet header: 14 bytes */
#define ETHER_ADDR_LEN  0x6	/* Ethernet address len: 6 bytes */	
#define IP_ADDR_LEN	0x4
#define VLAN_H		0x12	/* Ethernet header + vlan header*/	
#define IP_H		0x20	/* IP header: 20 bytes */	
#define UDP_H		0x8	/* UDP header: 8 bytes */		
#define ICMP_H		0x8
#define ICMP_PAYLOAD	0x3c	/* 60 bytes of ICMP payload */
#define DHCPV4_H	0xf0    /**< DHCP v4 header:     240 bytes */
#define ARP_H_LEN	0x08


/* convert int to short  */
union cv 
{
    u_int32_t i32_value;
    u_int16_t i16_value[2];
} ;


typedef enum {
  DHCPMSG_NONE, /* first in list */
  DHCPMSG_REQUEST,
  DHCPMSG_DISCOVER,
  DHCPMSG_OFFER,
  DHCPMSG_ACK,
  DHCPMSG_RELEASE,
  DHCPMSG_LAST /* last in list */
} MRJ_DhcpMsg;


/* long may be 32 or 64 bits, but we should never depend on anything else
   but 32 */
#define MRJOPTTYPE_LONG          0
#define MRJOPTTYPE_OBJECTPOINT   10000
#define MRJOPTTYPE_FUNCTIONPOINT 20000
#define MRJOPTTYPE_OFF_T         30000

/* name is uppercase MRJOPT_<name>,
   type is one of the defined MRJOPTTYPE_<type>
   number is unique identifier */
#ifdef MINIT
#undef MINIT
#endif

struct DhclientOptions
{
    u_int16_t server_ident_flag;
    u_int32_t server_ident;
    
    u_int32_t dhcp_xid;  
    u_int16_t bcast_flag; /* DHCP broadcast flag */ 
    u_int8_t vci_buff[256]; /* VCI buffer*/
    u_int16_t vci_flag;

    u_int16_t hostname_flag;
    char *hostname_buff;             /* Hostname buffer*/

    u_int8_t fqdn_buff[256]; /* FQDN buffer*/
    u_int16_t fqdn_flag;
    u_int16_t fqdn_n;
    u_int16_t fqdn_s;
    
    u_int16_t option50_ip_flag;
    u_int32_t option50_ip;

    u_int16_t option51_flag;
    u_int32_t option51_lease_time;

    u_int8_t release_flag;

    u_int32_t unicast_ip;
    u_int8_t unicast_ip_flag;

    u_int16_t vlan;
    u_int8_t padding_flag;
        
    u_char opt_buff[500];
    u_int32_t opt_size;
};

struct UserDefined
{
    Bool verbose;
    long timeout;
    Bool vlan;
    MRJ_DhcpMsg sendmsg;
    MRJ_DhcpMsg recvmsg;
};

struct DhclientLeases
{
    char if_name[32];
    char fixed_address[16];
    char subnet_mask[16];
    char routers[16];
    char server_address[16];
    char broadcast_address[16];
    int lease_time;
    int renewal_time;
    int rebinding_time;
    char renew[20];
    char rebind[20];
    char expire[20];
};

struct DhclientOut
{
    char if_name[32];
    char fixed_address[16];
    char serv_ident[16];
    char sub_netmask[16];
    char router[16];
    int lease_time;
};


struct SessionHandle
{
    mrj_socket_t sockfd;
    struct sockaddr_ll sall;
    struct ethernet_hdr *eth_hdrp;
    struct vlan_hdr     *vlan_hdrp; 
    struct iphdr        *ip_hdrp;
    struct udphdr       *udp_hdrp;
    struct dhcpv4_hdr   *dhcp_hdrp;
    u_int8_t            *dhcp_optp;
    u_char srcmac[ETHER_ADDR_LEN];
    u_char dstmac[ETHER_ADDR_LEN];
    int ifindex;
    char *ifname;
    char ipstr[32];
    char *leasepath;
    u_char send_buf[1518];
    u_char recv_buf[1518];
    struct DhclientOptions opt;
    struct DhclientLeases leases;
    struct DhclientOut resout;
    struct UserDefined  set;
};

typedef enum 
{
    MRJE_ERR = -1,
    MRJE_OK  = 0,
	MRJE_UNKNOWN,                /* 1 */
	MRJE_URLINLEGAL,             /* 2 */
	MRJE_CRSOCKFAL,              /* 3 */
	MRJE_IOCTLFAL,               /* 4 */
	MRJE_NOTFINDHOST,            /* 5 */
	MRJE_NOTCONNHOST,            /* 6 */
	MRJE_DISCONNHOST,            /* 7 */
	MRJE_NOCOMPLETERES,          /* 8 */
	MRJE_SNDREQERR,              /* 9 */
	MRJE_EMPTYRES,               /* 10 */
	MRJE_ENOCOMPLETERES,          /* 11 */
	MRJE_CRLOCALFILEFAL,         /* 12 */
	MRJE_SELECTFAL,              /* 13 */
	MRJE_TIME_OUT,                /* 14 */
	MRJE_NORESPONSE,             /* 15 */
	MRJE_HTTP_SERVERE400,             /* 16 */
	MRJE_HTTP_SERVERE401,             /* 17 */
	MRJE_HTTP_SERVERE403,             /* 18 */
	MRJE_HTTP_SERVERE404,             /* 19 */
	MRJE_HTTP_SERVERE500,             /* 20 */
	MRJE_HTTP_SERVERE503,             /* 21 */
	MRJE_BAD_FUNCTION_ARGUMENT,       /* 22 */
	MRJE_OUT_OF_MEMORY,               /* 23 */
	MRJE_UNSUPPORTED_PROTOCOL,        /* 24 */  
	MRJE_FAILED_INIT,                 /* 25 */
	MRJE_UNKNOWN_OPTION,              /* 26 */
	MRJE_WRITE_ERROR,                 /* 27 */  
    MRJE_UPLOAD_FAILED,               /* 28 */  
    MRJE_READ_ERROR,                 /* 39 */
    MRJE_OPERATION_TIMEDOUT,        /* 30 */
    MRJE_FUNCTION_NOT_FOUND,        /* 31 */
    MRJE_INTERFACE_FAILED,          /* 32 */
    MRJE_SEND_ERROR,                /* 33 */
    MRJE_RECV_ERROR,                /* 34 */
    MRJE_AGAIN,                     /* 35 */
    MRJE_NO_CONNECTION_AVAILABLE,   /* 36 */    
    MRJE_ABORTED_BY_CALLBACK,       /* 37 */    
    MRJE_OPEN_FILE_ERROR,           /* 38 */
    MRJE_SELECT_FAIL,               /* 39 */
    MRJE_SELECT_TIME_OUT,           /* 40 */
    MRJE_GET_SOCKOPT_ERROR,         /* 41 */
    MRJE_FCNTL_ERROR,               /* 42 */
    MRJE_CONNECT_NONB_ERROR,         /* 43 */
    MRJE_DNS_RESLOVE_ERROR,          /* 44 */
    MRJE_FILE_NOT_FOUND,               /* 45 */
    MRJE_RECV_NAK,
    
} MRJEcode;

struct updhdr {
    unsigned short source;
    unsigned short dest;
    unsigned short len;
    unsigned short check;
};

/*
 *  ARP header
 *  Address Resolution Protocol
 *  Base header size: 8 bytes
 */
struct arp_hdr
{
	u_int16_t ar_hrd;         /* format of hardware address */
#define ARPHRD_NETROM   0   /* from KA9Q: NET/ROM pseudo */
#define ARPHRD_ETHER    1   /* Ethernet 10Mbps */
#define ARPHRD_EETHER   2   /* Experimental Ethernet */
#define ARPHRD_AX25     3   /* AX.25 Level 2 */
#define ARPHRD_PRONET   4   /* PROnet token ring */
#define ARPHRD_CHAOS    5   /* Chaosnet */
#define ARPHRD_IEEE802  6   /* IEEE 802.2 Ethernet/TR/TB */
#define ARPHRD_ARCNET   7   /* ARCnet */
#define ARPHRD_APPLETLK 8   /* APPLEtalk */
#define ARPHRD_LANSTAR  9   /* Lanstar */
#define ARPHRD_DLCI     15  /* Frame Relay DLCI */
#define ARPHRD_ATM      19  /* ATM */
#define ARPHRD_METRICOM 23  /* Metricom STRIP (new IANA id) */
#define ARPHRD_IPSEC    31  /* IPsec tunnel */
	u_int16_t ar_pro;         /* format of protocol address */
	u_int8_t  ar_hln;         /* length of hardware address */
	u_int8_t  ar_pln;         /* length of protocol addres */
	u_int16_t ar_op;          /* operation type */
#define ARPOP_REQUEST    1  /* req to resolve address */
#define ARPOP_REPLY      2  /* resp to previous request */
#define ARPOP_REVREQUEST 3  /* req protocol address given hardware */
#define ARPOP_REVREPLY   4  /* resp giving protocol address */
#define ARPOP_INVREQUEST 8  /* req to identify peer */
#define ARPOP_INVREPLY   9  /* resp identifying peer */
	u_int8_t sender_mac[ETHER_ADDR_LEN];
	u_int8_t sender_ip[IP_ADDR_LEN];
	u_int8_t target_mac[ETHER_ADDR_LEN];
	u_int8_t target_ip[IP_ADDR_LEN];
};

/*
 *  Ethernet II header
 *  Static header size: 14 bytes
 */
struct ethernet_hdr
{
	u_int8_t  ether_dhost[ETHER_ADDR_LEN];/* destination ethernet address */
	u_int8_t  ether_shost[ETHER_ADDR_LEN];/* source ethernet address */
	u_int16_t ether_type;                 /* protocol */
};

/**
 * IEEE 802.1Q (Virtual Local Area Network) VLAN header, static header 
 * size: 18 bytes
 */
struct vlan_hdr
{
	u_int8_t vlan_dhost[ETHER_ADDR_LEN];  /**< destination ethernet address */
	u_int8_t vlan_shost[ETHER_ADDR_LEN];  /**< source ethernet address */
	u_int16_t vlan_tpi;                   /**< tag protocol ID */
	u_int16_t vlan_priority_c_vid;        /**< priority | VLAN ID */
#define VLAN_PRIMASK   0x0007    /**< priority mask */
#define VLAN_CFIMASK   0x0001    /**< CFI mask */
#define VLAN_VIDMASK   0x0fff    /**< vid mask */
	u_int16_t vlan_len;                   /**< length or type (802.3 / Eth 2) */
};  

/*
 * IP header included from netinet/ip.h
 */
#include <netinet/ip.h>

/*
 *  ICMP header
 *  Internet Control Message Protocol
 *  Base header size: 4 bytes
 */
struct icmp_hdr
{
	u_int8_t icmp_type;       			/* ICMP type */
#define     ICMP_ECHOREPLY                  0
#define     ICMP_ECHO                       8
	u_int8_t icmp_code;       			/* ICMP code */
	u_int16_t icmp_sum;   			/* ICMP Checksum */
	u_int16_t id; 				/* ICMP id */
	u_int16_t seq;				/* ICMP sequence number */
};

/*
 * UDP header included from netinet/udp.h
 */
#include <netinet/udp.h>


/*
 *  DHCP header
 *  Dynamic Host Configuration Protocol
 *  Static header size: f0 bytes
 */
struct dhcpv4_hdr
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

#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP            0x0800  /* IP protocol */
#endif
#ifndef ETHERTYPE_VLAN
#define ETHERTYPE_VLAN          0x8100  /* IEEE 802.1Q VLAN tagging */
#endif
#define ETHERTYPE_ARP		    0x806


/*
 * FQDN options flags
 */
#define FQDN_N_FLAG   0x08
#define FQDN_E_FLAG   0x04
#define FQDN_O_FLAG   0x02
#define FQDN_S_FLAG   0x01a

/*
 * Minimum DHCP packet size
 */
#define MINIMUM_PACKET_SIZE 300

#ifdef __cplusplus
extern "C" {
#endif

int set_if_ip(char *ip, char *ifname);
int set_if_mask(char *mask, char *ifname);
int set_if_gw(char *gw, char *ifname);
int get_if_mac(const char *if_name, char *mac, int length);

#ifdef __cplusplus
}
#endif

#pragma pack()

#endif  /* _MRJ_DHCLIENT_H_ */

