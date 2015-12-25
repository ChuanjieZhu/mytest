
#include <stdio.h>

void checkSum(unsigned char *sumBuffer, unsigned char *checkBuffer, int dataLen)
{
    int i;
    for (i = 0; i < dataLen; i++)
	{
		sumBuffer[0] = sumBuffer[0] + checkBuffer[i];
		sumBuffer[1] = sumBuffer[1] + sumBuffer[0];
	}

	sumBuffer[0] = sumBuffer[0] & 0xFF;
	sumBuffer[1] = sumBuffer[1] & 0xFF;		    
}

void ubx_checksum(unsigned char *packet, int size)
{
	unsigned long a = 0x00;
	unsigned long b = 0x00;
	int i = 0;

    while(i < size) 
    {
		a += packet[i++];
		b += a;
	}

    printf("0x%02X 0x%02X \r\n", a & 0xff, b & 0xff);
	//packet[size] = a & 0xFF;
	//packet[size+1] = b & 0xFF;
}

int main()
{
    unsigned char checkBuffer[16];
    checkBuffer[0] = 0x05;
    checkBuffer[1] = 0x01;
    checkBuffer[2] = 0x02;
    checkBuffer[3] = 0x00;
    checkBuffer[4] = 0x06;
    checkBuffer[5] = 0x01;

    unsigned char sum[2];
    checkSum(sum, checkBuffer, 6);

    ubx_checksum(checkBuffer, 6);

    printf("0x%02X, 0x%02X\r\n", sum[0], sum[1]);
}   


