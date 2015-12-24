
#include <stdio.h>
#include <stdlib.h>


void change(int i)
{
    int j=2;
    int a[20]={0};
    int m;
    int k=0;

    while(i)
    {
        a[k]=i%j;
        k++;
        i=i/j;
    }

    for(k=k-1;k>=0;k--)
    {
        printf("%X",a[k]);
    }
    
    printf("\n");
}

int test(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
    unsigned int one = 0;
    unsigned int two = 0;
    unsigned int three = 0;
    unsigned int four = 0;
    int voiceSN = 0;
#if 0
    one = c1;
    two = c2;
    three = c3;
    four = c4;

    one = one << 24;
    two = two << 16;
    three = three << 8;
#else
    /* 4字节SN */
    one = ((one >> 24) | c1) << 24;
    two = ((two >> 16) | c2) << 16;
    three = ((three >> 8) | c3) << 8;
    four = (four | c4);
#endif

    voiceSN = one | two | three | four;

    printf("----------- voiceSN: %d \r\n", voiceSN);

    return 0;
}

int main()
{
    char upper = 0;
    char low = 0;

    upper = ((upper >> 5) | 0x01) << 5;             /* 高3位 */
    low = low | 0x01;                               /* 低5位 */
    
    char date = upper | low;
    printf("------------ date = 0x%02x\r\n", date); 

    //change(date);

    unsigned char c1 = 0x08;
    unsigned char c2 = 0x69;
    unsigned char c3 = 0x06;
    unsigned char c4 = 0x1C;
    
    test(c1, c2, c3, c4);
        
    return 0;
}
