
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int test()
{   
    const char *str = "c8:3a:35:30:ab:d8	2457	2	[WPA-PSK-CCMP][ESS]	XIAOZHOU_TENDA";
    char buf[1024];
    char *saveptr1, *saveptr2, *str2;
    char *token;

    snprintf(buf, sizeof(buf) - 1, "%s", str);

    /* 2种使用strstok_r函数的方法 */
    
#if 1   
    token = strtok_r(buf, "\t", &saveptr1);
    while (token != NULL) {
        printf(" --> %s\n", token);
        token = strtok_r(NULL, "\t", &saveptr1);
    }
#else

    for (str2 = buf; ; str2 = NULL) 
    {
        token = strtok_r(str2, "\t", &saveptr2);
        if (token == NULL)
            break;
        printf(" --> %s\n", token);
    }
#endif

    return 0;
}


int
main(int argc, char *argv[])
{
   char *str1, *str2, *token, *subtoken;
   char *saveptr1, *saveptr2;
   int j;

   test();
    
   if (argc != 4) {
       fprintf(stderr, "Usage: %s string delim subdelim\n",
               argv[0]);
       exit(EXIT_FAILURE);
   }

   for (j = 1, str1 = argv[1]; ; j++, str1 = NULL) 
   {
       token = strtok_r(str1, argv[2], &saveptr1);
       if (token == NULL)
           break;
       printf("%d: %s\n", j, token);

       for (str2 = token; ; str2 = NULL) 
       {
           subtoken = strtok_r(str2, argv[3], &saveptr2);
           if (subtoken == NULL)
               break;
           printf(" --> %s\n", subtoken);
       }
   }

   exit(EXIT_SUCCESS);
}

