#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    int i = 16777216;
    int j = ntohl(i);

    printf("j %d \r\n", j);

    return 0;
}
