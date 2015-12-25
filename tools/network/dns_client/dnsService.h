
#ifndef _DNS_SERVICE_H_
#define _DNS_SERVICE_H_

#include "dns.h"

#ifdef __cplusplus
extern "C" {
#endif

/* dns request */
void *build_dns_request_packet(const char *domain_name, 
                                size_t *packet_size, 
			                    int *request_id,
			                    int request_q_type, 
			                    char *error_message);

void free_dns_request_packet(void *buffer);

/* dns response */
struct dns_response *parse_dns_response(void *packet_buffer, 
					                size_t packet_length, 
					                int expected_id, 
					                const char *domain_name, 
					                int *answer_count, 
					                char *error_message);

int read_domain_name(char *packet_index, 
                        char *packet_start, 
			            int packet_size, 
			            char *dest_buffer);

int increment_buffer_index(char **buffer_pointer, 
				        char *max_index, 
				        int bytes);

/* dns query */
void *query_dns_server(void *request_buffer, 
                            size_t *packet_size, 
		                    const char *server, 
		                    int port, 
		                    int timeout,  
		                    char *error_message);

void free_response_buffer(void *buffer);

#ifdef __cplusplus
}
#endif

#endif /* _DNS_SERVICE_H_ */

