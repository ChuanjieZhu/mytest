
#include <sys/types.h>
#include <sys/stat.h>

#include "http_client.h"
#include "platform_json.h"
#include "platform.h"

using namespace Json;
using namespace std;

#define MRJDEBUG
#define PLATFORMDEBUG
#define MAX_URL_LEN          128
#define MAX_DATA_LEN         1024
#define DEFAULT_AUTH_IP_ADDR "112.74.199.152"
#define DEFAULT_AUTH_PORT    8888


struct platform_auth_response g_auth_response;


size_t platform_parse_auth(void *buffer, size_t size, size_t nitems, void *outstream)
{
    struct platform_auth_response *response = (struct platform_auth_response *)outstream;
    size_t datasize = 0;
    int return_code = 0;
    Value jsonValue;
    char *bufp = (char *)buffer;
    int port = 0;

    response->result = -1;
    datasize = size * nitems;
    bufp[datasize] = '\0';

    TRACE("* %s %s %d\r\n", bufp, MDL);

    if (datasize <= 0) 
    {   
        TRACE("> data size error. %s %d\r\n", MDL);
        goto done;
    }

    if (string_to_json(bufp, jsonValue) != 0) 
    {
        TRACE("> string to json error. %s %d\r\n", MDL);
        goto done;
    }

    if (jsonValue["success"].isNull()             /* success为空或者不为字符串 */
        || !jsonValue["success"].isString())
    {
        TRACE("> json element success (is null/not string). %s %d\r\n", MDL);
        goto done;
    }

    return_code = atoi(jsonValue["success"].asCString());

    TRACE("* platform success(%d). %s %d\r\n", return_code, MDL);
    
    if (return_code != 10000)
    {
        goto done;
    }

    /* 判断json数据类型是否正确 */
    if (jsonValue["devsecret"].isNull()
        || !jsonValue["devsecret"].isString()
        || jsonValue["url"].isNull()
        || !jsonValue["url"].isString()
        || jsonValue["ip"].isNull()
        || !jsonValue["ip"].isString()
        || jsonValue["port"].isNull()
        || !jsonValue["port"].isString())
    {
        TRACE("> error, element is NULL. %s %d\r\n", MDL);
        goto done;
    }

/* 
    if (IsIP(jsonValue["ip"].asCString()) != 0)
    {
        TRACE("> error, ip element is error. %s %d\r\n", MDL);
        goto done;    
    }
*/

    port = atoi(jsonValue["port"].asCString());
    if (port <= 0 || port >= 65535)
    {   
        TRACE("> error, port element is error. %s %d\r\n", MDL);
        goto done;    
    }
    
    response->result = 0;
    strncpy(response->dev_secret, jsonValue["devsecret"].asCString(), sizeof(response->dev_secret) - 1);
    strncpy(response->domain, jsonValue["url"].asCString(), sizeof(response->domain) - 1);
    strncpy(response->ip_addr, jsonValue["ip"].asCString(), sizeof(response->ip_addr) - 1);
    response->port = port;
    TRACE("* url:%s, ip:%s, port:%d %s %d\r\n", response->domain, response->ip_addr, response->port, MDL);
   
    
done:
    if (response->result != 0) 
    {
        ;
    }
    
    return datasize;    
}

size_t platform_parse_longin(void *buffer, size_t size, size_t nitems, void *outstream)
{
    CONFIG_LOGIN *strLogin = (CONFIG_LOGIN *)outstream;
    int returnCode = -1;
    Value jsonReceive;    //表示一个json格式的对象
    size_t data_size = size * nitems;
    char *pBuffer = (char *)buffer;
    
    printf("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        printf("Receive data error, data_size: %d. %s %d\r\n", data_size, __FUNCTION__, __LINE__);
        strLogin->result = -1;
        goto End;
    }

    if (string_to_json(pBuffer, jsonReceive) != 0)
    {
        printf("String to json object fail. %s %d \r\n", __FUNCTION__, __LINE__);
        strLogin->result = -1;
        goto End;
    }

    if (jsonReceive["success"].isNull()             /* success为空或者不为字符串 */
        || !jsonReceive["success"].isString())
    {
        printf("Json element success (is null/not string). %s %d\r\n", 
            __FUNCTION__, __LINE__);
        
        strLogin->result = -1;
        goto End;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != 10000)
    {
        strLogin->result = -1;
        goto End;
    }

    /* 判断json数据类型是否正确 */
    if (jsonReceive["lastReport"].isNull()
        || !jsonReceive["lastReport"].isString()
        || jsonReceive["token"].isNull()
        || !jsonReceive["token"].isString()
        || jsonReceive["currentTime"].isNull()
        || !jsonReceive["currentTime"].isString())
    {
        strLogin->result = -1;
        goto End;
    }

    strLogin->result = 0;
    printf("> token: %s \r\n", jsonReceive["token"].asCString());
    printf("> lastReport: %s \r\n", jsonReceive["lastReport"].asCString());
    printf("> currentTime: %s \r\n", jsonReceive["currentTime"].asCString());

    strncpy(strLogin->token, jsonReceive["token"].asCString(), sizeof(strLogin->token) - 1);
    strncpy(strLogin->lastReportTime, jsonReceive["lastReport"].asCString(), sizeof(strLogin->lastReportTime) - 1);
    strncpy(strLogin->syncTime, jsonReceive["currentTime"].asCString(), sizeof(strLogin->syncTime) - 1);
    
End:    
    return data_size;
}


int platform_post_data(const char *url, const char *content, MRJ_HTTP_CALLBACK func, void *arg)
{
    MRJEcode res = MRJE_OK;
    MRJHTTP *handle = NULL;

    handle = http_client_init();
    if (!handle) 
    {
        TRACE("> http client init fail. %s %d\r\n", MDL);
        return -1; 
    }

    http_client_setopt(handle, MRJOPT_URL, url);
    http_client_setopt(handle, MRJOPT_POSTFIELDS, content);
    http_client_setopt(handle, MRJOPT_WRITEFUNCTION, func);
    http_client_setopt(handle, MRJOPT_WRITEDATA, arg);
    http_client_setopt(handle, MRJOPT_POST, 1L);

    http_client_setopt(handle, MRJOPT_HTTP_VERSION, MRJ_HTTP_VERSION_1_0);
    http_client_setopt(handle, MRJOPT_TIMEOUT, 30L);
    http_client_setopt(handle, MRJOPT_CONNECTTIMEOUT, 10L);
    
#ifdef MRJDEBUG
    http_client_setopt(handle, MRJOPT_VERBOSE, 1L);
#endif  

    res = http_client_perform(handle);
    if (res != MRJE_OK) 
    {
        TRACE("> http perform error. %s %d\r\n", MDL);
        return -1;
    }

    long httpcode = 0;
    res = http_client_getinfo(handle, MRJINFO_RESPONSE_CODE, &httpcode);

    TRACE("* httpcode: %ld %s %d\r\n", httpcode, MDL);
    
    http_client_cleanup(handle);

    return 0;
}


MRJEcode platform_get_data(const char *url, long form, MRJ_HTTP_CALLBACK func, void *arg)
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJHTTP *http = NULL;
    
    http = http_client_init();
    if (!http) {
        TRACE("Mrj http init fail. %s %d\r\n", MDL);
        return MRJE_FAILED_INIT; 
    }

    http_client_setopt(http, MRJOPT_URL, url);
    http_client_setopt(http, MRJOPT_WRITEFUNCTION, func);  
    http_client_setopt(http, MRJOPT_WRITEDATA, arg);  
    http_client_setopt(http, MRJOPT_RESUME_FROM, form);    /* 从offset处开始断点续传 */
    http_client_setopt(http, MRJOPT_TIMEOUT, 600L);          /* 升级文件下载最长时间600s */
    http_client_setopt(http, MRJOPT_CONNECTTIMEOUT, 30L);
    
#ifdef MRJDEBUG
    http_client_setopt(http, MRJOPT_VERBOSE, 1);
#endif

    res = http_client_perform(http);
    if (res != MRJE_OK)
    {
        TRACE("Http client perform failed: %d(%s) %s %d\n", res, http_client_strerror(res), __FUNCTION__, __LINE__);
        http_client_cleanup(http);
        return res;
    }

    http_client_cleanup(http);

    return MRJE_OK;
}





int platform_download_init(struct DownloadInfo *pInfo, long *from, size_t downsize)
{
    if (!pInfo || !from) 
        return -1;

    memset(pInfo, 0, sizeof(struct DownloadInfo));
    pInfo->downsize = downsize;
    pInfo->result = -1;
    pInfo->alreadysize = 0;
    pInfo->flushsize = 0;
    pInfo->fp = NULL;
    
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    
    if (stat("TEST.bin", &st) == 0
        && st.st_size > 0 
        && st.st_size < pInfo->downsize)
    {
        pInfo->fp = fopen("TEST.bin", "ab+");
        if (pInfo->fp == NULL)
        {
            TRACE("fopen %s(%s) fail. %s %d\r\n", "TEST.bin", STR_ERRNO, MDL);
            return -1;
        }
        
        *from = st.st_size;
        pInfo->alreadysize = st.st_size;       /* 已经下载的文件大小 */

        TRACE("Download resume, already download size(%ld). %s %d\r\n", pInfo->alreadysize, __FUNCTION__, __LINE__);
    }
    else
    {
        /* 重新下载文件 */
        pInfo->fp = fopen("TEST.bin", "wb+");
        if (NULL == pInfo->fp)
        {
            TRACE("fopen %s(%s) fail. %s %d\r\n", "TEST.bin", STR_ERRNO, MDL);
            return -1;
        }
        
        *from = 0;
        pInfo->alreadysize = 0;
    }

    return 0;
}

static size_t platform_parse_download(void *buffer, size_t size, size_t nmemb, void *user_p) 
{ 	
    struct DownloadInfo *downInfo = (struct DownloadInfo *)user_p; 	
    size_t return_size = 0;

    if (downInfo->fp == NULL)
    {
        TRACE("Error, update fp is NULL. %s %d\r\n", __FUNCTION__, __LINE__);
        downInfo->result = -1;
        goto End;
    }
  
    return_size = fwrite(buffer, size, nmemb, downInfo->fp);
    if (return_size != nmemb) {
        TRACE("Fwrite receive download data error(%s). %s %d\r\n", STR_ERRNO, MDL);
        downInfo->result = -1;
        goto End;
    }
    
    downInfo->alreadysize += return_size;
    
    TRACE("+++ down bytes %d(%d, %d) download size(%ld) total size(%ld) +++ \r\n", 
          size*nmemb, size, nmemb, downInfo->alreadysize, downInfo->downsize);
    
    downInfo->flushsize += return_size;
    if (downInfo->flushsize >= MRJ_FLUSH_BLOCK) {
        fflush(downInfo->fp);
	    downInfo->flushsize = 0;
    } 
    
    if (downInfo->alreadysize >= downInfo->downsize)
    {
        TRACE("Downlaod upgrade file success. %s %d\r\n", __FUNCTION__, __LINE__);
        fflush(downInfo->fp);
        fclose(downInfo->fp);
        downInfo->fp = NULL;
        downInfo->result = 0;
        goto End;
    }
    
End:        
    return return_size; 
}

size_t platform_parse_binding(void *buffer, size_t size, size_t items, void *stream)
{   
    struct DeviceBind *bindp = (struct DeviceBind *)stream;
    size_t ret_size = size * items;
    int pfcode = -1;
    Value jsValue;
    char *ptr = (char *)buffer;

    if (ret_size <= 0) {
        bindp->result = -1;
        goto done;
    }

    if (0 != string_to_json(ptr, jsValue)) {
        bindp->result = -1;
        goto done;
    }

    if (jsValue["success"].isNull() ||
        !jsValue["success"].isString()) {
        bindp->result = -1;
        goto done;
    }

    if ((pfcode = atoi(jsValue["success"].asCString())) != 10000) {
        bindp->result = -1;
        goto done;
    }

    if (jsValue["isBind"].isNull() ||
        !jsValue["isBind"].isString())
    {
        bindp->result = -1;
        goto done;
    }
    
    bindp->result = 0;
    bindp->bindFlag = atoi(jsValue["isBind"].asCString());

done:

    return ret_size;
}


size_t platform_parse_upgrade(void *buffer, size_t size, size_t items, void *stream)
{
    struct UpgradeInfo *infop = (struct UpgradeInfo *)stream;
    int pfcode = -1;
    size_t ret_size = size * items;
    Value jsValue;
    char *ptr = (char *)buffer;

    if (ret_size <= 0)
    {
        TRACE("Receive data error, data_size: %d. %s %d\r\n", ret_size, __FUNCTION__, __LINE__);
        infop->pfcode = -1;
        goto End;
    }

    if (string_to_json(ptr, jsValue) != 0)
    {
        TRACE("String to json object fail. %s %d \r\n", __FUNCTION__, __LINE__);
        infop->pfcode = -1;
        goto End;
    }

    if (jsValue["success"].isNull() ||
        !jsValue["success"].isString())
    {
        TRACE("Platform response json success elem is null. %s %d\r\n", __FUNCTION__, __LINE__); 
        infop->pfcode = -1;
        goto End;
    }

    pfcode = atoi(jsValue["success"].asCString());
    if (pfcode != 10000)
    {
        if (pfcode == 10004         /* 当前版本已是最新版本 */
            || pfcode == 10003)     /* 没有最新版本 */
        {
            infop->pfcode = 1;
            goto End;
        }
        else
        {
            infop->pfcode = -1;
            goto End;
        }
    }

    if (jsValue["url"].isNull()
        || !jsValue["url"].isString()
        || jsValue["filesize"].isNull()
        || !jsValue["filesize"].isString()
        || jsValue["version"].isNull()
        || !jsValue["version"].isString())
    {
        TRACE("Json (url/filesize/version) elem null or not string. %s %d\r\n", __FUNCTION__, __LINE__); 
        infop->pfcode = -1;
        goto End;
    }

    infop->pfcode = 0;
    infop->filesize = atoi(jsValue["filesize"].asCString());
    strncpy(infop->url, jsValue["url"].asCString(), sizeof(infop->url) - 1);
    strncpy(infop->version, jsValue["version"].asCString(), sizeof(infop->version) - 1);  

End:    
    return ret_size;
}


int platform_auth()
{
    int ret = -1;
    struct platform_auth_response *auth_response = &g_auth_response;
    char url_buff[MAX_URL_LEN] = {0};
    char data_buff[MAX_DATA_LEN] = {0};
    const char *version = "1.5.0";
    const char *auth_ip_addr = DEFAULT_AUTH_IP_ADDR;
    int auth_port = DEFAULT_AUTH_PORT;

    snprintf(url_buff, sizeof(url_buff) - 1, 
                "http://%s:%d/index.php?action=auth",
                auth_ip_addr, auth_port);

    snprintf(data_buff, sizeof(data_buff) - 1, 
            "imei=%s&version=%s", "1011411000014", version);

    memset(auth_response, 0, sizeof(struct platform_auth_response));
    auth_response->result = -1;
    
    ret = platform_post_data(url_buff, data_buff, platform_parse_auth, auth_response);
    if (ret == 0 && auth_response->result== 0) 
    {
        return 0;
    }

    return -1;
}

int platform_login()
{
    int ret = -1;
    char url[MAX_URL_LEN] = {0};
    char data[MAX_DATA_LEN] = {0};
    struct platform_login_response login_response;

    snprintf(url, sizeof(url) - 1, "http://%s:%d/index.php?action=login",
        g_auth_response.ip_addr, g_auth_response.port);

    snprintf(data, sizeof(data) - 1, "")    
    const char *data = "data={\"imei\":\"1011411000014\",\"ip\":\"192.168.0.22\",\"alias\":\"test0014\"}";

    memset(&g_cfgLogin, 0, sizeof(struct configLogin));
    g_cfgLogin.result = -1;

    ret = platform_post_data(url, data, platform_parse_longin, &g_cfgLogin);
    if (ret == 0 && g_cfgLogin.result == 0)
    {
        return 0;
    }
    
    return -1;
}


int platform_download(char *url, size_t downsize)
{
    int ret = -1;
    struct DownloadInfo info;

    long from = 0;
#if 0
    const char *downUrl = "http://192.168.0.251:8080/myCount/upload/softVersion/1418623640948.bin";

    memset(&info, 0, sizeof(struct DownloadInfo));
    size_t downsize = 1206850;
#endif
    
    ret = platform_download_init(&info, &from, downsize);
    if (ret != 0) 
        return -1;

    ret = platform_get_data(url, from, platform_parse_download, &info);

    return ret;
}

int platform_heart_beat()
{
    int ret = -1;
    char url[256] = {0};
    char content[256] = {0};
    int result;

    snprintf(content, sizeof(content) - 1, "imei=%s", "1011411000014");

    result = -1;
    ret = platform_post_data(url, content, platform_parse_binding, &result);

    return ret;
}   

int platform_upgrade(struct UpgradeInfo *infop)
{
    int ret = -1;
    char url[128] = {0};
    char content[256] = {0};
    string jsonString; 

#ifdef PLATFORMDEBUG
    snprintf(url, sizeof(url) - 1, 
                "http://%s/index.php?action=upgrade", 
                HTTP_SERVER_IP);
#else
    ;
#endif

    /* 生产json数据 */
    jsonString = create_upgrade_json_string();
    snprintf(content, sizeof(content) - 1, "data=%s", jsonString.c_str());
    
    /* 发送数据到平台 */
    ret = platform_post_data(url, content, platform_parse_upgrade, infop);

    return ret;
}


int platform_device_upgrade()
{   
    int ret = -1;
    struct UpgradeInfo info;

    memset(&info, 0, sizeof(struct UpgradeInfo));
    info.pfcode = -1;
    
    ret = platform_upgrade(&info);
    if (ret != 0)
    {
        return ret;
    }
    
    if (info.pfcode != 0) 
    {
        if (info.pfcode == 1) 
            return 0;
        else 
            return -1;
    }

    size_t size = info.filesize;
    ret = platform_download(info.url, size);

    return ret;
}



int main(int argc, char **argv)
{   
    platform_auth();
    platform_login();
    //platform_device_upgrade();
    //platform_binding();
    return 0;
}

