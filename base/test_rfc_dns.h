
#ifndef TEST_RFC_DNS_H
#define TEST_RFC_DNS_H

#include <stdint.h>

#define TRACE printf
#define MDL __FUNCTION__, __LINE__
#define STR_ERRNO strerror(errno)

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

/* Specified in the RFC for DNS. */
#define MAX_DOMAIN_LENGTH	    255
#define MAX_SUBDOMAIN_LENGTH    63
#define ERROR_BUFFER            255
#define MAX_RES_NUM             10
#define MAX_IP_STRING_SIZE	    15

struct dns_header 
{
    uint16_t id;
    uint16_t flags;
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
};

struct dns_question_trailer 
{
    uint16_t q_type;
    uint16_t q_class;
};

struct dns_response 
{
    uint16_t response_type; /* Either A, CNAME, MX, or NS. */
    uint16_t preference; /* MX only. */
    uint32_t cache_time; /* All. */
    uint32_t ip_address; /* A only. */
    char name[MAX_DOMAIN_LENGTH + 1]; /* CNAME, MX, and NS only. */
    uint8_t authoritative; /* All. 0 false, 1 true. */
    uint8_t unused[7];
};

typedef struct dns_result 
{
    char domian_name[MAX_DOMAIN_LENGTH + 1];
    char resolve_ip[MAX_IP_STRING_SIZE + 1];
} dns_result;


typedef struct dns_result_cache 
{
    int count;
    dns_result result[MAX_RES_NUM]; 
} dns_result_cache;

struct dns_client 
{
    int sockfd;
    int timeout;
    int dns_port;
    char *send_buff;
    size_t send_len;
    char *recv_buff;
    size_t recv_len;
    char server_ip[MAX_DOMAIN_LENGTH + 1];
    char domain[MAX_DOMAIN_LENGTH + 1];
    int request_q_type;
    int request_id;
    struct sockaddr_in serv_addr;
    char error_msg[ERROR_BUFFER + 1];
    char dns_resolve_ip[MAX_IP_STRING_SIZE + 1];
};

#ifdef __cplusplus
extern "C" {
#endif
    
#ifdef __cplusplus
}
#endif

#endif //TEST_RFC_DNS_H

