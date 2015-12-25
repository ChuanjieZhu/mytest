
#ifndef _DVS_HTTP_H_
#define _DVS_HTTP_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int Bool;
typedef int	mrj_socket_t;
typedef void MRJHTTP;

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


#define MDL __FILE__, __LINE__
#define TRACE printf

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

/* long may be 32 or 64 bits, but we should never depend on anything else
   but 32 */
#define MRJOPTTYPE_LONG          0
#define MRJOPTTYPE_OBJECTPOINT   10000
#define MRJOPTTYPE_FUNCTIONPOINT 20000
#define MRJOPTTYPE_OFF_T         30000

/* name is uppercase CURLOPT_<name>,
   type is one of the defined CURLOPTTYPE_<type>
   number is unique identifier */
#ifdef MINIT
#undef MINIT
#endif

#ifdef MRJ_ISOCPP
#define MINIT(na,t,nu) MRJOPT_ ## na = MRJOPTTYPE_ ## t + nu
#else
/* The macro "##" is ISO C, we assume pre-ISO C doesn't support it. */
#define LONG          CURLOPTTYPE_LONG
#define OBJECTPOINT   CURLOPTTYPE_OBJECTPOINT
#define FUNCTIONPOINT CURLOPTTYPE_FUNCTIONPOINT
#define OFF_T         CURLOPTTYPE_OFF_T
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
    MINIT(CONNECTTIMEOUT, LONG, 10)
} MRJHTTPoption;


typedef size_t (*write_callback)(char *buffer,
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

  CURL_HTTP_VERSION_LAST /* *ILLEGAL* http version */
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

typedef struct UserDefined 
{
    Bool                 verbose;                        /* output verbosity */
    MRJ_HttpReq         httpreq;                        /* milliseconds, 0 means no timeout */
    long                httpversion;
    long                timeout;                        /* milliseconds, 0 means no timeout */
    long                connecttimeout;
    FILE *				local_stream;
    write_callback      fwrite_func;
    int                 is_fwrite_set;                  /* boolean, has write callback been set to non-NULL? */
    size_t              range_low;
    size_t              range_hight;
    void *              event_args;
    char *              url;                            /* url */
    void *              send;
    void *              recv;
} USER_DEFINED;


typedef struct SessionHandle
{
	mrj_socket_t	    sock;                           /* sockfd */
	struct sockaddr_in	remote_addr;                    /* server sockaddr_in */
	int 				port;                           /* port */
	char				host[DVS_MAX_HOST];             /* host ip */
	char 				remote_path[DVS_MAXPATH];       /* request remote path */ 
	char 				remote_file[DVS_MAXPATH];       
	char 				file_type[DVS_MAX_FILETYPE];
	unsigned int 		file_length;
    int                 return_code;
	char				local_filepath[DVS_MAXPATH];
    size_t              send_len;
    size_t              recv_len;
    char				request_head[DVS_MAX_REQHEAD];
	char				response_head[DVS_MAX_RESPHEAD];
    struct UserDefined  set;
} SESSION_HANDLE;

#ifdef __cplusplus
}
#endif

#endif /* _DVS_HTTP_H_ */

