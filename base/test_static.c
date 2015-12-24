#include <stdio.h>

unsigned int integer_sum(unsigned int base) 
{
    unsigned int index;
    unsigned int sum = 0;
    //static unsigned int sum = 0;
    
    for (index = 1; index <= base; index++)
    {
        sum += index;
    }


    return sum;
}

int main()
{
    unsigned int sum = integer_sum(5);
    printf("sum: %d %s %d\r\n", sum, __FUNCTION__, __LINE__);
    return 0;
}

