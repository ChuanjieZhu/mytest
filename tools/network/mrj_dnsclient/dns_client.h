
#ifndef _DNS_CLIENT_H_
#define _DNS_CLIENT_H_

#include <stdint.h>
#include <stdlib.h>

typedef long Bool;
typedef int	mrj_socket_t;
typedef void MRJVOID;

#ifndef MDL
#define MDL __FUNCTION__, __LINE__
#endif

#ifndef STR_ERROR
#define STR_ERROR   (strerror(errno))
#endif

#ifndef TRACE
#define TRACE printf
#endif

#define WHILE_FALSE while(0)
#define safefree(ptr) \
    do {if((ptr)) {free((ptr)); (ptr) = NULL;}} WHILE_FALSE

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#if defined(__STDC__) || defined(_MSC_VER) || defined(__cplusplus) || \
  defined(__HP_aCC) || defined(__BORLANDC__) || defined(__LCC__) || \
  defined(__POCC__) || defined(__SALFORDC__) || defined(__HIGHC__) || \
  defined(__ILEC400__)
  /* This compiler is believed to have an ISO compatible preprocessor */
#define MRJ_ISOCPP
#else
  /* This compiler is believed NOT to have an ISO compatible preprocessor */
#undef MRJ_ISOCPP
#endif

/* long may be 32 or 64 bits, but we should never depend on anything else
   but 32 */
#define DNSOPTTYPE_LONG          0
#define DNSOPTTYPE_OBJECTPOINT   10000
#define DNSOPTTYPE_FUNCTIONPOINT 20000
#define DNSOPTTYPE_OFF_T         30000

#ifdef DNSINIT
#undef DNSINIT
#endif

#ifdef MRJ_ISOCPP
#define DNSINIT(na,t,nu) DNSOPT_ ## na = DNSOPTTYPE_ ## t + nu
#else
/* The macro "##" is ISO C, we assume pre-ISO C doesn't support it. */
#define LONG          DNSOPTTYPE_LONG
#define OBJECTPOINT   DNSOPTTYPE_OBJECTPOINT
#define FUNCTIONPOINT DNSOPTTYPE_FUNCTIONPOINT
#define OFF_T         DNSOPTTYPE_OFF_T
#define DNSINIT(name,type,number) DNSOPT_/**/name = type + number
#endif

typedef enum {
    DNSINIT(DOMAIN_NAME, OBJECTPOINT, 1),               /* Domain name */
    DNSINIT(SERVER_1, OBJECTPOINT, 2),              /* Dns server ip 1 */
    DNSINIT(SERVER_2, OBJECTPOINT, 3),              /* Dns server ip 2 */
    DNSINIT(MSG_REQ, OBJECTPOINT, 4),                   
    DNSINIT(WRITEFUNCTION, FUNCTIONPOINT, 5),
    DNSINIT(POSTFIELDS, OBJECTPOINT, 6),
    DNSINIT(TIMEOUT, LONG, 7),
    DNSINIT(VERBOSE, LONG, 8)
} DNSoption;

#define DNS_QR_RESPONSE		0x8000
#define DNS_AUTH_ANS		0x0400
#define DNS_TRUNCATED		0x0200
#define DNS_USE_RECURSION	0x0100
#define DNS_RECURSION_AVAIL	0x0080
#define DNS_FORMAT_ERROR	0x0001
#define DNS_SERVER_FAILURE	0x0002
#define DNS_NAME_ERROR		0x0003
#define DNS_NOT_IMPLEMENTED	0x0004
#define DNS_REFUSED		    0x0005
#define DNS_ERROR_MASK      0x000f
#define DNS_INET_ADDR		0x0001

#define DNS_A_RECORD		0x0001
#define DNS_NS_RECORD		0x0002
#define DNS_CNAME_RECORD	0x0005
#define DNS_MX_RECORD		0x000f


#define DNS_POINTER_FLAG	    0xc0
#define DNS_POINTER_OFFSET_MASK 0x3fff
#define DNS_LABEL_LENGTH_MASK	0x3f
#define MAX_AN_COUNT            50
#define RESPONSE_BUFFER_SIZE    1000


/* Specified in the RFC for DNS. */
#define DNS_DEFAULT_PORT        53
#define MAX_DOMAIN_LENGTH	    255
#define MAX_SUBDOMAIN_LENGTH    63
#define ERROR_BUFFER_LEN        128
#define MAX_CACHE_NUM           10
#define MAX_IP_STRING_SIZE	    16

typedef enum {
    MRJE_OK = 0,
    MRJE_UNKNOW,
    MRJE_SEND_ERR,
    MRJE_RECV_ERR,
    MRJE_BAD_FUNCTION_ARGUMENT,
    MRJE_OUT_OF_MEMORY,
    MRJE_FUNC_SOCKET_ERR,
    MRJE_FUNC_SELECT_ERR,
    MRJE_FUNC_PARSE_ERR,
    MRJE_TIMEOUT,
    MRJE_UNKNOW_OPTION
    
} MRJEcode;


struct dns_header {
  uint16_t id;
  uint16_t flags;
  uint16_t qd_count;
  uint16_t an_count;
  uint16_t ns_count;
  uint16_t ar_count;
};

struct dns_question_trailer {
  uint16_t q_type;
  uint16_t q_class;
};

struct dns_response {
    uint16_t response_type;             /* Either A, CNAME, MX, or NS. */
    uint16_t preference;                /* MX only. */
    uint32_t cache_time;                /* All. */
    uint32_t ip_address;                /* A only. */
    char name[MAX_DOMAIN_LENGTH + 1];   /* CNAME, MX, and NS only. */
    uint8_t authoritative;              /* All. 0 false, 1 true. */
    uint8_t unused[7];
};


struct dns_req_use_defined {
    long timeout;
    Bool verbose;
    int port;
    char err_msg[ERROR_BUFFER_LEN];
    char server[MAX_IP_STRING_SIZE];
    char *server1;
    char *server2;
    char *domain_name;
    char *send;
    size_t send_len;
    char *recv;
    size_t recv_len;
};

struct dns_req_protocol_defined {
    int request_id;
    int request_q_type;
};


struct dns_result {
    char domain_name[MAX_DOMAIN_LENGTH + 1];
    char result_ip[MAX_IP_STRING_SIZE];
};

struct dns_result_cache {
    int count;
    struct dns_result result[MAX_CACHE_NUM];
};

struct session_handle {
    mrj_socket_t sockfd;
    struct sockaddr_in server_addr;
    int answer_count;
    char result_ip[MAX_IP_STRING_SIZE];
    struct dns_req_use_defined useset;
    struct dns_req_protocol_defined proset;
    struct dns_response *resps;
};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif /* _DNS_CLIENT_H_ */

