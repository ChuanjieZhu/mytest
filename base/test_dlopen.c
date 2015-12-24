/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  �ļ�˵��: libc dlopen�ӿ�ʹ�ò��Գ���
**  ��������: 2014.02.26
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>


/*
* If this program were in a file named "foo.c", you would build the program with the following command:
* gcc -rdynamic -o dlopen dlopen.c -ldl
* Libraries exporting _init() and _fini() will want to be compiled as follows, using bar.c as the example name:
* gcc -shared -nostartfiles -o bar bar.c
*/
int main(int argc, char *argv[])
{
    void *handle;
    double (*cosine)(double);  /* function pointer */
    char *error;

    handle = dlopen("libm.so", RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror(); /* clear any existing error */
    *(void **) (&cosine) = dlsym(handle, "cos");

    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    printf("%f\n", (*cosine)(2.0));
    dlclose(handle);
    exit(EXIT_SUCCESS);
}

