
#ifndef _DVS_HTTP_H_
#define _DVS_HTTP_H_

#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int Bool;
typedef int	mrj_socket_t;
typedef void MRJHTTP;

#ifndef TRACE
#define STR_ERRNO   (strerror(errno))
#define MDL __FILE__, __LINE__
#define TRACE printf
#endif

#ifndef WHILE_FALSE
#define WHILE_FALSE  while(0)
#define MRJ_safefree(ptr) \
  do {if((ptr)) {free((ptr)); (ptr) = NULL;}} WHILE_FALSE
#endif

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

#define closesocket(s) close(s)

#define DEFAULT_CONNECT_TIMEOUT 300000 /* milliseconds == five minutes */

#define MRJ_DEF_PORT			80			/* 默认的HTTP连接端口号 */
#define MRJ_MAX_PATH				256			/* 默认最大的URL路径 */
#define MRJ_MAX_REQHEAD			1024
#define MRJ_MAX_RESPHEAD		10 * 1024
#define MRJ_MAX_HOST			128
#define MRJ_MAX_FILETYPE		64
#define MRJ_MAX_BUFSIZE         2048
#define MRJ_DOWN_PERSIZE		1440
#define MRJ_FLUSH_BLOCK			2048
#define MRJ_MAX_URL             1024

#define MRJ_HTTPID			"HTTP/1.1"
#define MRJ_CONTENTLEN		"Content-Length:"
#define MRJ_ENDOFHEAD       "\r\n\r\n"

/* long may be 32 or 64 bits, but we should never depend on anything else
   but 32 */
#define MRJOPTTYPE_LONG          0
#define MRJOPTTYPE_OBJECTPOINT   10000
#define MRJOPTTYPE_FUNCTIONPOINT 20000
#define MRJOPTTYPE_OFF_T         30000

/* name is uppercase MRJOPT_<name>,
   type is one of the defined MRJOPTTYPE_<type>
   number is unique identifier */
#ifdef MINIT
#undef MINIT
#endif

#ifdef MRJ_ISOCPP
#define MINIT(na,t,nu) MRJOPT_ ## na = MRJOPTTYPE_ ## t + nu
#else
/* The macro "##" is ISO C, we assume pre-ISO C doesn't support it. */
#define LONG          MRJOPTTYPE_LONG
#define OBJECTPOINT   MRJOPTTYPE_OBJECTPOINT
#define FUNCTIONPOINT MRJOPTTYPE_FUNCTIONPOINT
#define OFF_T         MRJOPTTYPE_OFF_T
#define MINIT(name,type,number) MRJOPT_/**/name = type + number
#endif

typedef enum {
    MINIT(WRITEDATA, OBJECTPOINT, 1),
    MINIT(URL, OBJECTPOINT, 2),
    MINIT(WRITEFUNCTION, FUNCTIONPOINT, 3),
    /* POST static input fields. */
    MINIT(POSTFIELDS, OBJECTPOINT, 4),
    MINIT(POST, LONG, 5),          /* HTTP PUT */
    MINIT(GET, LONG, 6),          /* HTTP PUT */
    MINIT(VERBOSE, LONG, 7),
    MINIT(HTTP_VERSION, LONG, 8),
    MINIT(TIMEOUT, LONG, 9),
    MINIT(CONNECTTIMEOUT, LONG, 10),
    MINIT(RESUME_FROM, LONG, 11)
} MRJHTTPoption;


typedef size_t (*write_callback)(void *buffer,
                                 size_t size,
                                 size_t nitems,
                                 void *outstream);

enum {
  MRJ_HTTP_VERSION_NONE, /* setting this means we don't care, and that we'd
                             like the library to choose the best possible
                             for us! */
  MRJ_HTTP_VERSION_1_0,  /* please use HTTP 1.0 in the request */
  MRJ_HTTP_VERSION_1_1,  /* please use HTTP 1.1 in the request */
  MRJ_HTTP_VERSION_2_0,  /* please use HTTP 2.0 in the request */

  MRJ_HTTP_VERSION_LAST /* *ILLEGAL* http version */
};

typedef enum {
  HTTPREQ_NONE, /* first in list */
  HTTPREQ_GET,
  HTTPREQ_POST,
  HTTPREQ_POST_FORM, /* we make a difference internally */
  HTTPREQ_PUT,
  HTTPREQ_HEAD,
  HTTPREQ_CUSTOM,
  HTTPREQ_LAST /* last in list */
} MRJ_HttpReq;


#define MRJINFO_STRING   0x100000
#define MRJINFO_LONG     0x200000
#define MRJINFO_DOUBLE   0x300000
#define MRJINFO_SLIST    0x400000
#define MRJINFO_MASK     0x0fffff
#define MRJINFO_TYPEMASK 0xf00000

typedef enum {
  MRJINFO_NONE, /* first, never use this */
  MRJINFO_EFFECTIVE_URL    = MRJINFO_STRING + 1,
  MRJINFO_RESPONSE_CODE    = MRJINFO_LONG   + 2,
  MRJINFO_TOTAL_TIME       = MRJINFO_DOUBLE + 3,
  MRJINFO_NAMELOOKUP_TIME  = MRJINFO_DOUBLE + 4,
  MRJINFO_CONNECT_TIME     = MRJINFO_DOUBLE + 5,
  MRJINFO_PRETRANSFER_TIME = MRJINFO_DOUBLE + 6,
  MRJINFO_SIZE_UPLOAD      = MRJINFO_DOUBLE + 7,
  MRJINFO_SIZE_DOWNLOAD    = MRJINFO_DOUBLE + 8,
  MRJINFO_SPEED_DOWNLOAD   = MRJINFO_DOUBLE + 9,
  MRJINFO_SPEED_UPLOAD     = MRJINFO_DOUBLE + 10,
  MRJINFO_HEADER_SIZE      = MRJINFO_LONG   + 11,
  MRJINFO_REQUEST_SIZE     = MRJINFO_LONG   + 12,
  MRJINFO_SSL_VERIFYRESULT = MRJINFO_LONG   + 13,
  MRJINFO_FILETIME         = MRJINFO_LONG   + 14,
  MRJINFO_CONTENT_LENGTH_DOWNLOAD   = MRJINFO_DOUBLE + 15,
  MRJINFO_CONTENT_LENGTH_UPLOAD     = MRJINFO_DOUBLE + 16,
  MRJINFO_STARTTRANSFER_TIME = MRJINFO_DOUBLE + 17,
  MRJINFO_CONTENT_TYPE     = MRJINFO_STRING + 18,
  MRJINFO_REDIRECT_TIME    = MRJINFO_DOUBLE + 19,
  MRJINFO_REDIRECT_COUNT   = MRJINFO_LONG   + 20,
  MRJINFO_PRIVATE          = MRJINFO_STRING + 21,
  MRJINFO_HTTP_CONNECTCODE = MRJINFO_LONG   + 22,
  MRJINFO_HTTPAUTH_AVAIL   = MRJINFO_LONG   + 23,
  MRJINFO_PROXYAUTH_AVAIL  = MRJINFO_LONG   + 24,
  MRJINFO_OS_ERRNO         = MRJINFO_LONG   + 25,
  MRJINFO_NUM_CONNECTS     = MRJINFO_LONG   + 26,
  MRJINFO_SSL_ENGINES      = MRJINFO_SLIST  + 27,
  MRJINFO_COOKIELIST       = MRJINFO_SLIST  + 28,
  MRJINFO_LASTSOCKET       = MRJINFO_LONG   + 29,
  MRJINFO_FTP_ENTRY_PATH   = MRJINFO_STRING + 30,
  MRJINFO_REDIRECT_URL     = MRJINFO_STRING + 31,
  MRJINFO_PRIMARY_IP       = MRJINFO_STRING + 32,
  MRJINFO_APPCONNECT_TIME  = MRJINFO_DOUBLE + 33,
  MRJINFO_CERTINFO         = MRJINFO_SLIST  + 34,
  MRJINFO_CONDITION_UNMET  = MRJINFO_LONG   + 35,
  MRJINFO_RTSP_SESSION_ID  = MRJINFO_STRING + 36,
  MRJINFO_RTSP_CLIENT_CSEQ = MRJINFO_LONG   + 37,
  MRJINFO_RTSP_SERVER_CSEQ = MRJINFO_LONG   + 38,
  MRJINFO_RTSP_CSEQ_RECV   = MRJINFO_LONG   + 39,
  MRJINFO_PRIMARY_PORT     = MRJINFO_LONG   + 40,
  MRJINFO_LOCAL_IP         = MRJINFO_STRING + 41,
  MRJINFO_LOCAL_PORT       = MRJINFO_LONG   + 42,
  MRJINFO_TLS_SESSION      = MRJINFO_SLIST  + 43,
  /* Fill in new entries below here! */

  MRJINFO_LASTONE          = 43
} MRJINFO;

/* MRJINFO_RESPONSE_CODE is the new name for the option previously known as
   MRJINFO_HTTP_CODE */
#define MRJINFO_HTTP_CODE MRJINFO_RESPONSE_CODE


typedef enum
{   
    MRJE_ERR = -1,               /* -1 */   
    MRJE_OK  = 0,                /* 0 */
	MRJE_UNKNOWN,                /* 1 */
	MRJE_URLINLEGAL,             /* 2 */
	MRJE_CRSOCKFAL,              /* 3 */
	MRJE_IOCTLFAL,               /* 4 */
	MRJE_NOTFINDHOST,            /* 5 */
	MRJE_NOTCONNHOST,            /* 6 */
	MRJE_DISCONNHOST,            /* 7 */
	MRJE_NOCOMPLETERES,          /* 8 */
	MRJE_SNDREQERR,              /* 9 */
	MRJE_EMPTYRES,               /* 10 */
	MRJE_ENOCOMPLETERES,          /* 11 */
	MRJE_CRLOCALFILEFAL,         /* 12 */
	MRJE_SELECTFAL,              /* 13 */
	MRJE_TIME_OUT,                /* 14 */
	MRJE_NORESPONSE,             /* 16 */
	MRJE_HTTP_SERVERE400,             /* 17 */
	MRJE_HTTP_SERVERE401,             /* 18 */
	MRJE_HTTP_SERVERE403,             /* 19 */
	MRJE_HTTP_SERVERE404,             /* 20 */
	MRJE_HTTP_SERVERE500,             /* 21 */
	MRJE_HTTP_SERVERE503,             /* 22 */
	MRJE_BAD_FUNCTION_ARGUMENT,       /* 23 */
	MRJE_OUT_OF_MEMORY,               /* 24 */
	MRJE_UNSUPPORTED_PROTOCOL,        /* 25 */  
	MRJE_FAILED_INIT,                 /* 26 */
	MRJE_UNKNOWN_OPTION,              /* 27 */
	MRJE_WRITE_ERROR,                 /* 28 */  
    MRJE_UPLOAD_FAILED,               /* 29 */  
    MRJE_READ_ERROR,                 /* 30 */
    MRJE_OPERATION_TIMEDOUT,        /* 31 */
    MRJE_FUNCTION_NOT_FOUND,        /* 32 */
    MRJE_INTERFACE_FAILED,          /* 33 */
    MRJE_SEND_ERROR,                /* 34 */
    MRJE_RECV_ERROR,                /* 35 */
    MRJE_AGAIN,                     /* 36 */
    MRJE_NO_CONNECTION_AVAILABLE,   /* 37 */    
    MRJE_ABORTED_BY_CALLBACK,       /* 38 */    
    MRJE_OPEN_FILE_ERROR,           /* 39 */
    MRJE_SELECT_FAIL,               /* 40 */
    MRJE_SELECT_TIME_OUT,           /* 41 */
    MRJE_GET_SOCKOPT_ERROR,         /* 42 */
    MRJE_FCNTL_ERROR,               /* 43 */
    MRJE_CONNECT_NONB_ERROR         /* 44 */
} MRJEcode;

struct UserDefined 
{
    Bool                verbose;                        /* output verbosity */
    MRJ_HttpReq         httpreq;                        /* milliseconds, 0 means no timeout */
    long                httpversion;
    long                timeout;                        /* milliseconds, 0 means no timeout */
    long                connecttimeout;
    FILE *				local_stream;
    write_callback      fwrite_func;
    int                 is_fwrite_set;                  /* boolean, has write callback been set to non-NULL? */
    void *              event_args;
    char *              url;                            /* url */
    size_t              send_data_len;
    void *              send_data;
    size_t              recv_data_len;
    void *              recv_data;
    size_t              set_resume_from;
};

struct ContentRange {
    size_t range_low;
    size_t range_high;
    size_t range_total;
};

struct PureInfo {
  int httpcode;  /* Recent HTTP response code */
  size_t content_len;
  struct ContentRange range;
};

struct SessionHandle
{
	mrj_socket_t	    sock;                           /* sockfd */
	struct sockaddr_in	remote_addr;                    /* server sockaddr_in */
	int 				port;                           /* port */
	char				host[MRJ_MAX_HOST];             /* host ip */
	char 				remote_path[MRJ_MAX_PATH];       /* request remote path */ 
	char 				remote_file[MRJ_MAX_PATH];       
	char 				file_type[MRJ_MAX_FILETYPE];
	char				local_filepath[MRJ_MAX_PATH];
    size_t              request_send_len;
    char				request_send[MRJ_MAX_REQHEAD];
    size_t              response_recv_len;
	char				response_recv[MRJ_MAX_RESPHEAD];
    struct UserDefined  set;
    struct PureInfo     info;
};


MRJHTTP *http_client_init(void);
void http_client_cleanup(MRJHTTP *handle);
MRJEcode http_client_setopt(MRJHTTP *http,  MRJHTTPoption tag, ...);
MRJEcode http_client_perform(MRJHTTP *handle);
MRJEcode http_client_getinfo(MRJHTTP *mrjhttp, MRJINFO info, ...);
const char *http_client_strerror(MRJEcode error);
#ifdef __cplusplus
}
#endif

#endif /* _DVS_HTTP_H_ */

