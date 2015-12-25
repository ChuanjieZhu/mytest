
#include "mrj_http.h"

static const char * S_pos_head = "POST %s HTTP/1.0\r\n" \
                                 "Accept: */*\r\n" \
                                 "HOST: %s\r\n" \
                                 "Content-Length: %d\r\n" \
                                 "Content-Type: application/x-www-form-urlencoded\r\n\r\n";

static const char * S_get_head = "GET %s HTTP/1.1\r\n" \
								 "Accept:*/*\r\n" \
								 "Host:%s:%d\r\n" \
								 "Connection:Keep-alive\r\n\r\n";

static const char * S_get_head_range = "GET %s HTTP/1.1\r\n" \
								       "Accept:*/*\r\n" \
								       "Range: bytes=%d-\r\n" \
								       "Host:%s:%d\r\n" \
								       "Connection:Keep-alive\r\n\r\n";

struct timeval before = {0, 0};

/*****************************************************************************/

static struct timeval mrjx_tvnow(void)
{
  struct timeval now;
  (void)gettimeofday(&now, NULL);
  return now;
}

static long mrjx_tvdiff(struct timeval newer, struct timeval older)
{
  return (newer.tv_sec - older.tv_sec) * 1000+
    (newer.tv_usec - older.tv_usec)/1000;
}

static long timeleft(struct SessionHandle *data, 
                     struct timeval *beforep,
                     Bool duringconnect)
{
    struct timeval now = mrjx_tvnow();
    long diff = mrjx_tvdiff(now, *beforep);

    diff = data->set.timeout - diff;

    //TRACE("time left: %ld(s) %s %d\r\n", diff / 1000, MDL);
    
    return diff;
}

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

static MRJEcode wselect(int sock, long timeout)
{
    int ret = -1;
    fd_set rd_set, ex_set;
    struct timeval tv = {0};

    FD_ZERO(&rd_set);
    FD_SET(sock, &rd_set);
    FD_ZERO(&ex_set);
    FD_SET(sock, &ex_set);
    tv.tv_sec = (timeout / 1000L);
	tv.tv_usec = 0;

    ret = select(sock + 1, &rd_set, NULL, &ex_set, &tv);
    if (ret == 0) {
        printf("select time out, return. %s %d\r\n", __FUNCTION__, __LINE__);
        return MRJE_TIME_OUT;
    } else if (ret < 0 || FD_ISSET(sock, &ex_set)) {
        printf("select error, return. %s %d\r\n", __FUNCTION__, __LINE__);
        return MRJE_SELECTFAL;
    } else {
        if (FD_ISSET(sock, &rd_set) > 0) {
            return MRJE_OK;
        }
    }

    return MRJE_SELECTFAL;
}

static MRJEcode show_req_head(const char *head, size_t len)
{
    if (!head) 
        return MRJE_BAD_FUNCTION_ARGUMENT;

    char *headp = (char *)malloc(len + 1);
    if (!headp) {
        TRACE("malloc fail to print response head info. %s %d\r\n", MDL); 
        return MRJE_OUT_OF_MEMORY;
    }

    memset(headp, 0, len + 1);
    strncpy(headp, head, len);
    
    /* Print response header */
    char *pptr = NULL;  
    char *token = strtok_r(headp, "\r\n", &pptr);
    while (token != NULL) {
        TRACE("* %s \r\n", token);
        token = strtok_r(NULL, "\r\n", &pptr);
    }

    TRACE("\r\n");
        
    MRJ_safefree(headp);

    return MRJE_OK;
}

static MRJEcode show_resp_head(char *head, size_t len)
{   
    if (!head)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    char *headp = (char *)malloc(len + 1);
    if (!headp) {
        TRACE("malloc fail to print response head info. %s %d\r\n", MDL); 
        return MRJE_OUT_OF_MEMORY;
    }

    memset(headp, 0, len + 1);
    strncpy(headp, head, len);
    
    /* Print response header */
    char *pptr = NULL;  
    char *token = strtok_r(headp, "\r\n", &pptr);
    while (token != NULL) {
        TRACE("> %s \r\n", token);
        token = strtok_r(NULL, "\r\n", &pptr);
    }

    TRACE("\r\n");
    
    MRJ_safefree(headp);

    return MRJE_OK;
}


static MRJEcode parse_response(struct SessionHandle *data)
{
	int i = 1;
	MRJEcode status = MRJE_UNKNOWN;
	char *pstatus = p_strcasestr(data->response_recv, MRJ_HTTPID);
	char *plen = p_strcasestr(data->response_recv, MRJ_CONTENTLEN);
    char *peoh = p_strcasestr(data->response_recv, MRJ_ENDOFHEAD);

    data->info.httpcode = 404;    
    
	if ( pstatus ) {
		pstatus += strlen(MRJ_HTTPID);
		i = 1;	
		while ( *(pstatus+i) == ' ' ) ++i;
		data->info.httpcode = atoi(pstatus+i);
	} else
		data->info.httpcode = 0;

	if ( plen ) {
		plen += strlen(MRJ_CONTENTLEN);
		i = 1;
		while ( *(plen+i) == ' ' ) ++i;
		data->info.content_len = (unsigned int)atoi(plen+i); 
	} else
		data->info.content_len = 0;

    if ( peoh ) 
    {
        peoh += strlen(MRJ_ENDOFHEAD);
        size_t len = 0;
        if (data->info.content_len)   
            len = data->info.content_len + 1;
        else 
            len = strlen(peoh) + 1;

        //TRACE("len: %d %s %d\r\n", len, MDL);

        /* Request malloc recv buffer */
        if (data->set.httpreq == HTTPREQ_POST) {
            data->set.recv_data = malloc(len);
            if (!data->set.recv_data) {
                TRACE("malloc recv buffer fail. %s %d\r\n", MDL);
                return MRJE_OUT_OF_MEMORY;
            } else {
                memset((char *)data->set.recv_data, 0, len);
                strncpy((char *)data->set.recv_data, peoh, len - 1);
                data->set.recv_data_len = len - 1;
            }
        }
        
        status = MRJE_OK;
    }

	return status;
}


static MRJEcode connect_nnob(mrj_socket_t sockfd, const struct sockaddr *saptr, socklen_t salen, long nsec)
{
    int flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;

    flags = fcntl(sockfd, F_GETFL, 0);
    if ((n = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK)) == -1) {
        TRACE("Fcntl fail(%s). %s %d\r\n", STR_ERRNO, MDL);
        return MRJE_FCNTL_ERROR;
    }

    error = 0;
    if ((n = connect(sockfd, saptr, salen)) < 0) {
        if (errno != EINPROGRESS)
            return MRJE_NOTCONNHOST;
    }

    if (n == 0) {
        goto done;
    }

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;

    n = select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL);
    if (n < 0) {
        TRACE("Select error(%s). %s %d\r\n", STR_ERRNO, MDL);
        return MRJE_SELECT_FAIL;
    } else if (n == 0) {
        errno = ETIMEDOUT;
        return MRJE_SELECT_TIME_OUT;
    } else {
        ;
    }

    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
            return MRJE_GET_SOCKOPT_ERROR;
        }
    } else {
        return MRJE_SELECT_FAIL;
    }

done:
    if (fcntl(sockfd, F_SETFL, flags) == -1) {
        TRACE("Fcntl fail(%s). %s %d\r\n", STR_ERRNO, MDL);
        return MRJE_FCNTL_ERROR;
    }

    if (error) {
        errno = error;
        return MRJE_CONNECT_NONB_ERROR;
    }

    return MRJE_OK;
}

static MRJEcode connect_host(struct SessionHandle *data)
{
	if ( -1 == data->sock ) {
		data->sock = socket(AF_INET, SOCK_STREAM, 0);
		if ( -1 == data->sock )
			return MRJE_CRSOCKFAL;
	}

    long nsec = data->set.connecttimeout / 1000L;
    MRJEcode res = connect_nnob(data->sock, (const struct sockaddr *)&data->remote_addr, sizeof(struct sockaddr), nsec);
    if (res != MRJE_OK) return res;
    
#if 0
	if ( connect(data->sock, (struct sockaddr *)&data->remote_addr, sizeof(struct sockaddr)) == -1 ) {
		switch ( errno ) {
		case EALREADY:	/* socket为不可阻断且先前的连线操作还未完成 */	
		case EBADF:		/* 参数sockfd 非合法socket处理代码 */
		case EISCONN: 	/* 参数sockfd的socket已是连线状态 */
			closesocket(data->sock);
			data->sock = socket(AF_INET, SOCK_STREAM, 0);
			if ( -1 == data->sock )	
				return MRJE_CRSOCKFAL;
			if ( connect(data->sock, (struct sockaddr *)&data->remote_addr, sizeof(struct sockaddr)) == -1 )
				return MRJE_NOTCONNHOST;
			break;
		default:
			return MRJE_NOTCONNHOST;
		}	
	}


    unsigned long ul = 1;
    if ( -1 == ioctl(data->sock, FIONBIO, &ul)) {
        TRACE("set ioctl fail(%s). %s %d\r\n", strerror(errno), MDL);
        closesocket(data->sock);
        return MRJE_IOCTLFAL;
    }
#endif
    
	return MRJE_OK;
}

static MRJEcode send_data(struct SessionHandle *data, const char *hcmd, size_t n)
{
    size_t per_bytes = 0;
    size_t already_bytes = 0;
    
	do {
		per_bytes = send(data->sock, hcmd + already_bytes, n - already_bytes, 0);
        if ( per_bytes < 0) {
            if ( errno == EWOULDBLOCK ) {
                per_bytes = 0;
                usleep(20 * 1000);
		    } else return (already_bytes < n ? MRJE_SNDREQERR : MRJE_OK);
        } else if ( 0 == per_bytes ) {
            return MRJE_DISCONNHOST;
		} else {
            already_bytes += per_bytes; 
		}

        if (timeleft(data, &before, TRUE) <= 0)
            return MRJE_TIME_OUT;
        
	} while ( already_bytes < n );

    return MRJE_OK;
}

static MRJEcode recv_data(struct SessionHandle *data, int recv_flag)
{   
    char ch = 0;
    MRJEcode res = MRJE_UNKNOWN;
    size_t per_bytes = 0;
    int over_flag = 0;
    
    do {
        res = wselect(data->sock, data->set.timeout);
        if ( res != MRJE_OK ) { 
            TRACE("select timeout. %s %d\r\n", MDL);
            return res;
        }
        
		per_bytes = recv(data->sock, &ch, 1, 0);        /* recv只是从接收缓冲区拷贝接收数据，缓存区一个字节一个字节读取 */
		if ( per_bytes > 0 ) {
			if ( '\r' == ch || '\n' == ch ) ++over_flag;
			else over_flag = 0;
            
            if (data->response_recv_len >= sizeof(data->response_recv)) {
                TRACE("response recv buffer is lower. %s %d\r\n", MDL);
                return MRJE_OUT_OF_MEMORY;
            }
            
			data->response_recv[data->response_recv_len++] = ch;    
		} else if ( 0 == per_bytes ) {
		    TRACE("server close, over_flag:%d, recv_flag: %d  %s %d\r\n", over_flag, recv_flag, MDL);
            break;
		} else { 
		    return ( over_flag < recv_flag ? MRJE_NOCOMPLETERES : MRJE_OK); 
		}

        if ( timeleft(data, &before, TRUE) <= 0 ) {
            return MRJE_TIME_OUT;
        }
    } while ( over_flag < recv_flag );
    
    return MRJE_OK;
}


static MRJEcode recv_data_dl(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;
    size_t per_bytes = 0;
    size_t already_bytes = 0;
    char recv_buff[MRJ_MAX_BUFSIZE] = {0};
    
    do {
        res = wselect(data->sock, data->set.timeout);
        if (res != MRJE_OK) return res;

        memset(recv_buff, 0, sizeof(recv_buff));
        per_bytes = recv(data->sock, recv_buff, MRJ_DOWN_PERSIZE, 0);
        if (per_bytes > 0) {
            already_bytes += per_bytes;
            if (already_bytes > data->info.content_len) {
                per_bytes -= (already_bytes - data->info.content_len);
		        already_bytes = data->info.content_len;
            }

            if (data->set.fwrite_func)
                data->set.fwrite_func(recv_buff, 1, per_bytes, data->set.event_args);
            
        } else if (0 == per_bytes) {
            if ( already_bytes < data->info.content_len ) {
                res = MRJE_DISCONNHOST;
	            break;
            }
        } else {
            if ( already_bytes < data->info.content_len ) {
		        res = MRJE_NOCOMPLETERES;
		        break;				
	        }
        }

        if (timeleft(data, &before, TRUE) <= 0) {
            res = MRJE_TIME_OUT;
            break;
        }
    } while ( already_bytes < data->info.content_len );

    if (data->set.verbose)
        TRACE("already_bytes: %d, data->info.content_len: %d %s %d\r\n", 
               already_bytes, data->info.content_len, MDL);

    return res;
}

static MRJEcode send_hcmd(struct SessionHandle *data, const char *hcmd, size_t n)
{
	MRJEcode res = MRJE_UNKNOWN;
    
	res = connect_host(data);
	if ( res != MRJE_OK ) {
        TRACE("connect host error. %s %d\r\n", MDL);
        return res;
	}

    if (data->set.verbose)
        show_req_head(hcmd, n);
    
	res = send_data(data, hcmd, n);
    if ( res != MRJE_OK ) {
        TRACE("send request data error. %s %d\r\n", MDL);
        return MRJE_SEND_ERROR;
    }

    data->response_recv_len = 0;
    res = recv_data(data, 4);
    if (res != MRJE_OK) {
        TRACE("recv response data error. %s %d\r\n", MDL);
        return MRJE_RECV_ERROR;
    }

    /* HUOQU DATA */
    if (data->set.httpreq == HTTPREQ_POST) {
        res = recv_data(data, 2);
        if (res != MRJE_OK) {
            TRACE("recv response data error. %s %d\r\n", MDL);
            return MRJE_RECV_ERROR;
        }
    }
        
    if (data->set.verbose)
        show_resp_head(data->response_recv, data->response_recv_len);
    
	res = parse_response(data);
    if (res != MRJE_OK) {
        TRACE("parse response head error. %s %d\r\n", MDL);
        return res;
    }
    
	switch ( data->info.httpcode ) {
		case 200: return MRJE_OK;
        case 206: return MRJE_OK;
		case 400: return MRJE_HTTP_SERVERE400;
		case 401: return MRJE_HTTP_SERVERE401;
		case 403: return MRJE_HTTP_SERVERE403;
		case 404: return MRJE_HTTP_SERVERE404;
		case 500: return MRJE_HTTP_SERVERE500;
		case 503: return MRJE_HTTP_SERVERE503;
		default: return MRJE_UNKNOWN;
	}

    return res;
}


/******************************************************************************/
static MRJEcode parse_url(struct SessionHandle *data)
{
	char ch;
	int start;
	int flag = 0;
	int itr = 0;
    int oitr = 0;

	if ( !strncasecmp(data->set.url, "http://", 7) ) itr = 7;
	else if ( !strncasecmp(data->set.url, "https://", 8) ) itr = 8;	
    
    oitr = itr;
    data->port = 0;

    /* 已经找到web, 先判断是否存在':' */

    //TRACE("URL: %s %s %d\r\n", data->set.url, MDL);
    
    flag = 0;
    start = itr;
    while ((ch = *(data->set.url + itr)) != '\0') {
        if (':' == ch) {
            if (itr > start) {
                memcpy(data->host, data->set.url + start, itr - start);
                data->host[itr - start] = '\0';
                flag = 1;
            }
            break;
        }
        itr++;
    }

    /* 已经找到web, 接着找port */
    if (flag == 1) {
        start = itr++;      /* 跳过 ':' */
        while ((ch = *(data->set.url + itr)) != '/') {
            if ( ch >= '0' && ch <= '9' ) {
				data->port = (data->port)*10 + (int)(ch-'0');
			} else break;

            itr++;
        }
    } else {
        /*搜寻web*/
        itr = oitr;
        flag = 0;
    	start = itr;
    	while ( (ch = *(data->set.url+itr)) != '\0' ) {
    		if ( '/' == ch ) {
    			if ( itr > start ) {
    				memcpy(data->host, data->set.url + start, itr - start);
                    data->host[itr - start] = '\0';
    				flag = 1;			
    			}	
    			break;
    		}
    		++itr;
    	}
    }

    //TRACE("web: %s, port: %d %s %d\r\n", data->host, data->port, MDL);
    
    if ( flag != 1 ) return MRJE_URLINLEGAL;    
    
    if ( 0 == data->port ) data->port = MRJ_DEF_PORT;	
	//printf("web: %s, port: %d \r\n", data->host, data->port);

    flag = 0;
    start = itr;
    while ((ch = *(data->set.url + itr)) != '\0') {
        itr++;
    }

    memcpy(data->remote_path, data->set.url + start, itr - start);
    data->remote_path[itr - start] = '\0';
    flag = 1;

    //printf("remote_path: %s \r\n", data->remote_path);
    
#if 0    
    int last = -1;
    /* 搜寻目录和文件 */
    if (remote_path) {
    	flag = 0;
    	start = itr++;		/* 跳过'/' */
    	while ( 1 ) {
    		ch = *(url+itr);
    		if ( ch == '/' ) last = itr;		/* 记录最后一次出现'/'的位置 */
    		else if ( '?' == ch || ':' == ch || '#' == ch || '\0' == ch ) {
    			if ( -1 == last ) return MRJE_URLINLEGAL;
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
    }
#endif

    
	if ( flag != 1 ) return MRJE_URLINLEGAL;

	return MRJE_OK;
}


static MRJEcode setstropt(char **charp, char *s)
{
    MRJ_safefree(*charp);

    if(s) {
        s = strdup(s);

        if(!s)
            return MRJE_OUT_OF_MEMORY;

        *charp = s;
    }
    
    return MRJE_OK;
}

static MRJEcode getinfo_long(struct SessionHandle *data, MRJINFO info,
                             long *param_longp)
{
  mrj_socket_t sockfd;

  union {
    unsigned long *to_ulong;
    long          *to_long;
  } lptr;

  switch(info) {
  case MRJINFO_RESPONSE_CODE:
    *param_longp = data->info.httpcode;
    break;

  default:
    return MRJE_BAD_FUNCTION_ARGUMENT;
  }
  return MRJE_OK;
}


static MRJEcode http_setopt(struct SessionHandle *data, MRJHTTPoption option,
                     va_list param)
{
    char *argptr;
    MRJEcode result = MRJE_OK;
    long arg;

    switch (option) {
    case MRJOPT_VERBOSE:
        data->set.verbose = (0 != va_arg(param, long)) ? TRUE : FALSE;
        break;
        
    case MRJOPT_WRITEDATA:
        data->set.event_args = va_arg(param, void *);
        break;
        
    case MRJOPT_URL:
        result = setstropt(&data->set.url, va_arg(param, char *));      /* set url */
        break;
        
    case MRJOPT_WRITEFUNCTION:
        data->set.fwrite_func = va_arg(param, write_callback);
        if (!data->set.fwrite_func) {
            data->set.is_fwrite_set = 0;
            //data->fwrite_func = (write_callback)fwrite;
        } else data->set.is_fwrite_set = 1;
        break;
        
    case MRJOPT_POSTFIELDS:
        data->set.send_data = va_arg(param, void *);        /* use static param */
        data->set.send_data_len = strlen((char *)data->set.send_data);
        data->set.httpreq = HTTPREQ_POST;
        break;
        
    case MRJOPT_POST:
        if(va_arg(param, long)) {
            data->set.httpreq = HTTPREQ_POST;
        }
        else
            data->set.httpreq = HTTPREQ_GET;
        break;
        
    case MRJOPT_GET:
        data->set.httpreq = HTTPREQ_GET;
        break;

    case MRJOPT_TIMEOUT:
        data->set.timeout = va_arg(param, long) * 1000L;
        break;
            
    case MRJOPT_CONNECTTIMEOUT:
        data->set.connecttimeout = va_arg(param, long) * 1000L;
        break;

    case MRJOPT_HTTP_VERSION:
        arg = va_arg(param, long);
#ifndef USE_NGHTTP2
        if(arg == MRJ_HTTP_VERSION_2_0)
            return MRJE_UNSUPPORTED_PROTOCOL;
#endif
        data->set.httpversion = arg;
        break;
    case MRJOPT_RESUME_FROM:
        data->set.set_resume_from = va_arg(param, long);
        break;
        
    default:
        result = MRJE_UNKNOWN_OPTION;
        break;
    }

    return result;
}

static MRJEcode http_load_url(struct SessionHandle *data)
{
    MRJEcode res = MRJE_OK;

    res = parse_url(data);

    if (res != MRJE_OK)
        return res;
    
    memset(&data->remote_addr, 0, sizeof(data->remote_addr));
	data->remote_addr.sin_family = AF_INET;
	data->remote_addr.sin_port = htons(data->port);
	data->remote_addr.sin_addr.s_addr = inet_addr(data->host);
		
    return MRJE_OK;
}

static MRJEcode http_create_head(struct SessionHandle *data)
{
    MRJEcode res = MRJE_UNKNOWN;

    if (!data)
        return res;

    int total = 0;
    int n = 0;
    char req[256] = {0};
    char host[128] = {0};
    char accept[128] = {0};
    char content_len[128] = {0};
    char content_type[128] = {0};

    if (data->set.httpreq == HTTPREQ_POST) {
        n = snprintf(req, sizeof(req) - 1, "POST %s HTTP/1.0\r\n", data->remote_path);
        total += n;
        
        n = snprintf(host, sizeof(host) - 1, "HOST: %s\r\n", data->host);
        total += n;

        n = snprintf(accept, sizeof(accept) - 1, "Accept: */*\r\n");
        total += n;

        n = snprintf(content_len, sizeof(content_len) - 1, "Content-Length: %d\r\n", data->set.send_data_len);
        total += n;

        n = snprintf(content_type, sizeof(content_type) - 1, 
                    "Content-Type: application/x-www-form-urlencoded\r\n\r\n"); /* 注意: 最后这里要空一行 */
        total += n;
    } else if (data->set.httpreq == HTTPREQ_GET) {
        
        
    } else {
        TRACE("unknow http request type. %s %d\r\n", MDL);
    }
        
    return res;
}


static MRJEcode http_transfer(struct SessionHandle *data)
{   
    size_t total_len = 0;
    size_t data_len = 0;
    MRJEcode res = MRJE_OK;
    
    if (data->set.httpreq == HTTPREQ_POST) {
        data_len = data->set.send_data_len;
        total_len = snprintf(data->request_send, 
                             sizeof(data->request_send) - 1, 
                             S_pos_head, 
                             data->remote_path, 
                             data->host, 
                             data_len);
    } else if (data->set.httpreq == HTTPREQ_GET) {
        if (data->set.set_resume_from == 0) {
            total_len = snprintf(data->request_send, 
                                 sizeof(data->request_send) - 1,
                                 S_get_head, 
                                 data->remote_path,
                                 data->host, 
                                 data->port);
        } else { 
            total_len = snprintf(data->request_send, 
                                 sizeof(data->request_send) - 1,
                                 S_get_head_range, 
                                 data->remote_path, 
                                 data->set.set_resume_from, 
                                 data->host,
                                 data->port);
        }
    } else {
        return MRJE_UNSUPPORTED_PROTOCOL;
    }

    total_len = total_len + data_len + 1;

    char *send = (char *)malloc(total_len);
    if (!send) {
        TRACE("malloc error. %s %d\r\n", MDL);
        return MRJE_OUT_OF_MEMORY;
    }

    data->request_send_len = total_len;
    
    memset(send, 0, total_len);
    if (data->request_send[0] != '\0')
        strcpy(send, data->request_send);
    if (data->set.send_data)
        strcat(send, (char *)data->set.send_data);

    before = mrjx_tvnow();
        
    res = send_hcmd(data, send, total_len);
    if (data->set.verbose)
        TRACE("send_hcmd ret: %d(%s) %s %d\r\n", res, mrjhttp_strerror(res), MDL);

    MRJ_safefree(send);
    
    if (res == MRJE_OK) {
        if (data->set.httpreq == HTTPREQ_GET) {
            if (data->info.httpcode == 206 ||
                data->info.httpcode == 200) {
                if (data->info.content_len <= 0) 
                    return MRJE_EMPTYRES;
                res = recv_data_dl(data);
            } else return MRJE_UNKNOWN;
            
        } else if (data->set.httpreq == HTTPREQ_POST) {
            if (data->info.httpcode == 200) {
                if (data->set.fwrite_func) {
                    size_t data_size = 0;
                    data_size = data->set.fwrite_func(data->set.recv_data, 1, 
                                          data->set.recv_data_len, 
                                          data->set.event_args);
                    if (data_size != data->set.recv_data_len)
                        return MRJE_ABORTED_BY_CALLBACK;
                }
            } else return MRJE_UNKNOWN;
            
        } else return MRJE_UNSUPPORTED_PROTOCOL;
    }
    
    return res;
}


static MRJEcode http_perform(struct SessionHandle *data, bool events)
{
    MRJEcode res = MRJE_OK;

    res = http_load_url(data);
    if (res != MRJE_OK) {
        TRACE("Http load url error. %s %d\r\n", MDL);
        return res;
    }

    res = http_transfer(data);
    if (res != MRJE_OK) {
        TRACE("Http transfer error. %s %d\r\n", MDL);
        return res;
    } 
    
    return res;
}

static MRJEcode http_getinfo(struct SessionHandle *data, MRJINFO info, ...)
{
    va_list arg;
    long *param_longp = NULL;
    double *param_doublep = NULL;
    char **param_charp = NULL;
    int type;
    MRJEcode ret = MRJE_BAD_FUNCTION_ARGUMENT;

    if (!data)
        return ret;

    va_start(arg, info);

    type = MRJINFO_TYPEMASK & (int)info;
    switch(type) {
    case MRJINFO_STRING:
        param_charp = va_arg(arg, char **);
        //if(NULL != param_charp)
        //    ret = getinfo_char(data, info, param_charp);
        break;
      case MRJINFO_LONG:
        param_longp = va_arg(arg, long *);
        if(NULL != param_longp)
          ret = getinfo_long(data, info, param_longp);
        break;
      case MRJINFO_DOUBLE:
        param_doublep = va_arg(arg, double *);
        //if(NULL != param_doublep)
          //ret = getinfo_double(data, info, param_doublep);
        break;
      case MRJINFO_SLIST:
        //param_slistp = va_arg(arg, struct curl_slist **);
        //if(NULL != param_slistp)
        //  ret = getinfo_slist(data, info, param_slistp);
        break;
      default:
        break;
      }

      va_end(arg);
      return ret;
}

static MRJHTTP *http_init(void)
{
    struct SessionHandle *data;

    data = (struct SessionHandle *)malloc(sizeof(struct SessionHandle));
    if (!data) {
        TRACE("malloc SessionHandle fail. %s %d\r\n", MDL);
        return NULL;
    }

    memset(data, 0, sizeof(struct SessionHandle));
    data->sock = -1;
    data->set.httpreq = HTTPREQ_GET;

    return data;   
}


static MRJEcode http_close(struct SessionHandle *data)
{
    if (data->sock != -1) {
        close(data->sock);
        data->sock = -1;
    }

    MRJ_safefree(data->set.url); /* strdup */
    MRJ_safefree(data->set.recv_data);

    free(data);
    
    return MRJE_OK;
}

/******************************************************************************/

MRJHTTP *mrjhttp_init(void)
{
    return http_init();
}

void mrjhttp_cleanup(MRJHTTP *handle)
{
    struct SessionHandle *data = (struct SessionHandle *)handle;

    if (!data)
        return;

    http_close(data);
}


MRJEcode mrjhttp_setopt(MRJHTTP *http,  MRJHTTPoption tag, ...) 
{
    va_list arg;
    struct SessionHandle *data = (struct SessionHandle *)http;
    MRJEcode res;

    if (!http)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    va_start(arg, tag);

    res = http_setopt(data, tag, arg);

    va_end(arg);

    return res;
}


MRJEcode mrjhttp_perform(MRJHTTP *handle)
{
    return http_perform((struct SessionHandle *)handle, FALSE);
}


MRJEcode mrjhttp_getinfo(MRJHTTP *mrjhttp, MRJINFO info, ...)
{
  va_list arg;
  void *paramp;
  MRJEcode ret;
  struct SessionHandle *data = (struct SessionHandle *)mrjhttp;

  va_start(arg, info);
  paramp = va_arg(arg, void *);

  ret = http_getinfo(data, info, paramp);

  va_end(arg);
  return ret;
}


const char *
mrjhttp_strerror(MRJEcode error)
{
    switch (error) {
    case MRJE_OK:
        return "No error";

    case MRJE_UNSUPPORTED_PROTOCOL:
        return "Unsupported protocol";

    case MRJE_IOCTLFAL:
        return "Ioctl opertion fail";

    case MRJE_NOTCONNHOST:
        return "Not connect the host";
        
    case MRJE_FAILED_INIT:
        return "Failed initialization";

    case MRJE_WRITE_ERROR:
        return "Failed writing received data to disk/application";

    case MRJE_UPLOAD_FAILED:
        return "Upload failed (at start/before it took off)";

    case MRJE_READ_ERROR:
        return "Failed to open/read local data from file/application";

    case MRJE_OUT_OF_MEMORY:
        return "Out of memory";

    case MRJE_OPERATION_TIMEDOUT:
        return "Timeout was reached";

    case MRJE_FUNCTION_NOT_FOUND:
        return "A required function in the library was not found";

    case MRJE_ABORTED_BY_CALLBACK:
        return "Operation was aborted by an application callback";

    case MRJE_BAD_FUNCTION_ARGUMENT:
        return "A libcurl function was given a bad argument";

    case MRJE_INTERFACE_FAILED:
        return "Failed binding local connection end";

    case MRJE_UNKNOWN_OPTION:
        return "An unknown option was passed in to libcurl";

    case MRJE_SEND_ERROR:
        return "Failed sending data to the peer";

    case MRJE_RECV_ERROR:
        return "Failure when receiving data from the peer";

    case MRJE_AGAIN:
        return "Socket not ready for send/recv";

    case MRJE_NO_CONNECTION_AVAILABLE:
        return "The max connection limit is reached";

    default:
        return "Unknown error";
    }

    return "Unknown error";
}


