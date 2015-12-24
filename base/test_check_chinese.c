
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_string_is_all_chinese(const char *s)
{   
    int ret = -1;
    
    if (!s)
        return -1;
    
    int flag = 1;
    
    if (strlen(s) % 2 == 0)
    {
        int i;
        for (i = 0; i < strlen(s); i++)
        {
            if (s[i] >= 0) 
            {
                flag = 0;           /* 非中文 */           
                break; 
            }
        }
    }
    else
    {
        flag = 0;
    }
    
    if (flag == 1)
    {
        printf(" all chinesn \r\n");
        ret = 0;
    }
    else
    {
        printf(" not all chinese \r\n");
        ret = -1;
    }
    
    return ret; 
}

int main()
{
    const char *s = "我是_中国人";
    check_string_is_all_chinese(s);
    return 0;
}

