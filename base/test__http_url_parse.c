
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRACE printf

int http_url_parse(const char *urlp)
{
    if (!urlp)
        return (-1);

    char *token1 = strstr(urlp, "//");
    if (!token1) {
        TRACE("Get domain name fail. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    token1 = token1 + 2;        /* Ìø¹ý "//" */
    
    char *token2 = strchr(token1, '/');
    if (!token2) {
        TRACE("Get domain name fail. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    char domainbuf[128] = {0};
    int len = token2 - token1;
    if (len >= sizeof(domainbuf)) {
        TRACE("Download domain name is too long. %s %d\r\n", __FUNCTION__, __LINE__);
        return (-1);
    }

    memcpy(domainbuf, token1, len);
    TRACE("Download domain name: %s %s %d\r\n", domainbuf, __FUNCTION__, __LINE__);

    return (0);
}

int main()
{
    const char *p = "http://app.meirenji.cn/upload/softVersion/11111111111.bin";
    int ret = http_url_parse(p);

    return ret;
}

