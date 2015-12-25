/*
	$Id: pqhttp.h, v1.0.0, 2011.6.28, YellowBug $
	$@@: http下载 
*/

#ifndef PENQ_HTTP_DOWNLOAD_H_
#define PENQ_HTTP_DOWNLOAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pqlog.h"

#if defined(_WIN32)

#else	/* #if defined(_WIN32) */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define PQH_PATHSPLIT		'/'
#define closesocket(s)		close(s)

typedef int	PQH_SOCKET;

#endif /* #if defined(_WIN32) */



#define PQH_DEF_PORT			80			/* 默认的HTTP连接端口号 */
#define PQH_MAXPATH				260			/* 默认最大的URL路径 */
#define PQH_MAX_REQHEAD			1024
#define PQH_MAX_RESPHEAD		2048
#define PQH_MAX_HOST			128
#define PQH_MAX_FILETYPE		64
#define PQH_DOWN_PERSIZE		128	
#define PQH_FLUSH_BLOCK			1024


/*
	$@@ 错误码
*/
#define PQH_OK			0
enum PQH_Error
{
	PQH_ERR_UNKNOWN = 0x80001000,
	PQH_ERR_URLINLEGAL,
	PQH_ERR_CRSOCKFAL,
	PQH_ERR_NOTFINDHOST,
	PQH_ERR_NOTCONNHOST,
	PQH_ERR_DISCONNHOST,
	PQH_ERR_NOCOMPLETEREQ,
	PQH_ERR_SNDREQERR,
	PQH_ERR_EMPTYRES,
	PQH_ERR_NOCOMPLETERES,
	PQH_ERR_CRLOCALFILEFAL,
	PQH_ERR_TIMEOUT,
	PQH_ERR_NORESPONSE,
	PQH_ERR_SERVERE400,
	PQH_ERR_SERVERE401,
	PQH_ERR_SERVERE403,
	PQH_ERR_SERVERE404,
	PQH_ERR_SERVERE500,
	PQH_ERR_SERVERE503
};



typedef struct
{
	PQH_SOCKET			sock;
	struct sockaddr_in	remote_addr;
	int 				port;
	char				host[PQH_MAX_HOST+1];
	char 				remote_path[PQH_MAXPATH+1];
	char 				remote_file[PQH_MAXPATH+1];
	char 				file_type[PQH_MAX_FILETYPE+1];
	unsigned int 		file_length;
	FILE *				local_stream;
	char				local_filepath[PQH_MAXPATH+1];
	char				recv_buf[PQH_DOWN_PERSIZE];
	char				request_head[PQH_MAX_REQHEAD+1];
	char				response_head[PQH_MAX_RESPHEAD+1];
}PQ_Http;


/*
	$@@ 下载监听,主要用于监听下载过程
	$@ 第一个参数: 需要下载的总字节数
	$@ 第二个参数: 已经下载的总字节数
	$@ 第三个参数: 本次下载的字节数
	$@ 返回值: 0:停止下载, 非0继续下载
*/
typedef int (*PQH_DOWNEVENT)(unsigned int, unsigned int, unsigned int);


PQ_Http * pqhttp_create();
int pqhttp_load_url(PQ_Http *http, const char *url);
int pqhttp_load_host(PQ_Http *http, const char *ip, int port, const char *remote_path, const char *remote_file);
int pqhttp_test(PQ_Http *http);
int pqhttp_download(PQ_Http * http, const char *local_path, const char *local_file, PQH_DOWNEVENT http_event);
void pqhttp_free(PQ_Http * http);

#endif /* #ifndef PENQ_HTTP_DOWNLOAD_H_ */

/* END */


