//des.c

#include <stdio.h>
#include <string.h>
#include "des.h"

// 初始置换 
const unsigned char Table_IP[64] = 
{ 
	58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 
	62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8, 
	57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3, 
	61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7 
}; 

// 末置换 
const unsigned char Table_InverseIP[64] = 
{ 
	40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31, 
	38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29, 
	36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27, 
	34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25 
}; 

// 扩展置换
const unsigned char Table_E[48] = 
{ 
	32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9, 
	8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17, 
	16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25, 
	24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1 
}; 

// 密钥初始置换 
const unsigned char Table_PC1[56] = { 
	57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18, 
	10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36, 
	63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22, 
	14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4 
}; 

// 左右移运算 
const signed char Table_Move[2][16] = 
{ 
	//加密左移
	{1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1},

	//解密右移
	{0, -1, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -2, -2, -2, -1}
}; 

// 密钥压缩置换 
const unsigned char Table_PC2[48] = 
{ 
	14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10, 
	23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2, 
	41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48, 
	44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32 
}; 

// S盒 
const unsigned char Table_SBOX[8][4][16] = 
{ 
	// S1 
	14, 4, 13, 1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7, 
	0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8, 
	4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0, 
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13, 
	// S2 
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10, 
	3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5, 
	0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15, 
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9, 
	// S3 
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8, 
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1, 
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7, 
	1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12, 
	// S4 
	7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15, 
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9, 
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4, 
	3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14, 
	// S5 
	2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9, 
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6, 
	4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14, 
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3, 
	// S6 
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11, 
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8, 
	9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6, 
	4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13, 
	// S7 
	4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1, 
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6, 
	1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2, 
	6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12, 
	// S8 
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7, 
	1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2, 
	7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8, 
	2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 
}; 

// P盒置换 
const unsigned char Table_P[32] = 
{ 
	16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5, 18, 31, 10, 
	2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6, 22, 11, 4,  25 
}; 

//对两块大小相同的内存区进行异或
//异或结果保存到第一块内存
//unsigned char * p_buf_1		内存区1
//const unsigned char * p_buf_2	内存区2
//unsigned char bytes			内存区大小(单位：字节)
void Xor(unsigned char * p_buf_1, const unsigned char * p_buf_2, unsigned char bytes)
{
	while(bytes > 0)
	{
		bytes--;

		p_buf_1[bytes] ^= p_buf_2[bytes];
	}
}

//将缓冲区从第bit_start位到第bit_end进行循环左移
//offset只能是1，2
//本段代码还可以优化。
void move_left(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, unsigned char offset) 
{ 
	unsigned char b_val = 0;
	unsigned char b_tmp1 = 0;
	unsigned char b_tmp2 = 0;

	//读取bit_start位
	b_tmp1 = GET_BIT(p_input, bit_start);
	b_tmp2 = GET_BIT(p_input, bit_start + 1);

	//循环左移offset位
	for(; bit_start <= (bit_end - offset); bit_start++)
	{
		b_val = GET_BIT(p_input, bit_start + offset);
		SET_BIT(p_input, bit_start, b_val);
	}

	//将bit_start开始的offset位移到bit_end后头来
	if (1 == offset)
	{
		SET_BIT(p_input, bit_end, b_tmp1);
	}
	else
	{
		SET_BIT(p_input, bit_end, b_tmp2);
		SET_BIT(p_input, bit_end - 1, b_tmp1);
	}
} 

//将缓冲区从第bit_start位到第bit_end进行循环右移
//offset只能是1，2
//本段代码在性能上还可以优化。
void move_right(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, unsigned char offset) 
{ 
	unsigned char b_val = 0;
	unsigned char b_tmp1 = 0;
	unsigned char b_tmp2 = 0;

	//读取bit_end位
	b_tmp1 = GET_BIT(p_input, bit_end);
	b_tmp2 = GET_BIT(p_input, bit_end - 1);

	//循环左移offset位
	for(; bit_end >= (bit_start + offset); bit_end--)
	{
		b_val = GET_BIT(p_input, bit_end - offset);
		SET_BIT(p_input, bit_end, b_val);
	}

	//将bit_end倒数的offset位移到bit_start来
	if (1 == offset)
	{
		SET_BIT(p_input, bit_start, b_tmp1);
	}
	else
	{
		SET_BIT(p_input, bit_start, b_tmp2);
		SET_BIT(p_input, bit_start + 1, b_tmp1);
	}
} 

//缓冲区移位
//offset大于0时左移
//offset小于0时右移
void move_bits(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, char offset)
{
	if(0 < offset)	//左移
	{
		move_left(p_input, bit_start, bit_end, offset);	
	}	
	else if(0 > offset)	//右移
	{
		move_right(p_input, bit_start, bit_end, -offset);
	}
}

//通用置换函数, bits <= 64
//p_input与p_output不能指向同一个地址，否则置换会出错。
void Permutation(unsigned char * p_input, unsigned char * p_output, const unsigned char * Table, unsigned char bits) 
{ 
	unsigned char b_val = 0;
	unsigned char bit_index = 0;

	for(bit_index = 0; bit_index < bits; bit_index++) 
	{
		b_val = GET_BIT(p_input, Table[bit_index] - 1);
		
		SET_BIT(p_output, bit_index, b_val);
	}
} 

//获取从bit_s为起始的第1, 6 位组成行
unsigned char S_GetLine(unsigned char * p_data_ext, unsigned char bit_s)
{
	return (GET_BIT(p_data_ext, bit_s + 0) << 1) + GET_BIT(p_data_ext, bit_s + 5);
}

//获取从bit_s为起始的第2,3,4,5位组成列
unsigned char S_GetRow(unsigned char * p_data_ext, unsigned char bit_s)
{
	unsigned char row;

	//2,3,4,5位组成列
	row = GET_BIT(p_data_ext, bit_s + 1);
	row <<= 1; 
	row += GET_BIT(p_data_ext, bit_s + 2);
	row <<= 1; 
	row += GET_BIT(p_data_ext, bit_s + 3);
	row <<= 1; 
	row += GET_BIT(p_data_ext, bit_s + 4);

	return row;
}

/********************************************************************************************
函 数 名 : print_binary
函数功能 : 以二进制的形式打印缓冲区中的指定个数位
返 回 值 : 
参数说明 :	
    char * tip		提示信息，ASCII串 以0x00结束
    unsigned char * buff	要打印内容的缓冲区指针
    unsigned char bits		要打印的位的个数
********************************************************************************************/
void print_binary(char * tip, unsigned char * buff, unsigned char bits)
{
	unsigned char bit_index = 0;

	for(bit_index = 0; bit_index < bits; bit_index++)
	{
		printf("%d", (buff[bit_index >> 3] >> (7 - (bit_index % 8))) & 0x01);

		if((bit_index + 1) % 4 == 0)
		{
			printf(" ");
		}

		if((bit_index + 1) % 64 == 0)
		{
			printf("\r\n");
		}
	}
}

/********************************************************************************************
函 数 名 : print_hex
函数功能 : 以十六进制的形式打印缓冲区中的指定个数位 
返 回 值 : 
参数说明 :	char * tip		提示信息，ASCII串 以0x00结束
			unsigned char * buff	要打印内容的缓冲区指针
			unsigned char bytes		要打印的缓冲区的字节数
********************************************************************************************/
static void print_hex(char * tip, unsigned char * buff, short bytes, const char *pFuntion, int iLineNum)
{
	unsigned char ucindex = 0;
	
	printf("%s: \r\n", tip);

	for(ucindex = 0; ucindex < bytes; ucindex++)
	{
		if (ucindex != 0 && ucindex % 16 == 0)
		{
			printf("\r\n");
		}
		printf("%02X ", buff[ucindex]);
	}

	printf("\r\n");
	
	if (pFuntion != NULL && iLineNum != 0)
	{
		printf("%s %d\r\n", pFuntion, iLineNum);
	}
}

/********************************************************************************************
函 数 名 : Des
函数功能 : DES加解密
处理过程 : 根据标准的DES加密算法用输入的64位密钥对64位密文进行加/解密
				并将加/解密结果存储到p_output里
返 回 值 : 
参数说明 :	unsigned char *p_data		输入, 加密时输入明文, 解密时输入密文, 64位(8字节)
			unsigned char *p_key		输入, 密钥, 64位(8字节)
			unsigned char *p_output		输出, 加密时输出密文, 解密时输入明文, 64位(8字节)
			DES_MODE mode		DES_ENCRYPT 加密  DES_DECRYPT 解密
********************************************************************************************/
void Des(unsigned char *p_data, unsigned char *p_key, unsigned char *p_output, DES_MODE mode)
{
	unsigned char loop = 0;		//16轮运算的循环计数器
	unsigned char key_tmp[8];	//密钥运算时存储中间结果
	unsigned char sub_key[6];	//用于存储子密钥

	unsigned char * p_left;
	unsigned char * p_right;

	unsigned char p_right_ext[8];	//R[i]经过扩展置换生成的48位数据(6字节), 及最终结果的存储
	unsigned char p_right_s[4];		//经过S_BOX置换后的32位数据(4字节)

	unsigned char s_loop = 0;		//S_BOX置换的循环计数器

	//密钥第一次缩小换位, 得到一组56位的密钥数据
	Permutation(p_key, key_tmp, Table_PC1, 56);

	//明文初始化置换
	Permutation(p_data, p_output, Table_IP, 64);

	p_left  = p_output;		//L0
	p_right = &p_output[4];	//R0

	for(loop = 0; loop < 16; loop++)
	{
		//把缩进小后的把这56位分为左28位和右28位,
		//对左28位和右28位分别循环左/右移, 得到一组新数据
		//加解密操作时只在移位时有差异
		move_bits(key_tmp, 0, 27, Table_Move[mode][loop]);
		move_bits(key_tmp, 28, 55, Table_Move[mode][loop]);

		//密钥第二次缩小换位，得到一组子48位的子密钥
		Permutation(key_tmp, sub_key, Table_PC2, 48);

		//R0扩展置换
		Permutation(p_right, p_right_ext, Table_E, 48);

		//将R0扩展置换后得到的48位数据(6字节)与子密钥进行异或
		Xor(p_right_ext, sub_key, 6);

		//S_BOX置换
		for(s_loop = 0; s_loop < 4; s_loop++)
		{
			unsigned char s_line = 0;
			unsigned char s_row = 0;
			unsigned char s_bit = s_loop * 12;

			s_line = S_GetLine(p_right_ext, s_bit);
			s_row  = S_GetRow(p_right_ext,	s_bit);
			
			p_right_s[s_loop] = Table_SBOX[s_loop * 2][s_line][s_row];

			s_bit += 6;
			
			s_line = S_GetLine(p_right_ext, s_bit);
			s_row  = S_GetRow(p_right_ext,	s_bit);
			
			p_right_s[s_loop] <<= 4;
			p_right_s[s_loop] += Table_SBOX[(s_loop * 2) + 1][s_line][s_row];
		}

		//P置换
		Permutation(p_right_s, p_right_ext, Table_P, 32);

		Xor(p_right_ext, p_left, 4);

		memcpy(p_left, p_right, 4);
		memcpy(p_right, p_right_ext, 4);
	}

	memcpy(&p_right_ext[4], p_left, 4);
	memcpy(p_right_ext,	p_right, 4);

	//最后再进行一次逆置换, 得到最终加密结果
	Permutation(p_right_ext, p_output, Table_InverseIP, 64);		
}

/********************************************************************************************
函 数 名 : DesRun
函数功能 : 3DES加解密
处理过程 : 根据标准的DES加密算法用输入的128位密钥对64位密文进行加/解密
				并将加/解密结果存储到p_output里
返 回 值 : 
参数说明 :	unsigned char *p_data		输入, 加密时输入明文, 解密时输入密文, 64位(8字节)
			unsigned char *p_key		输入, 密钥, 128位(16字节)
			unsigned char *p_output		输出, 加密时输出密文, 解密时输入明文, 128位(16字节)
			DES_MODE mode		DES_ENCRYPT 加密  DES_DECRYPT 解密
********************************************************************************************/
int DesRun(unsigned char *p_data, unsigned char *p_key, unsigned char *p_output, DES_MODE mode)
{
	unsigned char *pKeyA = p_key;
	unsigned char *pKeyB = p_key + 8;

	if (mode == DES_ENCRYPT)
	{
		Des(p_data, pKeyA, p_output, DES_ENCRYPT);
		Des(p_output, pKeyB, p_data, DES_DECRYPT);
		Des(p_data, pKeyA, p_output, DES_ENCRYPT);
	}
	else
	{
		Des(p_data, pKeyA, p_output, DES_DECRYPT);
		Des(p_output, pKeyB, p_data, DES_ENCRYPT);
		Des(p_data, pKeyA, p_output, DES_DECRYPT);
	}

	return 0;
}

/********************************************************************************************
函 数 名 : TDesRun
函数功能 : 3DES加解密
处理过程 : 根据标准的DES加密算法用输入的128位密钥对输入数据进行加/解密
返 回 值 : 
参数说明 :	unsigned char *ucPlainText		输入, 加密时输入明文，长度无限制
			int iPlainTextLen		输入, 明文长度
			unsigned char *p_key			密钥, 128位(16字节)
			unsigned char *ucCipherText		输出, 加密结果缓存空间
			int iCipherTextLen		加密结果缓存空间大小
			int *pOutLen			加密结果长度
			DES_MODE mode			DES_ENCRYPT 加密  DES_DECRYPT 解密
********************************************************************************************/
int TDesRun(unsigned char *ucPlainText, int iPlainTextLen, unsigned char *p_key, unsigned char *ucCipherText, int iCipherTextLen, int *pOutLen, DES_MODE mode)
{
	unsigned char ucDesInput[8] = {0};		/* 3des 输入8字节 */
	unsigned char ucDesOutput[8] = {0};		/* 3des 输出8字节 */

	unsigned char uByteA = 0x80;
	unsigned char uByteB = 0x00;

	int isize = iPlainTextLen;

	int i, j;
	int inum = isize / 8;
	if (isize % 8 != 0)
	{
		inum++;
	}

	/* 如果3des加密结果缓存空间太小 */
	if (iCipherTextLen < inum * 8)
	{
		return -1;
	}
	else
	{
		*pOutLen = inum * 8;
	}

	/* 明文长度小于等于8字节情况 */
	if (isize <= 8) 
	{
		memcpy(ucDesInput, ucPlainText, isize);
		for (j = 0; j < 8 - isize; j++)
		{
			if (0 == j)
			{
				memcpy(ucDesInput + (8 - isize) + j, &uByteA, 1);
			}
			else
			{
				memcpy(ucDesInput + (8 - isize) + j, &uByteB, 1);
			}
		}

		DesRun(ucDesInput, p_key, ucDesOutput, DES_ENCRYPT);
		memcpy(ucCipherText, ucDesOutput, 8);
	}
	else
	{
		for (i = 0; i < inum; i++)
		{
			memset(ucDesInput, 0, sizeof(ucDesInput));
			memset(ucDesOutput, 0, sizeof(ucDesOutput));

			if (i < inum - 1)
			{
				memcpy(ucDesInput, ucPlainText + i * 8, 8);
				DesRun(ucDesInput, p_key, ucDesOutput, DES_ENCRYPT);
				memcpy(ucCipherText + i * 8, ucDesOutput, 8);
			}
			else
			{
				memcpy(ucDesInput, ucPlainText + i * 8, (isize - 8 * i));

				for (j = 0; j < (isize - 8 * i); j++)
				{
					if (0 == j)
					{
						memcpy(ucDesInput + (isize - 8 * i) + j, &uByteA, 1);
					}
					else
					{
						memcpy(ucDesInput + (isize - 8 * i) + j, &uByteB, 1);
					}
				}

				DesRun(ucDesInput, p_key, ucDesOutput, DES_ENCRYPT);
				memcpy(ucCipherText + i * 8, ucDesOutput, 8);
			}
		}
	}

	return 0;
}

