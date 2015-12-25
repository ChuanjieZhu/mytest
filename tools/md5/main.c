#include "md5encode.h"
#include <stdio.h>
#include <string.h>

const char *pSrcFile = "1.txt";
const char *pDstFile = "2.dat";

int main(int argc, char *argv[])
{
	int ret = -1;
	char acMd5Code[64] = {0};
	ret = GetFileMd5Code(pSrcFile, pDstFile, acMd5Code, 64);
	if (0 == ret)
	{
		printf("acMd5Code: %s %s %d\r\n", acMd5Code, __FUNCTION__, __LINE__);
	}
	else
	{
		printf("GetFileMd5Code Fail!\r\n");
	}

	char acSrcData[64] = {0};
	snprintf(acSrcData, sizeof(acSrcData), "%s", "aaaaaaaaaaaaaaaaaaa");
	memset(acMd5Code, 0, sizeof(acMd5Code));
	ret = GetDataMd5Code(acSrcData, strlen(acSrcData), acMd5Code, sizeof(acMd5Code));
	if (0 == ret)
	{
		printf("acMd5Code: %s %s %d\r\n", acMd5Code, __FUNCTION__, __LINE__);
	}
	else
	{
		printf("GetDataMd5Code Fail!\r\n");
	}

	return ret;
}
