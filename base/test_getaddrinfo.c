
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
   getaddrinfo:�ӱ���/etc/hosts, /etc/resolve.conf�ļ��ж�ȡdns������Ϣ
   ���/etc/hosts�ļ����Ѿ��д���ѯ������ip��ַ��ӳ���ϵ����ֱ��ʹ��
   ���/etc/hosts�ļ��в���������ӳ���ϵ����ͨ��/etc/resolve.conf�ļ������õ�
   DNS��������ַ��ѯ������Ӧ�ĵ�ַ
   ͬʱע��getaddrinfo�����ص�������
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

