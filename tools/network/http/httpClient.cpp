
#include "json/json.h"
#include "httpClient.h"
#include "netPlatform.h"
#include "http.h"

using namespace Json;
using namespace std;



static const char * S_pos_head = "POST %s HTTP/1.0\r\n" \
                                 "Accept: */*\r\n" \
                                 "HOST: %s\r\n" \
                                 "Content-Length: %d\r\n" \
                                 "Content-Type: application/x-www-form-urlencoded\r\n\r\n";


static CONFIG_LOGIN g_strCfgLogin;
static CONFIG_PLATFORM g_strCfgAlgo;

static int string_to_json(char *content, Value &jsonValue)
{
    Reader jsonReader;  //json解析 
    string Str = string(content);
    if (!jsonReader.parse(Str, jsonValue)) //解析出json放到json中区  
    {
        return -1;
    }

    return 0;
}


int http_socket_create(HTTP_CLIENT *http_client, const char *host_ip, const int port)
{
    if (http_client == NULL) {
        printf("create socket fail, param is NULL. \r\n");
        return (-1);
    }

    memset(http_client, 0, sizeof(HTTP_CLIENT));

    http_client->serv_port = port;
    strncpy(http_client->serv_addr, host_ip, sizeof(http_client->serv_addr) - 1);
    
    http_client->addr.sin_family        = AF_INET;
    http_client->addr.sin_port          = htons(http_client->serv_port);
    http_client->addr.sin_addr.s_addr   = inet_addr(http_client->serv_addr);
    //inet_pton(AF_INET, http_client->serv_addr, (void *)&http_client->addr.sin_addr);
    
    http_client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (http_client->sockfd == -1) {
        printf("socket error, %s \r\n", strerror(errno));
        return (-1);
    }

    int flag = 1;
    socklen_t len = sizeof(flag);
    if (setsockopt(http_client->sockfd, SOL_SOCKET, SO_REUSEADDR, 
                (const char *)&flag, len) == -1)
    {
        printf("setsockopt error, %s \r\n", strerror(errno));
        http_socket_close(http_client);
        return (-1);
    }

    http_client->connected = 0;

    return (0);
}

int http_socket_close(HTTP_CLIENT *http_client)
{
    close(http_client->sockfd);
    http_client->connected = 0;
    return (0);
}


int http_connect(HTTP_CLIENT *http_client)
{
    int ret = -1;
    socklen_t length = 0;
    
    if (http_client->connected) {
        printf("http client already connected. %s %d\r\n", __FUNCTION__, __LINE__);
        http_socket_close(http_client);
        return (-1);
    }

    length = sizeof(struct sockaddr);
    ret = connect(http_client->sockfd, (struct sockaddr *)&http_client->addr, length);
    if (ret == -1)
    {
        printf("connect server fail(%s). %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        http_socket_close(http_client);
        return (-1);
    }

    unsigned long ul = 1;
    ret = ioctl(http_client->sockfd, FIONBIO, &ul);
    if (ret == -1)
    {
        printf("set ioctl fail(%s). %s %d\r\n", strerror(errno), __FUNCTION__, __LINE__);
        http_socket_close(http_client);
        return (-1);
    }

    /* Http client connected..... */
    http_client->connected = 1;
    
    return (0);        
}

int http_send_request(HTTP_CLIENT *http_client, void *buffer, size_t length)
{
    size_t      left;
    ssize_t     send_len;
    char        *ptr;

    ptr  = (char *)buffer;
    left = length;

    while (left > 0)
    {
        send_len = send(http_client->sockfd, ptr, left, 0);
        if (send_len < 0) { 
            if (errno == EWOULDBLOCK) {
                printf("Error(EWOULDBLOCK): Tcp send message return: %d \r\n", send_len);
                send_len = 0;
                usleep(20 * 1000);
            } else {
                printf("Error(%s): Tcp send message return: %d \r\n", strerror(errno), send_len);
                return (-1);
            }
        }

        left -= send_len;
		ptr  += send_len;
    }
    
    return (length);
}


int http_recv_response(HTTP_CLIENT *http_client, HTTP_RESPONSE *http_response)
{
    int    ret = 0;
    int    rc  = 0;
    int    offset = 0;
    int    timewait = 0;
    size_t length = 0;
    fd_set rdset, exset;
    char   *buffer = NULL;
    time_t tt = 0;
    struct timeval tv = {0};
    
    tt        = time(NULL);
    timewait  = http_response->time_wait;
    buffer    = http_response->recv_data;
    length    = http_response->recv_max_len;
    
    if (length == 0) {
        length = sizeof(http_response->recv_data);
    }
    
    while (length > 0)
    {
        FD_ZERO(&rdset);
        FD_SET(http_client->sockfd, &rdset);
        FD_ZERO(&exset);
        FD_SET(http_client->sockfd, &exset);
        tv.tv_sec	= timewait;
  	    tv.tv_usec	= 0;
        
        ret = select(http_client->sockfd + 1, &rdset, NULL, &exset, &tv);
        if (ret == 0) {
            printf("select time out, return. %s %d\r\n", __FUNCTION__, __LINE__);
            return (-1);
        } else if (ret < 0 || FD_ISSET(http_client->sockfd, &exset)) {
            printf("select error, return. %s %d\r\n", __FUNCTION__, __LINE__);
            return (-1);
        } else {
            ; /* ok, do noting */
        }

        rc = recv(http_client->sockfd, (char *)buffer + offset, length, 0);
        printf("----------- rc: %d \r\n", rc);
        if (rc < 0) 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
				printf("End of response. \r\n");
			}

            return (-1);
        }
        else if (rc == 0)   /* 服务器关闭连接 */
        {
            printf("server close connect...... \r\n");
            break;    
        }
        else 
        {
            length -= rc;
            offset += rc;
        }

        if (time(NULL) - tt > timewait + 1) {
            printf("recv response time out, break. %s %d\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    return (0);
}


void http_parse_response_head(HTTP_RESPONSE *http_response, char *head_line)
{
    int i;
    char *token = NULL;
    
    if ((token = strstr(head_line, "HTTP/1.1")) != NULL) 
    {
        token += strlen("HTTP/1.1");
        i = 1;
        while (*(token + i) == ' ') ++i;
        http_response->return_code = atoi(token + i);
        printf("return code: %d, \r\n", http_response->return_code);
    }
    else if ((token = strstr(head_line, "Content-Length:")) != NULL) 
    {
        token += strlen("Content-Length:");
        i = 1;
        while (*(token + i) == ' ') ++i;
        http_response->content_len = atoi(token + i);
        printf("content len: %d, \r\n", http_response->content_len);
    }
    else if ((token = strstr(head_line, "Content-Range:")) != NULL) {
        token += strlen("Content-Range:");
        i = 1;
        while (*(token + i) != '/') ++i;
        ++i;
        http_response->total_len = atoi(token + i);
        printf("total len: %d, \r\n", http_response->total_len);
    }
}

int http_parse_response(HTTP_RESPONSE *http_response)
{
    int ret = -1;
    char head[1024] = {0};
    char *content = NULL;
    char *ptr = NULL;
    char *buffer = http_response->recv_data;
    
    ptr = strstr(buffer, "\r\n\r\n");
    if (ptr == NULL) 
    {
        printf("recv buffer error. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    /* Copy response header */
    memcpy(head, buffer, ptr - buffer);

    /* Print response header */
    char *pptr = NULL;  
    char *token = strtok_r(head, "\r\n", &pptr);
    while (token != NULL) {
        printf(">%s \r\n", token);
        http_parse_response_head(http_response, token);
        token = strtok_r(NULL, "\r\n", &pptr);
    }
    
    /* Skip the /r/n/r/n */
    ptr += 4;

    printf("------> LEN %d %s %d\r\n", strlen(ptr), __FUNCTION__, __LINE__);
    
    int len = strlen(ptr) + 1;
    
    content = (char *)malloc(len);
    if (content == NULL) {
        printf("parse malloc error. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    memset(content, 0, len);
    strncpy(content, ptr, len - 1);

    printf(">%s %s %d\r\n", content, __FUNCTION__, __LINE__);

    if (http_response->parseFunc)
    {
        size_t content_len = strlen(content);
        ret = http_response->parseFunc(content, content_len, http_response->args);
        if (ret != content_len) 
        {
            printf("parse content data error %d. %s %d\r\n", ret, __FUNCTION__, __LINE__);
            free(content);
            return (-1);
        }
    }
    
    free(content);
    
    return (0);
}

size_t http_parse_login(void *buffer, size_t size, void *arg)
{
    CONFIG_LOGIN *strLogin = (CONFIG_LOGIN *)arg;
    int returnCode = -1;
    Value jsonReceive;    //表示一个json格式的对象
    size_t data_size = size;
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


size_t http_parse_config(void *buffer, size_t size, void *stream)
{
    CONFIG_PLATFORM *config = (CONFIG_PLATFORM *)stream;
    int data_size = size;
    int returnCode = -1;
    string data;
    Value jsonValue;    //表示一个json格式的对象
    char *pBuffer = (char *)buffer;

    printf("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        config->result = 2;
        goto End;
    }

    if (string_to_json(pBuffer, jsonValue) != 0)
    {
        config->result = 2;
        goto End;
    }

    if (jsonValue["success"].isNull() ||
        !jsonValue["success"].isString())
    {
        config->result = 2;
        goto End;
    }

    returnCode = atoi(jsonValue["success"].asCString());
    if (returnCode != 10000)
    {
        if (returnCode == 10005)
        {
            config->result = 3;     /* 没有最新配置 */
            goto End;
        }
        else
        {
            config->result = 2;
            goto End;
        }
    }

    if (jsonValue["configId"].isNull()
        || !jsonValue["configId"].isString()
        || jsonValue["leftPoint"].isNull()
        || !jsonValue["leftPoint"].isString()
        || jsonValue["rightPoint"].isNull()
        || !jsonValue["rightPoint"].isString()
        || jsonValue["topPoint"].isNull()
        || !jsonValue["topPoint"].isString()
        || jsonValue["bottomPoint"].isNull()
        || !jsonValue["bottomPoint"].isString()
        || jsonValue["direction"].isNull()
        || !jsonValue["direction"].isString()
        || jsonValue["height"].isNull()
        || !jsonValue["height"].isString()
        || jsonValue["init"].isNull()
        || !jsonValue["init"].isString())
    {
        config->result = 2;
        goto End;
    }

    /* 获取配置成功 */
    config->result = 1;
    config->configId    = atoi(jsonValue["configId"].asCString());
    config->init        = atoi(jsonValue["init"].asCString());
    config->leftPoint   = atoi(jsonValue["leftPoint"].asCString());
    config->topPoint    = atoi(jsonValue["topPoint"].asCString());
    config->rightPoint  = atoi(jsonValue["rightPoint"].asCString());
    config->bottomPoint = atoi(jsonValue["bottomPoint"].asCString());
    config->direction   = atoi(jsonValue["direction"].asCString());
    config->height      = atoi(jsonValue["height"].asCString());

End:
    return data_size;
}

size_t http_parse_download(void *buffer, size_t size, void *stream)
{
    size_t data_size = size;
    
    return data_size;
}


int http_post(HTTP_CLIENT *http_client, HTTP_REQUEST *http_request, HTTP_RESPONSE *http_response)
{
    int ret = -1;
    char post[256] = {0};
    char host[128] = {0};
    char accept[128] = {0};
    char content_length[128] = {0};
    char content_type[128] = {0};
    char *send = NULL;

    char *data      = http_request->send_data;
    char *url       = http_request->url;
    size_t data_len = strlen(http_request->send_data);

    char *token = strstr(url, "index.php");
    if (token == NULL) {
        printf("http post url error. \r\n");
        return (-1);
    }

    token--;

    printf("token: %s \r\n", token);
    
    snprintf(post, sizeof(post) - 1, 
            "POST %s HTTP/1.0\r\n", token);

    snprintf(host, sizeof(host) - 1, 
            "HOST: %s\r\n", http_client->serv_addr);

    snprintf(accept, sizeof(accept) - 1, 
            "Accept: */*\r\n");

    snprintf(content_length, sizeof(content_length) - 1, 
            "Content-Length: %d\r\n", data_len);
    
    snprintf(content_type, sizeof(content_type) - 1, 
            "Content-Type: application/x-www-form-urlencoded\r\n\r\n"); /* 注意: 最后这里要空一行 */

    size_t len = strlen(post) + strlen(host) + strlen(accept) + 
              strlen(content_length) + strlen(content_type) + data_len + 1;

    printf("len: %d \r\n", len);

    send = (char *)malloc(len);
    if (send == NULL) {
        printf("http post send buffer malloc error. \r\n");
        return (-1);
    }

    memset(send, 0, len);
    strcpy(send, post);
    strcat(send, host);
    strcat(send, accept);
    strcat(send, content_length);
    strcat(send, content_type);
    strcat(send, data);

    if (!http_client->connected)
    {
        printf("http disconnected, %d %s %d\r\n", http_client->connected, __FUNCTION__, __LINE__);
        ret = http_connect(http_client);
        if (ret == -1) {
            printf("http connect error. %s %d\r\n", __FUNCTION__, __LINE__);
            free(send);
            return (-1);
        }
    }

    ret = http_send_request(http_client, send, len);
    if (ret != len) 
    { 
        printf("http send request error, ret=%d %s %d\r\n", ret, __FUNCTION__, __LINE__);
        free(send);
        http_socket_close(http_client);
        return (-1);
    }

    printf("send post request: %s %d \r\n", __FUNCTION__, __LINE__);
    printf("%s %s %d \r\n", send, __FUNCTION__, __LINE__);

    free(send);
    send = NULL;

    if (http_recv_response(http_client, http_response) == -1) 
    {
        printf("http recv response error. %s %d\r\n", __FUNCTION__, __LINE__);
        http_socket_close(http_client);
        return (-1);
    }
    
    /* Receive http server response data */
    //printf("%s \r\n", recv_buff);
    
    if (http_parse_response(http_response) == -1)
    {
        printf("http parse response error. %s %d\r\n", __FUNCTION__, __LINE__);
        http_socket_close(http_client);
        return (-1);
    }

    http_socket_close(http_client);
        
    return (0);
}

char *http_build_download_head(char *pget, int range_low, int range_height, char *addr, int port)
{
    char *send = NULL;
    char get[128] = {0};
    char host[128] = {0};
    char range[128] = {0};
    char accept[128] = {0};
    
    snprintf(get, sizeof(get) - 1, 
            "GET %s HTTP/1.1\r\n", pget);
    
    snprintf(host, sizeof(host) - 1, 
            "HOST: %s:%d\r\n", addr, port);

    snprintf(accept, sizeof(accept) - 1, 
            "Accept: */*\r\n\r\n");         /* 注意: 最后这里要空一行 */

    size_t total_len = strlen(get) + strlen(range) + strlen(host) + strlen(accept) + 1;

    printf("get request total len: %d %s %d\r\n", total_len, __FUNCTION__, __LINE__);

    send = (char *)malloc(total_len);
    if (send == NULL) {
        printf("http get send buffer malloc error. \r\n");
        return NULL;
    }

    memset(send, 0, total_len);
    strcpy(send, get);
    strcat(send, range);
    strcat(send, host);
    strcat(send, accept);

    return send;
}

int http_get(HTTP_CLIENT *http_client, HTTP_REQUEST *http_request, HTTP_RESPONSE *http_response)
{
    int ret = -1;
    
    char head[1024] = {0};
    char *send = NULL;

    char *token = strstr(http_request->url, "myCount");
    if (token == NULL) {
        printf("http post url error. \r\n");
        return (-1);
    }

    token--;

    printf("token: %s \r\n", token);

    send = http_build_download_head(token, 
                                    http_request->range_low,
                                    http_request->range_height,
                                    http_client->serv_addr, 
                                    http_client->serv_port);
    if (send == NULL) 
    {
        printf("build http download head error. \r\n");
        http_socket_close(http_client);
        return (-1);
    }

    if (!http_client->connected)
    {
        printf("http disconnected, %d %s %d\r\n", http_client->connected, __FUNCTION__, __LINE__);
        ret = http_connect(http_client);
        if (ret == -1) {
            printf("http connect error. %s %d\r\n", __FUNCTION__, __LINE__);
            free(send);
            return (-1);
        }
    }

    size_t send_len = strlen(send);
    ret = http_send_request(http_client, send, send_len);
    if (ret != send_len) 
    { 
        printf("http send request error, ret=%d %s %d\r\n", ret, __FUNCTION__, __LINE__);
        free(send);
        http_socket_close(http_client);
        return (-1);
    }

    printf("----------- send post request ----------- %s %d \r\n", __FUNCTION__, __LINE__);
    printf("%s\r\n", send);
    printf("----------- send post request ----------- %s %d \r\n", __FUNCTION__, __LINE__);
    
    free(send);
    send = NULL;

    ret = http_recv_response(http_client, http_response);
    if (ret == -1) 
    {
        printf("http recv response error. %s %d\r\n", __FUNCTION__, __LINE__);
        http_socket_close(http_client);
        return (-1);
    }
    
    /* Receive http server response data */
    //printf("%s \r\n", http_response->recv_data);
    
    if (http_parse_response(http_response) == -1) 
    {
        printf("http parse response error. %s %d\r\n", __FUNCTION__, __LINE__);
        http_socket_close(http_client);
        return (-1);
    }

    /* 重新开始下载文件 */
    if (http_response->return_code == 200 &&
        http_response->content_len > 0)
    {   
        //g_total_size = http_response->content_len;

#if 0
        if (g_total_size % MAX_RECV_SIZE == 0) {
            g_down_count = g_total_size / MAX_RECV_SIZE;
        } else {
            g_down_count = g_total_size / MAX_RECV_SIZE + 1;
        }

        send = http_build_download_head(token, int range_low, int range_height, char * addr, int port)
#endif   
    }
    else if (http_response->return_code == 206 &&
        http_response->content_len > 0)
    {
        /* 下载数据 */
        int dw_len = http_response->content_len;
        int tl_len = http_response->total_len;
        int hv_len = tl_len - dw_len;

        printf("dw_len: %d, tl_len: %d, hv_len: %d \r\n", dw_len, tl_len, hv_len);
#if 0
        while (dw_len > 0) {
            send = http_build_download_head(token, 
                                    hv_len,
                                    hv_len + 1440 - 1,
                                    http_client->serv_addr, 
                                    http_client->serv_port);

            if (send == NULL) 
            {
                printf("build http download head error. \r\n");
                http_socket_close(http_client);
                return (-1);
            }

            if (!http_client->connected)
            {
                printf("http disconnected, %d %s %d\r\n", http_client->connected, __FUNCTION__, __LINE__);
                ret = http_connect(http_client);
                if (ret == -1) {
                    printf("http connect error. %s %d\r\n", __FUNCTION__, __LINE__);
                    free(send);
                    return (-1);
                }
            }

            size_t send_len = strlen(send);
            ret = http_send_request(http_client, send, send_len);
            if (ret != send_len) 
            { 
                printf("http send request error, ret=%d %s %d\r\n", ret, __FUNCTION__, __LINE__);
                free(send);
                http_socket_close(http_client);
                return (-1);
            }

            printf("----------- send post request ----------- %s %d \r\n", __FUNCTION__, __LINE__);
            printf("%s\r\n", send);
            printf("----------- send post request ----------- %s %d \r\n", __FUNCTION__, __LINE__);
            
            free(send);
            send = NULL;

            memset(http_response->recv_data, 0, sizeof(http_response->recv_data));
            http_response->content_len = 0;
            http_response->total_len = 0;
            http_response->return_code = 400;
            http_response->time_wait = 100;
            http_response->recv_max_len = 1439;
            ret = http_recv_response(http_client, http_response);
            if (ret == -1) 
            {
                printf("http recv response error. %s %d\r\n", __FUNCTION__, __LINE__);
                http_socket_close(http_client);
                return (-1);
            }
            
            /* Receive http server response data */
            printf("%s \r\n", http_response->recv_data);
            
            if (http_parse_response(http_response) == -1) 
            {
                printf("http parse response error. %s %d\r\n", __FUNCTION__, __LINE__);
                http_socket_close(http_client);
                return (-1);
            }
        }
#endif
        
    }

    http_socket_close(http_client);
        
    return (0);
    
}


int http_login()
{
    int ret = -1;
    HTTP_CLIENT http_client;
    HTTP_REQUEST http_request;
    HTTP_RESPONSE http_response;

    if (http_socket_create(&http_client, HTTP_SERVER_IP, HTTP_SERVER_PORT) == -1) 
    {
        printf("http create socket error. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    if (http_connect(&http_client) == -1) 
    {
        printf("http connect error. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }
    
    char url[256] = {0};
    snprintf(url, sizeof(url) - 1, "%s", "http://%s/index.php?action=login");
    const char *string = "data={\"imei\":\"1011411000014\",\"ip\":\"192.168.0.22\",\"alias\":\"test0014\"}";
    
    char data[256] = {0};
    snprintf(data, sizeof(data) - 1, "%s", string);

    memset(&http_request, 0, sizeof(http_request));
    strncpy(http_request.url, url, sizeof(http_request.url) - 1);
    strncpy(http_request.send_data, data, sizeof(http_request.send_data) - 1);

    memset(&http_response, 0, sizeof(http_response));
    http_response.parseFunc = http_parse_login;
    http_response.args      = malloc(sizeof(struct configLogin));
    if (http_response.args == NULL) {
        return (-1);
    }

    memset(http_response.args, 0, sizeof(struct configLogin));
    ((struct configLogin *)http_response.args)->result = -1;
    
    http_response.time_wait = 10;
    
    ret = http_post(&http_client, &http_request, &http_response);
    struct configLogin *pargs = (struct configLogin *)http_response.args;

    if (ret == 0 && pargs->result == 0)
    {
        printf("http login post request success. %s %d\r\n", __FUNCTION__, __LINE__);

        memset(&g_strCfgLogin, 0, sizeof(g_strCfgLogin));
        strncpy(g_strCfgLogin.token, pargs->token, sizeof(g_strCfgLogin.token) - 1);
        
        free(http_response.args);
        return (0);
    }

    printf("http login post request failure. %s %d\r\n", __FUNCTION__, __LINE__);

    free(http_response.args);
    
    return (-1);
}

int http_check_config()
{
    int ret = -1;
    HTTP_CLIENT http_client;
    HTTP_REQUEST http_request;
    HTTP_RESPONSE http_response;

    if (http_socket_create(&http_client, HTTP_SERVER_IP, HTTP_SERVER_PORT) == -1) 
    {
        printf("http create socket error. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    if (http_connect(&http_client) == -1)
    {
        printf("http connect error. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }
    
    char url[256] = {0};
    snprintf(url, sizeof(url) - 1, "%s", "http://%s/index.php?action=config");
    
    char data[256] = {0};
    snprintf(data, sizeof(data) - 1, "imei=%s&token=%s", "1011411000014", g_strCfgLogin.token);

    memset(&http_request, 0, sizeof(http_request));
    strncpy(http_request.url, url, sizeof(http_request.url) - 1);
    strncpy(http_request.send_data, data, sizeof(http_request.send_data) - 1);

    memset(&http_response, 0, sizeof(http_response));
    http_response.parseFunc = http_parse_config;
    http_response.time_wait = 10;
    http_response.args      = malloc(sizeof(struct configPlatform));
    if (http_response.args == NULL) {
        printf("malloc response args error. \r\n");
        return (-1);
    }

    memset(http_response.args, 0, sizeof(struct configPlatform));
    ((struct configPlatform *)http_response.args)->result = -1;
    
    ret = http_post(&http_client, &http_request, &http_response);

    CONFIG_PLATFORM *pargs = (struct configPlatform *)http_response.args;
    
    if (ret == 0 && 
        (pargs->result == 1 || 
        pargs->result == 3))
    {
        printf("http check config post request success. %s %d\r\n", __FUNCTION__, __LINE__);
        free(http_response.args);
        return (0);
    }

    printf("http check config post request failure. %s %d\r\n", __FUNCTION__, __LINE__);

    free(http_response.args);
    
    return (-1);
}   



/*****************************************************************************/

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

int wselect(int sock, int timeout)
{
    int ret = -1;
    fd_set rd_set, ex_set;
    struct timeval tv = {0};

    FD_ZERO(&rd_set);
    FD_SET(sock, &rd_set);
    FD_ZERO(&ex_set);
    FD_SET(sock, &ex_set);
    tv.tv_sec = (timeout / 1000);
	tv.tv_usec = 0;

    ret = select(sock + 1, &rd_set, NULL, &ex_set, &tv);
    if (ret == 0) {
        printf("select time out, return. %s %d\r\n", __FUNCTION__, __LINE__);
        return DVS_ERR_TIMEOUT;
    } else if (ret < 0 || FD_ISSET(sock, &ex_set)) {
        printf("select error, return. %s %d\r\n", __FUNCTION__, __LINE__);
        return DVS_ERR_SELECTFAL;
    } else {
        if (FD_ISSET(sock, &rd_set) > 0) {
            return MRJE_OK;
        }
    }

    return DVS_ERR_SELECTFAL;
}


#define MRJ_HTTPID			"HTTP/1.1"
#define MRJ_CONTENTLEN		"Content-Length:"
#define MRJ_ENDOFHEAD       "\r\n\r\n"

static MRJcode parse_response_head(struct SessionHandle *data)
{
	int i = 1;
	MRJcode status = MRJE_UNKNOWN;
	char *pstatus = p_strcasestr(data->response_head, MRJ_HTTPID);
	char *plen = p_strcasestr(data->response_head, MRJ_CONTENTLEN);
    char *peoh = p_strcasestr(data->response_head, MRJ_ENDOFHEAD);

    data->return_code = 404;    
    
	if ( pstatus ) {
		pstatus += strlen(MRJ_HTTPID);
		i = 1;	
		while ( *(pstatus+i) == ' ' ) ++i;
		data->return_code = atoi(pstatus+i);
	} else
		data->return_code = 0;

	if ( plen ) {
		plen += strlen(MRJ_CONTENTLEN);
		i = 1;
		while ( *(plen+i) == ' ' ) ++i;
		data->file_length = (unsigned int)atoi(plen+i); 
	} else
		data->file_length = 0;

    if ( peoh ) {
        peoh += strlen(MRJ_ENDOFHEAD);
        if (data->file_length && data->file_length > sizeof(data->recv_buf) - 1) {
            return MRJE_OUT_OF_MEMORY;    
        } else if (strlen(peoh) > sizeof(data->recv_buf) - 1) {
            return MRJE_OUT_OF_MEMORY;
        } else {
            if (data->file_length > 0) {
                strncpy(data->recv_buf, peoh, data->file_length);
                data->recv_len = data->file_length;
            } else {
                strncpy(data->recv_buf, peoh, strlen(peoh));
                data->recv_len = strlen(peoh);
            }

            status = MRJE_OK;
        }
    }

	return status;
}


MRJcode connect_host(mrj_socket_t sock, struct sockaddr_in *remote_addr)
{
	if ( -1 == sock ) {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if ( -1 == http->sock )
			return MRJE_CRSOCKFAL;
	}

	if ( connect(sock, (struct sockaddr *)remote_addr, sizeof(struct sockaddr)) == -1 ) {
		switch ( errno ) {
		case EALREADY:	/* socket为不可阻断且先前的连线操作还未完成 */	
		case EBADF:		/* 参数sockfd 非合法socket处理代码 */
		case EISCONN: 	/* 参数sockfd的socket已是连线状态 */
			closesocket(sock);
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if ( -1 == sock )	
				return MRJE_CRSOCKFAL;
			if ( connect(sock, (struct sockaddr *)remote_addr, sizeof(struct sockaddr)) == -1 )
				return MRJE_NOTCONNHOST;
			break;
		default:
			return MRJE_NOTCONNHOST;
		}	
	}

    unsigned long ul = 1;
    if ( -1 == ioctl(sock, FIONBIO, &ul)) {
        TRACE("set ioctl fail(%s). %s %d\r\n", strerror(errno), MDL);
        closesocket(sock);
        return MRJE_IOCTLFAL;
    }
    
	return MRJE_OK;
}

MRJcode send_hcmd(struct SessionHandle *data, const char *hcmd, int n, int recv_flag)
{
	MRJcode res = MRJE_UNKNOWN;
	int over_flag = 0;
	int per_bytes = 0;
	int already_bytes = 0;
    time_t now = 0;
	char ch = 0;
	char *response_head = data->response_head;
    
	res = connect_host(data->sock, &data->remote_addr);
	if ( res != MRJE_OK ) {
        if (data->set.verbose)
            TRACE("connect host error. %s %d\r\n", MDL);
        return res;
	}

	/* 发送请求报头 */
	do {
		per_bytes = send(data->sock, hcmd+already_bytes, n-already_bytes, 0);
        if ( per_bytes < 0) {
            if ( errno == EWOULDBLOCK ) {
                per_bytes = 0;
                usleep(20 * 1000);
		    } else {
		        TRACE("%s \r\n", strerror(errno), MDL);
		        return (already_bytes < n ? DVS_ERR_SNDREQERR : MRJE_OK);
		    }
        } else if ( 0 == per_bytes ) {
            TRACE("%s %d\r\n", MDL);
            return DVS_ERR_DISCONNHOST;
		} else {
            already_bytes += per_bytes; 
		}
	} while ( already_bytes < n );
	
	/* 获取响应报头 */
	data->recv_len = 0;;
    now = time(NULL);
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
			data->response_head[data->recv_len++] = ch;
		} else if ( 0 == per_bytes ) {
		    printf("server close connect...... \r\n");
			//return DVS_ERR_DISCONNHOST;
			break;
		} else { 
			break;
		}

        if (time(NULL) - now > data->set.timeout + 1) {
            printf("recv response time out, break. %s %d\r\n", __FUNCTION__, __LINE__);
            return DVS_ERR_TIMEOUT;
        }
        
	} while ( over_flag < recv_flag );          /* 接收到4个head标签后，退出接收 */

    printf("head %s \r\n", data->response_head);
    
	res = parse_response_head(data);
    if (res != MRJE_OK) {
        TRACE("parse response head error. %s %d\r\n", MDL);
        return res;
    }
    
    TRACE("ret: %d, code: %d, len: %d, recv: %s %s %d\r\n", res, data->return_code, 
                        data->file_length, data->recv_buf, MDL);
    
	switch ( data->return_code ) {
		case 200: return MRJE_OK;
        case 206: return MRJE_OK;
		case 400: return DVS_ERR_SERVERE400;
		case 401: return DVS_ERR_SERVERE401;
		case 403: return DVS_ERR_SERVERE403;
		case 404: return DVS_ERR_SERVERE404;
		case 500: return DVS_ERR_SERVERE500;
		case 503: return DVS_ERR_SERVERE503;
		default: return MRJE_UNKNOWN;
	}

    return res;
}


DVS_Http *dvshttp_create()
{
    DVS_Http *http = (DVS_Http *)malloc(sizeof(DVS_Http));

    if (http) {
        memset(http, 0, sizeof(DVS_Http));
        http->sock = -1;
        return http;
    } else {
        return NULL;
    }
}

/******************************************************************************/
static MRJcode parse_url(struct SessionHandle *data)
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
    data->port = 0;

    /* 已经找到web, 先判断是否存在':' */
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

    if ( flag != 1 ) return MRJE_URLINLEGAL;    

    if ( 0 == data->port ) data->port = MRJ_DEF_PORT;	
    
	printf("web: %s, port: %d \r\n", data->host, data->port);

    flag = 0;
    start = itr;
    while ((ch = *(data->set.url + itr)) != '\0') {
        itr++;
    }

    memcpy(data->remote_path, data->set.url + start, itr - start);
    data->remote_path[itr - start] = '\0';
    flag = 1;

    printf("remote_path: %s \r\n", data->remote_path);
    
#if 0    

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


static MRJcode setstropt(char *charp, char *s)
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

MRJcode http_setopt(struct SessionHandle *data, MRJHTTPoption option,
                     va_list param)
{
    char *argptr;
    MRJcode result = MRJE_OK;
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
        data->set.send = va_arg(param, void *);
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
        if(arg == CURL_HTTP_VERSION_2_0)
            return MRJE_UNSUPPORTED_PROTOCOL;
#endif
        data->set.httpversion = arg;
        break;
        
    default:
        result = MRJE_UNKNOWN_OPTION;
        break;
    }

    return result;
}

static MRJcode http_load_url(struct SessionHandle *data)
{
    MRJcode res = MRJE_OK;

    res = parse_url(data);

    if (res != MRJE_OK)
        return res;
    
    memset(&data->remote_addr, 0, sizeof(data->remote_addr));
	data->remote_addr.sin_family = AF_INET;
	data->remote_addr.sin_port = htons(data->port);
	data->remote_addr.sin_addr.s_addr = inet_addr(data->host);
		
    return MRJE_OK;
}


static MRJcode http_transfer(struct SessionHandle *data)
{
    int n = 0;
    size_t bytes;
    MRJcode res = MRJE_OK;

    bytes = strlen(data->set.send);
    if (data->set.httpreq == HTTPREQ_POST) {
        n = snprintf(data->request_head, sizeof(data->request_head) -1, 
                     S_pos_head, data->remote_path, 
                     data->host, bytes);
    } else {
        ;
    }

    TRACE("REQUEST head: %s %s %d\r\n", data->request_head, MDL);

    n = n + bytes + 1;

    char *send = (char *)malloc(n * sizeof(char));
    if (!send) {
        TRACE("malloc error. %s %d\r\n", MDL);
        return MRJE_OUT_OF_MEMORY;
    }

    memset(send, 0, n);
    strcpy(send, data->request_head);
    strcat(send, data->set.send);

    printf("send: %s %s %d\r\n", send, __FUNCTION__, __LINE__);
    printf("n: %d %s %d\r\n", n, __FUNCTION__, __LINE__);
    
    res = send_hcmd(data, send, n, 6);

    printf("ret: %d %s %d\r\n", res, MDL);
    
    if (MDL == MRJE_OK) {
        if (data->return_code == 200) {
            data->set.fwrite_func(data->recv_buf, 1, data->recv_len, data->set.event_args);
        }
    }

    free(send);
    
    return res;
}


static MRJcode http_perform(struct SessionHandle *data, bool events)
{
    MRJcode res = MRJE_OK;

    res = http_load_url(data);
    if (res != MRJE_OK) {
        //if (data->set.verbose)
            TRACE("Http load url error. %s %d\r\n", MDL);
        return res;
    }

    res = http_transfer(data);
    if (res != MRJE_OK) {
        //if (data->set.verbose)
            TRACE("Http transfer error. %s %d\r\n", MDL);
        return res;
    } 
    
    return res;
}

/******************************************************************************/

MRJHTTP *mrjhttp_init(void)
{
    MRJcode res;
    struct SessionHandle *data;

    data = (struct SessionHandle *)malloc(sizeof(struct SessionHandle));
    if (!data) {
        TRACE("Malloc SessionHandle fail. %s %d\r\n", MDL);
        return NULL;
    }

    memset(data, 0, sizeof(struct SessionHandle));
    data->sock = -1;

    return data;
}

MRJcode http_close(struct SessionHandle *data)
{
    if (data->sock != -1) {
        close(data->sock);
        data->sock = -1;
    }

    if (data->set.send) {
        free(data->set.send);
        data->set.send = NULL;
    }

    return MRJE_OK;
}

void mrjhttp_cleanup(MRJHTTP *handle)
{
    struct SessionHandle *data = (struct SessionHandle *)handle;

    if (!data)
        return;

    http_close(data);
}

MRJcode mrjhttp_setopt(MRJHTTP *http,  MRJHTTPoption tag, ...) 
{
    va_list arg;
    struct SessionHandle *data = http;
    MRJcode res;

    if (!http)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    va_start(arg, tag);

    res = http_setopt(data, tag, arg);

    va_end(arg);

    return res;
}

MRJcode mrjhttp_perform(MRJHTTP *handle)
{
    return http_perform(handle, FALSE);
}


#if 0
int dvshttp_load_url(DVS_Http *http, const char *url)
{		
	if ( parse_url(url, http) == MRJE_OK ) {
        
		memset(&http->remote_addr, 0, sizeof(http->remote_addr));
		http->remote_addr.sin_family = AF_INET;
		http->remote_addr.sin_port = htons(http->port);
		http->remote_addr.sin_addr.s_addr = inet_addr(http->host);
		
		return MRJE_OK;
	} else
		return MRJE_URLINLEGAL;
}


int dvshttp_load_data(DVS_Http *http, const char *data, int time_wait, int range_low, int range_height, 
                        DVS_HTTPEVENT event_func, void *event_args)
{
    http->time_wait = time_wait;
    http->range_low = range_low;
    http->range_hight = range_height;
    
    if (data) strncpy(http->send_buf, data, sizeof(http->send_buf) - 1);
    if (event_func) http->event_func = event_func;
    if (event_args) http->event_args = event_args;

    return MRJE_OK;
}


int dvshttp_load_host(DVS_Http *http, const char *ip, int port, const char *remote_path, const char *remote_file)
{
	int len;

	memset(&http->remote_addr, 0, sizeof(http->remote_addr));
	http->remote_addr.sin_family = AF_INET;
	http->remote_addr.sin_port = htons(port);
	http->remote_addr.sin_addr.s_addr = inet_addr(ip);
	
	http->port = port;
    if (ip) strncpy(http->host, ip, sizeof(http->host) - 1);
	if (remote_path) strncpy(http->remote_path, remote_path, sizeof(http->remote_path) - 1);
	if (remote_file) strncpy(http->remote_file, remote_file, sizeof(http->remote_file) - 1);

    return MRJE_OK;
}

void dvshttp_free(DVS_Http *http)
{
    if (http) {
	    free(http);
    }
}

int dvshttp_post(DVS_Http *http) 
{
    int n = 0;
	int ret = MRJE_UNKNOWN;
    char *buffer = NULL;
    size_t content_bytes = strlen(http->send_buf);
    
    n = sprintf(http->request_head, S_pos_head, http->remote_path, 
                http->host,/* http->port,*/ content_bytes);

    n = n + content_bytes + 1;

    buffer = (char *)malloc(n * sizeof(char));
    if (buffer == NULL) return DVD_ERR_OUTOFMEMORY;

    memset(buffer, 0, n);
    strcpy(buffer, http->request_head);
    strcat(buffer, http->send_buf);

    printf("buffer: %s %s %d\r\n", buffer, __FUNCTION__, __LINE__);
    printf("n: %d %s %d\r\n", n, __FUNCTION__, __LINE__);
    
    ret = send_hcmd(http, buffer, n, 6);

    printf("ret: errno=%#x %s %d\r\n", ret, __FUNCTION__, __LINE__);
    
    if (ret == MRJE_OK) {
        if (http->return_code == 200) {
            http->event_func(http->recv_buf, http->recv_len, http->event_args);
        }
    }

    free(buffer);
    
    return ret;
}
#endif

size_t dvshttp_parse_longin(char *buffer,
                                 size_t size,
                                 size_t nitems,
                                 void *outstream)
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


int MRJhttp_login(char *url, char *content, MRJ_HTTP_CALLBACK func, void *arg)
{
    MRJcode res = MRJE_OK;
    MRJHTTP *handle = NULL;

    handle = mrjhttp_init();
    if (!handle) {
        TRACE("Mrj http init fail. %s %d\r\n", MDL);
        return (-1); 
    }

    mrjhttp_setopt(handle, MRJOPT_URL, url);
    mrjhttp_setopt(handle, MRJOPT_POSTFIELDS, content);
    mrjhttp_setopt(handle, MRJOPT_WRITEFUNCTION, func);
    mrjhttp_setopt(handle, MRJOPT_WRITEDATA, arg);
    mrjhttp_setopt(handle, MRJOPT_POST, 1L);

    mrjhttp_setopt(handle, MRJOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    mrjhttp_setopt(handle, MRJOPT_TIMEOUT, 30L);
    mrjhttp_setopt(handle, MRJOPT_CONNECTTIMEOUT, 10L);
    
#ifdef CURL_DEBUG
    mrjhttp_setopt(handle, MRJOPT_VERBOSE, 1L);
#endif  

    res = mrjhttp_perform(handle);

    if (res != MRJE_OK) {
        TRACE("http perform error. %s %d\r\n", MDL);
    }

    mrjhttp_cleanup(handle);

    return res;
}


#if 0
int dvshttp_login()
{
    int ret = -1;

    DVS_Http *http = dvshttp_create();
    if ( !http ) {
		printf("create http failed!\n");
		return (-1);
	}

    const char *url = "http://192.168.0.251/index.php?action=login";
    
    if ((ret = dvshttp_load_url(http, url)) != MRJE_OK) {
        printf("load url failed! errno=%#x\n", ret);
        return (-1);	
    }

    const char *data = "data={\"imei\":\"1011411000014\",\"ip\":\"192.168.0.22\",\"alias\":\"test0014\"}";

    struct configLogin config_login;
    config_login.result = -1;
    dvshttp_load_data(http, data, 10, 0, 0, dvshttp_parse_longin, &config_login);

    ret = dvshttp_post(http);
    
    printf("ret: errno=%#x. result: %d. %s %d\r\n", ret, config_login.result, __FUNCTION__, __LINE__);
   
    if (ret == MRJE_OK && config_login.result == 0) {
        printf("http login post request success. %s %d\r\n", __FUNCTION__, __LINE__);
        memset(&g_strCfgLogin, 0, sizeof(g_strCfgLogin));
        strncpy(g_strCfgLogin.token, config_login.token, sizeof(g_strCfgLogin.token) - 1);
        dvshttp_free(http);
        return (0);
    }
    
    printf("http login post request failure. errno=%#x\n %s %d\r\n", ret, __FUNCTION__, __LINE__);

    dvshttp_free(http);
    
    return (-1);
}

#endif

int login_test()
{
    const char *url = "http://192.168.0.251/index.php?action=login";
    const char *data = "data={\"imei\":\"1011411000014\",\"ip\":\"192.168.0.22\",\"alias\":\"test0014\"}";
    struct configLogin config_login;
    config_login.result = -1;

    int ret = MRJhttp_login(url, data, dvshttp_parse_longin, &config_login);

    return ret;
}


    
int main(int argc, char **argv)
{
    int ret = 0;

    /*
    if (ret == 0)
        ret = http_login();
    if (ret == 0)
        ret = http_check_config();
    */
    
    return dvshttp_login();
}

