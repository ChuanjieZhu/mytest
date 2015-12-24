
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct _TEST_STRUCT_
{
	int one;
	int two;
	int three;
} TEST_STRUCT;


void setTest(TEST_STRUCT *pTest, int num)
{
	int i;
	for (i = 0; i < num; i++)
	{
		pTest[i]->one = 1;
		pTest[i]->two = 2;
		pTest[i]->three = 3;
	}
}

void showTest(TEST_STRUCT * pTest[], int num)
{
	int i;
	for (i = 0; i < num; i++)
	{
		printf("test[%d]-one: %d, tow: %d, three: %d \r\n", pTest[i]->one,
			pTest[i]->two, pTest[i]->three);
	}
}

int main(int argc, char *argv[])
{
	int num = 10;
	TEST_STRUCT *pTest = (TEST_STRUCT *)malloc(num * sizeof(TEST_STRUCT));
	if (pTest == NULL)
	{
		printf("malloc error!");
		return -1;
	}

	memset(pTest, 0, sizeof(TEST_STRUCT) * num);
	
	setTest(pTest, num);

	showTest(pTest, num);
}



