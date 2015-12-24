#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_func_1()
{
    int id = 10000000;

    unsigned char ucid[8 + 1];
    snprintf(ucid, sizeof(ucid), "%d", id);

    printf("ucid %s \r\n", ucid);
}

void print_hex(unsigned char data[], int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        printf("data[%d] 0x%02x ", i, data[i]);
    }
}

int test_func_2()
{
    char keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char keyB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char ctrlBytes[4] = {0xFF, 0x07, 0x80, 0x89};

    unsigned char data[16 + 1];

    memcpy(data, keyA, 6);
    memcpy(data + 6, ctrlBytes, 4);
    memcpy(data + 10, keyB, 6);
    //snprintf(data, 16, "%s%s%s", keyA, ctrlBytes, keyB);
        
    //unsigned char dst[16 + 1] = {0};
    //memcpy(dst, data, 16);

    print_hex(data, 16);
    
    return 0;
}

void test_func_3()
{
    char a[5][128] = {"aaa", "bbb", "ccc", "ddd", "fff"};
    char cmd[256] = {0};
    char tmp[256] = {0};
    int i;
    for (i = 0; i < 5; i++)
    {
        printf("%s \r\n", a[i]);
        snprintf(cmd, 255, "%s %s", tmp, a[i]);
        memcpy(tmp, cmd, 255);
    }

    printf("cmd: %s \r\n", cmd);
}


void test_func_4(int year, int month, int day, int hour, int minute, int seconds)
{
    char buf[20] = {0};
    snprintf(buf, sizeof(buf) - 1,
        "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, seconds);

    printf("buf: %s \r\n", buf);
}

int main()
{
    //test_func_1();
    //test_func_2();
    //test_func_3();
    test_func_4(2014,12,23,14,0,0);
    return 0;
}
