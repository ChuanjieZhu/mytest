
#ifndef __G711_H__
#define __G711_H__

#ifndef NULL
#define NULL	((void*)0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int G711AEnc(short* pSrc, unsigned char* pDst, int len);
int G711UEnc(short* pSrc, unsigned char* pDst, int len);
int G711ADec(unsigned char* pSrc, short* pDst, int len);
int G711UDec(unsigned char* pSrc, short* pDst, int len);

#ifdef __cplusplus
}
#endif

#endif /* __G711_H__ */

