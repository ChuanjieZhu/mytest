/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  文件说明: afg10 ic cpu卡读写测试
**  创建日期: 2014.02.26
**
**  当前版本：1.0
**  作者：
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
