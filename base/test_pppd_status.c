
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <asm/types.h>		/* glibc 2 conflicts with linux/types.h */
#include <linux/sockios.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>

int GetPppStatus(const char *pIfNmae)
{
    if (pIfNmae == NULL)
        return -1;

   	struct ifpppstatsreq req;
    int	s = -1;
    
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) 
    {
	    printf("> Couldn't create IP socket. %s %d\r\n", __FUNCTION__, __LINE__);
	    return -1;
	}

    memset (&req, 0, sizeof (req));

    req.stats_ptr = (caddr_t) &req.stats;
    strncpy(req.ifr__name, pIfNmae, sizeof(req.ifr__name));
    if (ioctl(s, SIOCGPPPSTATS, &req) < 0) 
    {
        return -1;
    }

    close(s);
    
	return 0;
}

int CheckPPPoEDevice()
{
    int ret = -1;
    struct stat st;

    if (stat("/dev/ttyUSB1", &st) == 0)
    {
        ret = 0;
    }

    return ret;
}

int main()
{
    int ret = -1;

    if (CheckPPPoEDevice() == 0)
    {
        printf("---------- ttyUSB1 exist.\r\n");
    }
    
    ret = GetPppStatus("ppp0");
    if (0 == ret)
    {
        printf("* ppp connect ok. \n");
    }
    else
    {
        printf("* ppp connect fail. \n");
    }

    return ret;
}

