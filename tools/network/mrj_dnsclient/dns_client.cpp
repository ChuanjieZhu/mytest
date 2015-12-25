

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "dns_client.h"

static struct dns_result_cache g_cache;

static void dns_init_cache()
{
    static int count = 0;

    if (count++ > 0) {
        TRACE("DNS cache init times: %d %s %d\r\n", count, MDL);
        return;
    }

    memset(&g_cache, 0, sizeof(struct dns_result_cache));
    g_cache.count = 0;
}

static int dns_rand()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );
	unsigned int seed = tv.tv_sec ^ tv.tv_usec;
	return rand_r(&seed);
}


void dns_trace(struct session_handle *data, const char *message, const char *func, int line)
{
    if (data->useset.verbose)
        TRACE("> %s %s %d\r\n", message, func, line);
}

MRJEcode dns_setstropt(char **charp, char *s)
{
    safefree(*charp);

    if(s) {
        s = strdup(s);

        if(!s)
            return MRJE_OUT_OF_MEMORY;

        *charp = s;
    }
    
    return MRJE_OK;
}


MRJEcode dns_isip(char *ipstr)
{
	int i;
    int j;
    int len;
    int dotnum = 0;
	Bool res = FALSE;
    Bool isdot = TRUE;

    if (!ipstr)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
	len = strlen(ipstr);
	if (len >= 7 && len <= 15) {
		for (i = j = 0; i < len; i++) {
			if (ipstr[i]=='.') {
				if (isdot)
                    break;

                isdot = TRUE;

                if (atoi(ipstr + j) > 255) 
                    break;
                
				dotnum++;
				j = i + 1;
			} else {
				if (ipstr[i] < '0' || ipstr[i] > '9') 
                    break;
                
				isdot = FALSE;
                
				if ((i - j) > 2) 
                    break;
			}
		}
        
		if (i == len && dotnum == 3 && atoi(ipstr + j) <= 255) 
            res = TRUE;
	}
    
	if (res == TRUE)
        return MRJE_OK;
    
    return MRJE_BAD_FUNCTION_ARGUMENT;
}

MRJEcode dns_open_socket(struct session_handle *data)
{
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    if (data->sockfd >= 0)
        close(data->sockfd);

    data->sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (data->sockfd < 0) {
        dns_trace(data, "create socket error", MDL);
        return MRJE_FUNC_SOCKET_ERR;
    }

    /* Setup the destination address for the packet. */
    memset(&data->server_addr, 0, sizeof(struct sockaddr_in));
    data->server_addr.sin_family      = AF_INET;
    data->server_addr.sin_addr.s_addr = inet_addr(data->useset.server);
    data->server_addr.sin_port	      = htons(data->useset.port);

    return MRJE_OK;
}

MRJEcode dns_select(struct session_handle *data)
{
    int res = -1;
    mrj_socket_t sockfd = data->sockfd;
    fd_set rset, eset;
    struct timeval tv;

    tv.tv_sec = data->useset.timeout / 1000L;
    tv.tv_usec = 0;
    
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    FD_ZERO(&eset);
    FD_SET(sockfd, &eset);

    res = select(sockfd + 1, &rset, NULL, &eset, &tv);
    if (res == 0) {
        dns_trace(data, "select time out error", MDL);
        return MRJE_TIMEOUT;
    } else if (res < 0 || FD_ISSET(sockfd, &eset)) {
        dns_trace(data, "select error", MDL);
        return MRJE_FUNC_SELECT_ERR;
    } else {
        if (FD_ISSET(sockfd, &rset) > 0) {
            return MRJE_OK;
        }
    }

    return MRJE_FUNC_SELECT_ERR;
}

MRJEcode dns_build_request_packet(struct session_handle *data)
{
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    struct dns_header header;
    struct dns_question_trailer q_trailer;
    size_t domain_lenght;
    size_t question_size;
    size_t header_size;
    size_t total_size;
    size_t token_length;
    char *token_index, *save_pointer;
    char *buffp = NULL;
    char temp_buffer[MAX_DOMAIN_LENGTH + 1];

    domain_lenght = strlen(data->useset.domain_name);
    if (domain_lenght > MAX_DOMAIN_LENGTH) {
        dns_trace(data, "domain name too long", MDL);
        return MRJE_BAD_FUNCTION_ARGUMENT;
    }

    question_size = domain_lenght + sizeof(struct dns_question_trailer) + 2;
    header_size = sizeof(struct dns_header);
    total_size = question_size + header_size;
    data->useset.send_len = total_size;

    safefree(data->useset.send);
    
    data->useset.send = (char *)malloc(total_size);
    if (!data->useset.send) {
        dns_trace(data, "error allocating memory", MDL);
        return MRJE_OUT_OF_MEMORY;
    }

    buffp = data->useset.send;
    memset(buffp, 0, total_size);
    
    data->proset.request_id = rand() % 65535;
    
    memset(&header, 0, header_size);
    header.id	      = htons(data->proset.request_id);
    header.flags	  = htons(DNS_USE_RECURSION);
    header.qd_count   = htons(1);
    header.an_count   = htons(0);
    header.ns_count   = htons(0);
    header.ar_count   = htons(0);

    memcpy(buffp, &header, header_size);
    buffp += header_size;

    strcpy(temp_buffer, data->useset.domain_name);
    
    token_index = strtok_r(temp_buffer, ".", &save_pointer);
    while (token_index != NULL) {
        token_length = strlen(token_index);
        if (token_length > MAX_SUBDOMAIN_LENGTH) 
        {
            dns_trace(data, "subdomain is too long", MDL);
            return MRJE_BAD_FUNCTION_ARGUMENT;
        }

        *buffp++ = token_length;
        while ((*buffp++ = *token_index++) != 0);

        buffp--;
        token_index = strtok_r(0, ".", &save_pointer);
    }

    *buffp++ = 0;
    q_trailer.q_type  = htons(data->proset.request_q_type);
    q_trailer.q_class = htons(DNS_INET_ADDR);

    memcpy(buffp, &q_trailer, sizeof(struct dns_question_trailer));

    return MRJE_OK;
}


MRJEcode dns_send_request_packet(struct session_handle *data)
{
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    size_t bytes_sent = 0;
    
    bytes_sent = sendto(data->sockfd, 
                        data->useset.send,
                        data->useset.send_len,
                        0,
                        (struct sockaddr *)&data->server_addr,
  		                sizeof(struct sockaddr_in));
    
    if (bytes_sent != data->useset.send_len) {
        dns_trace(data, "send request packet error", MDL);
        return MRJE_SEND_ERR;
    }

    return MRJE_OK;
}


MRJEcode dns_recv_response_packet(struct session_handle *data)
{   
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    MRJEcode res = MRJE_UNKNOW;
    size_t bytes_recv = 0;

    safefree(data->useset.recv);
    
    data->useset.recv = (char *)malloc(RESPONSE_BUFFER_SIZE);
    if (!data->useset.recv) {
        dns_trace(data, "dns recv response packet malloc error", MDL);
        return MRJE_OUT_OF_MEMORY;
    }
    
    res = dns_select(data);
    if (res != MRJE_OK)
        return res;

    bytes_recv = recvfrom(data->sockfd,
                          data->useset.recv,
                          RESPONSE_BUFFER_SIZE - 1,
                          0,
                          0,
                          0);
    
    if (bytes_recv <= 0 || bytes_recv < sizeof(struct dns_header)) {
        dns_trace(data, "recv response packet error", MDL);
        return MRJE_RECV_ERR;
    }

    TRACE("bytes_recv: %d %s %d\r\n", bytes_recv, MDL);
    
    data->useset.recv_len = bytes_recv;
    data->useset.recv[data->useset.recv_len] = '\0';
    
    return MRJE_OK;    
}


int dns_increment_buffer_index(char **buffer_pointer, char *max_index, int bytes) 
{
    *buffer_pointer += bytes;
    return *buffer_pointer >= max_index ? -1 : 0;
}

int dns_read_response_domainname(char *packet_index, char *packet_start, int packet_size, 
			                    char *dest_buffer)
{
    int bytes_read;
    uint8_t label_length;
    uint16_t offset;
    char *max_index;
    
    bytes_read = 0;
    max_index = packet_start + packet_size;

    /* The domain name is stored as a series of sub-domains or pointers to
     sub-domains. Each sub-domain contains the length as the first byte, 
     followed by LENGTH number of bytes (no null-terminator). If it's a pointer,
     the first two bits of the length byte will be set, and then the rest of
     the bits contain an offset from the start of the packet to another
     sub-domain (or set of sub-domains). 

     We first get the length of the sub-domain (or label), check if it's a
     pointer, and if not, read the that number of bytes into a buffer. Each
     sub-domain is separated by a period character. If a pointer is found,
     we can call this function recursively. 

     The end of the domain name is found when we read a label length of 
     0 bytes. */

    if (packet_index >= max_index) {
        return -1;
    }
  
    label_length = (uint8_t)*packet_index;

    while (label_length != 0) {

        /* If this isn't the first label, add a period in between
           the labels. */
        if (bytes_read > 0) {
            *dest_buffer++ = '.';
        }

        /* Check to see if this label is a pointer. */
        if ((label_length & DNS_POINTER_FLAG) == DNS_POINTER_FLAG) {
            char *new_packet_index;

            uint16_t tmp = 0;
            memcpy((char *)&tmp, packet_index, sizeof(tmp));
            offset = ntohs(tmp) & DNS_POINTER_OFFSET_MASK;
          
            new_packet_index = packet_start + offset;
            if (new_packet_index >= max_index) {
    	        return -1;
            }

            /* Recursively call this function with the packet index set to
    	    the offset value and the current location of the destination
    	    buffer. Since we're using an offset and reading from some
    	    other part of memory, we only need to increment the number
    	    of bytes read by 2 (for the pointer value). */
            dns_read_response_domainname(new_packet_index, packet_start, packet_size, dest_buffer);
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


MRJEcode dns_parse_response(struct session_handle *data)
{
    int i, bytes_read, authoritative;
    char buffer[MAX_DOMAIN_LENGTH + 1];
    char *buffer_index, *max_index, *packet_buff;
    uint8_t error_code;
    uint16_t rdata_length;
    uint32_t tmp_32;
    size_t header_size;
    size_t packet_length = 0;
    struct dns_header header;

    authoritative = 0;
    header_size = sizeof(struct dns_header);
    
    packet_length = data->useset.recv_len;
    if (packet_length < header_size) {
        dns_trace(data, "response has invalid format", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }
    
    packet_buff = data->useset.recv;
    buffer_index = data->useset.recv;
    max_index = buffer_index + packet_length;

    memset(&header, 0, sizeof(struct dns_header));
    memcpy(&header, buffer_index, header_size);
    buffer_index += header_size;

    header.id	    = ntohs(header.id);
    header.flags	= ntohs(header.flags);
    header.qd_count = ntohs(header.qd_count);
    header.an_count = ntohs(header.an_count);
    header.ns_count = ntohs(header.ns_count);
    header.ar_count = ntohs(header.ar_count);

    if (header.id != data->proset.request_id) {
        dns_trace(data, "response id does not match request id", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }

    if (!(header.flags & DNS_QR_RESPONSE)) {
        dns_trace(data, "header does not contain response flag", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }

    /* If the message was truncated, return an error. */
    if (header.flags & DNS_TRUNCATED) {
        dns_trace(data, "response was truncated", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }

    /* If no recursion is available, return an error. */
    if (!(header.flags & DNS_RECURSION_AVAIL)) {
        dns_trace(data, "no recursion available", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }

    /* Check for error conditions. */
    error_code = header.flags & DNS_ERROR_MASK;
    switch (error_code) {
    case DNS_FORMAT_ERROR:
        dns_trace(data, "server unable to interpret query", MDL);
        return MRJE_FUNC_PARSE_ERR;

    case DNS_SERVER_FAILURE:
        dns_trace(data, "unable to process due to server error", MDL);
        return MRJE_FUNC_PARSE_ERR;

    case DNS_NOT_IMPLEMENTED:
        dns_trace(data, "server does not support requested query type", MDL);
        return MRJE_FUNC_PARSE_ERR;

    case DNS_REFUSED:
        dns_trace(data, "server refused query", MDL);
        return MRJE_FUNC_PARSE_ERR;

    case DNS_NAME_ERROR:
        /* A name error indicates that the name was not found. This isn't due to
            an error, so we just indicate that the number of answers is 0 and return
            a null value. */
        data->answer_count = 0;
        return MRJE_FUNC_PARSE_ERR;

    default:
        break;
    }

    /*  Verify that there is at least one answer. We also put a limit on the number
        of answers allowed. This is to prevent a bogus response containing a very
        high answer count from allocating too much memory by setting an upper
        bound. */
    if (header.an_count < 1) { 
        data->answer_count = 0;
        return MRJE_FUNC_PARSE_ERR;
    }

    if (header.an_count > MAX_AN_COUNT) {
        header.an_count = MAX_AN_COUNT;
    }

    /* Is this response authoritative? */
    if (header.flags & DNS_AUTH_ANS) {
        authoritative = 1;
    }

    /* Verify that the question section contains the domain name we requested. */
    bytes_read = dns_read_response_domainname(buffer_index,     
                                              packet_buff, 
                                              packet_length, 
                                              buffer);
    if (bytes_read == -1) {
        dns_trace(data, "response has invalid format", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }

    if (0 != dns_increment_buffer_index(&buffer_index, max_index, bytes_read)) {
        dns_trace(data, "response has invalid format", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }

    if (strcmp(buffer, data->useset.domain_name) != 0) {
        dns_trace(data, "the response domain does not match the request", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }

    /* After the null root character, skip over the QTYPE and QCLASS sections which
     should put the buffer index at the start of the answer section. */
    if (0 != dns_increment_buffer_index(&buffer_index, max_index, 2 * sizeof(uint16_t))) {
        dns_trace(data, "response has invalid format", MDL);
        return MRJE_FUNC_PARSE_ERR;
    }
  
  /* Answer section. There may be multiple answer sections which we can determine from
     the packet header. Allocate enough space for all of the buffers. 

     The first part of each answer section is similar to the question section, containing
     the name  that we queried for. Ignore this for now, maybe verify that it is the
     same name later. */
    data->answer_count = header.an_count;
    safefree(data->resps);
    data->resps = (struct dns_response *)malloc(sizeof(struct dns_response) * header.an_count);
    if (!data->resps) {
        dns_trace(data, "unable to allocate memory for response", MDL);
        return MRJE_OUT_OF_MEMORY;
    }

    memset(data->resps, 0, sizeof(struct dns_response) * header.an_count);

    /* Fill out the dns_response structure for each answer. */
    for (i = 0; i < header.an_count; ++i) {

        data->resps[i].authoritative = authoritative;

        /* Read the domain name from the answer section and verify it matches
         the name in the question section. */
        bytes_read = dns_read_response_domainname(buffer_index, (char *)packet_buff, packet_length, buffer);
        if (bytes_read == -1) {
            dns_trace(data, "response has invalid format", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }

        if (0 != dns_increment_buffer_index(&buffer_index, max_index, bytes_read)) {
            dns_trace(data, "response has invalid format", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }

        /* The next part contains the type of response in 2 bytes. */
        uint16_t tmp = 0;
        memcpy((char *)&tmp, buffer_index, sizeof(tmp));
        data->resps[i].response_type = ntohs(tmp);
    
    
        if (0 != dns_increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) {
            dns_trace(data, "response has invalid format", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }

        /* The response class should be for an Internet address. */
        tmp = 0;
        memcpy((char *)&tmp, buffer_index, sizeof(tmp));
        if (ntohs(tmp) != DNS_INET_ADDR) {
            dns_trace(data, "invalid response class", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }

        if (0 != dns_increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) {
            dns_trace(data, "response has invalid format", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }

        /* The next 4 bytes contain the TTL value. */
        tmp = 0;
        memcpy((char *)&tmp, buffer_index, sizeof(tmp));
        data->resps[i].cache_time = ntohl(tmp);
    
        if (0 != dns_increment_buffer_index(&buffer_index, max_index, sizeof(uint32_t))) {
            dns_trace(data, "response has invalid format", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }

        /* The next 2 bytes contain the length of the RDATA field. */
        tmp = 0;
        memcpy((char *)&tmp, buffer_index, sizeof(tmp));
        rdata_length = ntohs(tmp);

        if (0 != dns_increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) {
            dns_trace(data, "response has invalid format", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }

        /* At the RDATA field. How we process the data depends on the type of response
           this is. */
        switch (data->resps[i].response_type) {
        case DNS_A_RECORD:
            memcpy((char *)&tmp_32, buffer_index, sizeof(tmp_32));
            data->resps[i].ip_address = ntohl(tmp_32);
            break;

        case DNS_NS_RECORD:
            bytes_read = dns_read_response_domainname(buffer_index, (char *)packet_buff, 
                                                    packet_length, data->resps[i].name);
            if (bytes_read == -1) {
	            dns_trace(data, "response has invalid format", MDL);
	            return MRJE_FUNC_PARSE_ERR;
            }
            break;

        case DNS_CNAME_RECORD:
            bytes_read = dns_read_response_domainname(buffer_index, (char *)packet_buff, 
                                                    packet_length, data->resps[i].name);
            if (bytes_read == -1) {
	            dns_trace(data, "response has invalid format", MDL);
	            return MRJE_FUNC_PARSE_ERR;
            }
            break;

        case DNS_MX_RECORD:
            tmp = 0;
            memcpy((char *)&tmp, buffer_index, sizeof(tmp));
            data->resps[i].preference = ntohs(tmp);
      

            if (0 != dns_increment_buffer_index(&buffer_index, max_index, sizeof(uint16_t))) {
	            dns_trace(data, "response has invalid format", MDL);
	            return MRJE_FUNC_PARSE_ERR;
            }

            bytes_read = dns_read_response_domainname(buffer_index, (char *)packet_buff, 
				                                    packet_length, data->resps[i].name);      
            if (bytes_read == -1) {
                dns_trace(data, "response has invalid format", MDL);
                return MRJE_FUNC_PARSE_ERR;
            }

            rdata_length -= sizeof(uint16_t);
            break;
        }

        /* When we increment the buffer, we may move past the end of the packet at this
           point. This is OK only if this is the last answer we are processing. */
        if (0 != dns_increment_buffer_index(&buffer_index, max_index, rdata_length) &&
            (i + 1 < header.an_count)) {
            dns_trace(data, "response has invalid format", MDL);
            return MRJE_FUNC_PARSE_ERR;
        }
    }  

    return MRJE_OK;
}

static MRJEcode dns_format_ipaddress(uint32_t ipaddr, char *ipbufp, size_t ip_buf_len) 
{
    uint8_t *segments = (uint8_t *)&ipaddr;
    
    snprintf(ipbufp, 
             ip_buf_len - 1, 
             "%d.%d.%d.%d", 
             segments[3], 
             segments[2],
  	         segments[1], 
  	         segments[0]);
    
    return MRJE_OK;
}

static MRJEcode dns_get_ipstr(char *bufp, size_t bufsize, u_int32_t ip)
{
    if (!bufp)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    struct in_addr src;
	src.s_addr = ip;

    memset(bufp, 0, bufsize);
    
	if (inet_ntop(AF_INET, ((struct sockaddr_in *)&src), bufp, bufsize) == NULL) {
        return MRJE_UNKNOW;
	}
    
	return MRJE_OK;
}

static MRJEcode dns_get_result(struct session_handle *data)
{
    if (!data)
    {
        return MRJE_BAD_FUNCTION_ARGUMENT;
    }

    TRACE("answer_count: %d %s %d\r\n", data->answer_count, MDL);

    int i;
    for (i = 0; i < data->answer_count; i++) 
    {
        if (data->resps[i].response_type == DNS_A_RECORD) 
        {
            dns_format_ipaddress(data->resps[i].ip_address, 
                data->result_ip, sizeof(data->result_ip));
            
            TRACE("result ip: %s %s %d\r\n", data->result_ip, MDL);
            
            if (dns_isip(data->result_ip) == MRJE_OK) 
            {
                return MRJE_OK;
            }
        }    
    }

    return MRJE_UNKNOW;
}


static MRJEcode dns_updata_cache(struct session_handle *data)
{
    int i;
    int count = 0;
    
    dns_init_cache();
    
    struct dns_result_cache *cachep = &g_cache;
    count = cachep->count;

    for (i = 0; i < count; i++) {
        if (strcmp(cachep->result[i].domain_name, data->useset.domain_name) == 0) {
            TRACE("Domain %s ip address is already in the cache, update it.... \r\n", data->useset.domain_name);
            memset(cachep->result[i].result_ip, 0, MAX_IP_STRING_SIZE);
            strncpy(cachep->result[i].result_ip, data->result_ip, MAX_IP_STRING_SIZE - 1);
            break;
        }
    }

    if (i < count) {
        return MRJE_OK;
    } else if (count >= MAX_CACHE_NUM) {
        int index = dns_rand() % MAX_CACHE_NUM;
        memset(&cachep->result[index], 0, sizeof(struct dns_result));
        strncpy(cachep->result[index].domain_name, data->useset.domain_name, MAX_DOMAIN_LENGTH);
        strncpy(cachep->result[index].result_ip, data->result_ip, MAX_IP_STRING_SIZE - 1);
    } else {
        memset(&cachep->result[count], 0, sizeof(struct dns_result));
        strncpy(cachep->result[count].domain_name, data->useset.domain_name, MAX_DOMAIN_LENGTH);
        strncpy(cachep->result[count].result_ip, data->result_ip, MAX_IP_STRING_SIZE - 1);
        cachep->count++;
    }

    return MRJE_OK;
}


static MRJEcode dns_set_serverip(struct session_handle *data)
{
    if (!data) {
        return MRJE_BAD_FUNCTION_ARGUMENT;
    }

    if (data->useset.server1 == NULL && 
        data->useset.server2 == NULL) {
        TRACE("Error, dns server ip is null. %s %d\r\n", MDL);
        return MRJE_BAD_FUNCTION_ARGUMENT;
    }
    
    static int now_serv = 0;
    
    if (1 == now_serv) {
        if (data->useset.server2) {
            strncpy(data->useset.server, data->useset.server2, 
                        sizeof(data->useset.server) - 1);
            now_serv = 2;
        } else {
            strncpy(data->useset.server, data->useset.server1, 
                        sizeof(data->useset.server) - 1);
            now_serv = 2;
        }
    } else if (2 == now_serv) {
        if (data->useset.server1) {
            strncpy(data->useset.server, data->useset.server1, 
                        sizeof(data->useset.server) - 1);
            now_serv = 1;
        } else {
            strncpy(data->useset.server, data->useset.server2, 
                        sizeof(data->useset.server) - 1);
            now_serv = 1;
        }
    } else {
        if (data->useset.server1) {
            strncpy(data->useset.server, data->useset.server1, 
                        sizeof(data->useset.server) - 1);
            now_serv = 1;
        } else {
            strncpy(data->useset.server, data->useset.server2, 
                        sizeof(data->useset.server) - 1);
            now_serv = 1;
        }
    }

    return MRJE_OK;
}

static MRJEcode dns_transfer(struct session_handle *data)
{
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    MRJEcode res = MRJE_UNKNOW;
    size_t len = sizeof(data->result_ip);
    
    if (len < MAX_IP_STRING_SIZE) {
        dns_trace(data, "ERROR min length of dns result", MDL);
        return MRJE_BAD_FUNCTION_ARGUMENT;
    }

    if (strlen(data->useset.domain_name) > MAX_DOMAIN_LENGTH) {
        dns_trace(data, "ERROR max length of server and domain", MDL);
        return MRJE_BAD_FUNCTION_ARGUMENT;
    }

    res = dns_set_serverip(data);
    if (res != MRJE_OK)
        return res;

    res = dns_open_socket(data);
    if (res != MRJE_OK)
        return res;

    res = dns_build_request_packet(data);
    if (res != MRJE_OK)
        return res;    
    
    res = dns_send_request_packet(data);
    if (res != MRJE_OK)
        return res;

    usleep(100 * 1000);
    
    res = dns_recv_response_packet(data);
    if (res != MRJE_OK)
        return res;

    res = dns_parse_response(data);
    if (res != MRJE_OK)
        return res;

    res = dns_get_result(data);
    if (res != MRJE_OK)
        return res;

    res = dns_updata_cache(data);
    if (res != MRJE_OK)
        return res;

    return res;
}

static MRJVOID *dns_init()
{
    struct session_handle *handle;
    handle = (struct session_handle *)malloc(sizeof(struct session_handle));
    if (!handle) {
        TRACE("> dns init malloc error. %s %d\r\n", MDL);
        return NULL;
    }

    memset(handle, 0, sizeof(struct session_handle));
    handle->sockfd = -1;
    handle->useset.timeout = 5L;
    handle->useset.port = DNS_DEFAULT_PORT;
    handle->proset.request_q_type = DNS_A_RECORD;

    return handle;
}

static MRJEcode dns_close(struct session_handle *data)
{
    if (data->sockfd != -1) {
        close(data->sockfd);
        data->sockfd = -1;
    }

    safefree(data->resps);               /* strdup */
    safefree(data->useset.send);
    safefree(data->useset.recv);
    safefree(data->useset.server1);
    safefree(data->useset.server2);
    safefree(data->useset.domain_name);        /* strdup */

    free(data);
    
    return MRJE_OK;
}


static MRJEcode dns_setopt(struct session_handle *handle, DNSoption option, va_list param)
{
    char *argptr;
    MRJEcode result = MRJE_OK;
    long arg;

    switch (option) {
    case DNSOPT_VERBOSE:
        arg = va_arg(param, long);
        handle->useset.verbose = (0 != arg) ? TRUE : FALSE;
        break;

    case DNSOPT_TIMEOUT:
        arg = va_arg(param, long);
        handle->useset.timeout = arg * 1000L;
        break;

    case DNSOPT_SERVER_1:
        argptr = va_arg(param, char *);
        if (dns_isip(argptr) != MRJE_OK) 
            return MRJE_BAD_FUNCTION_ARGUMENT;
        result = dns_setstropt(&handle->useset.server1, argptr);
        break;

    case DNSOPT_SERVER_2:
        argptr = va_arg(param, char *);
        if (dns_isip(argptr) != MRJE_OK) 
            return MRJE_BAD_FUNCTION_ARGUMENT;
        result = dns_setstropt(&handle->useset.server2, argptr);
        break;

    case DNSOPT_DOMAIN_NAME:
        argptr = va_arg(param, char *);
        result = dns_setstropt(&handle->useset.domain_name, argptr);
        break;
    
    default:
        result = MRJE_UNKNOW_OPTION;
        break;
    }

    return result;
}


static MRJEcode dns_perform(struct session_handle *data) 
{
    if (!data)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    return dns_transfer(data);
}


MRJVOID *mrj_dns_init()
{
    return dns_init();
}

MRJEcode mrj_dns_setopt(MRJVOID *handle, DNSoption tag, ...)
{
    va_list arg;
    struct session_handle *data = (struct session_handle *)handle;
    MRJEcode res;
     
    if (!data) 
        return MRJE_BAD_FUNCTION_ARGUMENT;

    va_start(arg, tag);
    
    res = dns_setopt(data, tag, arg);

    va_end(arg);
        
    return res;
}

MRJEcode mrj_dns_perform(MRJVOID *handle)
{
    return dns_perform((struct session_handle *)handle);
}

void mrj_dns_cleanup(MRJVOID *handle)
{
    struct session_handle *data = (struct session_handle *)handle;

    if (!data)
        return;

    dns_close(data);
}

int mrj_dns_jobs(const char *p_domain_name)
{
    MRJVOID *handle = NULL;
    MRJEcode res = MRJE_UNKNOW;
    const char *p_serv_ip_first = "202.96.134.133";
    const char *p_serv_ip_last  = "8.8.8.8";

    if (NULL == p_domain_name)
    {
        return -1;
    }

    handle = mrj_dns_init();
    if (!handle) 
    {
        TRACE("mrj dns init error. %s %d\r\n", MDL);
        return -1;
    }

    mrj_dns_setopt(handle, DNSOPT_SERVER_1, p_serv_ip_first);
    mrj_dns_setopt(handle, DNSOPT_SERVER_2, p_serv_ip_last);
    mrj_dns_setopt(handle, DNSOPT_DOMAIN_NAME, p_domain_name);
    mrj_dns_setopt(handle, DNSOPT_TIMEOUT, 5L);
    mrj_dns_setopt(handle, DNSOPT_VERBOSE, 1L);

    res = mrj_dns_perform(handle);
    if (res != MRJE_OK)
    {
        TRACE("Dns perform error, res=%d. %s %d\r\n", (int)res, MDL);
        mrj_dns_cleanup(handle);
        return -1;
    }

    mrj_dns_cleanup(handle);

    return 0;
}

int main(int argc, char **argv)
{
    int res = -1;
    const char *p_domain_name = "io.meirenji.cn";

    if (argc != 2)
    {
        TRACE("usage: dnsclient domain name %s %d\r\n", MDL);
        return -1;
    }
    
    res = mrj_dns_jobs(argv[1]);
    
    return res;
}

