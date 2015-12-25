
#ifndef _NET_PLATFORM_H_
#define _NET_PLATFORM_H_

#define HTTP_SERVER_IP      "192.168.0.251"
#define HTTP_SERVER_PORT    (80)
#define HTTP_DOWNLOAD_PORT  (8080)

typedef size_t (*MRJ_HTTP_CALLBACK)(void *buffer, size_t size, size_t nitems, void *outstream);

struct UpgradeInfo
{
    int pfcode;
    int filesize;
    char url[256];
    char version[16];
};

/* 升级文件信息 */
struct DownloadInfo
{
    int result;                    /* 升级文件下载最终结果，0-成功，other-失败 */ 
    long downsize;                  /* 升级文件下载总大小 */
    long alreadysize;               /* 已经下载的文件大小 */
    long flushsize;                 /* 缓 */
    FILE *fp;                       /* 升级文件保存文件指针 */
};

typedef struct configLogin
{
    int  result;
    char token[64];
    char lastReportTime[20];
    char syncTime[20];
} CONFIG_LOGIN;


/* 平台获取算法配置信息 */
typedef struct configPlatform
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
} CONFIG_PLATFORM;


struct DeviceBind 
{
    int result;
    int bindFlag;
};



#endif /* _NET_PLATFORM_H_ */

