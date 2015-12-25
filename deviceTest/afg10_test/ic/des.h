
#ifndef _DES_H_
#define _DES_H_

//���岿������
//typedef unsigned char unsigned char;
//typedef signed char int8;
//typedef unsigned char BOOLEAN;

#define TRUE	1
#define FALSE	0

//Ϊ����߳���Ч�ʣ���������λ�������ܶ����ںꡣ

//��ȡ��������ָ��λ.
#define GET_BIT(p_array, bit_index)  \
			((p_array[(bit_index) >> 3] >> (7 - ((bit_index) & 0x07))) & 0x01)

//���û�������ָ��λ.
#define SET_BIT(p_array, bit_index, bit_val) \
			if (1 == (bit_val)) \
			{\
				p_array[(bit_index) >> 3] |= 0x01 << (7 - ((bit_index) & 0x07));\
			}\
			else\
			{\
				p_array[(bit_index) >> 3] &= ~(0x01 << (7 - ((bit_index) & 0x07)));\
			}

//�ӽ��ܱ�ʶ����������ʶ�漰���Ա�Ķ�ȡλ��,
//���뱣֤DES_ENCRYPT = 0 DES_DECRYPT = 1
typedef enum
{
	DES_ENCRYPT = 0,
	DES_DECRYPT = 1
}DES_MODE;

///////////////////////////////////////////////////////////////
//	�� �� �� : print_binary
//	�������� : �Զ����Ƶ���ʽ��ӡ�������е�ָ������λ
//	������� : 
//	ʱ    �� : 2006��9��2��
//	�� �� ֵ : 
//	����˵�� :	char * tip		��ʾ��Ϣ��ASCII�� ��0x00����
//				unsigned char * buff	Ҫ��ӡ���ݵĻ�����ָ��
//				unsigned char bits		Ҫ��ӡ�Ļ�������λ��
///////////////////////////////////////////////////////////////
void print_binary(char * tip, unsigned char * buff, unsigned char bits);

///////////////////////////////////////////////////////////////
//	�� �� �� : nmcmd_execute
//	�������� : ��ʮ�����Ƶ���ʽ��ӡ�������е�ָ������λ
//	������� : 
//	ʱ    �� : 2006��9��2��
//	�� �� ֵ : 
//	����˵�� :	char * tip		��ʾ��Ϣ��ASCII�� ��0x00����
//				unsigned char * buff	Ҫ��ӡ���ݵĻ�����ָ��
//				unsigned char bytes		Ҫ��ӡ�Ļ��������ֽ���
///////////////////////////////////////////////////////////////
//void print_hex(char * tip, unsigned char * buff, short bytes, const char *pFunName = NULL, int iLineNum = 0);

///////////////////////////////////////////////////////////////
//	�� �� �� : des
//	�������� : DES�ӽ���
//	������� : ���ݱ�׼��DES�����㷨�������64λ��Կ��64λ���Ľ��м�/����
//				������/���ܽ���洢��p_output��
//	ʱ    �� : 2006��9��2��
//	�� �� ֵ : 
//	����˵�� :	const char * p_data		����, ����ʱ��������, ����ʱ��������, 64λ(8�ֽ�)
//				const char * p_key		����, ��Կ, 64λ(8�ֽ�)
//				char * p_output			���, ����ʱ�������, ����ʱ��������, 64λ(8�ֽ�)
//				unsigned char mode				0 ����  1 ����
///////////////////////////////////////////////////////////////
void Des(unsigned char *p_data, unsigned char *p_key, unsigned char *p_output, DES_MODE mode);

int DesRun(unsigned char *p_data, unsigned char *p_key, unsigned char *p_output, DES_MODE mode);

int TDesRun(unsigned char *ucPlainText, int iPlainTextLen, unsigned char *p_key, unsigned char *ucCipherText, int iCipherTextLen, int *pOutLen, DES_MODE mode);

#endif  //_DES_H_

