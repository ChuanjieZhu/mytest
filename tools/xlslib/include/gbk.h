/********************************************************************************
**  Copyright (c) 2010, 
**  All rights reserved.
**	
**  �ļ�˵��: ��
**  ��������: 2014.02.28
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#ifndef _GBK_H_
#define _GBK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TRACE   printf

extern int GBKBufToUTF8Buf(char *pcFrom, int iLen, char *pcTo);
extern wchar_t * ZhcnToUincode(char* strTemp);

#ifdef __cplusplus
}
#endif

#endif //_GBK_H_

