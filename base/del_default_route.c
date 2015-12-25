
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/route.h>


void del_default_route()
{
    static char dev_name[16][256];
    struct rtentry rt[16];

    char buf[1024];
    char *p = buf;
    char tBuf[256];
    int i;
    int j;
    int num;
    int len;
    int coldev = 0;
    int coldst = 1;
    int colgw  = 2;
    int colmask = 7;
    int colflag = 3;

    int fd = open("/proc/net/route",O_RDONLY);
	if (fd == -1) 
    {
		printf("Open /proc/net/route fail. errno=%d\r\n",errno);
		return;
	}

    len = read(fd, buf, sizeof(buf) - 1);
	buf[len] = 0;
	close(fd);

	memset(rt, 0, sizeof(struct rtentry)*16);

    printf("buf: %s \r\n", buf);
    
	i=0;
	while(*p) 
    {
		while(!((*p>='A' && *p<='Z') || (*p>='a' && *p<='z') || *p=='\r' || *p=='\n')) p++;
		if(*p==0 || *p=='\r' || *p=='\n') break;
		if(strncasecmp(p,"iface",5)==0) coldev=i;
		else if(strncasecmp(p,"destination",11)==0) coldst=i;
		else if(strncasecmp(p,"gateway",7)==0) colgw=i;
		else if(strncasecmp(p,"mask",4)==0) colmask=i;
		else if(strncasecmp(p,"flags",5)==0) colflag=i;
		while((*p>='A' && *p<='Z') || (*p>='a' && *p<='z')) p++;
		i++;
	}
	while(*p=='\r' || *p=='\n') p++;
	i=0;
	num=0;
	while(*p) 
    {
		j=0;
		while(*p==' ' || *p=='\t') p++;
		while(*p && (*p!=' ' && *p!='\t' && *p!='\r' && *p!='\n')) tBuf[j++]=*p++;
		tBuf[j]=0;
		if(j) {
			if(i==coldev) 
            {
				strcpy(dev_name[num],tBuf);
				rt[num].rt_dev=dev_name[num];
			}
            else 
            {
				unsigned long val=strtoul(tBuf,NULL,16);
				if(i==coldst) {
					((struct sockaddr_in*)&rt[num].rt_dst)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt[num].rt_dst)->sin_addr.s_addr=val;
				} else if(i==colgw) {
					((struct sockaddr_in*)&rt[num].rt_gateway)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt[num].rt_gateway)->sin_addr.s_addr=val;
				} else if(i==colmask) {
					((struct sockaddr_in*)&rt[num].rt_genmask)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt[num].rt_genmask)->sin_addr.s_addr=val;
				} else if(i==colflag) rt[num].rt_flags=(short)val;
			}
		}
		
		i++;
		if(*p==0 || *p=='\r' || *p=='\n') 
        {
			if((rt[num].rt_flags & RTF_UP) && ((struct sockaddr_in*)&rt[num].rt_dst)->sin_addr.s_addr==0 ) 
            {
				num++;
				if(num>=16)
				{
                    break;
				}
			}
            
			while(*p=='\r' || *p=='\n') 
                p++;

            i=0;
		}

        if(*p==0)
        {
            break;
        }
	}

    if(num) 
    {
		int sock = socket(AF_INET,SOCK_DGRAM,0);
		if (sock != -1) 
        {
			for(i=0;i<num;i++) 
            {
				struct in_addr inaddr;
				inaddr.s_addr=((struct sockaddr_in*)&rt[i].rt_gateway)->sin_addr.s_addr;
				if(ioctl(sock,SIOCDELRT,&rt[i]) < 0)
				{
                    printf("SIOCDELRT(%s): %s\r\n", inet_ntoa(inaddr), strerror(errno));
				}
			}
            
			close(sock);
		}
	}
}


int main(int argc, char *argv)
{
    del_default_route();

    return 0;
}

