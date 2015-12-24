#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    int i = 201326592;
    int j = htonl(i);

    printf("j %d \r\n", j);

    return 0;
}
