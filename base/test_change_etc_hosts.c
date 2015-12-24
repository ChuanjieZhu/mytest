

#include <stdio.h>
#include <string.h>

#define HOSTS_FILE              "/etc/hosts"
#define HOSTS_DEFAULT_CONTENTS  "127.0.0.1       localhost.localdomain   localhost"
#define HOSTS_AUTH_CONTENTS     "120.24.240.171       auth.meirenji.cn"

int SetSystemHosts(const char *ip, const char *domain)
{
    FILE *fp = NULL;
    char buff[1024] = {0};

    if (ip == NULL || 
        ip[0] == '\0' ||
        //IsPublicIp(ip) != 0 ||
        domain == NULL ||
        domain[0] == '\0')
    {
        return -1;
    }

    chmod(HOSTS_FILE, 0777);
	usleep(10);
	unlink(HOSTS_FILE);
    
    fp = fopen(HOSTS_FILE, "w");
    if (fp == NULL)
    {   
        printf("> open %s fail\n", HOSTS_FILE);
        return -1;
    }

    snprintf(buff, sizeof(buff) - 1, "%s       %s", ip, domain);
    
    fprintf(fp, "%s\n", HOSTS_DEFAULT_CONTENTS);
    fprintf(fp, "%s\n", buff);
	
	fclose(fp);

	return 0;
}


int main()
{
    int ret = SetSystemHosts("120.24.240.171", "auth.meirenji.cn");

    return ret;
}

