
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


unsigned int testAscii(const char *src, size_t srcLen, char *dst, size_t dstLen, unsigned int moveBits)
{
    unsigned int i;
    char c = 0;

    printf("origin: ");
    
    for (i = 0; i < srcLen; i++)
    {   
        c = src[i];
        printf("%c", c);
    }

    printf("\r\n");

    printf("new: ");
    for (i = 0; i < srcLen; i++)
    {   
        c = src[i];
        int v = (int)c;
        v += moveBits;
        
        printf("%c(%d)", c, v);
    }

    printf("\r\n");
    
    return 0;
}


int main()
{
    const char *src = "imei=1011411000151&token=9a299ab2697a024057dc93378dabb3a4&data=}";
    size_t len = strlen(src);
    char dst[1024] = {0};
    unsigned int moveBits = 5;
       
    testAscii(src, len, dst, sizeof(dst), moveBits);

    return 0;
}

