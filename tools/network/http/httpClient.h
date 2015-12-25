#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>
#include "http.h"

#ifdef __cplusplus
extern "C" {
#endif

#define closesocket(s)		close(s)

typedef size_t (*MRJ_HTTP_CALLBACK)(char *buffer,
                                 size_t size,
                                 size_t nitems,
                                 void *outstream);

typedef struct http_client {
    int sockfd;
    char serv_addr[16];
    int serv_port;
    struct sockaddr_in addr;
    int connected;
} HTTP_CLIENT;

typedef struct http_request_head {
    char action[64];
    char host_port[64];
    char accept[64];
    char content_len[64];
    char content_type[128];
    char range[64];
} HTTP_REQUEST_HEAD;

typedef struct http_request {
    char url[128];
    int range_low;
    int range_height;
    char send_data[2048];
} HTTP_REQUEST; 

typedef struct http_response {
    int time_wait;
    size_t recv_max_len;
    char recv_data[10 * 1024];
    DVS_HTTPEVENT parseFunc;
    void *args;
    int return_code;
    int content_len;
    int total_len;
} HTTP_RESPONSE;



#define MRJ_DEF_PORT			80			/* 默认的HTTP连接端口号 */
#define DVS_MAXPATH				256			/* 默认最大的URL路径 */
#define DVS_MAX_REQHEAD			1024
#define DVS_MAX_RESPHEAD		2048
#define DVS_MAX_HOST			128
#define DVS_MAX_FILETYPE		64
#define DVS_MAX_BUFSIZE         2048
#define DVS_DOWN_PERSIZE		1440
#define DVS_FLUSH_BLOCK			1024
#define MRJ_MAX_URL             1024

typedef enum
{
    MRJE_OK  = 0,
	MRJE_UNKNOWN,                /* 1 */
	MRJE_URLINLEGAL,             /* 2 */
	MRJE_CRSOCKFAL,              /* 3 */
	MRJE_IOCTLFAL,               /* 4 */
	DVS_ERR_NOTFINDHOST,            /* 5 */
	MRJE_NOTCONNHOST,            /* 6 */
	DVS_ERR_DISCONNHOST,            /* 7 */
	DVS_ERR_NOCOMPLETEREQ,          /* 8 */
	DVS_ERR_SNDREQERR,              /* 9 */
	DVS_ERR_EMPTYRES,               /* 10 */
	DVS_ERR_NOCOMPLETERES,          /* 11 */
	DVS_ERR_CRLOCALFILEFAL,         /* 12 */
	DVS_ERR_SELECTFAL,              /* 13 */
	DVS_ERR_TIMEOUT,                /* 14 */
	DVD_ERR_OUTOFMEMORY,            /* 15 */
	DVS_ERR_NORESPONSE,             /* 16 */
	DVS_ERR_SERVERE400,             /* 17 */
	DVS_ERR_SERVERE401,             /* 18 */
	DVS_ERR_SERVERE403,             /* 19 */
	DVS_ERR_SERVERE404,             /* 20 */
	DVS_ERR_SERVERE500,             /* 21 */
	DVS_ERR_SERVERE503,             /* 22 */
	MRJE_BAD_FUNCTION_ARGUMENT,       /* 23 */
	MRJE_OUT_OF_MEMORY,
	MRJE_UNSUPPORTED_PROTOCOL,
	MRJE_UNKNOWN_OPTION
} MRJcode;


typedef struct
{
	mrj_socket_t        sock;                           /* sockfd */
    int                 time_wait;                      /* recv time out */
	struct sockaddr_in	remote_addr;                    /* server sockaddr_in */
	int 				port;                           /* port */
	char				host[DVS_MAX_HOST];             /* host ip */
	char 				remote_path[DVS_MAXPATH];       /* request remote path */ 
	char 				remote_file[DVS_MAXPATH];       
	char 				file_type[DVS_MAX_FILETYPE];
	unsigned int 		file_length;
	FILE *				local_stream;
    DVS_HTTPEVENT       event_func;
    void *              event_args;
    int                 return_code;
    size_t              range_low;
    size_t              range_hight;
	char				local_filepath[DVS_MAXPATH];
    size_t              send_len;
    size_t              recv_len;
    char                send_buf[DVS_MAX_BUFSIZE];
	char				recv_buf[DVS_MAX_BUFSIZE];
	char				request_head[DVS_MAX_REQHEAD];
	char				response_head[DVS_MAX_RESPHEAD];
} DVS_Http;


DVS_Http *dvshttp_create();
int dvshttp_load_url(DVS_Http *http, const char *url);
int dvshttp_load_host(DVS_Http *http, const char *ip, int port, const char *remote_path, const char *remote_file);

int dvshttp_download(DVS_Http *http, const char *local_path, const char *local_file, DVS_HTTPEVENT http_event);
int dvshttp_post(DVS_Http *http);
void dvshttp_free(DVS_Http *http);

int http_socket_create(HTTP_CLIENT *http_client, const char *host_ip, const int port);
int http_socket_close(HTTP_CLIENT *http_client);
int http_connect(HTTP_CLIENT *http_client);
int http_send_request(HTTP_CLIENT *http_client, void *buffer, size_t length);
int http_recv_response(HTTP_CLIENT *http_client, HTTP_RESPONSE *http_response);
int http_parse_response(HTTP_RESPONSE *http_response);
int http_post(HTTP_CLIENT *http_client, HTTP_REQUEST *http_request, HTTP_RESPONSE *http_response);

#ifdef __cplusplus
}
#endif

#endif /* _HTTP_CLIENT_H_ */

