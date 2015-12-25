#include "appLib.h"
#include <stdio.h>

extern int gSysLogTotal;           //系统日志总条数
extern int gManLogTotal;           //管理日志总条数
extern int gAccessLogTotal;

/******************************************************************************
 * 函数名称： SetSysLogTotal
 * 功能： 设置系统日志数量
 * 参数： iSysLogNum:日志数量
 * 返回： 
 * 创建作者： Jason
 * 创建日期： 2012-10-09
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void SetSysLogTotal(int iSysLogNum)
{
	gSysLogTotal = iSysLogNum;
}


/******************************************************************************
 * 函数名称： SetManLogTotal
 * 功能： 设置系统日志数量
 * 参数： iManLogNum:管理日志数量
 * 返回： 
 * 创建作者： Jason
 * 创建日期： 2012-10-09
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void SetManLogTotal(int iManLogNum)
{
	gManLogTotal = iManLogNum;
}


/******************************************************************************
 * 函数名称： GetSysLogTotal
 * 功能： 获取已存记录数量
 * 参数： 无
 * 返回： 记录数目
 * 创建作者： Jason
 * 创建日期： 2012-10-09
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int GetSysLogTotal()
{
	return gSysLogTotal;
}

/******************************************************************************
 * 函数名称： GetManLogTotal
 * 功能： 获取已存记录数量
 * 参数： 无
 * 返回： 记录数目
 * 创建作者： Jason
 * 创建日期： 2012-10-09
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int GetManLogTotal()
{
	return gManLogTotal;
}


int GetAccessLogTotal()
{
    return gAccessLogTotal;
}

void SetAccessLogTotal(int iLogNum)
{
    gAccessLogTotal = iLogNum;
}

