
#include <stdio.h>

const char *GetDeviceModelString(int model)
{
    const char *pModel = NULL;
    if (model == 11)
    {
        pModel = "M2";    
    }
    else if (model == 12)
    {
        pModel = "M1+";
    }
    else if (model == 13)
    {
        pModel = "M2+";
    }
    else
    {   
        pModel = "M2";   
    }

    return pModel;
}

int main()
{
    int model = 11;

    const char *p = GetDeviceModelString(model);

    printf("---------- p = %s \r\n", p);

    return 0;
}

