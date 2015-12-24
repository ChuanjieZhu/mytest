
/*******************************************************************************
**  Copyright (c) 2014, a-eye ltd company
**  All rights reserved.
**	
**  �ļ�˵��: �ַ�������Ĵ������⣬�����Ϊ�ù��ĵ���ȷʵ��
**  ��������: 2014.04.10
**
**  ��ǰ�汾��1.0
**  ���ߣ�sar(chenpan@a-eye.cn)
********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>


int convert_string()
{
	char* src = "hello,world";
	int len = strlen(src);
	char* dest = (char*)malloc(len+1);//ҪΪ����������һ���ռ�
	if (dest == NULL)				//�ж�maloc�Ƿ�ִ�гɹ�
	{
		return 0;
	}
	
	char* d = dest;
	char* s = &src[len-1]; // ָ�����һ���ַ�
	while ( len-- != 0 )
	{
		*d++ = *s--;
	}
	
	*d = 0; 		//β��Ҫ�ӡ�\0��
	printf("%s\n",dest);

	free(dest); // ʹ���꣬Ӧ���ͷſռ䣬��������ڴ��й¶
	dest = NULL;
    return 0; 	//main����ǰ��û�ж���  ����Ĭ��Ϊvoid  ���return 0;����
}

void libc_strlen_test()
{
	char *p = "hello, world";
	char *p1 = "hello,world";
	char *p2 = "abcdefg";
	
	printf("strlen(p): %d, strlen(p1): %d, strlen(p2): %d \r\n",
		strlen(p), strlen(p1), strlen(p2));  //12,11,7
}

int main()					//ע��main�����з������ͣ����û�м�int��Ĭû�з���ֵ
{
	convert_string();
	libc_strlen_test();
	
	return 0;
}

