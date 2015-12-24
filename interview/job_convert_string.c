
/*******************************************************************************
**  Copyright (c) 2014, a-eye ltd company
**  All rights reserved.
**	
**  文件说明: 字符串倒序改错面试题，下面的为该过的的正确实现
**  创建日期: 2014.04.10
**
**  当前版本：1.0
**  作者：sar(chenpan@a-eye.cn)
********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>


int convert_string()
{
	char* src = "hello,world";
	int len = strlen(src);
	char* dest = (char*)malloc(len+1);//要为结束符分配一个空间
	if (dest == NULL)				//判断maloc是否执行成功
	{
		return 0;
	}
	
	char* d = dest;
	char* s = &src[len-1]; // 指向最后一个字符
	while ( len-- != 0 )
	{
		*d++ = *s--;
	}
	
	*d = 0; 		//尾部要加’\0’
	printf("%s\n",dest);

	free(dest); // 使用完，应当释放空间，以免造成内存汇泄露
	dest = NULL;
    return 0; 	//main函数前面没有定义  所以默认为void  因此return 0;不对
}

void libc_strlen_test()
{
	char *p = "hello, world";
	char *p1 = "hello,world";
	char *p2 = "abcdefg";
	
	printf("strlen(p): %d, strlen(p1): %d, strlen(p2): %d \r\n",
		strlen(p), strlen(p1), strlen(p2));  //12,11,7
}

int main()					//注意main函数有返回类型，如果没有加int则默没有返回值
{
	convert_string();
	libc_strlen_test();
	
	return 0;
}

