#ifndef __GB2312_H__
#define __GB2312_H__

void Gb2312ToUnicode( unsigned short* pOut,char *pGb2312Word );

void GB2312ToUTF_8(char *pOut,char *pText, int pLen);

#endif //__GB2312_H__

