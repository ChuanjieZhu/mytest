
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define NI_MAXHOST              1025
#define NI_MAXSERV              32


int getNameInfo(const char *pServ)
{
    int flag = 0;
    struct sockaddr_in sa;
    socklen_t len = sizeof(struct sockaddr_in);
    char hbuf[NI_MAXHOST] = {0}, sbuf[NI_MAXSERV] = {0};
    
    sa.sin_family = AF_INET;
    sa.sin_port = htons(2049);
    inet_pton(AF_INET, pServ, &sa.sin_addr.s_addr);

    if (getnameinfo((struct sockaddr *)&sa, len, hbuf, sizeof(hbuf), sbuf, 
                sizeof(sbuf), flag) == 0)
    {
        printf("host=%s, serv=%s \r\n", hbuf, sbuf);
    }

    return 0;
}


int main(int argc, char **argv)
{
    return getNameInfo(argv[1]);
}
