
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "http_client.h"
#include "platform_json.h"
#include "platform.h"
#include "cJSON.h"

#define MRJDEBUG
#define PLATFORMDEBUG
#define MAX_URL_LEN          128
#define MAX_DATA_LEN         1024
#define DEFAULT_AUTH_IP_ADDR "112.74.199.152"
#define DEFAULT_AUTH_PORT    8888
#define BASE_SN              "1011411000014"

struct platform_auth_response g_auth_response;
struct platform_login_response g_login_response;

int Base64Encode(char *dest, const char *src, int count)
{ 
	const unsigned char Base64_EnCoding[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int len = 0; 
	unsigned char *pt = (unsigned char *)src;

	if( count < 0 )
	{
		while( *pt++ )
		{ 
			count++; 
		}
		
		pt = (unsigned char *)src;
	}
	
	if( !count )
	{
		return 0;
	}
	
	while( count > 0 )
	{
		*dest++ = Base64_EnCoding[ ( pt[0] >> 2 ) & 0x3f];
		if( count > 2 )
		{
			*dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)];
			*dest++ = Base64_EnCoding[((pt[1] & 0xF) << 2) | (pt[2] >> 6)];
			*dest++ = Base64_EnCoding[ pt[2] & 0x3F];
		}
		else
		{
			switch( count )
			{
				case 1:
				{
					*dest++ = Base64_EnCoding[(pt[0] & 3) << 4 ];
					*dest++ = '=';
					*dest++ = '=';
					
					break;
				}
				case 2: 
				{
					*dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)]; 
					*dest++ = Base64_EnCoding[((pt[1] & 0x0F) << 2) | (pt[2] >> 6)]; 
					*dest++ = '='; 

					break;
				}
			} 
		} 
		
		pt += 3; 
		count -= 3; 
		len += 4; 
	} 
	
	*dest = 0; 

	return len; 
} 

int CopyFileToBuf(char *buf, int n, char *file)
{
	FILE *fp = NULL;
	int retVal = -1;
	unsigned int len = 0;

	if (buf != NULL && n > 0 && file != NULL)
	{
		len = (unsigned int)n;

		if ((fp = fopen(file, "r")) != NULL)
		{
			if (fread(buf, sizeof(char), len, fp) == len)
			{
				retVal = 0;
			}

			fclose(fp);
		}
	}

	return retVal;
}

size_t platform_parse_auth(void *buffer, size_t size, size_t nitems, void *outstream)
{
    struct platform_auth_response *resp = (struct platform_auth_response *)outstream;
    size_t datasize = 0;
    int pfcode = 0;
    char *bufp = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;

    resp->result = -1;
    datasize = size * nitems;
    bufp[datasize] = '\0';

    TRACE("* %s %s %d\r\n", bufp, MDL);

    if (datasize <= 0) 
    {   
        TRACE("> Error, data size error. %s %d\r\n", MDL);
        goto done;
    }

    json = cJSON_Parse(bufp);
    if (json == NULL)
    {
        TRACE("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto done;
    }

    json_value = cJSON_GetObjectItem(json, "success");
    if (json_value == NULL       
        || json_value->valuestring == NULL)
    {
        TRACE("> Element success (is null/not string). %s %d\r\n", MDL);
        goto done;
    }

    pfcode = atoi(json_value->valuestring);

    TRACE("* platform return code(%d). %s %d\r\n", pfcode, MDL);
    
    if (pfcode != 10000)
    {
        goto done;
    }

    json_value = cJSON_GetObjectItem(json , "devsecret");
    if (json_value == NULL             
        || json_value->valuestring == NULL)
    {   
        goto done;
    }
    strncpy(resp->dev_secret, json_value->valuestring, sizeof(resp->dev_secret) - 1);

    /* url */
    json_value = cJSON_GetObjectItem(json , "url");
    if (json_value == NULL             
        || json_value->valuestring == NULL)
    {   
        goto done;
    }
    strncpy(resp->domain, json_value->valuestring, sizeof(resp->domain) - 1);

    /* ip */
    json_value = cJSON_GetObjectItem(json , "ip");
    if (json_value == NULL             
        || json_value->valuestring == NULL)
    {   
        TRACE("> Error, ip element is error. %s %d\r\n", MDL);
        goto done;
    }

    strncpy(resp->ip_addr, json_value->valuestring, sizeof(resp->ip_addr) - 1);

    /* port */
    json_value = cJSON_GetObjectItem(json , "port");
    if (json_value == NULL             
        || json_value->valuestring == NULL)
    {   
        goto done;
    }
    resp->port = atoi(json_value->valuestring);
    if (resp->port <= 0 || resp->port >= 65535)
    {   
        TRACE("> Error, port element is error. %s %d\r\n", MDL);
        goto done;
    }

    TRACE("* domain:%s, ip:%s, port:%d %s %d\r\n", resp->domain, resp->ip_addr, resp->port, MDL);
    
    resp->result = 0;
    
done:
    cJSON_Delete(json);

    if (resp->result != 0) 
    {   
        ;
    }
    
    return datasize;  
}

size_t platform_parse_longin(void *buffer, size_t size, size_t nitems, void *outstream)
{
    struct platform_login_response *response = (struct platform_login_response *)outstream;
    int data_size = 0;
    int returnCode = -1;
    int state = 0;
    char *pBuffer = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;
    
    data_size = size * nitems;
    pBuffer[data_size] = '\0';
    
    TRACE("* %s %s %d\r\n", pBuffer, MDL);

    response->result = -1;
    
    if (data_size <= 0)
    {
        TRACE("Receive data error, data_size: %d. %s %d\r\n", data_size, MDL);
        goto End;
    }

    json = cJSON_Parse(pBuffer);
    if (json == NULL)
    {
        TRACE("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto End;
    }

    json_value = cJSON_GetObjectItem(json, "success");
    if (json_value == NULL || json_value->valuestring == NULL)
    {
        TRACE("> Element success (is null/not string). %s %d\r\n", MDL);
        goto End;
    }
    
    returnCode = atoi(json_value->valuestring);
    TRACE("* platform return code: %d %s %d\r\n", returnCode, MDL);
    if (returnCode != 10000)
    {   
        goto End;
    }

    /* state */
    json_value = cJSON_GetObjectItem(json, "state");
    if (json_value == NULL || json_value->valuestring == NULL)
    {
        TRACE("> Element state (is null/not string). %s %d\r\n", MDL);
        goto End;        
    }
    state = atoi(json_value->valuestring);
    TRACE("* platform state: %d %s %d\r\n", state, MDL);
    response->ap_state = state;
    
    /* token */
    json_value = cJSON_GetObjectItem(json, "token");
    if (json_value == NULL || json_value->valuestring == NULL)
    {
        goto End;
    }
    strncpy(response->token, json_value->valuestring, sizeof(response->token) - 1);

    /* lastReport */
    json_value = cJSON_GetObjectItem(json, "lastReport");
    if (json_value == NULL || json_value->valuestring == NULL)
    {
        goto End;
    }
    strncpy(response->last_report_time, json_value->valuestring, 
        sizeof(response->last_report_time) - 1);

    /* currentTime */
    json_value = cJSON_GetObjectItem(json, "currentTime");
    if (json_value == NULL || json_value->valuestring == NULL)
    {
        goto End;
    }
    strncpy(response->curr_time, json_value->valuestring, sizeof(response->curr_time) - 1);

    response->result = 0;

End: 
    cJSON_Delete(json);
    
    if (response->result != 0) 
    {
        //AddLogMsg("pf login fail(%s)", pBuffer);
    }
    
    return data_size;
}


static size_t platform_parse_heartbeat(void *buffer, size_t size, size_t nmemb, void *stream)
{
    int *result = (int *)stream;
    int data_size = 0;
    int returnCode = -1;
    char *pBuffer = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;

    *result = -1;
    data_size = size * nmemb;
    pBuffer[data_size] = '\0';

    TRACE("* %s %s %d\r\n", pBuffer, MDL);
    
    if (data_size <= 0)
    {
        goto End;
    }

    json = cJSON_Parse(pBuffer);
    if (json == NULL)
    {
        TRACE("> ERROR, json parse(%s). %s %d\r\n", cJSON_GetErrorPtr(), MDL);
        goto End;
    }

    json_value = cJSON_GetObjectItem(json, "success");
    if (json_value == NULL
        || json_value->valuestring == NULL)
    {
        goto End;
    }

    returnCode = atoi(json_value->valuestring);
    TRACE("* platform return code: %d %s %d\r\n", returnCode, MDL);
    if (returnCode != 10000)
    {
        goto End;
    }

    *result = 0;
    
End:

    cJSON_Delete(json);
    
    if (*result != 0) 
    {
        //AddLogMsg("pf heartbeat fail(%s)", pBuffer);
    }
    
    return data_size;   
}

static size_t platform_parse_upload_photo(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *result = (int *)stream;
    int data_size = 0;
    int returnCode = -1;
    char *pBuffer = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;
    
    *result = -1;
    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    TRACE("* %s %s %d\r\n", pBuffer, MDL);
    
    if (data_size <= 0)
    {
        goto Done;
    }

    json = cJSON_Parse(pBuffer);
    if (json == NULL)
    {
        goto Done;
    }

    json_value = cJSON_GetObjectItem(json, "success");
    if (json_value == NULL
        || json_value->valuestring == NULL)
    {
        goto Done;
    }

    returnCode = atoi(json_value->valuestring);
    
    TRACE("* platform return code: %d %s %d\r\n", returnCode, MDL);
    
    if (returnCode != 10000)
    {
        goto Done;
    }

    *result = 0;

Done:

    cJSON_Delete(json);
    
    if (*result != 0) 
    {
        //AddLogMsg("pf upload photo fail(%s)", pBuffer);
    }
   
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


size_t platform_parse_upgrade(void *buffer, size_t size, size_t items, void *stream)
{
    return size * items;
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
            "imei=%s&version=%s", BASE_SN, version);

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
    struct platform_login_response *login_response = &g_login_response;

    snprintf(url, sizeof(url) - 1, "http://%s:%d/index.php?action=login",
        g_auth_response.ip_addr, g_auth_response.port);

    char *json_string = create_login_json_string();
    if (!json_string)
    {
        TRACE("> create login json string failed. %s %d\r\n", MDL);
        return -1;
    }

    snprintf(data, sizeof(data) - 1, "data=%s", json_string);

    free_json_string(json_string);

    memset(login_response, 0, sizeof(struct platform_login_response));
    login_response->result = -1;
    
    ret = platform_post_data(url, data, platform_parse_longin, login_response);
    if (ret == 0 && login_response->result == 0)
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

int platform_heartbeat()
{
    int ret = -1;
    char url[MAX_URL_LEN] = {0};
    char data[MAX_DATA_LEN] = {0};

    snprintf(url, sizeof(url) - 1, "http://%s:%d/index.php?action=heart", 
                g_auth_response.ip_addr, g_auth_response.port);

    TRACE("* sn: %s, token: %s %s %d\r\n", BASE_SN, g_login_response.token, MDL);

    snprintf(data, sizeof(data) - 1, "imei=%s&token=%s", BASE_SN, g_login_response.token);

    int result = -1;
    ret = platform_post_data(url, data, platform_parse_heartbeat, &result);
    if (0 == ret && 0 == result)
    {
        return 0;
    }

    return -1;
}   

int platform_upload_photo()
{
    int ret = -1;
    int result;
    char url[MAX_URL_LEN] = {0};
    char param_buff[MAX_URL_LEN] = {0};
    int photo_size = 0;
    int base64_size = 0;
    int param_len = MAX_URL_LEN;
    struct stat st;
    const char *photo_path = "1.jpg";

    snprintf(url, sizeof(url) - 1, 
                "http://%s:%d/index.php?action=upload",
                g_auth_response.ip_addr, g_auth_response.port);

    if (stat(photo_path, &st) != 0 
        || st.st_size == 0)
    {
        TRACE("> %s not exist! %s %d\r\n", photo_path, MDL);
        return -1;
    }

    photo_size = st.st_size;

    TRACE("* upload photo size(%d). %s %d\r\n", photo_size, MDL);
    
    char *photo_buff = (char *)malloc(photo_size);
    if (photo_buff == NULL)
    {
        TRACE("Malloc buffer fail. %s %d\r\n", MDL);
        return -1;
    }
    
    memset(photo_buff, 0, photo_size);
    if (0 != CopyFileToBuf(photo_buff, photo_size, (char *)photo_path))
    {
        TRACE("> Copy file to buffer fail. %s %d\r\n", MDL);
        free(photo_buff);
        return -1;
    }

    base64_size = photo_size + photo_size / 2 + param_len;
    char *base64_buff = (char *)malloc(base64_size);
    if (base64_buff == NULL)
    {
        TRACE("> Malloc picture base64 buffer fail. %s %d\r\n", MDL);
        free(photo_buff);
        return -1;
    }
    
    snprintf(param_buff, sizeof(param_buff) - 1, "imei=%s&token=%s&sort=%d&data=",
        BASE_SN, g_login_response.token, 1);
    param_buff[sizeof(param_buff) - 1] = '\0';
    param_len = strlen(param_buff);
    
    memset(base64_buff, 0, base64_size);
    memcpy(base64_buff, param_buff, param_len);
    Base64Encode(base64_buff + param_len, photo_buff, photo_size);

    free(photo_buff);

    result = -1;
    ret = platform_post_data(url, base64_buff, platform_parse_upload_photo, &result);
    if (0 == ret && 0 == result)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    free(base64_buff);
    
    return ret;
}

int platform_upgrade(struct UpgradeInfo *infop)
{
    int ret = -1;
    char url[128] = {0};
    char content[256] = {0};

#ifdef PLATFORMDEBUG
    snprintf(url, sizeof(url) - 1, 
                "http://%s/index.php?action=upgrade", 
                HTTP_SERVER_IP);
#else
    ;
#endif

    /* 生产json数据 */
    //jsonString = create_upgrade_json_string();
    snprintf(content, sizeof(content) - 1, "data=%s", "111");
    
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

static int g_exit_flag = 0;

void quit(int no)
{	
    TRACE("\r\n* System will exit by ... (Ctrl ^ C). %s %d\r\n\r\n", MDL);
	g_exit_flag = 1;
}

int main(int argc, char **argv)
{       
    int ret = -1;
    int auth_flag = 0;
    int login_flag = 0;

    signal(SIGINT, quit);
    signal(SIGPIPE, SIG_IGN);
    int count = 0;
    
    while (1)
    {
        if (g_exit_flag == 1)
        {
            break;
        }

        count++;
        
        if (auth_flag == 0)
        {
           ret = platform_auth();
           if (0 == ret)
           {
                auth_flag = 1;
           }
        }

        if (auth_flag == 1 && login_flag == 0)
        {
            ret = platform_login();
            if (0 == ret)
            {
                login_flag = 1;
            }
        }

        if (auth_flag == 1 && login_flag == 1)
        {
            if (count % 2 == 0)
            {
                platform_heartbeat();
            }
            else
            {
                platform_upload_photo();    
            }   
        }

        sleep(2);
    }
    
    return 0;
}

