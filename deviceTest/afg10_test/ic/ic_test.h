/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  �ļ�˵��: afg10 ic cpu����д����
**  ��������: 2014.02.26
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#ifndef _IC_TEST_H_
#define _IC_TEST_H_

#define IC_DEVICE_NAME          "/dev/ttySAC3"

#ifdef  __cplusplus
extern "C" {
#endif
    
extern int ic_init(const char *pName);

#ifdef __cplusplus
}      
#endif

#endif //_IC_TEST_H_
