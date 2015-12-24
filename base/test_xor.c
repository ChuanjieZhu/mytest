
#include <stdio.h>
#include <string.h>

unsigned short getXorResult(unsigned char * buf, unsigned short len)
{
	unsigned char tmpXor = 0;
	unsigned short ret;
	
	if (buf == NULL || len <= 0)
	{
		return 0;
	}
	
	while (len != 0)
	{		
		tmpXor = tmpXor ^ (*buf);
		buf++;
		len--;
	}

	ret = (tmpXor & 0xFF);
	
	return ret;
}

int main()
{
    char buf[] = {0x23,0x23,0x12,0x00,0x0B,0x40,0x01,0x14,0x0D,0x0B,0x19,0x11,0x20,0x00};
    unsigned short s = getXorResult(buf, sizeof(buf));
    printf("0x%02x \r\n", s);
    return 0;
}

