/********************************************************************************
**  Copyright (c) 2010, 
**  All rights reserved.
**	
**  文件说明: xlslib excel接口文件实现。
**  创建日期: 2014.02.28
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#ifdef HAVE_CONFIG_H
#include "common/xlconfig.h"
#elif defined(_MSC_VER) && defined(WIN32)
#include "ac-config.win32.h"
#endif

#ifdef __BCPLUSPLUS__
#  include "ac-config.win32.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "iconv.h"

#ifdef _X_DEBUG_
#include <unistd.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include "include/xlslib/extformat.h"

#include "xlslib_test.h"
#include "include/gbk.h"
#include "include/gbcode.h"
#include "include/xlslib.h"

#if 1
//using namespace xlslib_core;
#endif 
//using namespace xls;
//using namespace std;
//#define XLS_FORMAT

int xlsCellMerge(worksheet *wsRecord,int sRow,int sCol, int eRow, int eCol, xf_t * pXf_t)
{	
    int ret = 0;    
    int iCol = 0;   
    xlsWorksheetMerge(wsRecord, sRow, sCol, eRow, eCol);    
    for(iCol = sCol; iCol <= eCol; iCol++)                      
    {
        xlsWorksheetLabel(wsRecord, sRow, iCol, "", pXf_t);                                     
    }                                                           

    return ret;
}

int CreateTestExcel(int iRowNum)
{
    workbook *w = NULL;
    worksheet *ws = NULL;
    wchar_t *pLabel = NULL;
    cell_t *c = NULL;
    char caExcelName[128];
    char caTmpName[128] = {0};
    
    int iCol = 0;
    int iRow = 0;
    int iTmpCol = 0;

    w = xlsNewWorkbook();
    
    pLabel = ZhcnToUincode((char *)"测试");
    
    ws = xlsWorkbookSheetW(w, pLabel);
    
    //xlsWorksheetMerge(ws, 0, 0, 0, 4);
    xlsCellMerge(ws, 0, 0, 0, 4, NULL);
    pLabel = ZhcnToUincode((char *)"Excel信息表");

    xlsWorksheetLabelW(ws, 0, 0, pLabel, NULL);
    xlsWorksheetRowheight(ws, 0, (unsigned16_t)XLS_HEIGHT_OF_ROW, NULL);
    c = xlsWorksheetFindCell(ws, 0, 0);

    xlsCellFontheight(c, (unsigned16_t)XLS_FONT_HEIGHT);
    
    /* 设置0，0位置单元格加粗 */
	xlsCellFontbold(c, BOLDNESS_BOLD);

    /* 设置0，0位置单元格水平方向居中 */
 	xlsCellHalign(c, HALIGN_CENTER);
    xlsCellValign(c, VALIGN_CENTER);
    
#if 1
    
	for(iTmpCol = 0; iTmpCol <= 4; iTmpCol++)
	{
        /* 查找单元格之前要确保其中有内容 否则会段错误 */
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}
#endif
    
	//c = xlsWorksheetFindCell(ws, 0, 4);
	//xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);

    /* 合并第1行的0-4列 */
	xlsWorksheetMerge(ws, 1, 0, 1, 4);	
    
    pLabel = ZhcnToUincode((char *)"提示：1、工号最大为9位（必填）；2、姓名中文最多7个字，"
   		"英文最多15个字（必填）；3、部门中文最多7个字，英文最多15个字（必填），"
   		"4、班次输入时有班次填1~5；无班次填0（必填）；"
   		"5、卡号最大10位（选填）。");

    /* 设置第一行表格内容 */
	xlsWorksheetLabelW(ws, 1, 0, pLabel, NULL);
    c = xlsWorksheetFindCell(ws, 1, 0);//查找单元格

    /* 设置自动换行 */
    xlsCellWrap(c, true);

    /* 设置行高 */
	xlsWorksheetRowheight(ws, 1, (unsigned16_t)60 * 20, NULL);
    
	//c = xlsWorksheetFindCell(ws, 1, 4);
	//xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);

	pLabel = ZhcnToUincode((char *)"工号");
	xlsWorksheetLabelW(ws, 2, 0, pLabel, NULL);         /* 设置单元格内容 */
	c = xlsWorksheetFindCell(ws, 2, 0);                 /* 选定单元格 */
	xlsCellFillstyle(c, FILL_SOLID);                    /* 填充单元格 */
	xlsCellFillfgcolor(c, CLR_GRAY25);                  /* 颜色 */

	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);  /* 设置单元格边框 */
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
	xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);

    pLabel = ZhcnToUincode((char *)"姓名");
   	xlsWorksheetLabelW(ws, 2, 1, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 1);//查找单元格
	xlsCellFillstyle(c, FILL_SOLID);
    xlsCellFillfgcolor(c, CLR_GRAY25);
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
    xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
	   
	pLabel = ZhcnToUincode((char *)"部门");
	xlsWorksheetLabelW(ws, 2, 2, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 2);//查找单元格
	xlsCellFillstyle(c, FILL_SOLID);
	xlsCellFillfgcolor(c, CLR_GRAY25);
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
	xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);

	pLabel = ZhcnToUincode((char *)"班次");
	xlsWorksheetLabelW(ws, 2, 3, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 3);//查找单元格	
	xlsCellFillstyle(c, FILL_SOLID);
	xlsCellFillfgcolor(c, CLR_GRAY25);
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
	xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
	
	pLabel = ZhcnToUincode((char *)"卡号");
	xlsWorksheetLabelW(ws, 2, 4, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 4);//查找单元格
	xlsCellFillstyle(c, FILL_SOLID);
	xlsCellFillfgcolor(c, CLR_GRAY25);
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
	xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);

    iRow = 3;
    int iId = 0;
    char acCardNum[32] = {0};
    
    while (iRow < (iRowNum + 3))
    {
        /* id列 */
        iCol = 0;
        xlsWorksheetNumberInt(ws, iRow, iCol, (iId++), NULL);

        c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
    	xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);        
		xlsCellFormat(c, FMT_TEXT);

        /* 姓名列 */
        iCol++;
        pLabel = ZhcnToUincode((char *)"李莉莉");
	    xlsWorksheetLabelW(ws, iRow, iCol, pLabel, NULL);

        /* 查找单元格之前要确保其中有内容 否则会段错误 */
		c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
        xlsCellWrap(c, true);   /* 设置自动换行 */ 
		xlsCellFormat(c, FMT_TEXT);

        /* 部门 */
		iCol++;
        pLabel = ZhcnToUincode((char *)"研发中心");
		xlsWorksheetLabelW(ws, iRow, iCol, pLabel, NULL);

        c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
        xlsCellWrap(c, true);   /* 设置自动换行 */ 
		xlsCellFormat(c, FMT_TEXT);

        /* 班次 */
        iCol++;
		xlsWorksheetNumberDbl(ws, iRow, iCol, 1.0, NULL);
		
		c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
		
		iCol++;

		/* 卡号 */
		sprintf(acCardNum, "%s", "10000000");
		//xlsWorksheetNumberDbl(ws, Row, Col, pUser->card, NULL);
		xlsWorksheetLabel(ws, iRow, iCol, acCardNum, NULL);
		
		c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
		xlsCellFormat(c, FMT_TEXT);
		
		iRow++;
    }
    
    memset(caExcelName, 0, sizeof(caExcelName));
    memset(caTmpName, 0, sizeof(caTmpName));

    sprintf(caExcelName, "/root/%s.xls", (char *)"Eexcel表格");

    //GB2312ToUTF_8(caTmpName, caName, strlen(caName));
    GBKBufToUTF8Buf(caExcelName, strlen(caExcelName), caTmpName);

	xlsWorkbookDump(w, caTmpName);

    sync();

	xlsDeleteWorkbook(w);

    return 0;
}

int main(int argc, char *argv[])
{
    int ret = -1;

    ret = CreateTestExcel(100);
    return 0;
}


