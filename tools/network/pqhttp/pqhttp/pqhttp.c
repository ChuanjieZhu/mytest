/*
	$Id: PQ_Http.h, v1.0.0, 2011.6.28, YellowBug $
	$@@: http下载 
*/

#include "pqhttp.h"


#define PQH_CAST(x, t)			((t)(x))



/* HTTP协议中的HEAD命令 */
static const char * S_hcmd_head = "HEAD %s/%s HTTP/1.1\r\n" \
							  	  "Accept:*/*\r\n" \
							  	  "Host:%s:%d\r\n" \
							  	  "Connection: close\r\n\r\n";


/* HTTP协议中的GET命令 */
static const char * S_hcmd_get = "GET %s/%s HTTP/1.1\r\n" \
								 "Accept:*/*\r\n" \
								 "Host:%s:%d\r\n" \
								 "Connection:Keep-alive\r\n\r\n";

/* HTTP协议中的GET命令 */
static const char * S_hcmd_get_range = "GET %s/%s HTTP/1.1\r\n" \
								       "Accept:*/*\r\n" \
								       "Range: bytes=%d-\r\n" \
								       "Host:%s:%d\r\n" \
								       "Connection:Keep-alive\r\n\r\n";


/*
	$@@ 找字符串函数, GNU的strcasestr由于种种原因，不管是否加_GNU_SOURCE还是__USE_GNU都返回int型
*/
#define PQH_ISCHAR(c)		(((c)>='a'&&(c)<='z')||((c)>='A'&&(c)<='Z'))
static char * p_strcasestr(const char *str1, const char *str2)
{
	int i = 0;
	int j = 0;
	char ch1, ch2;

	while ( (ch1 = *(str1+i)) != '\0' ) {
		j = 0;
		ch2 = *str2;
		while ( ch1 == ch2 || (PQH_ISCHAR(ch1) && PQH_ISCHAR(ch2) && ((ch1&0xdf)==(ch2&&0xdf))) ) {
			++j;
			ch1 = *(str1+i+j);
			ch2 = *(str2+j);
			if ( '\0' == ch2 ) return (char *)(str1+i);
		}
		++i;
	}

	return NULL;
}

static int parse_url(const char *url, char *web, int *port, char *remote_path, char *remote_file)
{
	char ch;
	int start;
	int flag = 0;
	int itr = 0;
    int oitr = 0;
	int last = -1;

	if ( !strncasecmp(url, "http://", 7) ) itr = 7;
	else if ( !strncasecmp(url, "https://", 8) ) itr = 8;	
    
    oitr = itr;
    *port = 0;

    /* 已经找到web, 先判断是否存在':' */
    flag = 0;
    start = itr;
    while ((ch = *(url + itr)) != '\0') {
        if (':' == ch) {
            if (itr > start) {
                memcpy(web, url + start, itr - start);
                web[itr - start] = '\0';
                flag = 1;
            }
            break;
        }
        itr++;
    }

    /* 已经找到web, 接着找port */
    if (flag == 1) {
        start = itr++;      /* 跳过 ':' */
        while ((ch = *(url + itr)) != '/') {
            if ( ch >= '0' && ch <= '9' ) {
				*port = (*port)*10 + (int)(ch-'0');
			} else break;

            itr++;
        }
    } else {
        /*搜寻web*/
        itr = oitr;
        flag = 0;
    	start = itr;
    	while ( (ch = *(url+itr)) != '\0' ) {
    		if ( '/' == ch ) {
    			if ( itr > start ) {
    				memcpy(web, url+start, itr-start);
    				web[itr-start] = '\0';
    				flag = 1;			
    			}	
    			break;
    		}
    		++itr;
    	}
    }

    if ( flag != 1 ) return PQH_ERR_URLINLEGAL;    

    if ( 0 == *port ) *port = PQH_DEF_PORT;	
    
	printf("web: %s, port: %d \r\n", web, *port);
    
	/* 搜寻目录和文件 */
	flag = 0;
	start = itr++;		/* 跳过'/' */
	while ( 1 ) {
		ch = *(url+itr);
		if ( ch == '/' ) last = itr;		/* 记录最后一次出现'/'的位置 */
		else if ( '?' == ch || ':' == ch || '#' == ch || '\0' == ch ) {
			if ( -1 == last ) return PQH_ERR_URLINLEGAL;
			else {
				memcpy(remote_path, url+start, last-start);
				remote_path[last-start] = '\0';
				memcpy(remote_file, url+last+1, itr-last-1);
				remote_file[itr-last-1] = '\0';
				flag = 1; 
				break;
			}
		}
		++itr;
	}

    printf("remote_path: %s, remote_file: %s \r\n", remote_path, remote_file);
    
	if ( flag != 1 ) return PQH_ERR_URLINLEGAL;

	return PQH_OK;
}


#if 0
/*
 	$@@ 解析url，分析出web地址,端口号,下载的资源名称
*/
static int parse_url(const char *url, char *web, int *port, char *remote_path, char *remote_file)
{
	char ch;
	int start;
	int flag = 0;
	int itr = 0;
    char *ptr = NULL;
	int last = -1;

	if ( !strncasecmp(url, "http://", 7) ) itr = 7;
	else if ( !strncasecmp(url, "https://", 8) ) itr = 8;	

	/*搜寻web*/
	flag = 0;
	start = itr;
	while ( (ch = *(url+itr)) != '\0' ) {
		if ( '/' == ch ) {
			if ( itr > start ) {
				memcpy(web, url+start, itr-start);
				web[itr-start] = '\0';
                if ((ptr = strstr(web, ":")) != NULL) {
                    
                }
				flag = 1;			
			}	
			break;
		}
		++itr;
	}

    printf("web: %s \r\n", web);
    
	if ( flag != 1 ) return PQH_ERR_URLINLEGAL;

    /* 从web中搜索是否存在port */
    *port = 0;
    itr2 = 0;
	while ( (ch = web[itr2]) != '\0' ) 
    {
        if ( ch == ':') 
        {
			if ( ch >= '0' && ch <= '9' ) 
            {
				*port = (*port)*10 + (int)(ch-'0');
			}
            else
            {
                break;
            }
        }
        
		++itr2;
	}

    printf("port: %d \r\n", *port);
     
	if ( 0 == *port ) *port = PQH_DEF_PORT;	
    
	/* 搜寻目录和文件 */
	flag = 0;
	start = itr++;		/* 跳过'/' */
	while ( 1 ) {
		ch = *(url+itr);
		if ( ch == '/' ) last = itr;		/* 记录最后一次出现'/'的位置 */
		else if ( '?' == ch || ':' == ch || '#' == ch || '\0' == ch ) {
			if ( -1 == last ) return PQH_ERR_URLINLEGAL;
			else {
				memcpy(remote_path, url+start, last-start);
				remote_path[last-start] = '\0';
				memcpy(remote_file, url+last+1, itr-last-1);
				remote_file[itr-last-1] = '\0';
				flag = 1; 
				break;
			}
		}
		++itr;
	}

    printf("remote_path: %s, remote_file: %s \r\n", remote_path, remote_file);
    
	if ( flag != 1 ) return PQH_ERR_URLINLEGAL;

	return PQH_OK;
}
#endif


/*
	$@@ 解析响应报头
*/
#define PQH_HTTPID			"HTTP/1.1"
#define PQH_CONTENTLEN		"Content-Length:"	
static int parse_response_head(const char *response_head, int n, unsigned int *down_total)
{
	int i = 1;
	int status = 404;
	char *pstatus = p_strcasestr(response_head, PQH_HTTPID);
	char *plen = p_strcasestr(response_head, PQH_CONTENTLEN);
	
	if ( pstatus ) {
		pstatus += strlen(PQH_HTTPID);
		i = 1;	
		while ( *(pstatus+i) == ' ' ) ++i;
		status = atoi(pstatus+i);
	} else
		status = 0;

	if ( plen ) {
		plen += strlen(PQH_CONTENTLEN);
		i = 1;
		while ( *(plen+i) == ' ' ) ++i;
		*down_total = (unsigned int)atoi(plen+i); 
	} else
		*down_total = 0;

	return status;
}


/*
	$@@ 连接主机， 并会根据情况判断是否需要重新创建socket
*/
int connect_host(PQ_Http *http)
{
	if ( -1 == http->sock ) {
		http->sock = socket(AF_INET, SOCK_STREAM, 0);
		if ( -1 == http->sock )
			return PQH_ERR_CRSOCKFAL;
	}

	if ( connect(http->sock, (struct sockaddr *)&http->remote_addr, sizeof(struct sockaddr)) == -1 ) {
		switch ( errno ) {
		case EALREADY:	/* socket为不可阻断且先前的连线操作还未完成 */	
		case EBADF:		/* 参数sockfd 非合法socket处理代码 */
		case EISCONN: 	/* 参数sockfd的socket已是连线状态 */
			closesocket(http->sock);
			http->sock = socket(AF_INET, SOCK_STREAM, 0);
			if ( -1 == http->sock )	
				return PQH_ERR_CRSOCKFAL;
			
			if ( connect(http->sock, (struct sockaddr *)&http->remote_addr, sizeof(struct sockaddr)) == -1 )
				return PQH_ERR_NOTCONNHOST;
            
			break;
		default:
			return PQH_ERR_NOTCONNHOST;
		}	
	}

    unsigned long ul = 1;
    if ( -1 == ioctl(http->sock, FIONBIO, &ul)) {
        printf("set ioctl fail(%s). %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        closesocket(http->sock);
        return PQH_ERR_IOCTLFAL;
    }
    
	return PQH_OK;
}


int send_hcmd(PQ_Http *http, const char *hcmd, int n, int recv_flag)
{
	int ret = PQH_ERR_UNKNOWN;
	int over_flag = 0;
	int per_bytes = 0;
	int already_bytes = 0;
	char ch = 0;
	char response_head[PQH_MAX_RESPHEAD];	
    fd_set rd_set, ex_set;
    time_t now = 0;
    struct timeval tv = {0};
    
	ret = connect_host(http);
	if ( ret != PQH_OK ) return ret;

	/* 发送请求报头 */
	do {
		per_bytes = send(http->sock, hcmd+already_bytes, n-already_bytes, 0);
        if ( per_bytes < 0) {
            if ( errno == EWOULDBLOCK ) {
                per_bytes = 0;
                usleep(20 * 1000);
		    } else {
		        return (already_bytes < n ? PQH_ERR_SNDREQERR : PQH_OK);
		    }
        } else if ( 0 == per_bytes ) {
            return PQH_ERR_DISCONNHOST;
		} else {
            already_bytes += per_bytes; 
		}
	} while ( already_bytes < n );
	
	/* 获取响应报头 */
	n = 0;
    now = time(NULL);
	do {
        FD_ZERO(&rd_set);
        FD_SET(http->sock, &rd_set);
        FD_ZERO(&ex_set);
        FD_SET(http->sock, &ex_set);
        tv.tv_sec = 10;
  	    tv.tv_usec = 0;

        ret = select(http->sock + 1, &rd_set, NULL, &ex_set, &tv);
        if (ret == 0) {
            printf("select time out, return. %s %d\r\n", __FUNCTION__, __LINE__);
            return PQH_ERR_TIMEOUT;
        } else if (ret < 0 || FD_ISSET(http->sock, &ex_set)) {
            printf("select error, return. %s %d\r\n", __FUNCTION__, __LINE__);
            //return PQH_ERR_SELECTFAL;
            break;
        } else {
            ; /* ok, do noting */
        }
        
		per_bytes = recv(http->sock, &ch, 1, 0);        /* recv只是从接收缓冲区拷贝接收数据，缓存区一个字节一个字节读取 */
		if ( per_bytes > 0 ) {
			if ( '\r' == ch || '\n' == ch ) ++over_flag;
			else over_flag = 0;
			response_head[n++] = ch;
		} else if ( 0 == per_bytes ) {
		    printf("%s %d\r\n", __FUNCTION__, __LINE__);
			return PQH_ERR_DISCONNHOST;
		} else { 
			break;
		}

        if (time(NULL) - now > 10 + 1) {
            printf("recv response time out, break. %s %d\r\n", __FUNCTION__, __LINE__);
            return PQH_ERR_TIMEOUT;
        }
        
	} while ( over_flag < recv_flag );          /* 接收到4个head标签后，退出接收 */

    printf("response_head: %s \r\n", response_head);
    
	ret = parse_response_head(response_head, n, &http->file_length);

    printf("ret: %d, file_length: %d \r\n", ret, http->file_length);
    
	switch ( ret ) {
		case 200: return PQH_OK;
        case 206: return PQH_OK;
		case 400: return PQH_ERR_SERVERE400;
		case 401: return PQH_ERR_SERVERE401;
		case 403: return PQH_ERR_SERVERE403;
		case 404: return PQH_ERR_SERVERE404;
		case 500: return PQH_ERR_SERVERE500;
		case 503: return PQH_ERR_SERVERE503;
		default: return PQH_ERR_UNKNOWN;
	}	
}


int create_local_file(PQ_Http *http, const char *local_path, const char *local_file)
{
	int n;
	
	if ( local_path ) {		
		if ( PQH_PATHSPLIT == local_path[strlen(local_path)-1] )
			n = sprintf(http->local_filepath, "%s%s", local_path, local_file?local_file:http->remote_file);
		else
			n = sprintf(http->local_filepath, "%s%c%s", local_path, PQH_PATHSPLIT, local_file?local_file:http->remote_file);	
	} else {
		n = sprintf(http->local_filepath, ".%c%s", PQH_PATHSPLIT, local_file?local_file:http->remote_file);
	}
	http->local_filepath[n] = '\0';

	http->local_stream = fopen(http->local_filepath, "wb+");
	return http->local_stream ? PQH_OK : PQH_ERR_CRLOCALFILEFAL;
}




#define PQH_FREERET(h)	free(h);return ((h) = NULL)
PQ_Http * pqhttp_create()
{
	PQ_Http *http = PQH_CAST(malloc(sizeof(PQ_Http)), PQ_Http *);
	
	if ( http ) {
		memset(http, 0, sizeof(PQ_Http));
		http->sock = -1;		
		return http;
	} else 
		return NULL;	 
}


int pqhttp_load_url(PQ_Http *http, const char *url)
{		
	if ( parse_url(url, http->host, &http->port, http->remote_path, http->remote_file) == PQH_OK ) {
        
		memset(&http->remote_addr, 0, sizeof(http->remote_addr));
		http->remote_addr.sin_family = AF_INET;
		http->remote_addr.sin_port = htons(http->port);
		http->remote_addr.sin_addr.s_addr = inet_addr(http->host);
		
		return PQH_OK;
	} else
		return PQH_ERR_URLINLEGAL;
}


#define PQH_STRCPY(s1, s2, n)	n=strlen(s2);memcpy(s1,s2,n);s1[n]=0
int pqhttp_load_host(PQ_Http *http, const char *ip, int port, const char *remote_path, const char *remote_file)
{
	int len;

	memset(&http->remote_addr, 0, sizeof(http->remote_addr));
	http->remote_addr.sin_family = AF_INET;
	http->remote_addr.sin_port = htons(port);
	http->remote_addr.sin_addr.s_addr = inet_addr(ip);
	
	http->port = port;
	PQH_STRCPY(http->host, ip, len);
	PQH_STRCPY(http->remote_path, remote_path, len);
	PQH_STRCPY(http->remote_file, remote_file, len);
	return PQH_OK;
}


int pqhttp_test(PQ_Http *http)
{
	int n = sprintf(http->request_head, S_hcmd_head, http->remote_path, http->remote_file, http->host, http->port);
	return send_hcmd(http, http->request_head, n, 7);
}


int pqhttp_download(PQ_Http * http, const char *local_path, const char *local_file, int low_range, int height_range, PQH_DOWNEVENT http_event)
{
	int n = 0;
    int ret_select = 0;
	int ret = PQH_ERR_UNKNOWN;
	int per_bytes = 0;
	int flush_bytes = 0;
	unsigned int already_bytes = 0;	
    fd_set rd_set, ex_set;
    time_t now = 0;
    struct timeval tv = {0};
    
    if (low_range > 0) {
        n = sprintf(http->request_head, S_hcmd_get_range, http->remote_path, http->remote_file, low_range, http->host, http->port);
    } else {
    n = sprintf(http->request_head, S_hcmd_get_range, http->remote_path, http->remote_file, low_range, http->host, http->port);
	    //n = sprintf(http->request_head, S_hcmd_get, http->remote_path, http->remote_file, http->host, http->port);
    }
    
	ret = send_hcmd(http, http->request_head, n, 4);
	if ( PQH_OK == ret ) {
		if ( http->file_length <= 0 ) return PQH_ERR_EMPTYRES;
		
		ret = create_local_file(http, local_path, local_file);
		if ( ret != PQH_OK ) return ret;
		
		ret = PQH_OK;
		do {
            FD_ZERO(&rd_set);
            FD_SET(http->sock, &rd_set);
            FD_ZERO(&ex_set);
            FD_SET(http->sock, &ex_set);
            tv.tv_sec = 100;
      	    tv.tv_usec = 0;

            ret_select = select(http->sock + 1, &rd_set, NULL, &ex_set, &tv);
            if (ret_select == 0) {
                printf("select time out, return. %s %d\r\n", __FUNCTION__, __LINE__);
                return PQH_ERR_TIMEOUT;
            } else if (ret_select < 0 || FD_ISSET(http->sock, &ex_set)) {
                printf("select error, return. %s %d\r\n", __FUNCTION__, __LINE__);
                return PQH_ERR_SELECTFAL;
            } else {
                ; /* ok, do noting */
            }
            
			per_bytes = recv(http->sock, http->recv_buf, PQH_DOWN_PERSIZE, 0);
			if ( per_bytes > 0 ) {
				already_bytes += per_bytes;
				if ( already_bytes > http->file_length ) { 	/* 处理最后多了\r\n两个字节 */
					per_bytes -= (already_bytes-http->file_length);
					already_bytes = http->file_length; 
				}
				
				if ( http_event ) 
					if ( !http_event(http->file_length, already_bytes, per_bytes) )
						break;

				fwrite(http->recv_buf, 1, per_bytes, http->local_stream);								
				flush_bytes += per_bytes;
				if ( flush_bytes >= PQH_FLUSH_BLOCK ) {					
					fflush(http->local_stream);
					flush_bytes = 0;
				}
			} else if ( 0 == per_bytes ) {
				ret = PQH_ERR_DISCONNHOST;
				break;
			} else {
				if ( already_bytes < http->file_length ) {
					ret = PQH_ERR_NOCOMPLETERES;
					break;				
				}
			}
		} while ( already_bytes < http->file_length );

		fclose(http->local_stream);
	}

	closesocket(http->sock);

  	return ret;
}

void pqhttp_free(PQ_Http * http)
{
	free(http);
}

/* END */

