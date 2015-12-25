

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

char * stringReplace(char *string)
{
	int len = strlen(string);

	char *pNew = (char *)malloc(len * 2);
	if (!pNew)
	{
		printf("malloc fail. \r\n");
		return NULL;
	}

	char c = '\\'; 
		
	memset(pNew, 0, 2 * len);

	char *pOld = string;

	while (*pOld != '\0')
	{
		if (*pOld == '"')
		{
			*pNew++ = c;
			*pNew++ = *pOld;
		}
		else
		{
			*pNew++ = *pOld;
		}
	}

	return pNew;
}


int main()
{
	char buf[100] = {0};
	sprintf(buf, "%s", "aaa\"aa\"bbbb\"ddd");
	char *pdst = stringReplace(buf);
	printf("%s\r\n", pdst);

	if (pdst)
		free(pdst);

	return 0;
}


