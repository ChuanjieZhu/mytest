
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <net/route.h>    
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>

#define BOOL int

#define FALSE   0
#define TRUE    1

BOOL isIP(const char *pAddr)
{
	int i;
    int j;
    int len;
    int nDot = 0;
    
	BOOL bRet = FALSE;
    BOOL bDot = TRUE;

    if(pAddr == NULL)
    {
        return bRet;
    }
    
	len = strlen(pAddr);
    
	if(len >= 7 && len <= 15)
    {
		for(i = j = 0; i < len; i++) 
        {
			if(pAddr[i] == '.') 
            {
				if (bDot)
				{
                    break;
				}
                
				bDot = TRUE;

                printf("%s %d \r\n", pAddr + j, atoi(pAddr + j));

                /* 参数nptr字符串，如果第一个非空格字符存在，是数字或者正负号则开始做类型转换，
                之后检测到非数字(包括结束符 \0) 字符时停止转换，返回整型数。否则，返回零， */
				if (atoi(pAddr + j) > 255)
				{
                    break;
				}
                
				nDot++;
                
				j = i + 1;
			}
            else 
			{
				if (pAddr[i] < '0' || pAddr[i] > '9') 
                {
                    break;
				}
                
				bDot = FALSE;
                
				if ((i - j) > 2)
                {
                    break;
				}
			}
		}
        
		if (i == len && nDot == 3 && atoi(pAddr + j) <= 255) 
		{
            bRet = TRUE;
		}
	}
    
	return bRet;
}


void del_default_route()
{
	static char devname[16][256];
	struct rtentry rt[16];

	char buf[1024], *p = buf, tBuf[256];
	int i, j, num, len, coldev=0, coldst=1, colgw=2, colmask=7, colflag=3;

    int fd = open("/proc/net/route", O_RDONLY);

    if(fd == -1) 
    {
		printf("Open /proc/net/route fail. errno=%d\r\n",errno);
		return;
	}
    
	len = read(fd, buf, sizeof(buf) - 1);
	buf[len] = 0;
	close(fd);

	memset(rt, 0, sizeof(struct rtentry) * 16);

	i = 0;
	while (*p) 
    {
		while(!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || *p == '\r' || *p == '\n'))
		{
            p++;
		}
        
		if(*p == 0 || *p == '\r' || *p == '\n')
        {
            break;
		}
        
		if(strncasecmp(p, "iface", 5) == 0)
		{
            coldev=i;
		}
		else if(strncasecmp(p, "destination", 11) == 0)
		{
            coldst = i;
		}
		else if(strncasecmp(p, "gateway", 7) == 0)
        {
            colgw = i;
		}
		else if(strncasecmp(p, "mask", 4) == 0) 
        {
            colmask = i;
		}
		else if(strncasecmp(p, "flags", 5) == 0)
		{
            colflag = i;
		}
        
		while((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')) 
		{
            p++;
		}
        
		i++;
	}
    
	while(*p == '\r' || *p == '\n')
	{
        p++;
	}
    
	i = 0;
	num = 0;
	while(*p) 
    {
		j=0;
		while(*p==' ' || *p=='\t') 
        {
            p++;
		}
		while(*p && (*p!=' ' && *p!='\t' && *p!='\r' && *p!='\n')) 
        {
            tBuf[j++] = *p++;
		}
        
		tBuf[j]=0;
		if(j) 
        {
			if(i == coldev) 
            {
				strcpy(devname[num], tBuf);
				rt[num].rt_dev = devname[num];
			} 
            else 
            {
				unsigned long val=strtoul(tBuf,NULL,16);
				if(i == coldst) 
                {
					((struct sockaddr_in*)&rt[num].rt_dst)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt[num].rt_dst)->sin_addr.s_addr=val;
				}
                else if(i == colgw) 
				{
					((struct sockaddr_in*)&rt[num].rt_gateway)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt[num].rt_gateway)->sin_addr.s_addr=val;
				}
                else if(i == colmask) 
                {
					((struct sockaddr_in*)&rt[num].rt_genmask)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt[num].rt_genmask)->sin_addr.s_addr=val;
				} 
                else if(i == colflag) 
                    rt[num].rt_flags=(short)val;
			}
		}

        i++;
		if(*p==0 || *p=='\r' || *p=='\n') 
        {
			if( (rt[num].rt_flags & RTF_UP) && ((struct sockaddr_in*)&rt[num].rt_dst)->sin_addr.s_addr==0 ) 
            {
				num++;
				if(num>=16) break;
			}
            
			while(*p=='\r' || *p=='\n') 
                p++;
			i=0;
		}
        
		if(*p == 0)
		{
            break;
		}
	}
    
	if(num) 
    {
		int sock=socket(AF_INET, SOCK_DGRAM, 0);
		if(sock != -1) 
        {
			for(i = 0; i < num; i++) 
            {
				struct in_addr inaddr;
				inaddr.s_addr = ((struct sockaddr_in *)&rt[i].rt_gateway)->sin_addr.s_addr;
				if(ioctl(sock, SIOCDELRT, &rt[i]) < 0)
				{
                    printf("SIOCDELRT(%s): %s\r\n", inet_ntoa(inaddr), strerror(errno));
				}
			}
            
			close(sock);
		}
	}
}

/******************************************************************************
 * 函数名称： get_default_route
 * 功能： 取默认路由
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 2012-6-21
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
unsigned long get_default_route()
{
	static char devname[256] = {0};
	static struct rtentry rt;
	char buf[1024] = {0};
    char *p = buf;
    char tBuf[256] = {0};
	int i = 0;
    int j = 0;
    int len = 0;
    int coldev = 0;
    int coldst = 1;
    int colgw = 2;
    int colmask = 7;
    int colflag = 3;
    
	int fd=open("/proc/net/route",O_RDONLY);
	if(fd==-1)
    {
		printf("Open /proc/net/route fail. errno=%d\r\n",errno);
		return 0;
	}
	len=read(fd,buf,1024-1);
	buf[len]=0;
	close(fd);

	i=0;
	while(*p)
    {
		while(!((*p>='A' && *p<='Z') || (*p>='a' && *p<='z') || *p=='\r' || *p=='\n'))
        {
            p++;
		}
        
		if(*p==0 || *p=='\r' || *p=='\n')
        {
            break;
		}
        
		if(strncasecmp(p,"iface",5)==0)
        {
            coldev = i;
		}
		else if(strncasecmp(p,"destination",11)==0)
        {
            coldst = i;
		}
		else if(strncasecmp(p,"gateway",7)==0)
        {
            colgw = i;
		}
		else if(strncasecmp(p,"mask",4)==0)
        {
            colmask = i;
		}
		else if(strncasecmp(p,"flags",5)==0)
        {
            colflag=i;
		}
        
		while((*p>='A' && *p<='Z') || (*p>='a' && *p<='z'))
        {
            p++;
		}
        
		i++;
	}
    
	while(*p=='\r' || *p=='\n')
    {
        p++;
	}
    
	i = 0;
	memset(&rt,0,sizeof(struct rtentry));
    
	while(*p)
    {
		j = 0;
        
		while(*p==' ' || *p=='\t')
        {
            p++;
		}

        while(*p && (*p!=' ' && *p!='\t' && *p!='\r' && *p!='\n'))
        {
            tBuf[j++] = *p++;
		}
        
		tBuf[j] = 0;
        
		if(j)
        {
			if(i == coldev)
            {
				strcpy(devname,tBuf);
				rt.rt_dev=devname;
			}
            else
            {
				unsigned long val=strtoul(tBuf,NULL,16);
                
				if(i == coldst)
                {
					((struct sockaddr_in*)&rt.rt_dst)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt.rt_dst)->sin_addr.s_addr=val;
				}
                else if(i == colgw) 
                {
					((struct sockaddr_in*)&rt.rt_gateway)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt.rt_gateway)->sin_addr.s_addr=val;
				}
                else if(i == colmask)
                {
					((struct sockaddr_in*)&rt.rt_genmask)->sin_family=PF_INET;
					((struct sockaddr_in*)&rt.rt_genmask)->sin_addr.s_addr=val;
				}
                else if(i==colflag)
                {
                    rt.rt_flags=(short)val;
                }
			}
        }

        i++;

        if(*p==0 || *p=='\r' || *p=='\n')
        {
			if((((rt.rt_flags & RTF_UP) & ((struct sockaddr_in*)&rt.rt_dst)->sin_addr.s_addr)==0)
                && ((struct sockaddr_in*)&rt.rt_gateway)->sin_addr.s_addr)
			{
				return ((struct sockaddr_in*)&rt.rt_gateway)->sin_addr.s_addr;
			}
            
			while(*p=='\r' || *p=='\n')
            {
                p++;
			}
            
			i = 0;
			memset(&rt,0,sizeof(struct rtentry));
		}
        
		if(*p == 0)
        {
            break;
		}
	}
    
	return 0;
}


/******************************************************************************
 * 函数名称： getnet
 * 功能： 得到网络参数
 * 参数： 	pIfName:设备名
 			pdwIpAddrp:IP地址
 			pdwSubMask:子网掩码
 			pGW:网关
 			pDNS:DNS服务器
 			pDNS2:
 			pMac:Mac地址
 			pHostname:主机名
 * 返回： 
 * 创建作者： 
 * 创建日期： 2012-6-21
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int getnet(char* pIfName, unsigned long* pdwIpAddr, unsigned long* pdwSubMask, char* pGW, char* pDNS, char* pDNS2, char* pMac, char* pHostname)
{
	int sock, ret=0;
	struct ifreq ifr;
	unsigned long dwIp, dwSubMask;
	
	if(pdwIpAddr==NULL) pdwIpAddr = &dwIp;
	if(pdwSubMask==NULL) pdwSubMask = &dwSubMask;

	if(pIfName==NULL) return -1;

	memset(pdwIpAddr,0,sizeof(unsigned long));
	memset(pdwSubMask,0,sizeof(unsigned long));
	if(pGW) pGW[0]=0;
	if(pDNS) pDNS[0]=0;
	if(pDNS2) pDNS2[0]=0;
	if(pMac) memset(pMac,0,6);
	if(pHostname) pHostname[0]=0;

	memset(&ifr,0,sizeof(ifr));

	strncpy(ifr.ifr_name,pIfName,IFNAMSIZ);

	sock=socket(AF_INET,SOCK_DGRAM,0);
	if(sock==-1) return -1;

	ret=ioctl(sock,SIOCGIFADDR,&ifr,sizeof(ifr));
	if(ret==0) {
		*pdwIpAddr=((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;

		ret=ioctl(sock,SIOCGIFNETMASK,&ifr,sizeof(ifr));
		if(ret==0) {
			*pdwSubMask=((struct sockaddr_in*)&ifr.ifr_netmask)->sin_addr.s_addr;

			if(pMac) {
				ret=ioctl(sock,SIOCGIFHWADDR,&ifr,sizeof(ifr));
				if(ret==0) memcpy(pMac,ifr.ifr_hwaddr.sa_data,6);
				ret=0;
			}
		}
	}
	close(sock);

	if(pHostname) if(gethostname(pHostname,255)) pHostname[0]=0;

	if(pGW) {
		int i, j, handle;
		char iffile[256], *pdata=NULL, *pmap=NULL;
		struct stat buf;

		sprintf(iffile,"/etc/sysconfig/network-scripts/ifcfg-%s",pIfName);
		handle=open(iffile,O_RDONLY);
		if(handle!=-1) {
			fstat(handle, &buf);
			if(buf.st_size) 
                pmap = pdata = (char*)malloc(buf.st_size);
			if(pmap) 
            {
				read(handle,pmap,buf.st_size);
				for(i=0;i<buf.st_size;i++,pdata++) 
                {
					if(strncasecmp(pdata,"GATEWAY",7)==0) 
                    {
						i+=7;
						pdata+=7;
						break;
					}
				}
                
				for(; i<buf.st_size; i++,pdata++) 
                {
					if(*pdata=='=') 
                    {
						i++;
						pdata++;
						break;
					}
				}
                
				for(;i<buf.st_size;i++,pdata++) 
                {
					if((*pdata>='0' && *pdata<='9') 
                        || (*pdata>='a' && *pdata<='z') 
                        || (*pdata>='A' && *pdata<='Z')) 
					{
                        break;
					}
				}

                j=0;

                for(;i<buf.st_size;i++,pdata++) 
                {
					if(*pdata=='\r' || *pdata=='\n' || *pdata=='\0')
                        break;
					pGW[j++]=*pdata;
				}
				pGW[j++]=0;
				free(pmap);
			}
			close(handle);
		}
		if(*pGW=='\0') {
			unsigned long dwGW=get_default_route();
			if(dwGW) {
				struct in_addr inaddr;
				inaddr.s_addr=dwGW;
				strcpy(pGW,inet_ntoa(inaddr));
			}
		}
	}

	if(pDNS) {
		int i, j, handle;
		char iffile[256], *pdata=NULL, *pmap=NULL;
		struct stat buf;

		strcpy(iffile,"/etc/resolv.conf");
		handle=open(iffile,O_RDONLY);
		if(handle!=-1) {
			fstat(handle,&buf);
			if(buf.st_size) pmap=pdata=(char*)malloc(buf.st_size);
			if(pmap) {
				read(handle,pmap,buf.st_size);
				for(i=0;i<buf.st_size;i++,pdata++) {
					if(strncasecmp(pdata,"nameserver",10)==0) {
						i+=10;
						pdata+=10;
						break;
					}
				}
				if(*pdata!=' ' && *pdata!='\t') i=buf.st_size;
				for(;i<buf.st_size;i++,pdata++) {
					if((*pdata>='0' && *pdata<='9') || (*pdata>='a' && *pdata<='z') || (*pdata>='A' && *pdata<='Z')) break; // DNS must be numeric or alpha head
				}
				j=0;
				for(;i<buf.st_size;i++,pdata++) {
					if(*pdata=='\r' || *pdata=='\n' || *pdata=='\0') {
						i++; pdata++; break;
					}
					pDNS[j++]=*pdata;
				}
				pDNS[j++]=0;

				if(pDNS2) {
					for(;i<buf.st_size;i++,pdata++) {
						if(strncasecmp(pdata,"nameserver",10)==0) {
							i+=10;
							pdata+=10;
							break;
						}
					}
					if(*pdata!=' ' && *pdata!='\t') i=buf.st_size;
					for(;i<buf.st_size;i++,pdata++) {
						if((*pdata>='0' && *pdata<='9') || (*pdata>='a' && *pdata<='z') || (*pdata>='A' && *pdata<='Z')) break;
					}
					j=0;
					for(;i<buf.st_size;i++,pdata++) {
						if(*pdata=='\r' || *pdata=='\n' || *pdata=='\0') break;
						pDNS2[j++]=*pdata;
					}
					pDNS2[j++]=0;
				}
				free(pmap);
			}
			close(handle);
		}
	}

	return ret;
}

int SetNet(char* pIfName, unsigned long dwIpAddr, unsigned long dwSubMask, 
            char* pGW, char* pDNS, char* pDNS2, char* pMac, char* pHostname, 
            unsigned short nMTU, int bOnBoot)
{
	int sock;
    int ret = 0;
    int handle;
	struct ifreq ifr;
	struct in_addr inaddr;
	char GW[256], DNS[256], DNS2[256], iffile[256], tmpBuf[256];

	if (pIfName == NULL || *pIfName == '\0') 
    {
        return -1;
	}

    /* ifName */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pIfName, IFNAMSIZ);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) 
    {
        return -1;
	}

    if (dwIpAddr)
    {
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_family = PF_INET;
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr = dwIpAddr;

        ret = ioctl(sock, SIOCSIFADDR, &ifr);
		if(ret < 0) 
        {
			inaddr.s_addr = dwIpAddr;
			printf("SIOCSIFADDR(%s): %s\r\n",inet_ntoa(inaddr),strerror(errno));
		}
	}
    
	if(ret == 0 && pGW && *pGW) 
    {
		if(isIP(pGW)) 
        {
            inet_aton(pGW, &inaddr);
		}
		else 
        {
			struct hostent *phost = gethostbyname(pGW);
            
			if(phost) 
            {
                inaddr.s_addr = *((unsigned long *)phost->h_addr);
			}
			else
            {
                inaddr.s_addr = 0;
			}
		}
        
		if (inaddr.s_addr) 
        {
			((struct sockaddr_in *)&ifr.ifr_dstaddr)->sin_family = PF_INET;
			((struct sockaddr_in *)&ifr.ifr_dstaddr)->sin_addr.s_addr = inaddr.s_addr;

            ret = ioctl(sock, SIOCSIFDSTADDR, &ifr);
			if(ret < 0)
			{
                printf("SIOCSIFDSTADDR(%s): %s\r\n", pGW, strerror(errno));
			}
		}
	}
    
	if (ret == 0 && dwSubMask) 
    {
		((struct sockaddr_in *)&ifr.ifr_netmask)->sin_family = PF_INET;
		((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr = dwSubMask;
        
		ret = ioctl(sock, SIOCSIFNETMASK, &ifr);
		if(ret < 0) 
        {
			inaddr.s_addr = dwSubMask;
			printf("SIOCSIFNETMASK(%s): %s\r\n", inet_ntoa(inaddr), strerror(errno));
		}
	}
    
	if(ret == 0 && nMTU) 
    {
		ifr.ifr_mtu = nMTU;

        ret = ioctl(sock, SIOCSIFMTU, &ifr);
		if(ret < 0)
		{
            printf("SIOCSIFMTU(%d): %s\r\n", nMTU, strerror(errno));
		}
	}

#if 0
	/* Change MAC in UBOOT-ENV, must reboot to be applied */
	if(ret == 0 && pMac && memcmp(pMac, "\0\0\0\0\0\0", 6)) 
    {
		char cmd[256], cmdEnv[512];
		sprintf(cmd, "ethaddr=");
		for(ret = 0; ret < 6; ret++) 
        {
			sprintf(tmpBuf, "%02X", (unsigned char)pMac[ret]);

            strcat(cmd, tmpBuf);
            
			if(ret < 5)
			{
                strcat(cmd, ":");
			}
		}
        
		cmd[strlen(cmd) + 1] = '\0';
        
	#ifdef PLAT_S3C2440
		ret = SetUbootEnv(cmd);
	#else
		ret = SetUbootEnv_L10orL20(cmd);
	#endif
	
		char* pBootArgs = GetUbootEnv("bootargs");
		if(pBootArgs) {
			char* pEthaddr = strstr(pBootArgs, "ethaddr=");
			if(pEthaddr) {
				memcpy(pEthaddr, cmd, strlen(cmd));
				memset(cmdEnv, 0, sizeof(cmdEnv));
				strcpy(cmdEnv, "bootargs=");
				strcat(cmdEnv, pBootArgs);
			#ifdef PLAT_S3C2440
				SetUbootEnv(cmdEnv);
			#else
				SetUbootEnv_L10orL20(cmdEnv);
			#endif
			}
		}
		
#ifdef PLAT_S3C2440
		pBootArgs = GetUbootEnv("preboot"); 
		if(pBootArgs) 
		{ 
			char* pEthaddr = strstr(pBootArgs, "ethaddr="); 
			/* 之前是直接把ethaddr=$ethaddr 替换成 ethaddr=00:12:11:45:72:27 
			造成把后面字符覆盖 */ 
			if(pEthaddr) 
			{ 
				if(strstr(pEthaddr, "ethaddr=$")) 
				{ 
					char *p = replace(pBootArgs, (char *)"ethaddr=$ethaddr", cmd); 
					if(p) 
					{ 
						strcpy(cmdEnv, "preboot="); 
						strcat(cmdEnv, p); 
						TRACE("cmdEnv:%s %s %d \r\n", cmdEnv, __FUNCTION__, __LINE__); 
						SetUbootEnv(cmdEnv); 
						TRACE("p:%p\r\n", p); 
						Free(p); 
						p = NULL; 
					} 
				} 
				else 
				{ 
					memcpy(pEthaddr, cmd, strlen(cmd)); 
					memset(cmdEnv, 0, sizeof(cmdEnv)); 
					strcpy(cmdEnv, "preboot="); 
					strcat(cmdEnv, pBootArgs); 
					TRACE("cmdEnv:%s %s %d \r\n", cmdEnv, __FUNCTION__, __LINE__); 
					SetUbootEnv(cmdEnv); 
				} 
			} 
		}
#endif
		ret = 0;
	}
#endif

	if(dwIpAddr)
	{
        del_default_route();
	}
    
	if(ret==0 && pGW && *pGW) 
    {
		struct rtentry rt;
		memset(&rt, 0, sizeof(struct rtentry));
		rt.rt_dev = pIfName;

		((struct sockaddr_in *)&rt.rt_dst)->sin_family = PF_INET;
		((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;
		((struct sockaddr_in *)&rt.rt_genmask)->sin_family = PF_INET;
		((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;
		((struct sockaddr_in *)&rt.rt_gateway)->sin_family = PF_INET;

        if(isIP(pGW))
        {
            inet_aton(pGW, &inaddr);
        }
		else 
        {
			struct hostent *phost = gethostbyname(pGW);
			if(phost) 
            {
                inaddr.s_addr = *((unsigned long *)phost->h_addr);
			}
			else 
            {
                inaddr.s_addr = 0;
			}
		}
        
		if(inaddr.s_addr) 
        {
			((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = inaddr.s_addr;
			rt.rt_flags = RTF_GATEWAY | RTF_UP;
            
			if (ioctl(sock, SIOCADDRT, &rt) < 0)
			{
                printf("SIOCADDRT(%s): %s\r\n", pGW, strerror(errno));
			}
		}
	}
    
	close(sock);

	if(pHostname && *pHostname)
	{
        sethostname(pHostname, strlen(pHostname));
	}
    
	if(dwIpAddr) 
    {
		getnet(pIfName, &dwIpAddr, &dwSubMask,(pGW && *pGW) ? NULL : GW,
            (pDNS && *pDNS) ? NULL : DNS,(pDNS2 && *pDNS2) ? NULL : DNS2, NULL, NULL);
        
		if(!(pGW &&  *pGW))
            pGW=GW;
		if(!(pDNS && *pDNS))
            pDNS=DNS;
		if(!(pDNS2 && *pDNS2))
            pDNS2=DNS2;
	}

	sprintf(iffile, "/etc/sysconfig/network-scripts/ifcfg-%s", pIfName);
    
	handle=open(iffile, O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if(handle != -1) 
    {
		if(memcmp(pIfName, "ppp", 3) == 0)
		{
			sprintf(tmpBuf,"DEVICE=%s\nONBOOT=no\nBOOTPROTO=ppp\n", pIfName);
		}
		else
		{
			sprintf(tmpBuf,"DEVICE=%s\nONBOOT=%s\nBOOTPROTO=%s\n",pIfName,bOnBoot?"yes":"no",dwIpAddr==0?"dhcp":"static");
		}
        
		write(handle, tmpBuf, strlen(tmpBuf));

        if(dwIpAddr) 
        {
			inaddr.s_addr = dwIpAddr;
			sprintf(tmpBuf, "IPADDR=%s\n", inet_ntoa(inaddr));
			write(handle, tmpBuf, strlen(tmpBuf));
			inaddr.s_addr = dwSubMask;
			sprintf(tmpBuf, "NETMASK=%s\n", inet_ntoa(inaddr));
			write(handle, tmpBuf, strlen(tmpBuf));
			if(pGW && *pGW) 
            {
				sprintf(tmpBuf, "GATEWAY=%s\n", pGW);
				write(handle, tmpBuf, strlen(tmpBuf));
			}
		}
		close(handle);
	}

	if(pDNS || pDNS2) 
    {
		strcpy(iffile, "/etc/resolv.conf");
		handle=open(iffile, O_WRONLY|O_CREAT|O_TRUNC, 0777);
		if(handle!=-1) 
        {
			if(pDNS) 
            {
				sprintf(tmpBuf,"nameserver %s\n", pDNS);
				write(handle, tmpBuf, strlen(tmpBuf));
			}

            if(pDNS2) 
            {
				sprintf(tmpBuf,"nameserver %s\n",pDNS2);
				write(handle,tmpBuf,strlen(tmpBuf));
			}
			close(handle);
		}
	}

    /* hostname */
	if(pHostname && *pHostname) 
    {
		strcpy(iffile,"/etc/sysconfig/network");
		handle=open(iffile, O_WRONLY|O_CREAT|O_TRUNC, 0777);
		if(handle!=-1) {
			strcpy(tmpBuf,"NETWORKING=yes\n");
			write(handle,tmpBuf,strlen(tmpBuf));
			sprintf(tmpBuf,"HOSTNAME=%s\n",pHostname);
			write(handle,tmpBuf,strlen(tmpBuf));
			close(handle);
		}
	}
    
	return ret;
}

int main()
{
    char *pIpAddress = "192.168.100.100";

    if (isIP(pIpAddress) == 1)
    {
        printf("ok is ip \r\n");
    }

    return -1;
}

