
#ifndef _DES_H_
#define _DES_H_

//定义部份类型
//typedef unsigned char unsigned char;
//typedef signed char int8;
//typedef unsigned char BOOLEAN;

#define TRUE	1
#define FALSE	0

//为了提高程序效率，把这两个位操作功能定义在宏。

//读取缓冲区的指定位.
#define GET_BIT(p_array, bit_index)  \
			((p_array[(bit_index) >> 3] >> (7 - ((bit_index) & 0x07))) & 0x01)

//设置缓冲区的指定位.
#define SET_BIT(p_array, bit_index, bit_val) \
			if (1 == (bit_val)) \
			{\
				p_array[(bit_index) >> 3] |= 0x01 << (7 - ((bit_index) & 0x07));\
			}\
			else\
			{\
				p_array[(bit_index) >> 3] &= ~(0x01 << (7 - ((bit_index) & 0x07)));\
			}

//加解密标识，这两个标识涉及到对表的读取位置,
//必须保证DES_ENCRYPT = 0 DES_DECRYPT = 1
typedef enum
{
	DES_ENCRYPT = 0,
	DES_DECRYPT = 1
}DES_MODE;

///////////////////////////////////////////////////////////////
//	函 数 名 : print_binary
//	函数功能 : 以二进制的形式打印缓冲区中的指定个数位
//	处理过程 : 
//	时    间 : 2006年9月2日
//	返 回 值 : 
//	参数说明 :	char * tip		提示信息，ASCII串 以0x00结束
//				unsigned char * buff	要打印内容的缓冲区指针
//				unsigned char bits		要打印的缓冲区的位数
///////////////////////////////////////////////////////////////
void print_binary(char * tip, unsigned char * buff, unsigned char bits);

///////////////////////////////////////////////////////////////
//	函 数 名 : nmcmd_execute
//	函数功能 : 以十六进制的形式打印缓冲区中的指定个数位
//	处理过程 : 
//	时    间 : 2006年9月2日
//	返 回 值 : 
//	参数说明 :	char * tip		提示信息，ASCII串 以0x00结束
//				unsigned char * buff	要打印内容的缓冲区指针
//				unsigned char bytes		要打印的缓冲区的字节数
///////////////////////////////////////////////////////////////
//void print_hex(char * tip, unsigned char * buff, short bytes, const char *pFunName = NULL, int iLineNum = 0);

///////////////////////////////////////////////////////////////
//	函 数 名 : des
//	函数功能 : DES加解密
//	处理过程 : 根据标准的DES加密算法用输入的64位密钥对64位密文进行加/解密
//				并将加/解密结果存储到p_output里
//	时    间 : 2006年9月2日
//	返 回 值 : 
//	参数说明 :	const char * p_data		输入, 加密时输入明文, 解密时输入密文, 64位(8字节)
//				const char * p_key		输入, 密钥, 64位(8字节)
//				char * p_output			输出, 加密时输出密文, 解密时输入明文, 64位(8字节)
//				unsigned char mode				0 加密  1 解密
///////////////////////////////////////////////////////////////
void Des(unsigned char *p_data, unsigned char *p_key, unsigned char *p_output, DES_MODE mode);

int DesRun(unsigned char *p_data, unsigned char *p_key, unsigned char *p_output, DES_MODE mode);

int TDesRun(unsigned char *ucPlainText, int iPlainTextLen, unsigned char *p_key, unsigned char *ucCipherText, int iCipherTextLen, int *pOutLen, DES_MODE mode);

#endif  //_DES_H_

