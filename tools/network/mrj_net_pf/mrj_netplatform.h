
#ifndef _MRJ_NET_PLATFORM_H_
#define _MRJ_NET_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mrj_http.h"

#define PF_MAX_UPLOAD               (100)
#define PLATFORM_OK                 (10000)                     /* ƽ̨Э����ȷӦ�� */                   
#define DOWNLOAD_FLUSH_SIZE         (1024)
#define PLATFORM_AUTH_NAME          "auth.meirenji.cn"
#define HTTP_SERVER_IP              "120.24.240.158"         /* �����ϴ�������ӦIP��ַ */
#define UPDATE_FILE_PATH            "/mnt/storage/record/update.bin" /* ƽ̨���������ļ����ر����ļ��� */

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
    int result;         // 1��ʾ�ɹ���2��ʾʧ�ܣ�3û����������
    int configId;       // ������Ϣ���ص�ID
    int leftPoint;      // ��߾�
    int rightPoint;     // �ұ߾�
    int topPoint;       // �ϱ߾�
    int bottomPoint;    // �±߾�
    int direction;      // ����
    int height;         // �豸��װ�߶�
    int init;           // �ָ�Ĭ�����ñ�ʶ��0-�������ã�1-�ָ�Ĭ������         
};

/* ƽ̨��ȡSEVEN�㷨������Ϣ */
struct SevenResponse
{
    int max_lost_frame;       /* ���ʧ֡����Ĭ��20 */
    int max_distance;         /* ������ͷ֮��ļ��С�����ֵ������һ���켣��Ĭ��65 */
    int max_frame_cnt;        /* �ڵ����֡�����Ĭ��3 */
    int good_track_cnt;       /* ��С�켣������,С�ڴ�ֵ�Ĺ켣���ᱻ������Ĭ��4 */
    int min_dis_len;          /* ��С����,���ڴ�ֵ�Ĺ켣�Ż�����ͳ�ƣ�Ĭ��30 */
    int min_angle;              
    int discard_open_flag;
    int discard_max_distance;
    int track_init_defalt;    /* �Ƿ��seven����Ĭ�����ã�1-�ָ�Ĭ�����ã�other-�������� */
    int result;           /* �㷨��ȡ�����½����1-��ȡ���³ɹ���2-��ȡ����ʧ�ܣ�3-û���������� */
    int config_id;              
};


/* ƽ̨��ȡ��Ƶ���ò��� */
typedef struct VideoResponse
{
    int configId;           /* ����id */
    int result;             /* ���ø��²������, 1-��ȡ���³ɹ���2-��ȡ����ʧ�ܣ�3-û���������� */
    int init;               /* �Ƿ�ָ�Ĭ������, 0-���ָ���1-�ָ� */
    int brightness;         /* ����, 0-255 */
    int contrast;           /* �Աȶ�, 0-255 */
    int hue;                /* ɫ��, -128-127 */
    int saturation;         /* ���Ͷ�, 0-255 */
    int exposure;           /* �ع��, 0-255 */
    
} VIDEO_RESPONSE;


/* ƽ̨��ȡ��������Ϣ */
typedef struct BindResponse
{
    int result;         //���������0-�ɹ�������-ʧ��
    int bindFlag;       //�󶨱�ʶ, 0-����豸�󶨣�1-���豸
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

/* ƽ̨�ϴ��������� */
typedef struct ReportRcdReq
{
    struct ReportRcdData data[PF_MAX_UPLOAD];
    int num;
    int reqdatalen;
} REPORT_RCD_REQ;


/* ƽ̨��ȡ�����ļ���Ϣ */
typedef struct UpgradeResponse
{
    int result;                 /* 0-���°汾���£�1-��ǰ�汾�������°汾�������°汾Ϊ�գ�-1��ȡ��Ϣʧ�� */
    int filesize;                   /* �����ļ���С */
    char downloadurl[256];          /* �����ļ����ص�ַ */
    char newversion[16];            /* �����ļ��汾�� */
} UPGRADE_RESPONSE;


/* �����ļ���Ϣ */
typedef struct DownloadResponse
{
    int result;                    /* �����ļ��������ս����0-�ɹ���other-ʧ�� */ 
    long totalsize;                  /* �����ļ������ܴ�С */
    long alreadysize;               /* �Ѿ����ص��ļ���С */
    long flushsize;                 /* �� */
    FILE *fp;                         /* �����ļ������ļ�ָ�� */
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

