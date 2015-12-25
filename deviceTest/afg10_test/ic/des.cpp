//des.c

#include <stdio.h>
#include <string.h>
#include "des.h"

// ��ʼ�û� 
const unsigned char Table_IP[64] = 
{ 
	58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 
	62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8, 
	57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3, 
	61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7 
}; 

// ĩ�û� 
const unsigned char Table_InverseIP[64] = 
{ 
	40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31, 
	38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29, 
	36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27, 
	34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25 
}; 

// ��չ�û�
const unsigned char Table_E[48] = 
{ 
	32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9, 
	8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17, 
	16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25, 
	24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1 
}; 

// ��Կ��ʼ�û� 
const unsigned char Table_PC1[56] = { 
	57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18, 
	10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36, 
	63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22, 
	14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4 
}; 

// ���������� 
const signed char Table_Move[2][16] = 
{ 
	//��������
	{1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1},

	//��������
	{0, -1, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -2, -2, -2, -1}
}; 

// ��Կѹ���û� 
const unsigned char Table_PC2[48] = 
{ 
	14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10, 
	23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2, 
	41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48, 
	44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32 
}; 

// S�� 
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

// P���û� 
const unsigned char Table_P[32] = 
{ 
	16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5, 18, 31, 10, 
	2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6, 22, 11, 4,  25 
}; 

//�������С��ͬ���ڴ����������
//��������浽��һ���ڴ�
//unsigned char * p_buf_1		�ڴ���1
//const unsigned char * p_buf_2	�ڴ���2
//unsigned char bytes			�ڴ�����С(��λ���ֽ�)
void Xor(unsigned char * p_buf_1, const unsigned char * p_buf_2, unsigned char bytes)
{
	while(bytes > 0)
	{
		bytes--;

		p_buf_1[bytes] ^= p_buf_2[bytes];
	}
}

//���������ӵ�bit_startλ����bit_end����ѭ������
//offsetֻ����1��2
//���δ��뻹�����Ż���
void move_left(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, unsigned char offset) 
{ 
	unsigned char b_val = 0;
	unsigned char b_tmp1 = 0;
	unsigned char b_tmp2 = 0;

	//��ȡbit_startλ
	b_tmp1 = GET_BIT(p_input, bit_start);
	b_tmp2 = GET_BIT(p_input, bit_start + 1);

	//ѭ������offsetλ
	for(; bit_start <= (bit_end - offset); bit_start++)
	{
		b_val = GET_BIT(p_input, bit_start + offset);
		SET_BIT(p_input, bit_start, b_val);
	}

	//��bit_start��ʼ��offsetλ�Ƶ�bit_end��ͷ��
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

//���������ӵ�bit_startλ����bit_end����ѭ������
//offsetֻ����1��2
//���δ����������ϻ������Ż���
void move_right(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, unsigned char offset) 
{ 
	unsigned char b_val = 0;
	unsigned char b_tmp1 = 0;
	unsigned char b_tmp2 = 0;

	//��ȡbit_endλ
	b_tmp1 = GET_BIT(p_input, bit_end);
	b_tmp2 = GET_BIT(p_input, bit_end - 1);

	//ѭ������offsetλ
	for(; bit_end >= (bit_start + offset); bit_end--)
	{
		b_val = GET_BIT(p_input, bit_end - offset);
		SET_BIT(p_input, bit_end, b_val);
	}

	//��bit_end������offsetλ�Ƶ�bit_start��
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

//��������λ
//offset����0ʱ����
//offsetС��0ʱ����
void move_bits(unsigned char * p_input, unsigned char bit_start, unsigned char bit_end, char offset)
{
	if(0 < offset)	//����
	{
		move_left(p_input, bit_start, bit_end, offset);	
	}	
	else if(0 > offset)	//����
	{
		move_right(p_input, bit_start, bit_end, -offset);
	}
}

//ͨ���û�����, bits <= 64
//p_input��p_output����ָ��ͬһ����ַ�������û��������
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

//��ȡ��bit_sΪ��ʼ�ĵ�1, 6 λ�����
unsigned char S_GetLine(unsigned char * p_data_ext, unsigned char bit_s)
{
	return (GET_BIT(p_data_ext, bit_s + 0) << 1) + GET_BIT(p_data_ext, bit_s + 5);
}

//��ȡ��bit_sΪ��ʼ�ĵ�2,3,4,5λ�����
unsigned char S_GetRow(unsigned char * p_data_ext, unsigned char bit_s)
{
	unsigned char row;

	//2,3,4,5λ�����
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
�� �� �� : print_binary
�������� : �Զ����Ƶ���ʽ��ӡ�������е�ָ������λ
�� �� ֵ : 
����˵�� :	
    char * tip		��ʾ��Ϣ��ASCII�� ��0x00����
    unsigned char * buff	Ҫ��ӡ���ݵĻ�����ָ��
    unsigned char bits		Ҫ��ӡ��λ�ĸ���
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
�� �� �� : print_hex
�������� : ��ʮ�����Ƶ���ʽ��ӡ�������е�ָ������λ 
�� �� ֵ : 
����˵�� :	char * tip		��ʾ��Ϣ��ASCII�� ��0x00����
			unsigned char * buff	Ҫ��ӡ���ݵĻ�����ָ��
			unsigned char bytes		Ҫ��ӡ�Ļ��������ֽ���
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
�� �� �� : Des
�������� : DES�ӽ���
�������� : ���ݱ�׼��DES�����㷨�������64λ��Կ��64λ���Ľ��м�/����
				������/���ܽ���洢��p_output��
�� �� ֵ : 
����˵�� :	unsigned char *p_data		����, ����ʱ��������, ����ʱ��������, 64λ(8�ֽ�)
			unsigned char *p_key		����, ��Կ, 64λ(8�ֽ�)
			unsigned char *p_output		���, ����ʱ�������, ����ʱ��������, 64λ(8�ֽ�)
			DES_MODE mode		DES_ENCRYPT ����  DES_DECRYPT ����
********************************************************************************************/
void Des(unsigned char *p_data, unsigned char *p_key, unsigned char *p_output, DES_MODE mode)
{
	unsigned char loop = 0;		//16�������ѭ��������
	unsigned char key_tmp[8];	//��Կ����ʱ�洢�м���
	unsigned char sub_key[6];	//���ڴ洢����Կ

	unsigned char * p_left;
	unsigned char * p_right;

	unsigned char p_right_ext[8];	//R[i]������չ�û����ɵ�48λ����(6�ֽ�), �����ս���Ĵ洢
	unsigned char p_right_s[4];		//����S_BOX�û����32λ����(4�ֽ�)

	unsigned char s_loop = 0;		//S_BOX�û���ѭ��������

	//��Կ��һ����С��λ, �õ�һ��56λ����Կ����
	Permutation(p_key, key_tmp, Table_PC1, 56);

	//���ĳ�ʼ���û�
	Permutation(p_data, p_output, Table_IP, 64);

	p_left  = p_output;		//L0
	p_right = &p_output[4];	//R0

	for(loop = 0; loop < 16; loop++)
	{
		//������С��İ���56λ��Ϊ��28λ����28λ,
		//����28λ����28λ�ֱ�ѭ����/����, �õ�һ��������
		//�ӽ��ܲ���ʱֻ����λʱ�в���
		move_bits(key_tmp, 0, 27, Table_Move[mode][loop]);
		move_bits(key_tmp, 28, 55, Table_Move[mode][loop]);

		//��Կ�ڶ�����С��λ���õ�һ����48λ������Կ
		Permutation(key_tmp, sub_key, Table_PC2, 48);

		//R0��չ�û�
		Permutation(p_right, p_right_ext, Table_E, 48);

		//��R0��չ�û���õ���48λ����(6�ֽ�)������Կ�������
		Xor(p_right_ext, sub_key, 6);

		//S_BOX�û�
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

		//P�û�
		Permutation(p_right_s, p_right_ext, Table_P, 32);

		Xor(p_right_ext, p_left, 4);

		memcpy(p_left, p_right, 4);
		memcpy(p_right, p_right_ext, 4);
	}

	memcpy(&p_right_ext[4], p_left, 4);
	memcpy(p_right_ext,	p_right, 4);

	//����ٽ���һ�����û�, �õ����ռ��ܽ��
	Permutation(p_right_ext, p_output, Table_InverseIP, 64);		
}

/********************************************************************************************
�� �� �� : DesRun
�������� : 3DES�ӽ���
�������� : ���ݱ�׼��DES�����㷨�������128λ��Կ��64λ���Ľ��м�/����
				������/���ܽ���洢��p_output��
�� �� ֵ : 
����˵�� :	unsigned char *p_data		����, ����ʱ��������, ����ʱ��������, 64λ(8�ֽ�)
			unsigned char *p_key		����, ��Կ, 128λ(16�ֽ�)
			unsigned char *p_output		���, ����ʱ�������, ����ʱ��������, 128λ(16�ֽ�)
			DES_MODE mode		DES_ENCRYPT ����  DES_DECRYPT ����
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
�� �� �� : TDesRun
�������� : 3DES�ӽ���
�������� : ���ݱ�׼��DES�����㷨�������128λ��Կ���������ݽ��м�/����
�� �� ֵ : 
����˵�� :	unsigned char *ucPlainText		����, ����ʱ�������ģ�����������
			int iPlainTextLen		����, ���ĳ���
			unsigned char *p_key			��Կ, 128λ(16�ֽ�)
			unsigned char *ucCipherText		���, ���ܽ������ռ�
			int iCipherTextLen		���ܽ������ռ��С
			int *pOutLen			���ܽ������
			DES_MODE mode			DES_ENCRYPT ����  DES_DECRYPT ����
********************************************************************************************/
int TDesRun(unsigned char *ucPlainText, int iPlainTextLen, unsigned char *p_key, unsigned char *ucCipherText, int iCipherTextLen, int *pOutLen, DES_MODE mode)
{
	unsigned char ucDesInput[8] = {0};		/* 3des ����8�ֽ� */
	unsigned char ucDesOutput[8] = {0};		/* 3des ���8�ֽ� */

	unsigned char uByteA = 0x80;
	unsigned char uByteB = 0x00;

	int isize = iPlainTextLen;

	int i, j;
	int inum = isize / 8;
	if (isize % 8 != 0)
	{
		inum++;
	}

	/* ���3des���ܽ������ռ�̫С */
	if (iCipherTextLen < inum * 8)
	{
		return -1;
	}
	else
	{
		*pOutLen = inum * 8;
	}

	/* ���ĳ���С�ڵ���8�ֽ���� */
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
