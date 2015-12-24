

#include <stdio.h>

/*******************************************************************************
** 函数名称：atoi 
** 函数功能：将字符串转换成对应的数字 
** 传入参数：s: 字符串 
** 传出参数：  
** 返回值：  对应的数字
** 创建作者：sar(chenpan@a-eye.cn)
** 创建日期：
** 修改作者： 
** 修改日期： 
********************************************************************************/
int atoi(const char *s)
{
	int result = 0;
	int sign = 0;

	if (s == NULL || *s == '\0')
	{
		return -1;
	}

	/* 跳过字符串之前的空格,\t,\n制表符 */
	while (*s == ' ' || *s == '\t' || *s == '\n')
	{
		s++;
	}

	/* 判断是否为负数 */
	if (*s == '-')
	{
		sign = 1;
		s++;
	}

	/*  */
	while (*s >= '0' && *s < '9')
	{
		result = result * 10 + *s - '0';
		s++;
	}

	if (sign == 1)
	{
		return -result;
	}
	else
	{
		return result;
	}
}

/*******************************************************************************
** 函数名称：itoa 
** 函数功能：将数字转换成对应的字符串
** 传入参数：value: 欲转换的数据
			 string: 目标字符串的地址
			 radix: 转换后的进制数，可以是10进制、16进制等
** 传出参数：  
** 返回值：  对应的数字字符串
** 创建作者：sar(chenpan@a-eye.cn)
** 创建日期：
** 修改作者： 
** 修改日期： 
********************************************************************************/
char *itoa(int value, char *string, int radix)
{
	char *p;
	char *firstDig;
	char temp;
	unsigned int digVal;

	p = string;

	if (value < 0)
	{
		*p++ = '-';
		value = (unsigned long)(-(long)value);
	}

	firstDig = p;

	do 
	{
		digVal = (unsigned)(value % radix);
		value /= radix;

		if (digVal > 9)
		{
			*p++ = (char)(digVal - 10 + 'a');
		}
		else
		{
			*p++ = (char)(digVal + '0');
		}
		
	} while(value > 0);
}


