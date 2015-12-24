/********************************************************************************
**  Copyright (c) 2010, 
**  All rights reserved.
**	
**  �ļ�˵��: xlslib excel�ӿ��ļ�ʵ�֡�
**  ��������: 2014.02.28
**
**  ��ǰ�汾��1.0
**  ���ߣ�
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
    
    pLabel = ZhcnToUincode((char *)"����");
    
    ws = xlsWorkbookSheetW(w, pLabel);
    
    //xlsWorksheetMerge(ws, 0, 0, 0, 4);
    xlsCellMerge(ws, 0, 0, 0, 4, NULL);
    pLabel = ZhcnToUincode((char *)"Excel��Ϣ��");

    xlsWorksheetLabelW(ws, 0, 0, pLabel, NULL);
    xlsWorksheetRowheight(ws, 0, (unsigned16_t)XLS_HEIGHT_OF_ROW, NULL);
    c = xlsWorksheetFindCell(ws, 0, 0);

    xlsCellFontheight(c, (unsigned16_t)XLS_FONT_HEIGHT);
    
    /* ����0��0λ�õ�Ԫ��Ӵ� */
	xlsCellFontbold(c, BOLDNESS_BOLD);

    /* ����0��0λ�õ�Ԫ��ˮƽ������� */
 	xlsCellHalign(c, HALIGN_CENTER);
    xlsCellValign(c, VALIGN_CENTER);
    
#if 1
    
	for(iTmpCol = 0; iTmpCol <= 4; iTmpCol++)
	{
        /* ���ҵ�Ԫ��֮ǰҪȷ������������ �����δ��� */
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}
#endif
    
	//c = xlsWorksheetFindCell(ws, 0, 4);
	//xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);

    /* �ϲ���1�е�0-4�� */
	xlsWorksheetMerge(ws, 1, 0, 1, 4);	
    
    pLabel = ZhcnToUincode((char *)"��ʾ��1���������Ϊ9λ�������2�������������7���֣�"
   		"Ӣ�����15���֣������3�������������7���֣�Ӣ�����15���֣������"
   		"4���������ʱ�а����1~5���ް����0�������"
   		"5���������10λ��ѡ���");

    /* ���õ�һ�б������ */
	xlsWorksheetLabelW(ws, 1, 0, pLabel, NULL);
    c = xlsWorksheetFindCell(ws, 1, 0);//���ҵ�Ԫ��

    /* �����Զ����� */
    xlsCellWrap(c, true);

    /* �����и� */
	xlsWorksheetRowheight(ws, 1, (unsigned16_t)60 * 20, NULL);
    
	//c = xlsWorksheetFindCell(ws, 1, 4);
	//xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);

	pLabel = ZhcnToUincode((char *)"����");
	xlsWorksheetLabelW(ws, 2, 0, pLabel, NULL);         /* ���õ�Ԫ������ */
	c = xlsWorksheetFindCell(ws, 2, 0);                 /* ѡ����Ԫ�� */
	xlsCellFillstyle(c, FILL_SOLID);                    /* ��䵥Ԫ�� */
	xlsCellFillfgcolor(c, CLR_GRAY25);                  /* ��ɫ */

	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);  /* ���õ�Ԫ��߿� */
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
	xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);

    pLabel = ZhcnToUincode((char *)"����");
   	xlsWorksheetLabelW(ws, 2, 1, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 1);//���ҵ�Ԫ��
	xlsCellFillstyle(c, FILL_SOLID);
    xlsCellFillfgcolor(c, CLR_GRAY25);
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
    xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
	   
	pLabel = ZhcnToUincode((char *)"����");
	xlsWorksheetLabelW(ws, 2, 2, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 2);//���ҵ�Ԫ��
	xlsCellFillstyle(c, FILL_SOLID);
	xlsCellFillfgcolor(c, CLR_GRAY25);
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
	xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);

	pLabel = ZhcnToUincode((char *)"���");
	xlsWorksheetLabelW(ws, 2, 3, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 3);//���ҵ�Ԫ��	
	xlsCellFillstyle(c, FILL_SOLID);
	xlsCellFillfgcolor(c, CLR_GRAY25);
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	
	xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
	
	pLabel = ZhcnToUincode((char *)"����");
	xlsWorksheetLabelW(ws, 2, 4, pLabel, NULL);
	c = xlsWorksheetFindCell(ws, 2, 4);//���ҵ�Ԫ��
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
        /* id�� */
        iCol = 0;
        xlsWorksheetNumberInt(ws, iRow, iCol, (iId++), NULL);

        c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
    	xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);        
		xlsCellFormat(c, FMT_TEXT);

        /* ������ */
        iCol++;
        pLabel = ZhcnToUincode((char *)"������");
	    xlsWorksheetLabelW(ws, iRow, iCol, pLabel, NULL);

        /* ���ҵ�Ԫ��֮ǰҪȷ������������ �����δ��� */
		c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
        xlsCellWrap(c, true);   /* �����Զ����� */ 
		xlsCellFormat(c, FMT_TEXT);

        /* ���� */
		iCol++;
        pLabel = ZhcnToUincode((char *)"�з�����");
		xlsWorksheetLabelW(ws, iRow, iCol, pLabel, NULL);

        c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
        xlsCellWrap(c, true);   /* �����Զ����� */ 
		xlsCellFormat(c, FMT_TEXT);

        /* ��� */
        iCol++;
		xlsWorksheetNumberDbl(ws, iRow, iCol, 1.0, NULL);
		
		c = xlsWorksheetFindCell(ws, iRow, iCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);
		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
		
		iCol++;

		/* ���� */
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

    sprintf(caExcelName, "/root/%s.xls", (char *)"Eexcel���");

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


