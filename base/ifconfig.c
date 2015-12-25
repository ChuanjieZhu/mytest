
#include <sys/ioctl.h>  
#include <net/if.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <stdio.h>  
#include <netinet/in.h>
#include <string.h>
#include <strings.h>

int set_broadcast(char *interface_name, char *broadcast)  
{  
    int s;  
    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0)  
    {  
        perror("Socket");  
        return -1;  
    }  

    struct ifreq ifr;  
    strcpy(ifr.ifr_name,interface_name);  

    struct sockaddr_in broadcast_addr; 
    
    bzero(&broadcast_addr, sizeof(struct sockaddr_in));  
    broadcast_addr.sin_family = PF_INET;  

    inet_aton(broadcast, &broadcast_addr.sin_addr); 
    
    memcpy(&ifr.ifr_ifru.ifru_broadaddr, &broadcast_addr, sizeof(struct sockaddr_in));    

    if(ioctl(s,SIOCSIFBRDADDR,&ifr) < 0)  
    {  
        perror("ioctl");  
        return -1;    
    }  
    return 0;  
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: <1> [interface_name] [broadcast] \r\n");
        return -1;
    }

    int ret = set_broadcast(argv[1], argv[2]);
    printf("ret: %d \r\n", ret);

    return 0;
}
