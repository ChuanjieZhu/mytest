

#include <stdio.h>

/*******************************************************************************
** �������ƣ�atoi 
** �������ܣ����ַ���ת���ɶ�Ӧ������ 
** ���������s: �ַ��� 
** ����������  
** ����ֵ��  ��Ӧ������
** �������ߣ�sar(chenpan@a-eye.cn)
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
int atoi(const char *s)
{
	int result = 0;
	int sign = 0;

	if (s == NULL || *s == '\0')
	{
		return -1;
	}

	/* �����ַ���֮ǰ�Ŀո�,\t,\n�Ʊ�� */
	while (*s == ' ' || *s == '\t' || *s == '\n')
	{
		s++;
	}

	/* �ж��Ƿ�Ϊ���� */
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
** �������ƣ�itoa 
** �������ܣ�������ת���ɶ�Ӧ���ַ���
** ���������value: ��ת��������
			 string: Ŀ���ַ����ĵ�ַ
			 radix: ת����Ľ�������������10���ơ�16���Ƶ�
** ����������  
** ����ֵ��  ��Ӧ�������ַ���
** �������ߣ�sar(chenpan@a-eye.cn)
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
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


