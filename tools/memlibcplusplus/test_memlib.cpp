/*
 *
 *
 */

#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MALLOC_SIZE 1024

char* gpcFile;
int giLine;
    
/*------------------A Test Class--------------------------*/
class Test 
{
    public:
        Test();
        ~Test();
        void SetValue(int one, int two, char *pString);
        void Print();
    private:
        int one;
        int two;
        char acString[64];    
};

Test::Test()
{
    one = 0;
    two = 0;
    memset(acString, 0, sizeof(acString));
}

Test::~Test()
{
    ;
}

void Test::Print()
{
    printf("one = %d, two = %d, string(%s) \r\n", one, two, acString);
}


void Test::SetValue(int one1, int two1, char * pString)
{
    one = one1;
    two = two1;
    if (pString)
    {
        memcpy(acString, pString, sizeof(acString) - 1);
    }
}

/*------------------A Test Class--------------------------*/
char *Mem_Test_1()
{
    char *p = NULL;

    p = (char *)Malloc(MALLOC_SIZE);

    if (p == NULL)
    {
        printf("Malloc memory fail! %s %d\r\n", __FUNCTION__, __LINE__);
    }

    return p;
}

Test *Mem_Test_2()
{
    Test *p = NULL;
    p = new class Test;

    if (p == NULL)
    {
        printf("new memory fail! %s %d\r\n", __FUNCTION__, __LINE__);
    }

    return p;
}

int main()
{
    char *p1 = NULL;
    Test *p2 = NULL;
    char buf[] = "malloc memory test";
    
    p1 = Mem_Test_1();
    if (p1 == NULL)
    {
        printf("Malloc memory fail! %s %d\r\n", __FUNCTION__, __LINE__);
        exit(1);
    }

    memset(p1, 0, MALLOC_SIZE);
    memcpy(p1, buf, MALLOC_SIZE);
    printf("p1(%s) %s %d\r\n", p1, __FUNCTION__, __LINE__);

    p2 = Mem_Test_2();
    if (p2 == NULL)
    {
        printf("new memory fail! %s %d\r\n", __FUNCTION__, __LINE__);
        exit(1);
    }

    p2->SetValue(10, 11, buf);
    p2->Print();

#if 0
    if (p1)
        Free(p1);


    if (p2)
        delete p2;
#endif

    return 0;
}

