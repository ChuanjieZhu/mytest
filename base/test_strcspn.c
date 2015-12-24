
#include <stdio.h>
#include <string.h>

/*
    size_t strcspn(const char *s, const char *set);
    在s中搜索set字符串中所包含的任何字符的第一次出现，跳过那些在set不存在的字符串。
*/

int main()
{
    const char *s = "flags=[WPA-PSK-CCMP][WPS][ESS]";
    const char *set = "=";

    size_t ret = strcspn(s, set);

    printf("* set = %d \r\n", ret);         //5

    return 0;
}

