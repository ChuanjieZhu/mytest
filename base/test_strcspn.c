
#include <stdio.h>
#include <string.h>

/*
    size_t strcspn(const char *s, const char *set);
    ��s������set�ַ��������������κ��ַ��ĵ�һ�γ��֣�������Щ��set�����ڵ��ַ�����
*/

int main()
{
    const char *s = "flags=[WPA-PSK-CCMP][WPS][ESS]";
    const char *set = "=";

    size_t ret = strcspn(s, set);

    printf("* set = %d \r\n", ret);         //5

    return 0;
}

