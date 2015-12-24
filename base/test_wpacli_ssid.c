
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MDL __FUNCTION__, __LINE__

int func(char *buf, char *dst, int dstsize)
{
    int ret = 0;
    char flag = 0;
    int index = 0;
    char tmp[8] = {0};
    char f[3][5] = {0};
    char *p = buf;
    int i = 0;
    int j = 0;
    
    while (*p != '\0')
    {
        char *t = p;

        /* 找到\\开头的字符 */
        if (*t == '\\' 
            && *(t + 1) == 'x'
            && (*(t + 2) >= '0' && *(t + 2) <= 'f')
            && (*(t + 3) >= '0' && *(t + 3) <= 'f'))
        {
            flag = 1;

            //printf("index = %d %s %d\r\n", index, MDL);
            
            if (index < 3)
            {
                /* 拷贝4个字符 */
                strncpy(f[index], t, 4);
                p += 4;
                index++;

                //printf("index = %d %s %d\r\n", index, MDL);
                
                if (index >= 3)
                {   
                    flag = 0;
                    index = 0;
                    for (i = 0; i < 3; i++)
                    {
                        char a[3] = {0};
                        strncpy(a, f[i] + 2, 2);
                        int v = strtol(a, NULL, 16);    /* 转换成16进制 */
                        tmp[i] = v;
                    }

                    //printf("tmp = %s \n", tmp);

                    if (j + 3 < dstsize)
                    {
                        memcpy(dst + j, tmp, 3);
                        //printf("dst = %s \n", dst);
                        j += 3;
                    }
                    else
                    {
                        ret = -1;
                        break;
                    }
                }
            }
        }
        else
        {
            if (flag == 1)
            {
                //printf("3.... index = %d \r\n", index);
                
                /* 不是汉字 */
                if (index < 3)
                {
                    for (i = 0; i < index; i++)
                    {
                        if (j + 4 < dstsize)
                        {
                            memcpy(dst + j, f[i], 4);
                            //printf("dst = %s %s %d\r\n", dst, MDL);
                            j += 4;
                        }
                        else
                        {
                            ret = -1;
                            break;
                        }
                    }

                    if (ret == -1)
                    {
                        break;
                    }
                }

                index = 0;
                flag = 0;

                if (j + 1 < dstsize) 
                {
                    dst[j] = *p;
                    //printf("dst = %s %s %d\r\n", dst, MDL);
                    j++;
                    p++;
                }
                else
                {
                    ret = -1;
                    break;
                }
            }
            else
            {
                index = 0;
                
                if (j + 1 < dstsize) 
                {
                    dst[j] = *p;
                    //printf("dst = %s %s %d\r\n", dst, MDL);
                    j++;
                    p++;
                }
                else
                {
                    ret = -1;
                    break;
                }
            }
        }
    }

    if (flag == 1 && index < 3)
    {
        for (i = 0; i < index; i++)
        {
            if (j + 4 < dstsize)
            {
                memcpy(dst + j, f[i], 4);
                printf("dst = %s %s %d\r\n", dst, MDL);
                j += 4;
            }
            else
            {
                ret = -1;
                break;
            }
        }                           
    }

    dst[dstsize - 1] = '\0';
    
    printf("dst = %s %s %d\r\n", dst, MDL);
    
    return ret;
}

int main(int argc, char *argv[])
{   
    //const char *s = "\\xe6\\xb5\\x8b\\xe8\\xaf\\x95-sdfsdfsdfsd";
    //const char *s = "xiaozhou-test";
    //const char *s = "\\xew\\xew\\xew";
    //const char *s = "\\xe6";
    //const char *s = "\\xe6\\xb5";
    //const char *s = "\\x";
    const char *s = "\\x12sd\\x3123123123123123";
    
    char dst[128] = {0};
    char buf[128] = {0};

    if (argc != 2)
    {
        snprintf(buf, sizeof(buf) - 1, "%s", s);
    }
    else
    {
        snprintf(buf, sizeof(buf) - 1, "%s", argv[1]);    
    }
    
    if (func(buf, dst, sizeof(dst)) != 0)
    {
        printf("> conver string(%s) fail. %s %d\r\n", buf, MDL);
    }
    else
    {
        dst[sizeof(dst) - 1] = '\0';
        printf("* conver string(%s) to (%s) ok. %s %d\r\n", buf, dst, MDL);
    }
    
    return 0;
}


