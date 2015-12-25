
#ifndef _MRJ_NET_PLATFORM_H_
#define _MRJ_NET_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mrj_http.h"

#define PF_MAX_UPLOAD               (100)
#define PLATFORM_OK                 (10000)                     /* 平台协议正确应答 */                   
#define DOWNLOAD_FLUSH_SIZE         (1024)
#define PLATFORM_AUTH_NAME          "auth.meirenji.cn"
#define HTTP_SERVER_IP              "120.24.240.158"         /* 数据上传域名对应IP地址 */
#define UPDATE_FILE_PATH            "/mnt/storage/record/update.bin" /* 平台下载升级文件本地保存文件名 */

#if defined(__STDC__) || defined(_MSC_VER) || defined(__cplusplus) || \
  defined(__HP_aCC) || defined(__BORLANDC__) || defined(__LCC__) || \
  defined(__POCC__) || defined(__SALFORDC__) || defined(__HIGHC__) || \
  defined(__ILEC400__)
  /* This compiler is believed to have an ISO compatible preprocessor */
#define PF_ISOCPP
#else
  /* This compiler is believed NOT to have an ISO compatible preprocessor */
#undef PF_ISOCPP
#endif

struct AuthResponse
{
    int result;                 /* 0-success, other-fail */
    char devsecret[32];         /* server response dev secret */
    char domain[128];       /* server response domain name */
};


struct LoginResponse
{
    int result;
    char token[64];
    char last_reptime[20];
    char cur_time[20];
};

struct PcResponse
{
    int result;         // 1表示成功，2表示失败，3没有最新配置
    int configId;       // 配置信息返回的ID
    int leftPoint;      // 左边距
    int rightPoint;     // 右边距
    int topPoint;       // 上边距
    int bottomPoint;    // 下边距
    int direction;      // 方向
    int height;         // 设备安装高度
    int init;           // 恢复默认配置标识，0-更新配置，1-恢复默认配置         
};

/* 平台获取SEVEN算法配置信息 */
struct SevenResponse
{
    int max_lost_frame;       /* 最大丢失帧数，默认20 */
    int max_distance;         /* 两个人头之间的间隔小于这个值，算入一个轨迹，默认65 */
    int max_frame_cnt;        /* 节点最大帧间隔，默认3 */
    int good_track_cnt;       /* 最小轨迹结点个数,小于此值的轨迹将会被丢弃，默认4 */
    int min_dis_len;          /* 最小距离,大于此值的轨迹才会纳入统计，默认30 */
    int min_angle;              
    int discard_open_flag;
    int discard_max_distance;
    int track_init_defalt;    /* 是否恢seven参数默认配置，1-恢复默认配置，other-更新配置 */
    int result;           /* 算法获取、更新结果，1-获取更新成功，2-获取更新失败，3-没有最新配置 */
    int config_id;              
};


/* 平台获取视频配置参数 */
typedef struct VideoResponse
{
    int configId;           /* 配置id */
    int result;             /* 配置更新操作结果, 1-获取更新成功，2-获取更新失败，3-没有最新配置 */
    int init;               /* 是否恢复默认配置, 0-不恢复，1-恢复 */
    int brightness;         /* 亮度, 0-255 */
    int contrast;           /* 对比度, 0-255 */
    int hue;                /* 色度, -128-127 */
    int saturation;         /* 饱和度, 0-255 */
    int exposure;           /* 曝光度, 0-255 */
    
} VIDEO_RESPONSE;


/* 平台获取绑定配置信息 */
typedef struct BindResponse
{
    int result;         //操作结果，0-成功，其他-失败
    int bindFlag;       //绑定标识, 0-解除设备绑定，1-绑定设备
} BIND_RESPONSE;


struct ReportRcdData
{
    char imsi[20];
    char time[20];
    int in;
    int out;
    int strand;
    int flag; 
};

/* 平台上传数人数据 */
typedef struct ReportRcdReq
{
    struct ReportRcdData data[PF_MAX_UPLOAD];
    int num;
    int reqdatalen;
} REPORT_RCD_REQ;


/* 平台获取升级文件信息 */
typedef struct UpgradeResponse
{
    int result;                 /* 0-有新版本更新，1-当前版本已是最新版本或者最新版本为空，-1获取信息失败 */
    int filesize;                   /* 升级文件大小 */
    char downloadurl[256];          /* 升级文件下载地址 */
    char newversion[16];            /* 升级文件版本号 */
} UPGRADE_RESPONSE;


/* 升级文件信息 */
typedef struct DownloadResponse
{
    int result;                    /* 升级文件下载最终结果，0-成功，other-失败 */ 
    long totalsize;                  /* 升级文件下载总大小 */
    long alreadysize;               /* 已经下载的文件大小 */
    long flushsize;                 /* 缓 */
    FILE *fp;                         /* 升级文件保存文件指针 */
} DOWNLOAD_RESPONSE;


typedef struct ReportPhotoReq
{
    char photopath[MRJ_MAX_PATH]; 
    int index;
} REPORT_PHOTOT_REQ;


typedef enum {
    PFREQ_UNKOWN,
    PFREQ_AUTH,
    PFREQ_LOGIN,
    PFREQ_DOWNLOAD,
    
    PFREQ_QUERY_BIND,
    PFREQ_QUERY_PC,
    PFREQ_QUERY_UPGRADE,
    PFREQ_QUERY_VIDEO,
    PFREQ_QUERY_SEVEN,
    
    PFREQ_REPORT_CONNTEST,
    PFREQ_REPORT_UPGRADE,
    PFREQ_REPORT_PC,                    /* pf modify the pc param, report */
    PFREQ_REPORT_PC_APP,                /* app modify the pc param, report */
    PFREQ_REPORT_PHOTO,
    PFREQ_REPORT_SEVEN,
    PFREQ_REPORT_VIDEO,
    PFREQ_REPORT_RCD,
    PFREQ_REPORT_DEV_NAME
    
} MRJ_PFReq;

typedef void MRJPF;
typedef size_t (*MRJ_HTTP_CALLBACK)(void *, size_t, size_t, void *);
typedef char *(*MRJ_PF_CALLBACK)(void *);


/* long may be 32 or 64 bits, but we should never depend on anything else
   but 32 */
#define PFOPTTYPE_LONG          0
#define PFOPTTYPE_OBJECTPOINT   10000
#define PFOPTTYPE_FUNCTIONPOINT 20000
#define PFOPTTYPE_OFF_T         30000

/* name is uppercase MRJOPT_<name>,
   type is one of the defined MRJOPTTYPE_<type>
   number is unique identifier */
#ifdef PFINIT
#undef PFINIT
#endif

#ifdef PF_ISOCPP
#define PFINIT(na,t,nu) PFOPT_ ## na = PFOPTTYPE_ ## t + nu
#else
/* The macro "##" is ISO C, we assume pre-ISO C doesn't support it. */
#define LONG          PFOPTTYPE_LONG
#define OBJECTPOINT   PFOPTTYPE_OBJECTPOINT
#define FUNCTIONPOINT PFOPTTYPE_FUNCTIONPOINT
#define OFF_T         PFOPTTYPE_OFF_T
#define PFINIT(name,type,number) PFOPT_/**/name = type + number
#endif

typedef enum {
    PFINIT(WRITEDATA, OBJECTPOINT, 1),
    PFINIT(WRITEFUNCTION, FUNCTIONPOINT, 2),

    PFINIT(CREATDATA, OBJECTPOINT, 3),
    PFINIT(CREATFUNCTION, FUNCTIONPOINT, 4),
    
    PFINIT(DOMAINFIELDS, OBJECTPOINT, 5),
    PFINIT(DNS_RESLOVE, LONG, 6),      
    PFINIT(VERBOSE, LONG, 7),

    PFINIT(AUTH, LONG, 27),
    PFINIT(LOGIN, LONG, 8),
    PFINIT(DOWNLOAD, LONG, 9),
    PFINIT(QUERY_SEVEN, LONG, 10),
    PFINIT(QUERY_VIDEO, LONG, 11),
    PFINIT(QUERY_PC, LONG, 12),
    PFINIT(QUERY_UPGRADE, LONG, 13),
    PFINIT(QUERY_BIND, LONG, 14),
    PFINIT(REPORT_CONNTEST, LONG, 15),
    PFINIT(REPORT_UPGRADE, LONG, 16),
    PFINIT(REPORT_PC, LONG, 17),
    PFINIT(REPORT_SEVEN, LONG, 18),
    PFINIT(REPORT_VIDEO, LONG, 19),
    PFINIT(REPORT_RCD, LONG, 20),
    PFINIT(REPORT_PHOTO, LONG, 21),
    PFINIT(REPORT_DEV_NAME, LONG, 22),
    PFINIT(REPORT_PC_APP, LONG, 23),

    PFINIT(PHOTO_INDEX, LONG, 24),
    PFINIT(REMOTE_FILE, OBJECTPOINT, 25),
    PFINIT(RESUME_FROM, LONG, 26)
    
} MRJPFoption;

typedef enum {
  DNSREQ_NONE, /* first in list */
  DNSREQ_IO,
  DNSREQ_APP,
  DNSREQ_LAST /* last in list */
} PF_DNSReq;


struct MRJpfSessionHandle
{
    long verbose;
    MRJ_PFReq pfreq;
    int is_pffunc_set;
    char *contentp;                     /* use malloc, use free */
    MRJ_PF_CALLBACK pffunc;
    void *pfarg;                        
    int is_httpfunc_set;
    MRJ_HTTP_CALLBACK httpfunc;
    void *httparg;                      /* use static param */
    char *domain;                       /* use strdup, use free */    
    long dnsflag;
    long photo_index;
    long resume_from;
    char *remote_file;                  /* download file name */                 
};


struct MRJpfRequest
{
    MRJ_PFReq pfreq;
    char url[256];
    char token[256];
    char serialsn[32];
    int dnsflag;
    
};


int MRJpf_login();
int MRJpf_download(const char *url, const char *filepath, char *remotename, long totalsize);
int MRJpf_report_conntest();
int MRJpf_report_rcd(struct ReportRcdReq *reqp);
int MRJpf_report_photo(char *photopath, long index);
int MRJpf_report_pc();
int MRJpf_report_pc_app();
int MRJpf_report_seven();
int MRJpf_report_video();
int MRJpf_report_devname();
int MRJpf_report_upgrade();
int MRJpf_query_pc();
int MRJpf_query_seven();
int MRJpf_query_video();
int MRJpf_query_upgrade();
int MRJpf_query_bind();

#ifdef __cplusplus
}
#endif

#endif /* _MRJ_NET_PLATFORM_H_ */

