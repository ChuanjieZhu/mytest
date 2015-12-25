
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h> 
#include <arpa/inet.h>
#include <sys/socket.h>

#include "dns.h"
#include "appLib.h"
#include "maintainLib.h"
#include "publicLib.h"

#define DEFAULT_PORT		    53
#define DEFAULT_TIMEOUT		    5000

static dns_result_cache g_dns_cache;

static int init_server_ip(char *server_ip, int length)
{   
    static char cur_dns_server = 0;
    CONFIG_NETWORK_STR strCfgNetwork;
    
    if (length < MAX_IP_STRING_SIZE) 
    {
        fprintf(stderr, "ERROR min length of server ip result is %d\n", 
                      MAX_IP_STRING_SIZE);
        return (-1);
    }

    memset(&strCfgNetwork, 0, sizeof(strCfgNetwork));
    GetConfigNetwork(&strCfgNetwork);

    /* 校验配置中的dns服务器地址是否合法 */
    if (isIP(strCfgNetwork.dnsPrimary) != TRUE) 
    {
        TRACE("Config primary dns server ip address %s is unavailable. %s %d \r\n", 
                    strCfgNetwork.dnsPrimary, __FUNCTION__, __LINE__);
        
        memset(strCfgNetwork.dnsPrimary, 0, sizeof(strCfgNetwork.dnsPrimary));
        strncpy(strCfgNetwork.dnsPrimary, SYS_CONFIG_DEFAULT_DNS, sizeof(strCfgNetwork.dnsPrimary) - 1);

        SetConfigNetwork(&strCfgNetwork);
    }

    if (isIP(strCfgNetwork.dnsSecondary) != TRUE) 
    {
        TRACE("Config secondary dns server ip address %s is unavailable. %s %d \r\n", 
                strCfgNetwork.dnsSecondary, __FUNCTION__, __LINE__);
        
        memset(strCfgNetwork.dnsSecondary, 0, sizeof(strCfgNetwork.dnsSecondary));
        strncpy(strCfgNetwork.dnsSecondary, SYS_CONFIG_DEFAULT_DNS2, sizeof(strCfgNetwork.dnsSecondary) - 1);

        SetConfigNetwork(&strCfgNetwork);
    }

    if (1 == cur_dns_server) 
    {
        strncpy(server_ip, strCfgNetwork.dnsSecondary, length - 1);
		cur_dns_server = 2;
    }
    else if (2 == cur_dns_server)
    {
        strncpy(server_ip, strCfgNetwork.dnsPrimary, length - 1);
		cur_dns_server = 1;    
    }
    else 
    {
        strncpy(server_ip, strCfgNetwork.dnsPrimary, length - 1);
		cur_dns_server = 1;
    }

    return (0);
}


int dns_resolve(const char *domain_name, const char *server_ip, char *dns_result, int result_len)
{
  char server[MAX_DOMAIN_LENGTH + 1], domain[MAX_DOMAIN_LENGTH + 1];
  char error_message[ERROR_BUFFER + 1], ip_buffer[MAX_IP_STRING_SIZE];
  int i, request_id, answer_count, flag = -1;
  size_t packet_size;
  int port, timeout, request_q_type;
  void *request_buffer = 0, *response_buffer = 0;
  struct dns_response *responses;

  *error_message = 0;

  /* Use the current time as a seed for the random number generator. */
  srand(time(0));
  
  /* set defaults for optional arguments in case none are specified */
  port		 = DEFAULT_PORT;
  timeout	 = DEFAULT_TIMEOUT;
  request_q_type = DNS_A_RECORD;

  if (result_len < MAX_IP_STRING_SIZE) 
  {
    fprintf(stderr, "ERROR min length of dns result is %d\n", 
            MAX_IP_STRING_SIZE);
    return (-1);
  }
    
  if (strlen(domain_name) > MAX_DOMAIN_LENGTH) 
  {
    fprintf(stderr, "ERROR max length of server and domain is %d\n", 
            MAX_DOMAIN_LENGTH);
    return (-1);
  }
    
  memset(domain, 0, sizeof(domain));    
  strncpy(domain, domain_name, MAX_DOMAIN_LENGTH);

  if (strlen(server_ip) > MAX_DOMAIN_LENGTH) 
  {
    fprintf(stderr, "ERROR max length of server and domain is %d\n", 
            MAX_DOMAIN_LENGTH);
    return (-1);
  }
  
  memset(server, 0, sizeof(server));
  strncpy(server, server_ip, MAX_DOMAIN_LENGTH);
  
  /* Build the DNS request packet for the supplied domain name. */
  request_buffer = build_dns_request_packet(domain, 
                                            &packet_size, 
                                            &request_id, 
					                        request_q_type, 
					                        error_message);
  if (request_buffer == 0) 
  {
    fprintf(stderr, "ERROR %s\n", error_message);
    return (-1);
  }
  
  /* Send the request packet and wait for a response from the server. */
  response_buffer = query_dns_server(request_buffer,    
                                     &packet_size, 
                                     server, 
				                     port, 
				                     timeout, 
				                     error_message);
  
  free_dns_request_packet(request_buffer);
  
  if (response_buffer == 0) 
  {
    fprintf(stderr, "ERROR %s\n", error_message);
    return (-1);
  }

  /* Parse the response from the server. */
  responses = parse_dns_response(response_buffer, 
                                 packet_size, 
                                 request_id, 
				                 domain, 
				                 &answer_count, 
				                 error_message);
  
  free_response_buffer(response_buffer);
  
  /* If a null value was returned, it could either mean there was an error or
     the domain name was not found. Check the error_message buffer to see
     if it contains any data. */
  if (responses == 0) 
  {
    if (*error_message != 0) 
    {
      fprintf(stderr, "ERROR %s\n", error_message);
      return (-1);
    }
    else 
    {
      fprintf(stdout, "NOTFOUND\n");
      return (-1);
    }
  }
  
  for (i = 0; i < answer_count; ++i)
  {
    if (responses[i].response_type == DNS_A_RECORD) 
    {
      format_ip_address(responses[i].ip_address, ip_buffer);
      if (isIP(ip_buffer) == TRUE)
      {
        fprintf(stdout, "DNS Resolve IP - %s. %s %d\r\n", ip_buffer, __FUNCTION__, __LINE__);
        memset(dns_result, 0, result_len);
        strncpy(dns_result, ip_buffer, result_len - 1);
        flag = 0;
        break;
      }
    }
  }

  free(responses);
  
  return flag == 0 ? 0 : -1;
}

/* Formats a 32-bit IP address into a dotted quad string and
   copies it into the given buffer. */
void format_ip_address(uint32_t ip_address, char *buffer) 
{
  uint8_t *segments = (uint8_t *)&ip_address;

  sprintf(buffer, "%d.%d.%d.%d", segments[3], segments[2],
  	  segments[1], segments[0]);
}


void init_dns_cache()
{
    memset(&g_dns_cache, 0, sizeof(dns_result_cache));
    g_dns_cache.count = 0;
}

int find_dns_cache(const char *domain_name, char *dns_result, int result_len)
{
    int i, ret = -1;
    int count = g_dns_cache.count;

    for (i = 0; i < count; i++)
    {
        if (strncmp(domain_name, g_dns_cache.result[i].domian_name, strlen(domain_name)) == 0) 
        {
            TRACE("Find domain name's ip address from local dns result cache.... \r\n");
            memset(dns_result, 0, result_len);
            strncpy(dns_result, g_dns_cache.result[i].resolve_ip, result_len - 1);
            ret = 0;
            break;
        }
    }

    if (ret == 0 && i < count) 
    {
        return (0);
    }

    return (-1);
}


int update_dns_cache(const char *domain_name, char *dns_result)
{
    int i;
    int count = g_dns_cache.count;

    TRACE("DNS local cache record count: %d %s %d\r\n", count, __FUNCTION__, __LINE__);
    
    for (i = 0; i < count; i++)
    {
        /* domain_name已经在cache中了,则更新 */
        if (strncmp(g_dns_cache.result[i].domian_name, domain_name, strlen(domain_name)) == 0)
        {
            TRACE("Domain %s ip address is already in the cache, update it.... \r\n", domain_name);
            memset(g_dns_cache.result[i].resolve_ip, 0, MAX_IP_STRING_SIZE);
            strncpy(g_dns_cache.result[i].resolve_ip, dns_result, MAX_IP_STRING_SIZE - 1);
            break;
        }
    }

    /* 已经找到 */
    if (i < count)
    {
        return (0);
    }
    
    /* cache已满，随机选择一个替换 */
    if (count >= MAX_RES_NUM)
    {
        int index = Rand() % MAX_RES_NUM;
        memset(&g_dns_cache.result[index], 0, sizeof(dns_result));
        strncpy(g_dns_cache.result[index].domian_name, domain_name, MAX_DOMAIN_LENGTH);
        strncpy(g_dns_cache.result[index].resolve_ip, dns_result, MAX_IP_STRING_SIZE - 1);
    }
    else /* cache未满，则增加 */
    {
        memset(&g_dns_cache.result[count], 0, sizeof(dns_result));
        strncpy(g_dns_cache.result[count].domian_name, domain_name, MAX_DOMAIN_LENGTH);
        strncpy(g_dns_cache.result[count].resolve_ip, dns_result, MAX_IP_STRING_SIZE - 1);
        g_dns_cache.count++;
    }

    return (0);
}


int dns_client(const char *domain_name, char *dns_result, int result_len)
{
    int ret = -1;
    char server_ip[16] = {0};
        
    ret = init_server_ip(server_ip, sizeof(server_ip));
    if (ret != 0) 
    {
        TRACE("Init dns server ip error. %s %d \r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    ret = dns_resolve(domain_name, server_ip, dns_result, result_len);
    if (ret != 0) 
    {
        TRACE("Dns resolve error. %s %d \r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    /* 更新本地dns缓存 */
    update_dns_cache(domain_name, dns_result);

    return (0);
}


int deal_dns_resolve(const char *domain_name, char *dns_result, int result_len, int flag)
{
    int ret = -1;

    /* 强制再次获取 */
    if (flag == 1)
    {
        ret = dns_client(domain_name, dns_result, result_len);
    }
    else
    {   
        if (find_dns_cache(domain_name, dns_result, result_len) == 0) 
        {
            return 0;
        }

        ret = dns_client(domain_name, dns_result, result_len);
    }

    return ret;
}


#if 0
int main(int argc, char **argv)
{
    const char *domain_name = "www.baidu.com";
    char dns_result[64] = {0};
    int flag = 1;

    int ret = deal_dns_resolve(domain_name, dns_result, sizeof(dns_result), flag);
    if (ret != 0)
    {
        TRACE("Deal dns resolve failure. %s %d\r\n", __FUNCTION__, __LINE__);
    }

    return 0;
}

#endif

