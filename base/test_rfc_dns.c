
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <time.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "test_rfc_dns.h"

#define DEFAULT_PORT		    53
#define DEFAULT_TIMEOUT		    3000        /* 3Ãë */
#define DNS_TRY_TIMES           3

/* DNS Optionas */
#define DNS_POINTER_FLAG	        0xc0
#define DNS_POINTER_OFFSET_MASK     0x3fff
#define DNS_LABEL_LENGTH_MASK	    0x3f
#define MAX_AN_COUNT                50
#define RESPONSE_BUFFER_SIZE        2048

int increment_buffer_index(char **buffer_pointer, char *max_index, int bytes) 
{
    *buffer_pointer += bytes;
    return *buffer_pointer >= max_index ? 0 : 1;
}

int read_domain_name(char *packet_index, char *packet_start, int packet_size, char *dest_buffer) 
{
  int bytes_read;
  uint8_t label_length;
  uint16_t offset;
  char *max_index;

  bytes_read = 0;
  max_index = packet_start + packet_size;
  
  if (packet_index >= max_index) 
  {
    return -1;
  }
  
  label_length = (uint8_t)*packet_index;

  while (label_length != 0) {
    if (bytes_read > 0) {
      *dest_buffer++ = '.';
    }

    if ((label_length & DNS_POINTER_FLAG) == DNS_POINTER_FLAG) {
      char *new_packet_index;

      uint16_t tmp = 0;
      memcpy((char *)&tmp, packet_index, sizeof(tmp));
      offset = ntohs(tmp) & DNS_POINTER_OFFSET_MASK;
      
      new_packet_index = packet_start + offset;
      if (new_packet_index >= max_index) {
	    return -1;
      }

      read_domain_name(new_packet_index, packet_start, 
		       packet_size, dest_buffer);
      return bytes_read + 2;
    }

    ++packet_index;
    label_length &= DNS_LABEL_LENGTH_MASK;

    if (packet_index + label_length >= max_index) {
      return -1;
    }

    memcpy(dest_buffer, packet_index, label_length);
    dest_buffer += label_length;
    *dest_buffer = 0;

    packet_index += label_length;
    bytes_read += label_length + 1;

    label_length = (uint8_t)*packet_index;
  }

  ++bytes_read; /* For the null root. */

  return bytes_read;
}


int dns_select(int sockfd, struct timeval *pstTv)
{
    int res = -1;
    fd_set rset, eset;

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    FD_ZERO(&eset);
    FD_SET(sockfd, &eset);

    res = select(sockfd + 1, &rset, NULL, &eset, pstTv);
    if (res == 0) 
    {
        return -1;
    }
    else if (res < 0 || FD_ISSET(sockfd, &eset)) 
    {
        return -1;
    } 
    else 
    {
        if (FD_ISSET(sockfd, &rset) > 0) 
        {
            return 0;
        }
    }

    return -1;
}

static void FormatIpAddress(uint32_t ip_address, char *buffer, size_t buffsize) 
{
    uint8_t *segments = (uint8_t *)&ip_address;
    snprintf(buffer, buffsize - 1, "%d.%d.%d.%d", 
        segments[3], segments[2], segments[1], segments[0]);
}

void init_dns_client_server(struct dns_client *client, const char *dns_serv_addr)
{
    if (client && dns_serv_addr)
    {
        if (strlen(dns_serv_addr) > MAX_IP_STRING_SIZE)
        {
            TRACE("> ERROR max length of server ip is %d. %s %d\n", MAX_IP_STRING_SIZE, MDL);
            return;
        }
        
        memset(client->server_ip, 0, sizeof(client->server_ip));
        strncpy(client->server_ip, dns_serv_addr, sizeof(client->server_ip) - 1);

        TRACE("* dns server address(%s) %s %d\r\n", client->server_ip, MDL);
        
        memset(&client->serv_addr, 0, sizeof(struct sockaddr_in));
        client->serv_addr.sin_family      = AF_INET;
        client->serv_addr.sin_addr.s_addr = inet_addr(client->server_ip);
        client->serv_addr.sin_port	      = htons(client->dns_port);
    }
}

int init_dns_client(struct dns_client *client, const char *domain, const char *dns_serv_addr)
{
    if (!client || !domain || !dns_serv_addr)
    {
        return -1;
    }
    
    memset(client, 0, sizeof(struct dns_client));
    client->sockfd = -1;
    client->timeout = DEFAULT_TIMEOUT;
    client->dns_port = DEFAULT_PORT;
    client->request_q_type = DNS_A_RECORD;

    if (strlen(domain) > MAX_DOMAIN_LENGTH) 
    {
        TRACE("> ERROR max length of domain is %d. %s %d\n", 
            MAX_DOMAIN_LENGTH, MDL);
        return -1;
    }
    
    strncpy(client->domain, domain, sizeof(client->domain) - 1);

    if (strlen(dns_serv_addr) > MAX_IP_STRING_SIZE)
    {
        TRACE("> ERROR max length of server is %d. %s %d\n", 
            MAX_IP_STRING_SIZE, MDL);
        return -1;    
    }   

    strncpy(client->server_ip, dns_serv_addr, sizeof(client->server_ip) - 1);

    client->sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); 
    if (client->sockfd < 0) 
    {
        TRACE("> dns error, could not open socket. %s %d\r\n", MDL);
        return -1;
    }

    client->serv_addr.sin_family      = AF_INET;
    client->serv_addr.sin_addr.s_addr = inet_addr(client->server_ip);
    client->serv_addr.sin_port	      = htons(client->dns_port);
    
    struct timeval tv;
    tv.tv_sec = client->timeout / 1000;
    tv.tv_usec = 0;
    setsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    client->recv_buff = (char *)malloc(RESPONSE_BUFFER_SIZE);
    if (client->recv_buff == NULL) 
    {
        TRACE("> dns error, could not allocate memory for response. %s %d\r\n", MDL);
        return -1;
    }

    client->recv_len = RESPONSE_BUFFER_SIZE;
    
    return 0;
}

int init_dns_client_ex(struct dns_client *client, const char *domain)
{
    if (!client || !domain)
    {
        return -1;
    }
    
    memset(client, 0, sizeof(struct dns_client));
    client->sockfd = -1;
    client->timeout = DEFAULT_TIMEOUT;
    client->dns_port = DEFAULT_PORT;
    client->request_q_type = DNS_A_RECORD;

    if (strlen(domain) > MAX_DOMAIN_LENGTH) 
    {
        TRACE("> ERROR max length of server and domain is %d. %s %d\n", 
            MAX_DOMAIN_LENGTH, MDL);
        return -1;
    }
    
    strncpy(client->domain, domain, sizeof(client->domain) - 1);

    client->sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); 
    if (client->sockfd < 0) 
    {
        TRACE("> dns error, could not open socket. %s %d\r\n", MDL);
        return -1;
    }
    
    struct timeval tv;
    tv.tv_sec = client->timeout / 1000;
    tv.tv_usec = 0;
    setsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    client->recv_buff = (char *)malloc(RESPONSE_BUFFER_SIZE);
    if (client->recv_buff == NULL) 
    {
        TRACE("> dns error, could not allocate memory for response. %s %d\r\n", MDL);
        return -1;
    }

    client->recv_len = RESPONSE_BUFFER_SIZE;
    
    return 0;
}

void deinit_dns_client(struct dns_client *client)
{
    if (client)
    {
        if (client->send_buff)
        {
            free(client->send_buff);
            client->send_buff = NULL;
        }

        if (client->recv_buff)
        {
            free(client->recv_buff);
            client->recv_buff = NULL;
        }

        if (client->sockfd >= 0)
        {
            close(client->sockfd);
            client->sockfd = -1;
        }
    }
}

int dns_build_request_packet(struct dns_client *client) 
{
    struct dns_header header;
    struct dns_question_trailer q_trailer;
    size_t domain_length, question_size, header_size, total_size, token_length;
    char *buffer, *token_index, *save_pointer;
    char temp_buffer[MAX_DOMAIN_LENGTH + 1];
  
    domain_length = strlen(client->domain);
    question_size = domain_length + sizeof(struct dns_question_trailer) + 2;
    header_size = sizeof(struct dns_header);
    total_size = question_size + header_size;

    client->send_buff = (char *)malloc(total_size);
    if (client->send_buff == NULL) 
    {
        TRACE("> dns error, malloc memory fail. %s %d\r\n", MDL);
        return -1;
    }
    client->send_len = total_size;
    
    buffer = client->send_buff;
    
    srand(time(0));
    client->request_id = rand() % 65535;

    memset(&header, 0, header_size);
    header.id	        = htons(client->request_id);
    header.flags	    = htons(DNS_USE_RECURSION);
    header.qd_count     = htons(1);
    header.an_count     = htons(0);
    header.ns_count     = htons(0);
    header.ar_count     = htons(0);

    memcpy(buffer, &header, header_size);
    buffer += header_size;

    strcpy(temp_buffer, client->domain);

    token_index = strtok_r(temp_buffer, ".", &save_pointer);
    while (token_index != 0) 
    {   
        TRACE("------------ token_index: %s \n", token_index);
        token_length = strlen(token_index);
        if (token_length > MAX_SUBDOMAIN_LENGTH) 
        {
            TRACE("> subdomain limited to %d chars %s %d\r\n", MAX_SUBDOMAIN_LENGTH, MDL);
            return -1;
        }

        *buffer++ = token_length;
        while ((*buffer++ = *token_index++) != 0);
        buffer--;
        token_index = strtok_r(0, ".", &save_pointer);
    }
    
    *buffer++ = 0;

    TRACE("------------ buffer: %s \n", buffer);
    
    q_trailer.q_type  = htons(client->request_q_type);
    q_trailer.q_class = htons(DNS_INET_ADDR);
  
    memcpy(buffer, &q_trailer, sizeof(struct dns_question_trailer));

    return 0;
}

int dns_send_request_packet(struct dns_client *client)
{
    int ret = 0;
    int retry = 0;
    size_t result = 0;  

    do {
        result = sendto(client->sockfd, client->send_buff, client->send_len, 0,
  		      (struct sockaddr *)&client->serv_addr, sizeof(client->serv_addr));

    } while (result < 0 &&
		 (errno == EHOSTUNREACH ||
		  errno == ECONNREFUSED) &&
		 retry++ < 10);
    
    if (result != client->send_len)
    {
        TRACE("> dns send request pakcet fail(%s). %s %d\r\n", STR_ERRNO, MDL);
        ret = -1;;
    }

    return ret;
}

int dns_recv_response_packet(struct dns_client *client)
{
    int ret = -1;
    struct timeval tv;

    tv.tv_sec = client->timeout / 1000L;
    tv.tv_usec = 0;

    while (tv.tv_sec != 0)
    {   
        usleep(1000);
        
        ret = dns_select(client->sockfd, &tv);
        if (ret != 0)
        {
            TRACE("> dns recv response select timeout, continue. %s %d\r\n", MDL);
            continue;
        }

        memset(client->recv_buff, 0, RESPONSE_BUFFER_SIZE);
        client->recv_len = recvfrom(client->sockfd, client->recv_buff, 
                        RESPONSE_BUFFER_SIZE - 1, 0, 0, 0);
        if (client->recv_len <= 0)
        {
            TRACE("> dns recvfrom response error. %s %d\r\n", MDL);
            return -1;
        }

        client->recv_buff[client->recv_len] = '\0';
        if (client->recv_len >= sizeof(struct dns_header))
        {
            return 0;
        }
    }

    return -1;         
}

int dns_parse_response(struct dns_client *client) 
{
    int ret = -1;
    int i;
    int bytes_read;
    int authoritative;
    char buffer[MAX_DOMAIN_LENGTH + 1];
    char ip_buffer[MAX_IP_STRING_SIZE + 1] = {0};
    char *buffer_index = NULL;
    char *max_index = NULL;
    uint8_t error_code;
    uint16_t rdata_length;
    uint32_t tmp_32;
    size_t header_size;
    struct dns_header header;
    struct dns_response *responses;
    int answer_count = 0;

    authoritative = 0;
    header_size = sizeof(struct dns_header);
  
    if (client->recv_len < header_size) 
    {
        TRACE("> DNS ERROR, response has invalid format. %s %d\r\n", MDL);
        return -1;
    }

    buffer_index = client->recv_buff;
    max_index = buffer_index + client->recv_len;

    memcpy(&header, buffer_index, header_size);
    buffer_index += header_size;

    header.id	    = ntohs(header.id);
    header.flags	= ntohs(header.flags);
    header.qd_count = ntohs(header.qd_count);
    header.an_count = ntohs(header.an_count);
    header.ns_count = ntohs(header.ns_count);
    header.ar_count = ntohs(header.ar_count);

    if (header.id != client->request_id) 
    {
        TRACE("> DNS ERROR, response id does not match request id. %s %d\r\n", MDL);
        return -1;
    }

    if (!(header.flags & DNS_QR_RESPONSE)) 
    {
        TRACE("DNS ERROR, header does not contain response flag. %s %d\r\n", MDL);
        return -1;
    }

    if (header.flags & DNS_TRUNCATED) 
    {
        TRACE("> DNS ERROR, response was truncated. %s %d\r\n", MDL);
        return -1;
    }

    if (!(header.flags & DNS_RECURSION_AVAIL)) 
    {
        TRACE("> DNS ERROR, no recursion available %s %d\r\n", MDL);
        return -1;
    }

    error_code = header.flags & DNS_ERROR_MASK;

    switch (error_code) 
    {
        case DNS_FORMAT_ERROR:
            TRACE("> DNS ERROR, server unable to interpret query %s %d\r\n", MDL);
            return -1;

        case DNS_SERVER_FAILURE:
            TRACE("> DNS ERROR, unable to process due to server error %s %d\r\n", MDL);
            return -1;

        case DNS_NOT_IMPLEMENTED:
            TRACE("> DNS ERROR, server does not support requested query type %s %d\r\n", MDL);
            return -1;

        case DNS_REFUSED:
            TRACE("> DNS ERROR, server refused query %s %d\r\n", MDL);
            return -1;

        case DNS_NAME_ERROR:
            answer_count = 0;
            return -1;

        default:
            break;
    }

    if (header.an_count < 1) 
    { 
        answer_count = 0;
        return -1;
    }

    if (header.an_count > MAX_AN_COUNT) 
    {
        header.an_count = MAX_AN_COUNT;
    }

    if (header.flags & DNS_AUTH_ANS) 
    {
        authoritative = 1;
    }

    bytes_read = read_domain_name(buffer_index, client->recv_buff, client->recv_len, buffer);
    if (bytes_read == -1) 
    {
        TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
        return -1;
    }

    if (!increment_buffer_index(&buffer_index, max_index, bytes_read)) 
    {
        TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
        return -1;
    }

    if (strcmp(buffer, client->domain) != 0) 
    {
        TRACE("> DNS ERROR, the response domain does not match the request %s %d\r\n", MDL);
        return -1;
    }
  
    if (!increment_buffer_index(&buffer_index, max_index, 2 * sizeof(uint16_t))) 
    {
        TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
        return -1;
    }
 
    answer_count = header.an_count;
    responses = (struct dns_response *)malloc(sizeof(struct dns_response) * header.an_count);
    if (responses == NULL) 
    {
        TRACE("> DNS ERROR, unable to allocate memory for response %s %d\r\n", MDL);
        return -1;
    }

    memset(responses, 0, sizeof(struct dns_response) * header.an_count);
    for (i = 0; i < header.an_count; ++i) 
    {
        responses[i].authoritative = authoritative;

        bytes_read = read_domain_name(buffer_index, client->recv_buff, client->recv_len, buffer);
        if (bytes_read == -1) 
        {
            TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
            free(responses);
            return -1;
        }

        if (!increment_buffer_index(&buffer_index, max_index, bytes_read)) 
        {
            TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
            free(responses);
            return -1;
        }

        uint16_t tmp = 0;
        memcpy((char *)&tmp, buffer_index, sizeof(tmp));
        responses[i].response_type = ntohs(tmp);
    
        if (!increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) 
        {
            TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
            free(responses);
            return -1;
        }

        memcpy((char *)&tmp, buffer_index, sizeof(tmp));

        if (ntohs(tmp) != DNS_INET_ADDR) 
        {
            TRACE("> DNS ERROR, invalid response class %s %d\r\n", MDL);
            free(responses);
            return -1;
        }

        if (!increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) 
        {
            TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
            free(responses);
            return -1;
        }

        memcpy((char *)&tmp, buffer_index, sizeof(tmp));
        responses[i].cache_time = ntohl(tmp);
    
        if (!increment_buffer_index(&buffer_index, max_index, sizeof(uint32_t))) 
        {
            TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
            free(responses);
            return -1;
        }

        memcpy((char *)&tmp, buffer_index, sizeof(tmp));
        rdata_length = ntohs(tmp);

        if (!increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) 
        {
            TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
            free(responses);
            return -1;
        }
    
        switch (responses[i].response_type) 
        {
        case DNS_A_RECORD:
            memcpy((char *)&tmp_32, buffer_index, sizeof(tmp_32));
            responses[i].ip_address = ntohl(tmp_32);
            break;

        case DNS_NS_RECORD:
            bytes_read = read_domain_name(buffer_index, client->recv_buff, client->recv_len, responses[i].name);
            if (bytes_read == -1) 
            {
    	        TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
                free(responses);
    	        return -1;
            }
            break;

        case DNS_CNAME_RECORD:
            bytes_read = read_domain_name(buffer_index, client->recv_buff, client->recv_len, responses[i].name);
            if (bytes_read == -1) 
            {
    	        TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
                free(responses);
    	        return -1;
            }
            break;

        case DNS_MX_RECORD:
            memcpy((char *)&tmp, buffer_index, sizeof(tmp));
            responses[i].preference = ntohs(tmp);

            if (!increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) 
            {
    	        TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
                free(responses);
    	        return -1;
            }

            bytes_read = read_domain_name(buffer_index, client->recv_buff, client->recv_len, responses[i].name);      
            if (bytes_read == -1) 
            {
    	        TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
                free(responses);
    	        return -1;
            }
            rdata_length -= sizeof(uint16_t);
            break;
        }

        if (!increment_buffer_index(&buffer_index, max_index, rdata_length) 
            && (i + 1 < header.an_count)) 
        {
            TRACE("> DNS ERROR, response has invalid format %s %d\r\n", MDL);
            free(responses);
            return -1;
        }
    }  

    for (i = 0; i < answer_count; ++i)
    {
        if (responses[i].response_type == DNS_A_RECORD) 
        {
            memset(ip_buffer, 0, sizeof(ip_buffer));
            FormatIpAddress(responses[i].ip_address, ip_buffer, sizeof(ip_buffer));

            TRACE("* DNS resolve IP - %s. %s %d\r\n", ip_buffer, MDL);
            memset(client->dns_resolve_ip, 0, sizeof(client->dns_resolve_ip));
            strncpy(client->dns_resolve_ip, ip_buffer, sizeof(client->dns_resolve_ip) - 1);
            ret = 0;
            //break;
        }
    }

    free(responses);
    responses = NULL;
    
    return ret;
}

int dns_resolve(const char *domain, const char *dns_serv_addr, char *dns_result, size_t result_size)
{
    int ret = -1;
    int i;
    struct dns_client client;

    if (!domain || !dns_serv_addr || !dns_result)
    {
        return -1;
    }
    
    if (0 == init_dns_client(&client, domain, dns_serv_addr))
    {
        if (dns_build_request_packet(&client) == 0)
        {
            for (i = 0; i < DNS_TRY_TIMES; i++)
            {
                if (dns_send_request_packet(&client) == 0)
                {   
                    if (dns_recv_response_packet(&client) == 0)
                    {
                        if (dns_parse_response(&client) == 0)
                        {   
                            TRACE("* dns resolve success. %s %d\r\n", MDL);
                            memset(dns_result, 0, result_size);
                            strncpy(dns_result, client.dns_resolve_ip, result_size - 1);
                            ret = 0;
                            break;    
                        }
                        else
                        {
                            TRACE("> dns fail, parse dns response packet fail. %s %d\r\n", MDL);
                        }
                    }
                    else
                    {
                        TRACE("> dns fail, recv dns response packet fail. %s %d\r\n", MDL);
                    }
                }
                else
                {
                    TRACE("> dns fail, send dns request packet fail. %s %d\r\n", MDL);
                }
            }
        }
        else
        {
            TRACE("> dns fail, build dns request packet fail. %s %d\r\n", MDL);
        }

        deinit_dns_client(&client);
    }
    else
    {
        TRACE("> dns fail, init dns client fail. %s %d\r\n", MDL);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
         TRACE("> usage: dnsclient <domain> <serv_ip> \r\n");
         return -1;
    }

    char result[16] = {0};
    
    int ret = dns_resolve(argv[1], argv[2], result, sizeof(result));

    TRACE("* result = %s \r\n", result);
    
    return ret;
}
