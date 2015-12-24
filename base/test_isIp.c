
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BOOL int
#define TRUE 1
#define FALSE 0

BOOL isIP(const char *pAddr)
{
	int i;
    int j;
    int len;
    int nDot = 0;
	BOOL bRet = FALSE;
    BOOL bDot = TRUE;
    
	if (pAddr == NULL) 
    {
        return bRet;
	}
    
	len = strlen(pAddr);
	if (len >= 7 && len <= 15) 
    {
		for (i = j = 0; i < len; i++)
        {
			if (pAddr[i]=='.') 
            {
				if (bDot) 
                {
                    break;
				}
                
				bDot = TRUE;
                
                printf("atoi(pAddr + j): %d \r\n", atoi(pAddr + j));
				if (atoi(pAddr + j) > 255) 
                {
                    break;
				}                        

				nDot++;
				j = i + 1;
			}
            else 
            {
                printf("pAddr[i] : %d \r\n", pAddr[i] );
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

int IsIp(const char *pIpAddr)
{
    int iRet = -1;
    int iPart0 = 0;
    int iPart1 = 0;
    int iPart2 = 0;
    int iPart3 = 0;

    if (pIpAddr == NULL || pIpAddr[0] == '\0')
    {
        return -1;
    }

    /*
        排除下列不合法地址
        127.0.0.1本机
        0.0.0.0代表本机所有地址
        224.0.0.0~255.255.255.255 多播地址
    */

    if (isIP(pIpAddr) != TRUE)
    {
        return -1;
    }

    sscanf(pIpAddr, "%d.%d.%d.%d", &iPart0, &iPart1, &iPart2, &iPart3);

    if (iPart0 == 127
        && iPart1 == 0
        && iPart2 == 0
        && iPart3 == 1)
    {
        return -1;
    }
    else if (iPart0 == 0
        && (iPart1 >= 0 && iPart1 <= 255) 
        && (iPart2 >= 0 && iPart2 <= 255)
        && (iPart3 >= 0 && iPart3 <= 255))
    {
        return -1;
    }
    else if ((iPart0 >= 224 && iPart0 <= 255)
        && (iPart1 >= 0 && iPart1 <= 255)
        && (iPart2 >= 0 && iPart2 <= 255)
        && (iPart3 >= 0 && iPart3 <= 255))
    {
        iRet = -1;
    }
    else
    {
        iRet = 0;
    }

    return iRet;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: isIp <ip> \r\n");
        return -1;
    }

    int b = IsIp(argv[1]);
    if (b == 0)
        printf("is ok ip\r\n");
    else
        printf("is not ok ip\r\n");

    return 0;
}   

