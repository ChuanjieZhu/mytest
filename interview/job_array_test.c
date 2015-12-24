
#include <stdio.h>

int main()
{
    char a[] = {0x00, 0x11, 0x22, 0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    unsigned short int *s;
    int *i;
    s = (unsigned short int *)&a[4];
    s++;
    i = (int *)s;
    printf("%x \r\n", s[1]);    /* 9988 */
    printf("%x \r\n", i[1]);    /* ddccbbaa */
    
    return 0;
} 

//问程序最后输出什么？
