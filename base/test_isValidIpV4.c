
#include <stdio.h>
#include <stdlib.h>

#define BOOL int
#define FALSE 0
#define TRUE 1

int IsVaildIpv4Addr(const char *pIpv4Addr)
{
	int iRet			= -1;
	BOOL bAllZero		= FALSE;
	BOOL bAllOne		= FALSE;
	int nIpPartOne		= 0;
	int nIpPartTwo		= 0;
	int nIpPartThree	= 0;
	int nIpPartFour		= 0;

	if (NULL != pIpv4Addr)
	{
		sscanf(pIpv4Addr, "%d.%d.%d.%d", &nIpPartOne, &nIpPartTwo, &nIpPartThree, &nIpPartFour);
                    
                //printf("one: %d, two:%d, three: %d, four: %d\r\n", nIpPartOne, nIpPartTwo, nIpPartThree, nIpPartFour);

		/*
			A类地址将IP地址前8位做为网络ID，并且前1位必须以0开头，后24 位做为主机ID;
			由于网络ID的前1位必须以0开头，所以网络ID的范围是：
			最小：00000001=1
			最大：01111111=127
			
			注:A类地址以127开头的任何IP地址都不是合法的，127开头的地址是用于回环地址测试(127.X.X.X);
			如本地网络测试地址:127.0.0.1
			
			由于主机ID不能全0和全1，所以主机ID的范围是：
			最小：00000000.00000000.00000001=0.0.1
			最大：11111111.11111111.11111110=255.255.254
		*/
		if ((1 <= nIpPartOne) && (127 > nIpPartOne))
		{
			if( (0 <= nIpPartTwo) && (255 >= nIpPartTwo)
				&& (0 <= nIpPartThree) && (255 >= nIpPartThree)
				&& (0 <= nIpPartFour) && (255 >= nIpPartFour) )
			{
				( (0 == nIpPartTwo) && (0 == nIpPartThree) && (0 == nIpPartFour) ) 
					? (bAllZero = TRUE) : (bAllZero = FALSE);

				( (255 == nIpPartTwo) && (255 == nIpPartThree) && (255 == nIpPartFour) ) 
					? (bAllOne = TRUE) : (bAllOne = FALSE);
                                    
                                //printf("bAllZero: %d, bAllOne: %d\r\n", bAllZero, bAllOne);

				// 主机号不全0或者全1;
				if( !(bAllZero || bAllOne) )
				{
					iRet = 0;
				}
			}

                        //printf("iRet = %d \r\n", iRet);
		}
		/* 保留127.0.0.1为有效 */
		else if(127 == nIpPartOne)
		{
			if((0 == nIpPartTwo) && (0 == nIpPartThree) && (1 == nIpPartFour))
			{
				iRet = 0;
			}
		}
		/*
			B类地址将IP地址前16位做为网络ID，并且前2位必须以10开头，后16 位做为主机ID;
			B类网络ID必须以10开头，所以网络ID的范围是：
			最小：10000000.00000000=128.0
			最大：10111111.11111111=191.255

			由于主机ID不能全0和全1，所以主机ID的范围是：
			最小：00000000.00000001=0.1
			最大：11111111.11111110=255.254
		*/
                else if( (128 <= nIpPartOne) && (191 >= nIpPartOne) )
		{
			if( (0 <= nIpPartTwo) && (255 >= nIpPartTwo)
				&& (0 <= nIpPartThree) && (255 >= nIpPartThree)
				&& (0 <= nIpPartFour) && (255 >= nIpPartFour) )
			{
				( (0 == nIpPartThree) && (0 == nIpPartFour) ) 
					? (bAllZero = TRUE) : (bAllZero = FALSE);

				( (255 == nIpPartThree) && (255 == nIpPartFour) ) 
					? (bAllOne = TRUE) : (bAllOne = FALSE);

				// 主机号不全0或者全1;
				if( !(bAllZero || bAllOne) )
				{
					iRet = 0;
				}
			}
		}
		/*
			C类地址将IP地址前24位做为网络ID，并且前3位必须以110开头，后8位做为主机ID;

			C类网络ID必须以110开头，所以网络的范围是：
			最小：11000000.00000000=192.0 
			最大：11011111.11111111=223.255
			
			由于主机ID不能全0和全1，所以主机ID的范围是：
			最小：00000001=1
			最大：11111110=254
		*/
                else if( (192 <= nIpPartOne) && (223 >= nIpPartOne) )
		{
			(0 == nIpPartFour) ? (bAllZero = TRUE) : (bAllZero = FALSE);
			
			(255 == nIpPartFour) ? (bAllOne = TRUE) : (bAllOne = FALSE);
			
			// 主机号不全0或者全1;
			if( !(bAllZero || bAllOne) )
			{
				iRet = 0;
			}
		}
		/*
			D类为多播地址;
			E类为科研地址;
			均不允许使用;
		*/
		else
		{
			iRet = -1;
		}
	}

	return iRet;
}

int main(int argc, char **argv)
{
    int iRet = IsVaildIpv4Addr(argv[1]);
    printf("iRet = %d \r\n", iRet);
    return iRet;
}

