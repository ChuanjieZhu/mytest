#include <stdio.h>
#include <string.h>

static int myStrlen(unsigned char *string, int buffLen)
{
    int i;
    int len = 0;
    unsigned char *p = string;
    unsigned char *q = string + 1;

    for (i = 0; i < buffLen - 1; i++)
    {
        if (*p == 0x0d && *q == 0x0a)
        {
            printf("* find string end.........\r\n");
            break;
        }
        else
        {
            len++;
            p++;
            q++;
        }
    }

    len += 2;

    printf("* --------- len: %d \r\n", len);
    
    return len;
}

static void printHex(const char *pData, int iDataLen)
{
    int i;
    
    for (i = 0; i < iDataLen; ++i)
    {
        printf("0x%02X ", pData[i]);
    }

    printf("\r\n");
}

int main()
{
    unsigned char string[256] = {0};

    string[0] = 0x23;
    string[1] = 0x23;
    string[2] = 0x0a;
    
    memcpy(string + 3, "1011411006923", 13);

    string[16] = 0x00; 
    string[17] = 0x0d;
    string[18] = 0x0a;

    printHex((char *)string, 19);
    
    int len = myStrlen(string, 256);

    return len;
}

