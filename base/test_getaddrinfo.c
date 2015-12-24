
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* 
   getaddrinfo:从本地/etc/hosts, /etc/resolve.conf文件中读取dns配置信息
   如果/etc/hosts文件中已经有待查询域名到ip地址的映射关系，则直接使用
   如果/etc/hosts文件中不存在这种映射关系，则通过/etc/resolve.conf文件中配置的
   DNS服务器地址查询域名对应的地址
   同时注意getaddrinfo函数回导致阻塞
*/
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s hostname\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints;
    struct addrinfo *result, *result_pointer;

    int ret = -1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;

    ret = getaddrinfo(argv[1], NULL, &hints, &result);
    if (ret != 0)
    {
        printf("getaddrinfo:%s \r\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    for (result_pointer = result; result_pointer != NULL; result_pointer = result_pointer->ai_next)
    {
        char hostname[1025] = "";
        ret = getnameinfo(result_pointer->ai_addr, result_pointer->ai_addrlen, 
                hostname, sizeof(hostname), NULL, 0, NI_NUMERICHOST);
        if (ret != 0)
        {
            printf("error in getnameinfo: %s \r\n", gai_strerror(ret));
            continue;
        }
        else
        {
            printf("IP:%s \r\n", hostname);
        }
    }

    freeaddrinfo(result);
    exit(EXIT_SUCCESS);
}

