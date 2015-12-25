
#include <stdio.h>
#include <stdlib.h>

union cv 
{
    int a;
    short b[2];
};


short covert_int_to_short(int a)
{
    union cv c;
    c.a = a;

    return c.b[0];
}


int main()
{   
    int a = 100;
    short s = covert_int_to_short(a);
    printf("s: %d \r\n", s);

    return 0;
}

