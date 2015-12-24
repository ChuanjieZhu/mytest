
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TRACE printf
#define MDL __FUNCTION__, __LINE__
#define DNS_DEV_NAME    "/etc/resolv.conf"

char *FiStrsep( char **stringp, const char *delim )
{
	char *p;

	while (1)
	{
		p = strsep(stringp, delim);
		if (p == NULL)
		{
			return NULL;
		}
		else
		{
			if (p[0] == '\0')
				continue;
			else
				break;
		}
	}
	return p;
}


ssize_t readline(int fd, char *vptr, size_t maxlen)
{
	size_t n, rc;
	char c, *ptr;

	ptr = (char *)vptr;
	for (n = 1; n < maxlen; n++)
	{
	again:
		if ((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
				break; /* newline is stored, like fgets() */
		}
		else if (rc == 0)
		{
			*ptr = 0;
			return(n - 1); /* EOF, n-1 bytes were read */
		}
		else
		{
			if (errno == EINTR)
				goto again;
			return(-1);
		}
	}
	*ptr = 0; /* null terminate like fgets() */
	return(n);
}

int GetDns(char *pcDnsFirst, char *pcDnsSecond)
{
    int iFlag = 0;
	int iFd = -1;
	char acBuf[256] = {0};
	char *pcTmp = NULL;

	if ((NULL == pcDnsFirst) || (NULL == pcDnsSecond))
	{
		return -1;
	}
	
	if (NULL == DNS_DEV_NAME)
	{
		return -2;
	}

	if(0 > (iFd = open(DNS_DEV_NAME, O_RDONLY)))
	{
		return -3;
	}
	
	while (0 < readline(iFd, acBuf, sizeof(acBuf)))
	{
        TRACE("acBuf: %s %s %d\r\n", acBuf, MDL);
        
		if (0 == strncmp(acBuf, "nameserver", 10))
		{
			pcTmp = acBuf;
			FiStrsep(&pcTmp, " ");
    
            TRACE("acBuf: %s %s %d\r\n", pcTmp, MDL);
            
			if (0 == iFlag)
			{
				strncpy(pcDnsFirst, FiStrsep(&pcTmp, " "), 16+1);
                
                TRACE("pcDnsFirst: %s %s %d\r\n", pcDnsFirst, MDL);
                
				if (pcDnsFirst[strlen(pcDnsFirst) - 1] == '\n') 
                    pcDnsFirst[strlen(pcDnsFirst) - 1] = '\0';
				
				iFlag = 1;
				if (NULL == pcDnsSecond) 
                    break;
                
				continue;
			}
			else
			{
				strncpy(pcDnsSecond, FiStrsep(&pcTmp, " "), 16+1);
				if (pcDnsSecond[strlen(pcDnsSecond) - 1] == '\n') 
                    pcDnsSecond[strlen(pcDnsSecond) - 1] = '\0';

				iFlag = 2;
				break;
			}
		}
	}
	
	close(iFd);
	if (0 == iFlag)
	{
		return -4;
	}
	
	return 0;
}


int main()
{
    char dns[32] = {0};
    char dns2[32] = {0};
    
    GetDns(dns, dns2);

    printf("dns: %s, dns2: %s \r\n", dns, dns2);
    
    return 0;
}
