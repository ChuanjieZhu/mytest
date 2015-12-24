
#include <stdio.h>
#include <string.h>

/* && , ==, & 优先级测试 */
static void test_1()
{
    int a = 1;
    int b = 2;
    int c = 3;

    if (a && a == b & c) {
        printf("I am here...\r\n");
    }
    else
    {
        printf("I am not here... \r\n");
    }
}


int main(void)
{
    test_1();

    return 0;
}

/* ------------- 
测试结果:
优先级由高到低
== -> & -> &&
-----------------*/


