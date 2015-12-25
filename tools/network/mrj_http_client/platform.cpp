
#include <sys/types.h>
#include <sys/stat.h>

#include "mrj_http.h"
#include "mrj_json.h"
#include "platform.h"

using namespace Json;
using namespace std;

#define MRJDEBUG
#define PLATFORMDEBUG

struct configLogin g_cfgLogin;


size_t MRJpf_parse_longin(void *buffer, size_t size, size_t nitems, void *outstream)
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


MRJEcode MRJpf_post_data(const char *url, const char *content, MRJ_HTTP_CALLBACK func, void *arg)
{
    MRJEcode res = MRJE_OK;
    MRJHTTP *handle = NULL;

    handle = mrjhttp_init();
    if (!handle) {
        TRACE("Mrj http init fail. %s %d\r\n", MDL);
        return MRJE_INTERFACE_FAILED; 
    }

    mrjhttp_setopt(handle, MRJOPT_URL, url);
    mrjhttp_setopt(handle, MRJOPT_POSTFIELDS, content);
    mrjhttp_setopt(handle, MRJOPT_WRITEFUNCTION, func);
    mrjhttp_setopt(handle, MRJOPT_WRITEDATA, arg);
    mrjhttp_setopt(handle, MRJOPT_POST, 1L);

    mrjhttp_setopt(handle, MRJOPT_HTTP_VERSION, MRJ_HTTP_VERSION_1_0);
    mrjhttp_setopt(handle, MRJOPT_TIMEOUT, 30L);
    mrjhttp_setopt(handle, MRJOPT_CONNECTTIMEOUT, 10L);
    
#ifdef MRJDEBUG
    mrjhttp_setopt(handle, MRJOPT_VERBOSE, 1L);
#endif  

    res = mrjhttp_perform(handle);

    if (res != MRJE_OK) {
        TRACE("http perform error. %s %d\r\n", MDL);
    }

    long httpcode = 0;
    res = mrjhttp_getinfo(handle, MRJINFO_RESPONSE_CODE, &httpcode);

    TRACE("httpcode: %ld %s %d\r\n", httpcode, MDL);
    
    mrjhttp_cleanup(handle);

    return res;
}


MRJEcode MRJpf_download(const char *url, long form, MRJ_HTTP_CALLBACK func, void *arg)
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJHTTP *http = NULL;
    
    http = mrjhttp_init();
    if (!http) {
        TRACE("Mrj http init fail. %s %d\r\n", MDL);
        return MRJE_FAILED_INIT; 
    }

    mrjhttp_setopt(http, MRJOPT_URL, url);
    mrjhttp_setopt(http, MRJOPT_WRITEFUNCTION, func);  
    mrjhttp_setopt(http, MRJOPT_WRITEDATA, arg);  
    mrjhttp_setopt(http, MRJOPT_RESUME_FROM, form);    /* 从offset处开始断点续传 */
    mrjhttp_setopt(http, MRJOPT_TIMEOUT, 600L);          /* 升级文件下载最长时间600s */
    mrjhttp_setopt(http, MRJOPT_CONNECTTIMEOUT, 30L);
    
#ifdef MRJDEBUG
    mrjhttp_setopt(http, MRJOPT_VERBOSE, 1);
#endif

    res = mrjhttp_perform(http);
    if (res != MRJE_OK)
    {
        TRACE("Http client perform failed: %d(%s) %s %d\n", res, mrjhttp_strerror(res), __FUNCTION__, __LINE__);
        mrjhttp_cleanup(http);
        return res;
    }

    mrjhttp_cleanup(http);

    return MRJE_OK;
}





MRJEcode MRJpf_download_init(struct DownloadInfo *pInfo, long *from, size_t downsize)
{
    if (!pInfo || !from) 
        return MRJE_BAD_FUNCTION_ARGUMENT;

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
            return MRJE_OPEN_FILE_ERROR;
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
            return MRJE_OPEN_FILE_ERROR;
        }
        
        *from = 0;
        pInfo->alreadysize = 0;
    }

    return MRJE_OK;
}

static size_t MRJpf_parse_download(void *buffer, size_t size, size_t nmemb, void *user_p) 
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

size_t MRJpf_parse_binding(void *buffer, size_t size, size_t items, void *stream)
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


size_t MRJpf_parse_upgrade(void *buffer, size_t size, size_t items, void *stream)
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



MRJEcode MRJPF_login()
{
    MRJEcode res = MRJE_UNKNOWN;
    
    const char *url = "http://192.168.0.251/index.php?action=login";
    const char *data = "data={\"imei\":\"1011411000014\",\"ip\":\"192.168.0.22\",\"alias\":\"test0014\"}";

    memset(&g_cfgLogin, 0, sizeof(struct configLogin));
    g_cfgLogin.result = -1;

    res = MRJpf_post_data(url, data, MRJpf_parse_longin, &g_cfgLogin);

    return res;
}


MRJEcode MRJPF_download(char *url, size_t downsize)
{
    MRJEcode res = MRJE_UNKNOWN;
    struct DownloadInfo info;

    long from = 0;
#if 0
    const char *downUrl = "http://192.168.0.251:8080/myCount/upload/softVersion/1418623640948.bin";

    memset(&info, 0, sizeof(struct DownloadInfo));
    size_t downsize = 1206850;
#endif
    
    res = MRJpf_download_init(&info, &from, downsize);
    if (res != MRJE_OK) return res;

    res = MRJpf_download(url, from, MRJpf_parse_download, &info);

    return res;
}

MRJEcode MRJpf_binding(struct DeviceBind *pBind)
{
    MRJEcode res = MRJE_UNKNOWN;
    char url[256] = {0};
    char content[256] = {0};

#ifdef PLATFORMDEBUG
    snprintf(url, sizeof(url) - 1, 
            "http://%s/index.php?action=state", 
            HTTP_SERVER_IP);
#else
    ;
#endif

    snprintf(content, sizeof(content) - 1, "imei=%s", "1011411000014");

    res = MRJpf_post_data(url, content, MRJpf_parse_binding, &pBind);

    return res;
}   


MRJEcode MRJPF_binding()
{
    MRJEcode res = MRJE_UNKNOWN;
    struct DeviceBind bind;

    memset(&bind, 0, sizeof(struct DeviceBind));
    bind.result = -1;
    bind.bindFlag = 0;

    res = MRJpf_binding(&bind);

    return res;
}


MRJEcode MRJpf_upgrade(struct UpgradeInfo *infop)
{
    MRJEcode res = MRJE_UNKNOWN;
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
    res = MRJpf_post_data(url, content, MRJpf_parse_upgrade, infop);

    return res;
}


MRJEcode MRJPF_upgrade()
{
    MRJEcode res = MRJE_UNKNOWN;
    struct UpgradeInfo info;

    memset(&info, 0, sizeof(struct UpgradeInfo));
    info.pfcode = -1;
    
    res = MRJpf_upgrade(&info);
    if (res != MRJE_OK)
        return res;

    if (info.pfcode != 0) {
        if (info.pfcode == 1) return MRJE_OK;
        else return MRJE_UNKNOWN;
    }

    size_t size = info.filesize;
    res = MRJPF_download(info.url, size);

    return res;
}



int main(int argc, char **argv)
{
    MRJPF_login();
    MRJPF_upgrade();
    MRJPF_binding();
    return 0;
}

