
#include <sys/socket.h> 
#include <sys/ioctl.h> 
#include <linux/if.h> 
#include <string.h> 
#include <stdio.h>   

char *net_detect(char* net_name) 
{         
    int skfd = 0;         
    struct ifreq ifr;          
    skfd = socket(AF_INET, SOCK_DGRAM, 0);         
    if(skfd < 0) 
    {                 
        printf("%s:%d Open socket error!/n", __FILE__, __LINE__);                 
        return NULL;         
    }          

    strcpy(ifr.ifr_name, net_name);          
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 ) 
    {                 
        printf("%s:%d IOCTL error!/n", __FILE__, __LINE__);  
        printf("Maybe ethernet inferface %s is not valid!", ifr.ifr_name);  
        close(skfd);
        return NULL;         
    }

    if (ifr.ifr_flags & IFF_RUNNING) 
    {                 
        return "UP";         
    }
    else 
    {
        return "DOWN";         
    } 
}

int main() 
{
    printf("%s \n", net_detect("eth0"));  
    return 0; 
}
    
