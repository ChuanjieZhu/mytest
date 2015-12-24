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

#include<locale.h>

#include "extformat.h"

#include "publicLib.h"
#include "attendanceFunc.h"

#include "xlslib_rw.h"

#include "xls.h"

#include "gbcode.h"

#include "xlslib.h"
#include "appLib.h"
#include "attendanceLib.h"
#if 0
using namespace xlslib_core;
#endif 
using namespace xls;
using namespace std;

FILE* m_xls = NULL;
char* pcUserInfo = NULL;

xf_t * g_Xf_t[18]={0};

/* ���� */
typedef enum
{
  NOBORDER_BOLD_14 = 0,	/* ����14�żӴ֣��ޱ߿򣬺�������  */
  NOBORDER_12, 			/* ����12�Ų��Ӵ֣��ޱ߿򣬺���������� */
  BORDER_FILL_12,		/* ����12�żӴ֣�4�߿򣬻�ɫ���������������� */
  BORDER_RED_9,			/* ����9�ź�ɫ��4�߿򣬺������� */
  BORDER_RED_LEFT_9,	/* ����9�ź�ɫ��4�߿򣬺������ */
  BORDER_9,				/* ����9�ź�ɫ��4�߿򣬺������� */
  BORDER_RACCROSS_9,	/* ����9�ź�ɫ��4�߿򣬺����˶��������� */
  BORDER_10,			/* ����10�ź�ɫ��4�߿򣬺������� */
  BORDER_LEFT_10,		/* ����10�ź�ɫ��4�߿򣬺��������� */
  BORDER_CENTERACCROSS_FILL_10,	/* ����10�ź�ɫ��4�߿򣬻�ɫ����������о��������� */
  NOBORDER_LEFT_10,		/* ����10�ź�ɫ���ޱ߿򣬺��������� */
  BORDER_LEFT_FILL_10,	/* ����10�ź�ɫ��4�߿򣬻�ɫ���������������� */
  NOBORDER_BOLD_20,		/* ����20�żӴ֣��ޱ߿򣬺�������  */
  BORDER_LEFT_Text_10,	/* ����10�ź�ɫ��4�߿򣬺��������� �ı� */
  BORDER_RED_FILL_LEFT_10,	/* ����10�ź�ɫ��4�߿򣬺��������� ��ɫ������ */
  BORDER_RED_FILL_LEFT_Text_10,	/* ����10�ź�ɫ��4�߿��ı����������� ��ɫ������ */
  BORDER_RED_LEFT_TEXT_10,	/* ����10�ź�ɫ��4�߿򣬺��������� �ı�*/
  BORDER_LEFT_FILL_NOTRAP_10, /* ����10�ź�ɫ��4�߿򣬻�ɫ���������������� */
} Xft_Font;


#define XLS_FORMAT

/* ��Ԫ�����ú궨�� */
#define XLS_LABEL_DEFAULT_INFO(ws, row, col, pLabel, c, xf)     \
   	xlsWorksheetLabelW(ws, row, col, pLabel, NULL);             \
	c = xlsWorksheetFindCell(ws, row, col);/* ���ҵ�Ԫ�� */     \
	xlsCellFillstyle(c, FILL_SOLID);                        \
    xlsCellFillfgcolor(c, CLR_GRAY25);/* ��� */            \
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);      \
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	        \
    xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);       \
    xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);        \
    xlsCellWrap(c, true);   /* �����Զ����� */              \
 	xlsCellHalign(c, HALIGN_LEFT);                        \
    xlsCellValign(c, VALIGN_CENTER);                        \

#define XLS_CELL_DEFAULT_FORMAT(ws, Row, Col, c, xf)        \
		c = xlsWorksheetFindCell(ws, Row, Col);             \
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);  \
   		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);   \
   		xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);        \
        xlsCellWrap(c, true);   /* �����Զ����� */              \
     	xlsCellHalign(c, HALIGN_LEFT);                        \
        xlsCellValign(c, VALIGN_CENTER);                        \
		
extern void GB2312ToUTF_8(char *pOut,char *pText, int pLen);

xf_t *xlsWorkbookxFormatDefaultEx(workbook *w, int n)
{
	return NULL;
}

/**************************************************************\
** �������ƣ� xlsCellMerge
** ���ܣ�     �ϲ���Ԫ��
** ������     
** ���أ�    
** �������ߣ����ڻ�
** �������ڣ�2013-10-21
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int  xlsCellMerge(worksheet *wsRecord,int sRow,int sCol, int eRow, int eCol, xf_t * pXf_t)
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

/* �������:���� */
font_t* AddFont_Bold(workbook *w, int hight)
{
	font_t* pFont_t = NULL;
	char tmp[24] = {0};

	sprintf(tmp, "Bold%d", hight);

	pFont_t = xlsWorkbookFont(w, tmp);

	if(pFont_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	xlsFontSetHeight(pFont_t, (unsigned16_t)hight*20);
	/* ���� */
	xlsFontSetBoldStyle(pFont_t, BOLDNESS_BOLD);

	return pFont_t;
}

/* �������:Ĭ�ϷǴ����ɫ */
font_t* AddFont_Default(workbook *w, int hight)
{
	font_t* pFont_t = NULL;

	char tmp[24] = {0};

	sprintf(tmp, "default%d", hight);


	pFont_t = xlsWorkbookFont(w, tmp);

	if(pFont_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	//xlsFontSetColor(pFont_t, CLR_RED);

	xlsFontSetHeight(pFont_t, (unsigned16_t)hight * 20);
	/* ���� */
	//xlsFontSetBoldStyle(pFont_t, BOLDNESS_BOLD);

	return pFont_t;
}

/* �������:��ɫ�Ǵ��� */
font_t* AddFont_Red(workbook *w, int hight)
{
	font_t* pFont_t = NULL;

	char tmp[24] = {0};

	sprintf(tmp, "Red%d", hight);

	pFont_t = xlsWorkbookFont(w, tmp);

	if(pFont_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	xlsFontSetColor(pFont_t, CLR_RED);

	xlsFontSetHeight(pFont_t, (unsigned16_t)hight * 20);
	/* ���� */
	//xlsFontSetBoldStyle(pFont_t, BOLDNESS_BOLD);

	return pFont_t;
}

/* ���ӵ�Ԫ���ʽ���ޱ߿򣬺�������,����ΪNULLʱ��ʹ��Ĭ������ */
xf_t * AddCellXFT_NoBorder(workbook *w, font_t* pFont_t, halign_option_t ha_option )
{
	xf_t *pXf_t = NULL;

	if(pFont_t)
	{
		pXf_t = xlsWorkbookxFormatFont(w, pFont_t);
	}
	else
	{
		pXf_t = xlsWorkbookxFormat(w);
	}

	if(pXf_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	/* ���� */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);
	xlsXformatSetWrap(pXf_t, true);   /* �����Զ����� */

	return pXf_t;
}

/* ���ӵ�Ԫ���ʽ���߿򣬺���������, ��ɫ���� ����ΪNULLʱ��ʹ��Ĭ������ */
xf_t * AddCellXFT_Center_Fill(workbook *w, font_t* pFont_t, halign_option_t ha_option )
{
	xf_t *pXf_t = NULL;

	if(pFont_t)
	{
		pXf_t = xlsWorkbookxFormatFont(w, pFont_t);
	}
	else
	{
		pXf_t = xlsWorkbookxFormat(w);
	}

	if(pXf_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	/* ���� */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);

	/* �߿� */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
		/* ��� */
	xlsXformatSetFillStyle(pXf_t, FILL_SOLID);
	xlsXformatSetFillFGColor(pXf_t, CLR_GRAY25);
	xlsXformatSetWrap(pXf_t, true);   /* �����Զ����� */

	return pXf_t;
}
/* ���ӵ�Ԫ���ʽ����������, �߿� ����ΪNULLʱ��ʹ��Ĭ��10������ */
xf_t * AddCellXFT_Border(workbook *w, font_t* pFont_t, halign_option_t ha_option )
{
	xf_t *pXf_t = NULL;

	if(pFont_t)
	{
		pXf_t = xlsWorkbookxFormatFont(w, pFont_t);
	}
	else
	{
		pXf_t = xlsWorkbookxFormat(w);
	}

	if(pXf_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);
	/* �߿� */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
	/* �ı���ʽ */
    //xlsXformatSetFormat(pXf_t, FMT_TEXT);

	xlsXformatSetWrap(pXf_t, true);   /* �����Զ����� */

	return pXf_t;
}

/* ���ӵ�Ԫ���ʽ����������, �߿��ı� ����ΪNULLʱ��ʹ��Ĭ��10������ */
xf_t * AddCellXFT_Border_Text(workbook *w, font_t* pFont_t, halign_option_t ha_option )
{
	xf_t *pXf_t = NULL;

	if(pFont_t)
	{
		pXf_t = xlsWorkbookxFormatFont(w, pFont_t);
	}
	else
	{
		pXf_t = xlsWorkbookxFormat(w);
	}

	if(pXf_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);
	/* �߿� */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
	/* �ı���ʽ */
    xlsXformatSetFormat(pXf_t, FMT_TEXT);

	xlsXformatSetWrap(pXf_t, true);   /* �����Զ����� */

	return pXf_t;
}


/* ���ӵ�Ԫ���ʽ���߿򣬺���������, ��ɫ���� ����ΪNULLʱ��ʹ��Ĭ������ �ı� */
xf_t * AddCellXFT_Center_FillText(workbook *w, font_t* pFont_t, halign_option_t ha_option )
{
	xf_t *pXf_t = NULL;

	if(pFont_t)
	{
		pXf_t = xlsWorkbookxFormatFont(w, pFont_t);
	}
	else
	{
		pXf_t = xlsWorkbookxFormat(w);
	}

	if(pXf_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	/* ���� */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);

	/* �߿� */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
		/* ��� */
	xlsXformatSetFillStyle(pXf_t, FILL_SOLID);
	xlsXformatSetFillFGColor(pXf_t, CLR_GRAY25);
	xlsXformatSetWrap(pXf_t, true);   /* �����Զ����� */

	/* �ı���ʽ */
    xlsXformatSetFormat(pXf_t, FMT_TEXT);

	return pXf_t;
}

/* ���ӵ�Ԫ���ʽ���߿򣬺���������, ��ɫ���� ����ΪNULLʱ��ʹ��Ĭ������ ��ʹ���Զ����� */
xf_t * AddCellXFT_Center_NoTrap_Fill(workbook *w, font_t* pFont_t, halign_option_t ha_option )
{
	xf_t *pXf_t = NULL;

	if(pFont_t)
	{
		pXf_t = xlsWorkbookxFormatFont(w, pFont_t);
	}
	else
	{
		pXf_t = xlsWorkbookxFormat(w);
	}

	if(pXf_t == NULL)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	/* ���� */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);

	/* �߿� */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
		/* ��� */
	xlsXformatSetFillStyle(pXf_t, FILL_SOLID);
	xlsXformatSetFillFGColor(pXf_t, CLR_GRAY25);
	//xlsXformatSetWrap(pXf_t, true);   /* �����Զ����� */

	return pXf_t;
}

int initParticulXFTFont(workbook *w)
{
	font_t* pFont_t = NULL;
	
	xf_t * pXf_t = NULL;

	memset(g_Xf_t, 0, sizeof(g_Xf_t));

	/* ���ӵ�Ԫ��ʽ:����14�żӴ֣��ޱ߿򣬺������� */
	pFont_t = AddFont_Bold(w, 14);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t,HALIGN_CENTER);
	g_Xf_t[NOBORDER_BOLD_14] =  pXf_t;

	/* ���ӵ�Ԫ��ʽ:����12�Ų��Ӵ֣��ޱ߿򣬺���������� */
	pFont_t = AddFont_Default(w, 12);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t, HALIGN_LEFT);
	g_Xf_t[NOBORDER_12] =  pXf_t;

	/* ���ӵ�Ԫ��ʽ:����12�żӴ֣�4�߿򣬻�ɫ�������������� */
	pFont_t = AddFont_Bold(w, 12);
	pXf_t = AddCellXFT_Center_Fill(w,pFont_t,HALIGN_LEFT);
	g_Xf_t[BORDER_FILL_12] =  pXf_t;

	/* ���ӵ�Ԫ��ʽ:����9�ź�ɫ��4�߿򣬺������� */
	pFont_t = AddFont_Red(w, 9);
	pXf_t = AddCellXFT_Border(w,pFont_t,HALIGN_CENTER);
	g_Xf_t[BORDER_RED_9] =  pXf_t;

	/* ���ӵ�Ԫ��ʽ:�����ɫ9�ţ�4�߿򣬺��������� */
	pXf_t = AddCellXFT_Border(w,pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_LEFT_9] =  pXf_t;
	/* ���ӵ�Ԫ��ʽ:����14�żӴ֣��ޱ߿򣬺������� */
	pFont_t = AddFont_Bold(w, 14);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t, HALIGN_CENTER);
	g_Xf_t[NOBORDER_BOLD_14] =  pXf_t;
	/* ���ӵ�Ԫ��ʽ:�����ɫ9�ţ�4�߿򣬺������� */
	pFont_t = AddFont_Default(w, 9);
	pXf_t = AddCellXFT_Border(w,pFont_t, HALIGN_CENTER);
	g_Xf_t[BORDER_9] =  pXf_t;
	
	/* ���ӵ�Ԫ��ʽ:�����ɫ9�ţ�4�߿򣬺����߶��������� */
	pXf_t = AddCellXFT_Border(w,pFont_t, HALIGN_JUSTIFY);
	g_Xf_t[BORDER_RACCROSS_9] =  pXf_t;

	/* ���ӵ�Ԫ��ʽ:����10��4�߿򣬺������� */
	pXf_t = AddCellXFT_Border(w, NULL,HALIGN_CENTER);
	g_Xf_t[BORDER_10] =  pXf_t;
	
	/* ���ӵ�Ԫ��ʽ:����10��4�߿򣬺��������� */
	pXf_t = AddCellXFT_Border(w, NULL,HALIGN_LEFT);
	g_Xf_t[BORDER_LEFT_10] =  pXf_t;

	/*���ӵ�Ԫ��ʽ:����10��4�߿򣬺���о���������*/
	pXf_t = AddCellXFT_Center_Fill(w, NULL, HALIGN_CENTERACCROSS);
	g_Xf_t[BORDER_CENTERACCROSS_FILL_10] =  pXf_t;

	/*���ӵ�Ԫ��ʽ:����10���ޱ߿򣬺���������*/
	pXf_t = AddCellXFT_NoBorder(w, NULL, HALIGN_LEFT);
	g_Xf_t[NOBORDER_LEFT_10] =  pXf_t;

	/* ���ӵ�Ԫ��ʽ:����20�żӴ֣��ޱ߿򣬺������� */
	pFont_t = AddFont_Bold(w, 20);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t, HALIGN_CENTER);
	g_Xf_t[NOBORDER_BOLD_20] =  pXf_t;

	/*���ӵ�Ԫ��ʽ:����10���ޱ߿򣬻�ɫ����������������*/
	pXf_t = AddCellXFT_Center_Fill(w, NULL, HALIGN_LEFT);
	g_Xf_t[BORDER_LEFT_FILL_10] =  pXf_t;

    /*���ӵ�Ԫ��ʽ:����10���ޱ߿򣬻�ɫ��������������,���Զ�����*/
	pXf_t = AddCellXFT_Center_NoTrap_Fill(w, NULL, HALIGN_CENTER);
	g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10] =  pXf_t;

	/*���ӵ�Ԫ��ʽ:����10��4�߿򣬺��������� �ı� */
	pXf_t = AddCellXFT_Border_Text(w, NULL, HALIGN_LEFT);
	g_Xf_t[BORDER_LEFT_Text_10] =  pXf_t;

	/*���ӵ�Ԫ��ʽ:����10��4�߿򣬺��������� �ı� */
	pFont_t = AddFont_Red(w, 10);
	pXf_t = AddCellXFT_Center_Fill(w, pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_FILL_LEFT_10] =  pXf_t;

	/*���ӵ�Ԫ��ʽ:����10��4�߿򣬺��������� �ı� */
	pFont_t = AddFont_Red(w, 10);
	pXf_t = AddCellXFT_Center_FillText(w, pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_FILL_LEFT_Text_10] =  pXf_t;

	/*���ӵ�Ԫ��ʽ:����10��4�߿򣬺��������� �ı� */
	pFont_t = AddFont_Red(w, 10);
	pXf_t = AddCellXFT_Border_Text(w, pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_LEFT_TEXT_10] =  pXf_t;
	return 0;
}

/**************************************************************\
** �������ƣ� ParticulUserHaedInfo
** ���ܣ�     ������ϸ��ͨ�ñ�ͷ
** ������     
** ���أ�    
** �������ߣ����ڻ�
** �������ڣ�2013-10-21
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ParticulUserHaedInfo(worksheet *wsRecord, int iUserNo, char* szUserName, int iSh, TIMERANGE timeRange )
{
	wchar_t *pLabel = NULL;
	int Col = 0;
	int Row = 0;
	char buf[1024] = {0};
	char tmpbuf[1024] = {0};
	char * pcTmp = NULL;
    int iTmpCol = 0;
    cell_t *c = NULL;

	CONFIG_BASE_STR strConfigBase;
	memset(&strConfigBase, 0, sizeof(CONFIG_BASE_STR));
	GetConfigBase(&strConfigBase);

	sprintf(buf,"%s", (char *)GetLangText(11942, "������ϸ��"));

	//xf = xlsWorkbookxFormatDefaultEx(w, 5);

	/* ��ѯ�������ϲ� д���ͷ */
	xlsWorksheetMerge(wsRecord, 0, 0, 0, (XLS_ORI_RECORD_TAB_COLS - 2));
	/* �����и�25���� */
	xlsWorksheetRowheight(wsRecord, 0, (unsigned16_t)60 * 20, NULL);
    pLabel = ZhcnToUincode(buf);
    xlsWorksheetLabelW(wsRecord, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_14]);

    for(iTmpCol = 0; iTmpCol < XLS_PARTICUL_TABLE_COLS; iTmpCol++)
	{
		c = xlsWorksheetFindCell(wsRecord, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}

	xlsWorksheetMerge(wsRecord, 1, 0, 1, (XLS_ORI_RECORD_TAB_COLS - 2));
	xlsWorksheetRowheight(wsRecord, 1, (unsigned16_t)20*20, NULL);
	sprintf(buf,"%s:%04d-%02d-%02d %s %04d-%02d-%02d", (char *)GetLangText(10013, "����"), timeRange.iStartYear, timeRange.iStartMonth,
		timeRange.iStartDay, (char *)GetLangText(12140, "��"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
	pLabel = ZhcnToUincode(buf);
    xlsWorksheetLabelW(wsRecord, 1, 0, pLabel, g_Xf_t[NOBORDER_12]);

	Row = 2;
	Col = 0;
	xlsWorksheetRowheight(wsRecord, 3, (unsigned16_t)20*20, NULL);
	/* ���źϲ�����д */
	sprintf(buf, "%s:%d", (char *)GetLangText(11295, "����"), iUserNo);
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* �����ϲ�����д */
	sprintf(buf, "%s:%s", (char *)GetLangText(10006, "����"), szUserName);
	Col = 3;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* ���źϲ�����д */
	sprintf(buf, "%s:%s",(char *)GetLangText(10012, "����"), "");
	Col = 6;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* ��κϲ�����д */
	memset(tmpbuf, 0, sizeof(tmpbuf));
	 /* ������� */
	pcTmp = GetScheduleNameByNo(iSh);
	if (pcTmp)
	{
	    memcpy(tmpbuf, pcTmp, 32);
	}
	else
	{
		sprintf(tmpbuf,"%s",(char *)GetLangText(12155, "��"));
	}

	sprintf(buf, "%s:%s",(char *)GetLangText(12144, "���"), tmpbuf);

	Col = 9;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* ���ںϲ�����д */
	sprintf(buf, "%s:%d.%d~%d.%d",(char *)GetLangText(10013, "����"), timeRange.iStartMonth,timeRange.iStartDay,
				timeRange.iEndMonth,timeRange.iEndDay);

	Col = 12;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+3, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	Row = 4;
	xlsWorksheetLabel(wsRecord, Row, 0, "", g_Xf_t[BORDER_10]);
	xlsWorksheetLabel(wsRecord, Row, 1, "", g_Xf_t[BORDER_10]);

	Col = 2;
	/* ��߰��һ */	
	pLabel = ZhcnToUincode((char *)GetLangText(11928, "���һ"));
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col = 10;
	/* �ұ߰��һ */
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);
	
	/* ��߰�ζ� */
	Col = 4;
	pLabel = ZhcnToUincode((char *)GetLangText(11929, "��ζ�"));
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	/* �ұ߰�ζ� */
	Col = 12;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	/* ��߰���� */
	Col = 6;
	pLabel = ZhcnToUincode((char *)GetLangText(11930, "�����"));
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	/* �ұ߰���� */
	Col = 14;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	xlsWorksheetLabel(wsRecord, Row, 8, "", g_Xf_t[BORDER_10]);
	xlsWorksheetLabel(wsRecord, Row, 9, "", g_Xf_t[BORDER_10]);

	Row++;
	Col = 0;
	pLabel = ZhcnToUincode((char *)GetLangText(10013, "����"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12175, "����"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col = 8;
	pLabel = ZhcnToUincode((char *)GetLangText(10013, "����"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12175, "����"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col = 2;
	sprintf(tmpbuf, "%s(IN)",(char *)GetLangText(11403, "�ϰ�"));
	pLabel = ZhcnToUincode(tmpbuf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);
	
	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	Col+=4;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	Col = 3;
	sprintf(tmpbuf, "%s(OUT)",(char *)GetLangText(11404, "�°�"));
	pLabel = ZhcnToUincode(tmpbuf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);
	
	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	Col+=4;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);
	
	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);
	
	Col+=2;
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RACCROSS_9]);

	TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
	return 0;
}

int MonthRecordHadInfo(worksheet *ws,TIMERANGE timeRange )
{
	char buf[1024] = {0};
	int Col = 0;
	int Row = 0;
	wchar_t *pLabel = NULL;
    int iTmpCol = 0;
    cell_t *c = NULL;

	int  aiColWidth[XLS_MONTH_TABLE_COLS] = {(32), (56),
                                             (56), (56),
                                             (40), (40),
                                             (40), (40),
                                             (40), (40),
                                             (40), (40),
                                             (40), (40)};

	sprintf(buf, (char *)GetLangText(12143, "��ͳ�Ʊ�"));

    ws->defaultColwidth(8); /* ����Ĭ�Ͽ�ȣ���������Ϊ8��xlsWorksheetColwidth�Ż������� */

	Row = 0;
	Col = 0;

    for(iTmpCol = 0; iTmpCol < XLS_MONTH_TABLE_COLS; iTmpCol++)
	{
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}

	//xlsCellMerge(ws,Row,Col, Row, XLS_MONTH_TABLE_COLS - 1, g_Xf_t[NOBORDER_BOLD_14]);
	xlsWorksheetMerge(ws, Row, Col, Row, Col+(XLS_MONTH_TABLE_COLS - 1));
	xlsWorksheetRowheight(ws, Row, (unsigned16_t)60*20, NULL);
    pLabel = ZhcnToUincode(buf);
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

    for (Col = 0; Col < XLS_MONTH_TABLE_COLS; Col++)
    {
        xlsWorksheetColwidth(ws, Col, (aiColWidth[Col])*32, NULL);
    }

	Row++;
  	xlsWorksheetMerge(ws, Row, Col, Row, Col+(XLS_MONTH_TABLE_COLS - 1));
	xlsWorksheetRowheight(ws, Row, (unsigned16_t)22*20, NULL);
	sprintf(buf,"%s:%04d-%02d-%02d %s %04d-%02d-%02d", (char *)GetLangText(10013, "����"), timeRange.iStartYear, timeRange.iStartMonth,
		timeRange.iStartDay, (char *)GetLangText(12140, "��"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
	pLabel = ZhcnToUincode(buf);
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[NOBORDER_12]);

	/* д���ͷ */
	Row = 2;
	Col = 0;

	if (GetLangID() ==  1)
	{
		xlsWorksheetRowheight(ws, Row, (unsigned16_t)60*20, NULL);   /* �����и� */
	}
	else
	{
		xlsWorksheetRowheight(ws, Row, (unsigned16_t)30*20, NULL);   /* �����и� */
	}

	//xlsWorksheetRowheight(ws, Row, (unsigned16_t)15 * 20, NULL);
	pLabel = ZhcnToUincode((char *)GetLangText(11295, "����"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);  

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(10006, "����"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]); 

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(10012, "����"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]); 

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12144, "���"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12145, "��������"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12146, "��������"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12147, "ȱ������"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12148, "�ٵ�����"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12149, "�ٵ�����"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12150, "���˷���"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12151, "���˴���"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12152, "�Ӱ�Сʱ"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12153, "�������"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12154, "��������"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

	return 0;
}

/**************************************************************\
** �������ƣ� ExportUserInfo
** ���ܣ�     ������Ա��Ϣ��
** ������     pUser:��Ա��Ϣ����ͷָ��
              UserNum:��Ա����
** ���أ�     0:�����ɹ�
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ExportUserInfo(LPUSER_LIST_ATT pUser,int UserNum)
{
	workbook *w;
	worksheet *ws;
    wchar_t *pLabel = NULL;
	cell_t *c = NULL;
	int Col = 0;
	int Row = 0;
	int iTmpCol = 0;
	char cardNum[12] = {0};
    char caName[128] = {0};
    char caTmpName[128] = {0};
    
	w = xlsNewWorkbook();

	/* ��ʼ������ */
	initParticulXFTFont(w);
	
    pLabel = ZhcnToUincode((char *)GetLangText(12123, "��Ա��Ϣ��"));
	ws = xlsWorkbookSheetW(w, pLabel);

    //xlsCellMerge(ws,0,0, 0, 4, g_Xf_t[NOBORDER_BOLD_20]);

	xlsWorksheetMerge(ws, 0, 0, 0, 4);
	xlsWorksheetRowheight(ws, 0, (unsigned16_t)60*20, NULL);
    pLabel = ZhcnToUincode((char *)GetLangText(12123, "��Ա��Ϣ��"));
    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

	for(iTmpCol = 0; iTmpCol <= 4; iTmpCol++)
	{
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}
	
	xlsWorksheetMerge(ws, 1, 0, 1, 4);	
	xlsWorksheetRowheight(ws, 1, (unsigned16_t)60*20, NULL);
	pLabel = ZhcnToUincode((char *)GetLangText(12156, "��ʾ��1.�������Ϊ9λ�����,2.�����������7���֣�Ӣ�����15���֣����,3.���ű������ʱ��1~16�����,4.�������ʱ�а����1~5,�ް����0�����,5.�������10λ��ѡ���"));
	xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);

	pLabel = ZhcnToUincode((char *)GetLangText(11295, "����"));
	xlsWorksheetLabelW(ws, 2, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

    pLabel = ZhcnToUincode((char *)GetLangText(10006, "����"));
   	xlsWorksheetLabelW(ws, 2, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	   
	pLabel = ZhcnToUincode((char *)GetLangText(10007, "���ű��"));
	xlsWorksheetLabelW(ws, 2, 2, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	pLabel = ZhcnToUincode((char *)GetLangText(12144, "���"));
	xlsWorksheetLabelW(ws, 2, 3, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	
	pLabel = ZhcnToUincode((char *)GetLangText(10004, "����"));
	xlsWorksheetLabelW(ws, 2, 4, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	/*ѭ��д��ÿ����Ա��Ϣ*/
	Row = 3;

	while(pUser != NULL && Row < (UserNum + 3))
	{
		TRACE("pUser->userno %d pUser->sUserName %s pUser->sDepName %s pUser->schedule %d pUser->card %d %s %d\r\n", 
			(int)pUser->userno , pUser->sUserName, pUser->sDepName, pUser->schedule, (int)pUser->card, __FUNCTION__,__LINE__);

		Col = 0;

		/* ���� */
		if(pUser->userno != 0)
		{
			xlsWorksheetNumberInt(ws, Row, Col, pUser->userno, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}
		
		Col++;
		
        /* �û��� */
		if(pUser->sUserName[0] != '\0')
		{
			pLabel = ZhcnToUincode(pUser->sUserName);
			xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}
				
		Col++;

        /* ������ */
		if(pUser->sDepName[0] != '\0')
		{
			pLabel = ZhcnToUincode(pUser->sDepName);
			xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_Text_10]);
		}
        else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}

		Col++;

		/* ��� */
		if(pUser->schedule >= 0)
		{
			xlsWorksheetNumberDbl(ws, Row, Col, pUser->schedule, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}

		Col++;

		 /* ���� */
		if( pUser->card > 0)
		{
			sprintf(cardNum,"%s",CardNumToStr(pUser->card));
			//xlsWorksheetNumberDbl(ws, Row, Col, pUser->card, NULL);
			xlsWorksheetLabel(ws, Row, Col, cardNum, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}
		
		Row++;
		
		pUser = pUser->pUserNext;
        
        AddRecordNum(3);    /* �����Ѵ��������ݶ� */
	}

	while(Row < 303)
	{
		Col = 0;
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		
		Col++;
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		
		Col++;
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		
		Col++;
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		
		Col++;
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		
		Row++;		
	}

	if (GetUsbStatus() == TRUE)
	{
        memset(caName, 0, sizeof(caName));
        memset(caTmpName, 0, sizeof(caTmpName));

        sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12123, "��Ա��Ϣ��"));

        GBKBufToUTF8Buf(caName, strlen(caName), caTmpName);

		xlsWorkbookDump(w, caTmpName);

        TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
	}

	sync();

	xlsDeleteWorkbook(w);
	
	return 0;
}

/******************************************************************************
 * �������ƣ� CheckImportUserNo
 * ���ܣ� ��鵼��Ĺ�����Ч��
 * ������ pcUserNo:�����ַ�������
 * ���أ� 0 ��ʾ��Ч ��1��ʾ��Ч
 * �������ߣ� Jason
 * �������ڣ� 2013-5-23
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int CheckImportUserNo(char* pcUserNo)
{
	int i = 0;
	
	if (pcUserNo == NULL)
	{
		return -1;
	}

	if (pcUserNo[0] == 0)
	{
		return -1;
	}

	/* �ж��Ƿ��в���0��9֮������� */
	for (i = 0; i < (int)strlen(pcUserNo); i++)
	{
		if ((pcUserNo[i] < 0x30 || pcUserNo[i] > 0x39)
			&& pcUserNo[i] != 0x2e)
		{
			return -1;
		}
	}

	/* ���Ų���Ϊ0 */
	if (i == 1 && pcUserNo[0] == 0x30)
	{
		return -1;
	}
	
	return 0;
}

/**************************************************************\
** ��������:ImportUserInfo
** ���ܣ�   �����û���Ϣ
** ������   pstrUserList:LPUSER_LIST_ATT��ָ��
** ���أ�   �û���Ϣ����ͷָ��LPUSER_LIST_ATT
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ImportUserInfo(LPUSER_LIST_ATT * pstrUserList)
{
	struct stat st;
    char acFilePath[128];
    char caName[128];
	int usernum = 0;	
	/* ����ͷָ�� */
	LPUSER_LIST_ATT pUserHead = NULL;  
	LPUSER_LIST_ATT pUserList = NULL; 
	LPUSER_LIST_ATT pUserTmp = NULL; 
	xlsWorkBook* pWB  = NULL;
	xlsWorkSheet* pWS = NULL;	

#if 0
	struct st_row_data* row;
#else
	xls::st_row::st_row_data *row;
#endif

	WORD usRowIndex;
	WORD usUserIndex;
	char cardNum[32] = {0};
	int len = 0;
	int k = 0;
	int iNullNum = 0;
	int iNullUserInfo = 0;
	int i = 0;
	int ret = 0;

	TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

    memset(acFilePath, 0, sizeof(acFilePath));
    memset(caName, 0, sizeof(caName));

    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12123, "��Ա��Ϣ��"));

    //GB2312ToUTF_8(acFilePath, caName, strlen(caName));
    GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);

	if(stat(acFilePath, &st) != 0)
    {
		TRACE("%s not exist! %s %d\r\n", acFilePath, __FUNCTION__, __LINE__);

		goto IMPORTERR;
    }

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

    pWB = xls_open(acFilePath, "UTF-8");
	if (pWB == NULL)
	{
		TRACE("Open xls failed! %s %d\r\n", __FUNCTION__, __LINE__);
		
		goto IMPORTERR;
	}
	else
	{
        TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

		/* ���ȡ�ı��ĵ�Ԫ����䲻��������ʹ��������������ʱ����ִ��� */
		pWS = xls_getWorkSheet(pWB, 0);		
		
		xls_parseWorkSheet(pWS);
		
		if(pWS->rows.lastrow < 3)		
		{
            TRACE("pWS->rows.lastrow %d %s %d\r\n", pWS->rows.lastrow, __FUNCTION__, __LINE__);

		    goto IMPORTERR;
		}

        TRACE("pWS->rows.lastrow %d %s %d\r\n", pWS->rows.lastrow, __FUNCTION__, __LINE__);

		for (usRowIndex = 3; usRowIndex <= pWS->rows.lastrow; usRowIndex++)		
		{	
			row = &pWS->rows.row[usRowIndex];
			
      		usUserIndex = 0;

			iNullNum = 0;
			iNullUserInfo = 0;
			
			for(i = 0; i < 5; i++)
			{
				if((((&row->cells.cell[i])->str != NULL)
						&&(0 == strcmp((const char*)(&row->cells.cell[i])->str, (const char*)" ")))
					||((&row->cells.cell[i])->str == NULL)
					||(&row->cells.cell[i])->str[0] == '\0')
				{
					if(i < 2)
					{
						iNullUserInfo += 1;
					}
					
					iNullNum += 1;
				}
			}

            /* ����������ݲ����������������� */
			if((0 < iNullUserInfo ) && (iNullNum < 5))
			{
                TRACE("iNullUserInfo %d iNullNum %d %s %d\r\n", iNullUserInfo, iNullNum, __FUNCTION__, __LINE__);

                continue;
                //goto IMPORTERR;
			}

            TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

			ret = CheckImportUserNo((char *)(&row->cells.cell[usUserIndex])->str);
			if (ret != 0)
			{
                TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
				continue;
			}
			
			if(5 == iNullNum)
			{
				continue;
			}

			usernum += 1;
			
			/* ����������һ���ڵ� */
			pUserTmp = (USER_LIST_ATT*)Malloc(sizeof(USER_LIST_ATT));
			if (pUserTmp != NULL)
			{
				memset(pUserTmp, 0, sizeof(USER_LIST_ATT));
			}
			else
			{
				TRACE("pUserTmp %p %s %d\r\n", pUserTmp, __FUNCTION__, __LINE__);
			}
			
			//if(3 == usRowIndex)
			if (pUserHead == NULL)
			{
				pUserHead = pUserTmp;
				pUserList = pUserHead;	
			}
			else
			{
				pUserList->pUserNext = pUserTmp;
				pUserList = pUserTmp;
			}
			
            /* ���� */ 
			if ((&row->cells.cell[usUserIndex])->str != NULL && (&row->cells.cell[usUserIndex])->str[0] != '\0')
			{
				pUserTmp->userno = strtoul((const char *)(&row->cells.cell[usUserIndex])->str, NULL, 10);
			}
			
			usUserIndex++;
			
			/* �û��� */
			if ((&row->cells.cell[usUserIndex])->str != NULL && (&row->cells.cell[usUserIndex])->str[0] != '\0')
			{
				memcpy(pUserTmp->sUserName, (&row->cells.cell[usUserIndex])->str, 32);
				
				while(pUserTmp->sUserName[0] == ' ')
				{
					memmove(pUserTmp->sUserName, &(pUserTmp->sUserName[1]), 31);
				}
				
				TRACE("pUserTmp->sUserName %s %s %d\r\n", pUserTmp->sUserName, __FUNCTION__, __LINE__);
			}

			usUserIndex++;

            TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
			
			/* ������ */
			if ((&row->cells.cell[usUserIndex])->str != NULL && (&row->cells.cell[usUserIndex])->str[0] != '\0')
			{
				memcpy(pUserTmp->sDepName, (&row->cells.cell[usUserIndex])->str, 32);

				while(pUserTmp->sDepName[0] == ' ')
				{
					memmove(pUserTmp->sDepName, &(pUserTmp->sDepName[1]), 31);
				}

                TRACE("pUserTmp->sDepName %s %s %d\r\n", pUserTmp->sDepName, __FUNCTION__, __LINE__);
		 	}
			
			usUserIndex++;
			
			/* ��� */
			if ((&row->cells.cell[usUserIndex])->str != NULL && (&row->cells.cell[usUserIndex])->str[0] != '\0')
			{
				pUserTmp->schedule = atoi((const char *)(&row->cells.cell[usUserIndex])->str);
			}
			else
			{
				/* δ������ʱ��Ĭ��Ϊ�����ް�� */
				pUserTmp->schedule = 0;
			}
			
			TRACE("pUserTmp->schedule %d %s %d\r\n", pUserTmp->schedule, __FUNCTION__, __LINE__);
			
			usUserIndex++;

			/* ���� */
			len = 0;

			if ((&row->cells.cell[usUserIndex])->str != NULL && (&row->cells.cell[usUserIndex])->str[0] != '\0')
			{				
				len = strlen((const char *)(&row->cells.cell[usUserIndex])->str);

				while((&row->cells.cell[usUserIndex])->str[0] == ' ')
				{
					memmove((&row->cells.cell[usUserIndex])->str, &((&row->cells.cell[usUserIndex])->str[1]), len);
				}
				
				len = 0;

				if(10 > strlen((const char *)(&row->cells.cell[usUserIndex])->str))
				{
					len = 10 - strlen((const char *)(&row->cells.cell[usUserIndex])->str);

					for(k = 0; k < len; k++)
					{
						cardNum[k] = '0';
					}					
				}
				
				memcpy(cardNum + len, (&row->cells.cell[usUserIndex])->str, 10 - len);
				cardNum[10] = '\0';
				
				pUserTmp->card = CardStrToNum(cardNum);
			}
			else
			{
				pUserTmp->card = 0;
			}
		}

        TRACE(" usernum %d %s %d\r\n",usernum, __FUNCTION__, __LINE__);

		xls_close_WS(pWS);

        TRACE(" usernum %d %s %d\r\n",usernum, __FUNCTION__, __LINE__);
	} 

    TRACE(" usernum %d %s %d\r\n",usernum, __FUNCTION__, __LINE__);

	*pstrUserList = pUserHead;

	xls_close_WB(pWB);

	TRACE(" usernum %d %s %d\r\n",usernum, __FUNCTION__, __LINE__);

	return usernum; //����ͷָ��

IMPORTERR:   /* �������֧ */
    
    if (pWS != NULL)
    {
	    xls_close_WS(pWS);
    }

    if (pWB != NULL)
    {
	    xls_close_WB(pWB);
    }

    while (pUserHead)
    {
        pUserTmp = pUserHead->pUserNext;
        Free(pUserHead);
        pUserHead = pUserTmp;
    }

    return -1;
}

/**************************************************************\
** �������ƣ� ExportHoliday
** ���ܣ�     �����ڼ��ձ�
** ������     �ڼ������� LP_ALL_HOLIDAY���ڼ�������Hodaynum
** ���أ�     Holiday.xls
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ExportHoliday(ALL_HOLIDAY Holiday)
{	
	workbook *w;
	worksheet *ws;
    wchar_t *pLabel = NULL;
	cell_t *c=NULL;
	int Col=0;
	int Row=0;
	int iTmpCol = 0;
	int Holidaynum = 0;
    char acFilePath[128];
    char caName[128];
    
	Holidaynum = Holiday.iHolidayNo;

	w = xlsNewWorkbook();

	/* ��ʼ������ */
	initParticulXFTFont(w);

	pLabel = ZhcnToUincode((char *)GetLangText(12125, "�ڼ���Ϣ��"));
	ws = xlsWorkbookSheetW(w, pLabel);	

	xlsWorksheetMerge(ws, 0, 0, 0, 3);
    pLabel = ZhcnToUincode((char *)GetLangText(12125, "�ڼ���Ϣ��"));
	xlsWorksheetRowheight(ws,0,(unsigned16_t)60*20, NULL);
    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

	for(iTmpCol = 0; iTmpCol <= 3; iTmpCol++)
	{
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}

	xlsWorksheetMerge(ws, 1, 0, 1, 3);
	xlsWorksheetRowheight(ws, 1, (unsigned16_t)60*20, NULL);
	pLabel = ZhcnToUincode((char *)GetLangText(12161, "��ʾ�� 1���ڼ����������7���ֻ�Ӣ�����15���֣���� 2�����24���ڼ��� 3��ʱ������ʱ��ʽΪYYYY-MM-DD"));
    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);
	
	//c = xlsWorksheetFindCell(ws, 1, 3);
	//xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
		
	xlsWorksheetLabel(ws, 2, 0, "ID", g_Xf_t[BORDER_LEFT_FILL_10]);
	
	pLabel = ZhcnToUincode((char *)GetLangText(12162, "�ڼ�������"));
	xlsWorksheetLabelW(ws, 2, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	
    pLabel = ZhcnToUincode((char *)GetLangText(12163, "��ʼ����"));
   	xlsWorksheetLabelW(ws, 2, 2, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	

	pLabel = ZhcnToUincode((char *)GetLangText(12164, "��������"));
   	xlsWorksheetLabelW(ws, 2, 3, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	/* ѭ��д��ÿ���ڼ��� */	
	Row = 3;	

	while(Row < (Holidaynum + 3))
	{
		Col = 0;
		
		xlsWorksheetNumberInt(ws, Row, Col, Row - 2, g_Xf_t[BORDER_LEFT_Text_10]); 
		
		Col++;

		if(Holiday.HoliDay[Row-3].szHldyName[0] != '\0')
		{
			pLabel = ZhcnToUincode(Holiday.HoliDay[Row - 3].szHldyName);
			xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}
		
		Col++;
		
		if(Holiday.HoliDay[Row-3].szStartTime[0] != '\0')
		{
			//pLabel = ZhcnToUincode(Holiday.HoliDay[Row-2].szStartTime);
			xlsWorksheetLabel(ws, Row, Col, Holiday.HoliDay[Row-3].szStartTime, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}

		Col++;
		
		if(Holiday.HoliDay[Row-3].szEndTime[0] != '\0')
		{
			//pLabel = ZhcnToUincode(Holiday.HoliDay[Row-2].szEndTime);
			xlsWorksheetLabel(ws, Row, Col, Holiday.HoliDay[Row-3].szEndTime, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}
		
		Row++;
	}	

	while(Row < 27)
	{
		Col = 0;

		xlsWorksheetNumberInt(ws, Row, 0, Row - 2, g_Xf_t[BORDER_LEFT_Text_10]);
		
		Col++;

		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		
		Col++;		
		
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		
		Col++;		
		
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		Row++;
	}

	if (GetUsbStatus() == TRUE)
	{
        memset(acFilePath, 0, sizeof(acFilePath));
        memset(caName, 0, sizeof(caName));

    	sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12125, "�ڼ���Ϣ��"));

        GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);

		xlsWorkbookDump(w, acFilePath);
	}
	
	sync();
	
	xlsDeleteWorkbook(w);

	return 0;
}

/**************************************************************\
** ��������:ImportHoday
** ���ܣ�   ����ڼ���
** ������   pHoliday :�ڼ�������
** ���أ�   -1:��ȡxls�ļ�ʧ��; >=0:�ڼ�����Ŀ
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ImportHoliday(LP_ALL_HOLIDAY* pstrHoliday)
{
	struct stat st;
    char acFilePath[128];
    char caName[128];
	int Holidaynum = 0;
	xlsWorkBook* pWB    = NULL;
	xlsWorkSheet* pWS   = NULL;
	LP_ALL_HOLIDAY pHoliday = NULL;
	int iHoliday = 0;

#if 0
	struct st_row_data* row;
#else
	xls::st_row::st_row_data *row;
#endif

	WORD t;
	WORD tt;
	int i = 0;
	int iNullNum = 0;

    memset(acFilePath, 0, sizeof(acFilePath));
    memset(caName, 0, sizeof(caName));

    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12125, "�ڼ���Ϣ��"));

    //GB2312ToUTF_8(acFilePath, caName, strlen(caName));
    GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);

    if(stat(acFilePath, &st) != 0)
    {
		TRACE("%s not exist! %s %d\r\n", acFilePath, __FUNCTION__, __LINE__);

		goto IMPORTERR;
    }
    
	pWB = xls_open(acFilePath, "UTF-8");

	if (pWB == NULL)
	{
		TRACE("Open xls failed! %s %d\r\n", __FUNCTION__, __LINE__);
		
		goto IMPORTERR;
	}
	else
	{
		/* ���ȡ�ı��ĵ�Ԫ����䲻��������ʹ��������������ʱ����ִ��� */
		pWS = xls_getWorkSheet(pWB, 0);	
		if (pWS == NULL)
		{
		    TRACE("xls_getWorkSheet xls failed! %s %d\r\n", __FUNCTION__, __LINE__);
		
		    goto IMPORTERR;
	    }
        
		xls_parseWorkSheet(pWS);

		if(pWS->rows.lastrow < 2)		
		{	
			TRACE("pWS->rows.lastrow error %d! %s %d\r\n", pWS->rows.lastrow, __FUNCTION__, __LINE__);

			goto IMPORTERR;
		}

		/* ����ռ� �ſ��Դ������� */
        pHoliday = (LP_ALL_HOLIDAY)Malloc(sizeof(ALL_HOLIDAY));
        if (pHoliday == NULL)
        {	
			TRACE("pHoliday Malloc fail! %s %d\r\n", __FUNCTION__, __LINE__);

			goto IMPORTERR;
		}
        
		memset(pHoliday, 0, sizeof(ALL_HOLIDAY));

        iHoliday = 0;

        TRACE("pWS->rows.lastrow %u %s %d\r\n", pWS->rows.lastrow, __FUNCTION__, __LINE__);
        
		/* ѭ�������û���Ϣ������ṹ������ */			
		for (t = 3; t <= pWS->rows.lastrow; t++)		
		{				
			row = &pWS->rows.row[t];
      		tt = 0;

			iNullNum = 0;
			
			for(i = 0; i < 4; i++)
			{
				if((((char *)(&row->cells.cell[i])->str != NULL) 
						&&	(0 == strcmp((char *)(&row->cells.cell[i])->str, " ")))
					|| ((&row->cells.cell[i])->str == NULL)
					|| ((&row->cells.cell[i])->str[0] == '\0'))
				{
					iNullNum += 1;
				}
			}

            /* ����������ݲ����������������� */
			if((0 < iNullNum) && (4 > iNullNum))
			{
                TRACE("iNullNum %d %s %d\r\n", iNullNum, __FUNCTION__, __LINE__);

                continue;
			    //goto IMPORTERR;
			}

			if(4 == iNullNum)
			{
				continue;
			}

			/* ������ ���� */
			if((&row->cells.cell[tt])->str != NULL 
				&& (&row->cells.cell[tt])->str[0] != '\0')
			{				
	            tt++;

				/* �����Ѷ�ȡ�ĸ������� */
				/* �ڼ������� */				
				memset(pHoliday->HoliDay[iHoliday].szHldyName, 0 , 32);
				
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					strncpy(pHoliday->HoliDay[iHoliday].szHldyName, (char *)(&row->cells.cell[tt])->str, 31);

					while(pHoliday->HoliDay[iHoliday].szHldyName[0] == ' ')
					{
						memmove(pHoliday->HoliDay[iHoliday].szHldyName, &(pHoliday->HoliDay[iHoliday].szHldyName[1]),31);
					}
				}
				
				tt++;
				/* ��ʼʱ�� */
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					strncpy(pHoliday->HoliDay[iHoliday].szStartTime, (char *)(&row->cells.cell[tt])->str, 31);
                    pHoliday->HoliDay[iHoliday].szStartTime[31] = 0;
				}
				
				tt++;
				/* ����ʱ�� */
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					strncpy(pHoliday->HoliDay[iHoliday].szEndTime, (char *)(&row->cells.cell[tt++])->str, 31);
                    pHoliday->HoliDay[iHoliday].szEndTime[31] = 0;
				}
				
	            Holidaynum++;

				if(Holidaynum == MAX_HOLIDAY_NUM)
				{
					break;
				}

				TRACE(" pHoliday->HoliDay[iHoliday].szHldyName  %s pHoliday->HoliDay[iHoliday].szStartTime %s "
					"pHoliday->HoliDay[iHoliday].szEndTime %s %s %d\r\n",
					pHoliday->HoliDay[iHoliday].szHldyName, pHoliday->HoliDay[iHoliday].szStartTime,
					pHoliday->HoliDay[iHoliday].szEndTime, __FUNCTION__, __LINE__);

                iHoliday++;
			}
		}

		xls_close_WS(pWS);
	}

	pHoliday->iHolidayNo = Holidaynum;	

	TRACE("pHoliday->iHolidayNo %d %s %d\r\n", pHoliday->iHolidayNo, __FUNCTION__, __LINE__);

	*pstrHoliday = pHoliday;

	xls_close_WB(pWB);

	return Holidaynum; 

IMPORTERR:
    if (pHoliday)
    {
        Free(pHoliday);
    }

    if (pWS != NULL)
    {
        xls_close_WS(pWS);
    }

    if (pWB != NULL)
    {
		xls_close_WB(pWB);    
    }

    return -1;
}

/**************************************************************\
** �������ƣ� ExportSchedules
** ���ܣ�     �����Ű��
** ������     pshedule:�������
** ���أ�     �Ű��ScheduleTable.xls
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int  ExportSchedules(SCHEDULE_INFO* pschedule)
{	
	workbook *w;
	worksheet *ws;
    wchar_t *pLabel = NULL;
	char tmp[124] = {0};
	cell_t *c = NULL;
	int Col = 0;
	int Row = 0;
	int iDutytime = 0;
	int iInterval = 0;
	int iScheduleNo = 0;
	int RowSchedule = 0;
	int tmpRow = 0;
	int iTmpSchedule = 0;
	int iType = 0;
	int iTmpCol = 0;
    char acFilePath[128];
    char caName[128];
    int  aiColWidth[12] = {(12 * 256), (7 * 256),
                           (7 * 256), (7 * 256),
                           (7 * 256), (7 * 256),
                           (7 * 256), (7 * 256),
                           (7 * 256), (7 * 256)};
    
	//printSchedules(pschedule);

	w = xlsNewWorkbook();	

	/* ��ʼ������ */
	initParticulXFTFont(w);

    pLabel = ZhcnToUincode((char *)GetLangText(12124, "�Ű���Ϣ��"));
    
	ws = xlsWorkbookSheetW(w, pLabel);

    ws->defaultColwidth(8); /* ����Ĭ�Ͽ�ȣ���������Ϊ8��xlsWorksheetColwidth�Ż������� */
    for (Col = 0; Col < 10; Col++)
    {
        xlsWorksheetColwidth(ws, Col, aiColWidth[Col], NULL);
    }       

	xlsWorksheetMerge(ws, 0, 0, 0, 9);
	xlsWorksheetRowheight(ws,0,(unsigned16_t)60*20, NULL);

    pLabel = ZhcnToUincode((char *)GetLangText(12124, "�Ű���Ϣ��"));
    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);
	
	for(iTmpCol = 0;iTmpCol <= 9; iTmpCol++)
	{
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}
	
	//c = xlsWorksheetFindCell(ws, 0, 9);
	//xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
	
	xlsWorksheetMerge(ws, 1, 0, 1, 9);

	if (GetLangID() == 1)
	{
		xlsWorksheetRowheight(ws, 1, (unsigned16_t)60*20, NULL);
	}
	else
	{
		xlsWorksheetRowheight(ws, 1, (unsigned16_t)30*20, NULL);
	}

    pLabel = ZhcnToUincode((char *)GetLangText(12157, "˵������ɫ����Ϊ�ɱ༭�������ͣ�0-��������; 1-�Ӱࣩʱ���ʽ:HH:MM ������7������, ��������:�ڿ���ʱ��֮ǰ�Ĵ���Ϊǰһ���"));
    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);

    /* ѭ��д��ÿ�������Ϣ*/			
	for (RowSchedule = 1; RowSchedule <= 55; RowSchedule += 11)		
	{	
		/* ���  */
		iScheduleNo++;

		/* ��ǰ�� */
		Row = RowSchedule + 1;
		     	
		/* д���ͷ */
		sprintf(tmp,"%s%d",(char *)GetLangText(12158, "��ţ�"), iScheduleNo);
		pLabel = ZhcnToUincode(tmp);
		xlsWorksheetLabelW(ws, Row, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(12159, "����"));
		xlsWorksheetLabelW(ws, Row, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
		
		//xlsWorksheetMerge(ws, Row, 2, Row, 6);	
		xlsCellMerge(ws, Row, 2, Row, 6, g_Xf_t[BORDER_RED_FILL_LEFT_10]);
        memset(tmp, 0, sizeof(tmp));

		if(pschedule[iTmpSchedule].caScheduleName[0] == '\0')
		{            
			switch(iScheduleNo)
			{
				case 1:
				{
					strcpy(tmp,(char *)GetLangText(11922, "���һ"));
					
					break;
				}
				case 2:
				{
					strcpy(tmp,(char *)GetLangText(11923, "��ζ�"));
					
					break;
				}
				case 3:
				{
					strcpy(tmp,(char *)GetLangText(11924, "�����"));
					
					break;	
				}
				case 4:
				{
					strcpy(tmp,(char *)GetLangText(11925, "�����"));
					
					break;
				}
				case 5:
				{
					strcpy(tmp,(char *)GetLangText(11926, "�����"));
					
					break;
				}
				default:
				{
					break;
				}
			}
		}
		else
		{
			strcpy(tmp, pschedule[iTmpSchedule].caScheduleName);
		}
		
		pLabel = ZhcnToUincode(tmp);
		xlsWorksheetLabelW(ws, Row, 2, pLabel, g_Xf_t[BORDER_RED_FILL_LEFT_10]);		

		xlsCellMerge(ws, Row, 7, Row, 8, g_Xf_t[BORDER_LEFT_FILL_10]);
		//xlsWorksheetMerge(ws, Row, 7, Row, 8);
		pLabel = ZhcnToUincode((char *)GetLangText(12160, "�������ã�"));
		xlsWorksheetLabelW(ws, Row, 7, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

        if(pschedule[iTmpSchedule].caDaySeparate != NULL 
			&& pschedule[iTmpSchedule].caDaySeparate[0] != '\0')
        {
			xlsWorksheetLabel(ws, Row, 9, pschedule[iTmpSchedule].caDaySeparate,  g_Xf_t[BORDER_RED_FILL_LEFT_Text_10]);			
        }
		else
		{
			xlsWorksheetLabel(ws, Row, 9, "00:00",  g_Xf_t[BORDER_RED_FILL_LEFT_Text_10]);
		}

		Row++;
		
		xlsWorksheetLabel(ws, Row, 0, "",  g_Xf_t[BORDER_LEFT_10]);

		xlsCellMerge(ws, Row, 1, Row, 3, g_Xf_t[BORDER_LEFT_10]);
		pLabel = ZhcnToUincode((char *)GetLangText(11928, "���һ"));
		xlsWorksheetLabelW(ws, Row, 1, pLabel, g_Xf_t[BORDER_LEFT_10]);

		xlsCellMerge(ws, Row, 4, Row, 6, g_Xf_t[BORDER_LEFT_10]);
		pLabel = ZhcnToUincode((char *)GetLangText(11929, "��ζ�"));
		xlsWorksheetLabelW(ws, Row, 4, pLabel, g_Xf_t[BORDER_LEFT_10]);

		xlsCellMerge(ws, Row, 7, Row, 9, g_Xf_t[BORDER_LEFT_10]);
		pLabel = ZhcnToUincode((char *)GetLangText(11930, "�����"));
		xlsWorksheetLabelW(ws, Row, 7, pLabel, g_Xf_t[BORDER_LEFT_10]);

		Row++;
		
		xlsWorksheetLabel(ws, Row, 0, "",  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(11403, "�ϰ�"));
		xlsWorksheetLabelW(ws, Row, 1, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(11404, "�°�"));
		xlsWorksheetLabelW(ws, Row, 2, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(10002, "����"));
		xlsWorksheetLabelW(ws, Row, 3, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(11403, "�ϰ�"));
		xlsWorksheetLabelW(ws, Row, 4, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(11404, "�°�"));
		xlsWorksheetLabelW(ws, Row, 5, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(10002, "����"));
		xlsWorksheetLabelW(ws, Row, 6, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(11403, "�ϰ�"));
		xlsWorksheetLabelW(ws, Row, 7, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(11404, "�°�"));
		xlsWorksheetLabelW(ws, Row, 8, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(10002, "����"));
		xlsWorksheetLabelW(ws, Row, 9, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		Row++;

		tmpRow = Row;
		pLabel = ZhcnToUincode((char *)GetLangText(10046, "��һ"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10047, "�ܶ�"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10048, "����"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10049, "����"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10050, "����"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10051, "����"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10045, "����"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
				
		iDutytime = 0;
		
        /* ����һ����ε��߸������տ���ʱ�� */
		for(tmpRow = Row; tmpRow < Row + 7; tmpRow++)
		{	
			Col = 1; 

			/* ѭ������ÿ����ε�������� */
			for(iInterval = 0; iInterval < 3; iInterval++)
			{
				/* ���� -1�Ƿ�*/
				iType = pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType;

				/* �ϰ� */
				if(iType != -1 && pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOnDuty[0] != '\0')
				{
					xlsWorksheetLabel(ws, tmpRow, Col, 
						pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOnDuty, g_Xf_t[BORDER_RED_LEFT_TEXT_10]);					
				}
				else
				{
					xlsWorksheetLabel(ws, tmpRow, Col, "",  g_Xf_t[BORDER_RED_LEFT_TEXT_10]);
				}
				
				Col++;

				/* �°� */
				if(iType != -1 && pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOffDuty[0] != '\0')
				{
					xlsWorksheetLabel(ws, tmpRow, Col, 
						pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOffDuty, g_Xf_t[BORDER_RED_LEFT_TEXT_10]);					
				}
				else
				{
					xlsWorksheetLabel(ws, tmpRow, Col, "",  g_Xf_t[BORDER_RED_LEFT_TEXT_10]);
				}
				
				Col++;
				
				/* ���� */	
				if((iType != -1)
					&& (pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOnDuty[0] != '\0')
					&& (pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOffDuty[0] != '\0'))
				{
					xlsWorksheetNumberInt(ws, tmpRow, Col, 
						pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType,g_Xf_t[BORDER_RED_LEFT_TEXT_10]);					
				}
				else
				{
					xlsWorksheetLabel(ws, tmpRow, Col, "",  g_Xf_t[BORDER_RED_LEFT_TEXT_10]);
				}
				
				Col++;
			}

		    /* ��һ�������� */
		    iDutytime++;
		}
		
		iTmpSchedule++;
		Row = Row + 7;
	}

	if (GetUsbStatus() == TRUE)
	{
        memset(acFilePath, 0, sizeof(acFilePath));
        memset(caName, 0, sizeof(caName));

    	sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12124, "�Ű���Ϣ��"));

        GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);
      
		xlsWorkbookDump(w, acFilePath);	
	}

	sync();

	xlsDeleteWorkbook(w);
	
	return 0;
}	

/**************************************************************\
** ��������:ImportSchedules
** ���ܣ�   �����Ű��
** ������   pShedule :�������
** ���أ�   -1:��ȡxls�ļ�ʧ��; >=0:�ڼ�����Ŀ
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ImportSchedules(SCHEDULE_INFO* pstrShedule)
{	
	xlsWorkBook* pWB = NULL;
	xlsWorkSheet* pWS = NULL;
	SCHEDULE_INFO* pShedule;
	struct stat st;
    char acFilePath[128];
    char caName[128];    
	xls::st_row::st_row_data *row;
	/* ��ѭ����� */
	WORD usRowIndex;
	/* cell��Ԫ���±꣬������� */
	WORD usColIndex;
	int iSchedule = 0;
	int iInterval = 0;
    int iDutytime = 0;
	int iTmpRow = 0;
	int iTmpSchedule = 0;
	int iOnFlag = 0;
	int iOffFlag = 0;
	int regDayFlag = 0;
	int regNoFlag = 0;
	char caScheduleName[32] = {0};

    if(NULL == pstrShedule)
	{
		goto IMPORTERR;
	}

	pShedule = pstrShedule;

    memset(acFilePath, 0, sizeof(acFilePath));
    memset(caName, 0, sizeof(caName));
    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12124, "�Ű���Ϣ��"));

    //GB2312ToUTF_8(acFilePath, caName, strlen(caName));
    GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);

    if(stat(acFilePath, &st) != 0)
    {
		TRACE("%s not exist! %s %d\r\n", acFilePath, __FUNCTION__, __LINE__);

		goto IMPORTERR;
    }
    
	//pWB = xls_open("/mnt/usb/ScheduleTable.xls", "UTF-8");
	pWB = xls_open(acFilePath, "UTF-8");
	if (pWB == NULL)
	{
		TRACE("Open xls failed! %s %d\r\n", __FUNCTION__, __LINE__);
		
		goto IMPORTERR;
	}
	else
	{		
		/* ���ȡ�ı��ĵ�Ԫ����䲻��������ʹ��������������ʱ����ִ��� */
		pWS = xls_getWorkSheet(pWB, 0);	
        if (pWS == NULL)
        {
    		TRACE("xls_getWorkSheet xls failed! %s %d\r\n", __FUNCTION__, __LINE__);
    		
    		goto IMPORTERR;
	    }
		
		xls_parseWorkSheet(pWS);

		/* ѭ�����������Ϣ������ṹ������ */			
		for (usRowIndex = 7; usRowIndex <= pWS->rows.lastrow; usRowIndex += 11)		
		{	
			iDutytime = 0;

			iSchedule++;

			/* ����ʱ�������� */
			iTmpRow = usRowIndex - 3;
			
			row = &pWS->rows.row[iTmpRow];
			
			if ((&row->cells.cell[2])->str != NULL 
				&& (&row->cells.cell[2])->str[0] != '\0')
			{
				strncpy(pShedule[iTmpSchedule].caScheduleName, (char *)(&row->cells.cell[2])->str, 31);
                pShedule[iTmpSchedule].caScheduleName[31] = 0;
			}
			else
			{   memset(caScheduleName, 0, sizeof(caScheduleName));
                memset(pShedule[iTmpSchedule].caScheduleName, 0, sizeof(pShedule[iTmpSchedule].caScheduleName));

				sprintf(caScheduleName, "%s", GetLangText(iTmpSchedule+11922,(char *)"���һ"));

                GBKBufToUTF8Buf(caScheduleName, sizeof (caScheduleName), pShedule[iTmpSchedule].caScheduleName);
			}

			TRACE("caScheduleName %s %s %d\r\n", pShedule[iTmpSchedule].caScheduleName, __FUNCTION__, __LINE__);

			if ((&row->cells.cell[9])->str != NULL 
				&& (&row->cells.cell[9])->str[0] != '\0')
			{
				strncpy(pShedule[iTmpSchedule].caDaySeparate, (char *)(&row->cells.cell[9])->str, 7);	
                pShedule[iTmpSchedule].caDaySeparate[7] = 0;
			}
			else
			{
				strcpy(pShedule[iTmpSchedule].caDaySeparate, (char *)"00:00");	
			}

			TRACE("pShedule[iTmpSchedule].caDaySeparate %s %s %d\r\n", pShedule[iTmpSchedule].caDaySeparate, __FUNCTION__, __LINE__);

			regNoFlag = 0;

			/* ÿ����ε��߸������� */	
			for (iTmpRow = usRowIndex; iTmpRow < (usRowIndex + 7); iTmpRow++)
			{				
				/* ��ǰ�� */
				row = &pWS->rows.row[iTmpRow];

				/* ��Ϊ��1��Ϊ���ڼ��У�����Ҫ�� */
				usColIndex = 1;

				regDayFlag = 0;

				/* ѭ������ÿ����ε�������� */
				for (iInterval = 0; iInterval < 3; iInterval++)
				{
					/* �ϰ��־  */
					iOnFlag = 0;
					/* �°��־  */
					iOffFlag = 0;								

					/* �ϰ���� */
					if ((&row->cells.cell[usColIndex])->str != NULL 
						&& (&row->cells.cell[usColIndex])->str[0] != '\0')
					{
						strncpy(pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOnDuty,
							(char *)(&row->cells.cell[usColIndex])->str, 15);
                        pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOnDuty[15] = 0;
					}
					else
					{
						pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType = -1;

						iOnFlag = 1;
					}

					usColIndex++;

					/* �°� */
					if ((&row->cells.cell[usColIndex])->str != NULL 
						&& (&row->cells.cell[usColIndex])->str[0] != '\0')
					{
						strncpy(pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOffDuty,
						(char *)(&row->cells.cell[usColIndex])->str, 15);
                        pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].caOffDuty[15] = 0;
					}
					else
					{
						pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType = -1;
						iOffFlag = 1;
					}

					usColIndex++;

					/* ֻ��д�ϰ�����°� ˵�������ݷǷ� ���� */
					if ((iOnFlag == 1 || iOffFlag == 1) 
						&& (iOnFlag == 0 || iOffFlag == 0))
					{
                		TRACE("iOnFlag %d iOffFlag %d %s %d\r\n", iOnFlag, iOffFlag, __FUNCTION__, __LINE__);

                        goto IMPORTERR;
					}

					/* ���°඼������ type = -1 */	
					if(iOnFlag == 1 
						&& iOffFlag == 1)
					{
						pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType = -1;

						usColIndex++;

						regDayFlag += 1;

						continue;
					}

					/* ���°඼����ʱ ��ȡ���� ���Ƿ�Ϸ� */
					if ((iOnFlag == 0 && iOffFlag == 0) 
						&& (&row->cells.cell[usColIndex])->str != NULL 
						&& (&row->cells.cell[usColIndex])->str[0] != '\0')
					{
						if(atoi((char *)(&row->cells.cell[usColIndex])->str) == 0 
							|| atoi((char *)(&row->cells.cell[usColIndex])->str) == 1)
						{
							pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType 
								= atoi((char *)(&row->cells.cell[usColIndex])->str);							
						}
						else
						{
                    		TRACE("iType %d %s %d\r\n", atoi((char *)(&row->cells.cell[usColIndex])->str), __FUNCTION__, __LINE__);

							/* ���ݷǷ� */
                            goto IMPORTERR;
						}

						usColIndex++;
					}					

					/* ��� */
					pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iInterval = iInterval + 1;
				}				

				/* ���regDayFlagΪ3�����������εı��ܼ��Ŀ��ڲ����� */
				if(3 == regDayFlag)
				{
					pShedule[iTmpSchedule].ScheduleTime[iDutytime].iDay = 0;
					regNoFlag += 1;
				}
				else
				{
					pShedule[iTmpSchedule].ScheduleTime[iDutytime].iDay = iDutytime + 1;
				}

				/* ��һ�������� */
				iDutytime++;								
			}

			/* ���regNoFlagΪ7��������ñ���ο��ڲ����� */
			if(7 == regNoFlag)
			{
				pShedule[iTmpSchedule].iNo = 0;
			}
			else
			{
				pShedule[iTmpSchedule].iNo = iSchedule; 
			}

			iTmpSchedule++;
		}		

		xls_close_WS(pWS);
    }

	xls_close_WB(pWB);
	
	return 0;

IMPORTERR:

    if (pWS != NULL)
    {
        xls_close_WS(pWS);
    }

    if (pWB != NULL)
    {
        xls_close_WB(pWB);  
    }

	return -1;    
}   

/**************************************************************\
** ��������:ExportsOriginalRecord
** ���ܣ�   ÿ��д��һ���û�ԭʼ��¼������ԭʼ��¼��
** ������   pOriginalRecord:ԭʼ��¼ָ��
            StartTime:��ʼʱ��
            EndTime:����ʱ��
            iUser:Ҫд��ڼ����û�
** ���أ�   ԭʼ��¼��OriginalTable.xls
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ExportOriginalRecord(TIMERANGE timeRange, ORIGINAL_RECORD OriginalRecord,char* StartTime,char* EndTime,int iUser,int iflag)
{
	static workbook *w;
    static worksheet *ws;
    wchar_t *pLabel = NULL;
	int Col = 0;
	int Row = 0;
	int iRowFlag = 0;
	int iColFlag = 0;
	char buf[1024] = {0};
	char tmpbuf[1024] = {0};
	char* pDay = NULL;
	int iTmpUser = 0;
    int iTmpCol = 0;
    cell_t *c = NULL;
	ATTENDANCE_PARAM attendanceParm;
 
	memset(&attendanceParm, 0, sizeof(ATTENDANCE_PARAM));

	GetConfigAttendance_Ex(&attendanceParm);

	CONFIG_BASE_STR strConfigBase;
	memset(&strConfigBase, 0, sizeof(CONFIG_BASE_STR));
	GetConfigBase(&strConfigBase);

	struct tm strTm;
	struct tm* pTm = &strTm;
	time_t tCurTime = time(NULL);
	tCurTime = time(NULL);
	localtime_r(&tCurTime, pTm);	

	iRowFlag = OriginalRecord.iDayNum > 16? 6:4;
	iRowFlag = (iUser - 1)*iRowFlag;

	iColFlag = OriginalRecord.iDayNum > 16? XLS_ORI_RECORD_TAB_COLS:OriginalRecord.iDayNum+1;

	if(iflag == 0 || iflag == 3)
	{
		w = xlsNewWorkbook();

		/* ��ʼ������ */
		initParticulXFTFont(w);

        pLabel = ZhcnToUincode((char *)GetLangText(11941, "���ڿ�ʽ����"));
      
		ws = xlsWorkbookSheetW(w, pLabel);

        ws->defaultColwidth(8); /* ����Ĭ�Ͽ�ȣ���������Ϊ8��xlsWorksheetColwidth�Ż������� */
        for (Col = 0; Col < iColFlag; Col++)
        {
			if(Col == 0)
			{
				xlsWorksheetColwidth(ws, Col, (16*32), NULL);
			}
			else
			{
				/* 32һ������,�п�37���� */
            	xlsWorksheetColwidth(ws, Col, 37*32, NULL);
			}
        }

		sprintf(buf,"%s", (char *)GetLangText(11941, "��ʽ����"));

		/* ��ѯ�������ϲ� д���ͷ */
		xlsWorksheetMerge(ws, 0, 0, 0, (iColFlag - 1));
		xlsWorksheetRowheight(ws, 0, (unsigned16_t)60*20, NULL);
	    pLabel = ZhcnToUincode(buf);
	    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

        for(iTmpCol = 0; iTmpCol < iColFlag; iTmpCol++)
    	{
    		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
    		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
    	}

		xlsWorksheetMerge(ws, 1, 0, 1, (iColFlag - 1));
		sprintf(buf,"%s:%04d-%02d-%02d %s %04d-%02d-%02d", (char *)GetLangText(10013, "����"), timeRange.iStartYear, timeRange.iStartMonth,
			timeRange.iStartDay, (char *)GetLangText(12140, "��"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
		pLabel = ZhcnToUincode(buf);
	    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);
	}
	
    Row = 2+iRowFlag;
    Col = 0;

	/* ��д��ͷ��Ϣ */
	pLabel = ZhcnToUincode((char *)GetLangText(12182, "Ա��"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

	sprintf(buf, "%s:%d  %s:%s ", (char *)GetLangText(11295, "����"), (int)OriginalRecord.UserNo,
										(char *)GetLangText(10006, "����"), OriginalRecord.szUserName);
	Col++;
	xlsCellMerge(ws,Row,Col, Row, iColFlag-1, g_Xf_t[BORDER_LEFT_FILL_10]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(ws, Row, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

    Row++;
    Col = 0;
	pLabel = ZhcnToUincode((char *)GetLangText(10013, "����"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

    Row++;
    Col = 0;
	pLabel = ZhcnToUincode((char *)GetLangText(12183, "����"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

	if(OriginalRecord.iDayNum > 16)
	{
        Row++;
		pLabel = ZhcnToUincode((char *)GetLangText(10013, "����"));
		xlsWorksheetLabelW(ws,  Row, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

        Row++;
		pLabel = ZhcnToUincode((char *)GetLangText(12183, "����"));
		xlsWorksheetLabelW(ws,  Row, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	}

	for(iTmpUser = 0; iTmpUser < OriginalRecord.iDayNum; iTmpUser++)
	{
		/* ��ʼд����к� */
		Row = 3 + iRowFlag + 2*(iTmpUser/16);

        Col = iTmpUser%16 + 1;

		/* �������� */
		pDay = strchr(OriginalRecord.DayRecord[iTmpUser].day, '-');
		if(NULL == pDay)
		{
			return -1;
		}
		else
		{
			pDay += 1;
		}

	 	xlsWorksheetLabel(ws, Row, Col ,pDay , g_Xf_t[BORDER_9]); 


		/* �������û�����Ĵ򿪼�¼ */			
		/* �д򿨼�¼ */
  		if(OriginalRecord.DayRecord[iTmpUser].caDayRecordTime[0] != '\0')
  		{
			xlsWorksheetLabel(ws, Row+1, Col, OriginalRecord.DayRecord[iTmpUser].caDayRecordTime, g_Xf_t[BORDER_9]);
  		}
		else
		{
			xlsWorksheetLabel(ws, Row+1, Col ,"" , g_Xf_t[BORDER_9]); 
		}
	}

	if(OriginalRecord.iDayNum >16)
	{
		Col = OriginalRecord.iDayNum%16+1;

		Row = 3 + iRowFlag + 2*(OriginalRecord.iDayNum/16);
		
		for(Col = OriginalRecord.iDayNum%16+1; Col < iColFlag; Col++)
		{
			xlsWorksheetLabel(ws, Row, Col ,"" , g_Xf_t[BORDER_9]); 
			xlsWorksheetLabel(ws, Row+1, Col ,"" , g_Xf_t[BORDER_9]); 
		}
	}

	if(iflag == 2 || iflag == 3)
	{
        TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

        memset(buf, 0, sizeof(buf));
        memset(tmpbuf, 0, sizeof(tmpbuf));

        sprintf(tmpbuf,"/mnt/usb/%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11941, "��ʽ����"));
        TRACE("tmpbuf %s %s %d\r\n", tmpbuf, __FUNCTION__, __LINE__);

        GBKBufToUTF8Buf(tmpbuf, strlen(tmpbuf), buf);

        TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

		if (GetUsbStatus() == TRUE)
		{
            TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
			xlsWorkbookDump(w, buf);
		}
		
		sync();

        TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
		
		xlsDeleteWorkbook(w);
	}

	//testLinkSheet();

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

	return 0;
}

/**************************************************************\
** ��������:ExportMonthParticularRecord
** ���ܣ�   ������ϸ�����±���ϲ�
** ������   pParticularRecord:LP_PARTICULAR_RECORD��ָ��
            StartTime:��ʼʱ��
            EndTime:����ʱ��
            daynum:����
            iUser:�ڼ����û�
            iflag:д���־ 
            0,��һ���û����������һ���û�
            1,���ǵ�һ���û�Ҳ�������һ���û�
            2,���ǵ�һ���û��������һ���û�
            3,���ǵ�һ���û�Ҳ�����һ���û�
            
** ���أ�   
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ExportMonthParticularRecord(TIMERANGE timeRange, USER_ATTENDANCE_INFO_STRU * pMonthParticularRecord, int iUser, int iflag, int userNum)
{
	int ret = 0;
	wchar_t *pLabel = NULL;
	wchar_t LabelVal[1024] = {0};
	cell_t *c = NULL;
	int Col = 0;
	int Row = 0;
	char buf[1024] = {0};
	char tmpbuf[1024] = {0};
	LP_PARTICULAR_RECORD pParticularRecord = &(pMonthParticularRecord->strUserParticularRecord);
	LP_MONTHFORM pMonthRecord = &(pMonthParticularRecord->strUserMonthForm);

	static workbook *w;
	static worksheet *ws;
	
	worksheet *wsRecord;

	int iSchedule = 0;
	int iStatus = 0;
    int iLoop = 0;
	int iRowNum = 0;
	int iDaynum = 0;

	CONFIG_BASE_STR strConfigBase;
	memset(&strConfigBase, 0, sizeof(CONFIG_BASE_STR));
	GetConfigBase(&strConfigBase);

	ATTENDANCE_PARAM  AttendanceParam;
	memset(&AttendanceParam, 0, sizeof(AttendanceParam));
	GetConfigAttendance_Ex(&AttendanceParam);

	if(iflag == 0 || iflag == 3)
	{
		w = xlsNewWorkbook();
		/* ��ʼ����ϸ��Ԫ���ʽ */
		initParticulXFTFont(w);

        pLabel = ZhcnToUincode((char *)GetLangText(12143, "��ͳ�Ʊ�"));

		/* ��Ϊ��ͳ�Ʊ�Ԥ�� */
		ws = xlsWorkbookSheetW(w, pLabel);
        /* д���±����ͷ */
		MonthRecordHadInfo(ws,timeRange);
	}

	sprintf(buf, "%d%s",iUser, pParticularRecord->szUserName);
	pLabel = ZhcnToUincode(buf);

	/* һ���û�һ��sheet */
	wsRecord = xlsWorkbookSheetW(w, pLabel);
	/* ����Ĭ�Ͽ�ȣ���������Ϊ8��xlsWorksheetColwidth�Ż������� */
    wsRecord->defaultColwidth(8); 
    for (Col = 0; Col < XLS_ORI_RECORD_TAB_COLS; Col++)
    {
		if((Col == 1)||(Col == 9))
		{
			xlsWorksheetColwidth(wsRecord, Col, 33*32, NULL);
		}
		else
		{
			/* 32һ������,�п�39���� */
        	xlsWorksheetColwidth(wsRecord, Col, 39*32, NULL);
		}
    }

    /* д���û���ϸ���ͷ */
	ParticulUserHaedInfo(wsRecord, pParticularRecord->UserNo, pParticularRecord->szUserName, pParticularRecord->iSchedule, 
						timeRange );

	/* totalRow ����д������� */
	iDaynum = pParticularRecord->nDayNum;
    if ((iDaynum % 2) == 0)
    {
        iRowNum = iDaynum >> 1;
    }
    else
    {
        iRowNum = (iDaynum >> 1) + 1;
    }

	Row = 4;
	/* ��������д��ÿһ�� */
    for (iLoop = 0; iLoop < iRowNum; iLoop++)
    {
        /* д�����Ϣ */
        /* ���ø�row������col */
		Col = 0;
		Row ++;
        sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop].szData);    /* ���� */
		xlsWorksheetLabel(wsRecord, Row, Col, buf, g_Xf_t[BORDER_9]);

		Col++;
        sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop].weekDay);   /* ���� */
		pLabel = ZhcnToUincode(buf);
		xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

		Col++;
        if ((pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == REST_DAY)
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[0].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[0].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[1].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[1].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[2].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[2].offduty[0] == '\0'))
        {
			sprintf(buf,"%s", (char *)GetLangText(12017, "��Ϣ"));   /* ���� */
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);

			Col+=6;
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == BUSINESS_OF_TRIP)
        {
			sprintf(buf,"%s", (char *)GetLangText(12018, "����"));
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);

			Col+=6;
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == LEAVE_DAY)
        {
			sprintf(buf,"%s", (char *)GetLangText(12016, "���")); 
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);

			Col+=6;
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == ABSENCE_FROM_DUTY)   /* ȱ�� */
        {
			xlsWorksheetLabel(wsRecord, Row, Col, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+1, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+2, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+3, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+4, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+5, "", g_Xf_t[BORDER_10]);
			
			Col+=6;
        }
        else
        {
            for (iSchedule = 0; iSchedule < XLS_MAX_SCHEDULE_NUM; iSchedule++)
            {
                /* �ϰ� */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].onduty,
						g_Xf_t[BORDER_RED_9]);
                }
                else
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].onduty,
						g_Xf_t[BORDER_9]);
                }
				
				Col++;
                /* �°� */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].offduty,
						g_Xf_t[BORDER_RED_9]);
                }
                else
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].offduty,
						g_Xf_t[BORDER_9]);
                }

				Col++;
            }
        }

        if ((iLoop + iRowNum) >= iDaynum)  /* �Ҳ���Ϣ�Ѿ�д�� */
        {
			xlsWorksheetLabel(wsRecord, Row, Col, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+1, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+2, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+3, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+4, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+5, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+6, "", g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+7, "", g_Xf_t[BORDER_10]);

            break;
        }

        /* д�Ҳ���Ϣ */
		sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop + iRowNum].szData);    /* ���� */
		xlsWorksheetLabel(wsRecord, Row, Col, buf, g_Xf_t[BORDER_9]);

		Col++;
        sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop + iRowNum].weekDay);   /* ���� */
		pLabel = ZhcnToUincode(buf);
		xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

		Col++;
        if ((pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == REST_DAY)
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[0].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[0].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[1].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[1].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[2].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[2].offduty[0] == '\0'))
        {
			sprintf(buf,"%s", (char *)GetLangText(12017, "��Ϣ"));   /* ���� */
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]); 
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == BUSINESS_OF_TRIP)
        {
			sprintf(buf,"%s", (char *)GetLangText(12018, "����"));
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]); 
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == LEAVE_DAY)
        {
			sprintf(buf,"%s", (char *)GetLangText(12016, "���")); 
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == ABSENCE_FROM_DUTY)   /* ȱ�� */
        {
			xlsWorksheetLabel(wsRecord, Row, Col, "",g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+1, "",g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+2, "",g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+3, "",g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+4, "",g_Xf_t[BORDER_10]);
			xlsWorksheetLabel(wsRecord, Row, Col+5, "",g_Xf_t[BORDER_10]);
        }
        else
        {
            for (iSchedule = 0; iSchedule < XLS_MAX_SCHEDULE_NUM; iSchedule++)
            {
                /* �ϰ� */
				
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].onduty, 
						g_Xf_t[BORDER_RED_9]);
                }
                else
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].onduty, 
						g_Xf_t[BORDER_9]);
                }

				Col++;
                /* �°� */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].offduty, 
						g_Xf_t[BORDER_RED_9]);
                }
                else
                {
					xlsWorksheetLabel(wsRecord, Row, Col, 
						pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].offduty, 
						g_Xf_t[BORDER_9]);
                }

				Col++;
            }
        }
    }

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

	Row++;
	Col = 0;

	memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s:  %.1lf  %s:  %.1lf   %s:  %.1lf   %s/%s:"
		"  %d/%d   %s/%s:%d/%d     %s:  %.1lf   %s:  %.1lf   %s:  %.1lf", 
        (char *)GetLangText(12145, "��������"), pParticularRecord->fWorkdayNo, 
        (char *)GetLangText(12146, "��������"), pParticularRecord->fAttendanceNo,
        (char *)GetLangText(12147, "ȱ������"), pParticularRecord->fAbsenceNo, 
        (char *)GetLangText(12149, "�ٵ�����"), (char *)GetLangText(11444, "����"), 
        pParticularRecord->iDelayTimes, pParticularRecord->iDelayMinute, 
        (char *)GetLangText(12151, "���˴���"), (char *)GetLangText(11444, "����"), 
        pParticularRecord->iLeaveEarlyTimes, pParticularRecord->iLeaveEarlyMinute, 
        (char *)GetLangText(12165, "�Ӱ�ʱ��"), pParticularRecord->fOvertimeHours, 
        (char *)GetLangText(12153, "�������"), pParticularRecord->fLeaveDays, 
        (char *)GetLangText(12154, "��������"), pParticularRecord->fTrips);

	xlsCellMerge(wsRecord, Row, Col, Row, Col+15, g_Xf_t[BORDER_LEFT_10]);
	xlsWorksheetRowheight(wsRecord, Row, (unsigned16_t)25*20, NULL);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

	Row++;
	Col = 0;
	memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s��    %s:     %s��    %s��     %s��    %s��     %s��    ",
		(char *)GetLangText(12166, "����ʱ��"),
		(char *)GetLangText(12167, "�¼�ʱ��"),
		(char *)GetLangText(12168, "��н����"),
		(char *)GetLangText(12169, "�Ӱ๤��"),
		(char *)GetLangText(12170, "��������"),
		(char *)GetLangText(12171, "�����ۿ�"),
		(char *)GetLangText(12172, "ʵ�ù���"));

	xlsCellMerge(wsRecord, Row, Col, Row, Col+15, g_Xf_t[BORDER_LEFT_10]);
	xlsWorksheetRowheight(wsRecord, Row, (unsigned16_t)25*20, NULL);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);

	Row++;
	Col = 0;
	memset(buf, 0, sizeof(buf));
 	sprintf(buf, "%s:                           %s:",
		 (char *)GetLangText(12173, "ȷ��"), (char *)GetLangText(12174, "���"));

	xlsCellMerge(wsRecord, Row, Col, Row, Col+15, g_Xf_t[BORDER_LEFT_10]);
	xlsWorksheetRowheight(wsRecord, Row, (unsigned16_t)25*20, NULL);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
    
	/* д�±����¼ */
	Row = 3+iUser-1;
	Col = 0;
	/* ���� */
	xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->nUserNo,  g_Xf_t[BORDER_LEFT_10]);

	Col++;
	
	memset(tmpbuf, 0, sizeof(tmpbuf));
	memset(buf, 0, sizeof(tmpbuf));

    //xlsWorksheetLabel(ws, Row, Col, "",g_Xf_t[BORDER_10]);

	sprintf(tmpbuf,"%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11942, "������ϸ��"));
	sprintf(buf, "[%s]%d%s!A1", tmpbuf, iUser,pMonthRecord->szUserName);

	pLabel = ZhcnToUincode(buf);
	memcpy(LabelVal ,pLabel, 1024);

	pLabel = ZhcnToUincode(pMonthRecord->szUserName);    

	expression_node_factory_t& maker = w->GetFormulaFactory();

	expression_node_t *binary_root = maker.f(FUNC_HYPERLINK, maker.text(LabelVal), maker.text(pLabel));
    
	c = ws->formula(Row, Col, binary_root, true);
	xlsCellSetXF(c, g_Xf_t[BORDER_LEFT_10]);

    /* �ݲ�֧�ֲ��ţ�����д���� */
    Col++;
    xlsWorksheetLabel(ws, Row, Col, "",g_Xf_t[BORDER_LEFT_10]);
	
	Col++;
	if((pMonthRecord->iSchedule > 0)&&(pMonthRecord->iSchedule < 6))
	{
		strcpy(buf,AttendanceParam.Schedules[pMonthRecord->iSchedule - 1].caScheduleName);
	}
	else
	{
		strcpy(buf,(char *)GetLangText(12155, "��"));
	}
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);
	
	Col++;

	if(pMonthRecord->fWorkdayNo)
	{
		xlsWorksheetNumberDbl(ws, Row, Col, pMonthRecord->fWorkdayNo, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

	Col++;

	if(pMonthRecord->fAttendanceNo)
	{
		sprintf(buf, "%.1lf",pMonthRecord->fAttendanceNo);
		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

	Col++;

	if(pMonthRecord->fAbsenceNo)
	{
		sprintf(buf, "%.1lf",pMonthRecord->fAbsenceNo);
		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}
	
	Col++;

	if(pMonthRecord->iDelayminute)
	{
		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iDelayminute, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

	Col++;
	if(pMonthRecord->iDelaytimes)
	{
		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iDelaytimes, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}
	
	Col++;

	if(pMonthRecord->iLeaveEarlyMinute)
	{
		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iLeaveEarlyMinute, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}
	
	Col++;

	if(pMonthRecord->iLeaveEarlyTimes)
	{
		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iLeaveEarlyTimes, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

	Col++;

	if(pMonthRecord->fOvertimeHours)
	{
		sprintf(buf, "%.1f", pMonthRecord->fOvertimeHours);
		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

	Col++;

	if(pMonthRecord->fLeaveDays)    /* ������� */
	{
		sprintf(buf, "%.1f", pMonthRecord->fLeaveDays);
		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

	Col++;

	if(pMonthRecord->fTrips)    /* �������� */
	{
		sprintf(buf, "%.1f", pMonthRecord->fTrips);
		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

	if(iflag == 2 || iflag == 3)
	{
        TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

        memset(buf, 0, sizeof(buf));
        memset(tmpbuf, 0, sizeof(tmpbuf));

		sprintf(tmpbuf,"/mnt/usb/%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11942, "������ϸ��"));

        //GB2312ToUTF_8(buf, tmpbuf, strlen(tmpbuf));
        GBKBufToUTF8Buf(tmpbuf, strlen(tmpbuf), buf);

		if (GetUsbStatus() == TRUE)
		{
			xlsWorkbookDump(w, buf);
		}
		
		sync();
		
		xlsDeleteWorkbook(w);
	}
	
	return ret;
}

/**************************************************************\
** ��������:ExportParticularRecord
** ���ܣ�   ������ϸ��
** ������   pParticularRecord:LP_PARTICULAR_RECORD��ָ��
            StartTime:��ʼʱ��
            EndTime:����ʱ��
            daynum:����
            iUser:�ڼ����û�
            iflag:д���־ 
            0,��һ���û����������һ���û�
            1,���ǵ�һ���û�Ҳ�������һ���û�
            2,���ǵ�һ���û��������һ���û�
            3,���ǵ�һ���û�Ҳ�����һ���û�
            
** ���أ�   
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ExportParticularRecord(TIMERANGE timeRange, LP_PARTICULAR_RECORD pParticularRecord, int iUser, int iflag, int userNum)
{
	char acName[128] = {0};
    char acTmpName[128];
    char acPara[16];
    int iDaynum;
    int iRowNum;
    int iLoop;
    int iSchedule;
    int iStatus;

	CONFIG_BASE_STR strConfigBase;
	memset(&strConfigBase, 0, sizeof(CONFIG_BASE_STR));
	GetConfigBase(&strConfigBase);

	struct tm strTm;
	struct tm* pTm = &strTm;
	time_t tCurTime = time(NULL);
	tCurTime = time(NULL);
	localtime_r(&tCurTime, pTm);

    TRACE("iUser %d iflag %d userNum %d %s %d\r\n", iUser, iflag, userNum, __FUNCTION__, __LINE__);
    
	if(iflag == 0 || iflag == 3)
	{
    	memset(acTmpName, 0, sizeof(acTmpName));
        memset(acName, 0, sizeof(acName));

    	//sprintf(acTmpName, "/mnt/usb/%s-%s-%s.xls", (char*)GetLangText(11942, "������ϸ��"),
    	//	pParticularRecord->szStartTime, pParticularRecord->szEndTime);

		//sprintf(acTmpName,"/mnt/usb/%03d_%d_%d_%s.xls",strConfigBase.machineID,
	    //pTm->tm_year + 1900, pTm->tm_mon + 1, (char*)GetLangText(11942, "������ϸ��"));
		sprintf(acTmpName,"/mnt/usb/%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11942, "������ϸ��"));
        //GB2312ToUTF_8(acName, acTmpName, strlen(acTmpName));
	    GBKBufToUTF8Buf(acTmpName, strlen(acTmpName), acName);

        unlink(acName);

        /* �����ĵ� */
        create_file(acName);
	}

    /* �ж��Ƿ���Ҫ����sheet */
    if ((iUser - 1) % 1 == 0)
    {
        /* �����ǵ�һ��sheet����ر���һ��sheet */
        if (iUser != 1)
        {
            end_sheet();
        }

        memset(acPara, 0, sizeof(acPara));
        sprintf(acPara, "%s", pParticularRecord->szUserName);
        
        /* ����sheet */
        start_new_sheet(acPara);

		fill_title_info(pParticularRecord->szStartTime, pParticularRecord->szEndTime);

		fill_company_attendaceinfo(timeRange);
    }
	else
	{
		start_new_row();
	}

     /* totalRow ����д������� */
	iDaynum = pParticularRecord->nDayNum;
    if ((iDaynum % 2) == 0)
    {
        iRowNum = iDaynum >> 1;
    }
    else
    {
        iRowNum = (iDaynum >> 1) + 1;
    }

	//start_new_row();	
    /* ���ӹ�����Ϣ */
    fill_user_headinfo((int)pParticularRecord->UserNo, pParticularRecord->szUserName, 
                        pParticularRecord->szDepName, pParticularRecord->iSchedule, 
                        pParticularRecord->szStartTime, pParticularRecord->szEndTime);

    /* ����ͨ�ñ�ͷ��Ϣ */
    fill_user_table_headinfo(); 

    /* ��������д��ÿһ�� */
    for (iLoop = 0; iLoop < iRowNum; iLoop++)
    {
        /* ����row�����ø������� */
        add_normal_attribute_of_row();

        /* д�����Ϣ */
        /* ���ø�row������col */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].szData);    /* ���� */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].weekDay);   /* ���� */

        if ((pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == REST_DAY)
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[0].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[0].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[1].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[1].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[2].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[2].offduty[0] == '\0'))
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12017, "��Ϣ"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == BUSINESS_OF_TRIP)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12018, "����"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == LEAVE_DAY)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12016, "���"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == ABSENCE_FROM_DUTY)   /* ȱ�� */
        {
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
        }
        else
        {
            for (iSchedule = 0; iSchedule < XLS_MAX_SCHEDULE_NUM; iSchedule++)
            {
                /* �ϰ� */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_RED_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }
                else
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }

                /* �°� */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_RED_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].offduty); 
                }
                else
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].offduty); 
                }
            }
        }

        if ((iLoop + iRowNum) >= iDaynum)  /* �Ҳ���Ϣ�Ѿ�д�� */
        {
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
			
            end_row();

            break;
        }

        /* д�Ҳ���Ϣ */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].szData);    /* ���� */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].weekDay);   /* ���� */

        if ((pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == REST_DAY)
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[0].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[0].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[1].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[1].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[2].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[2].offduty[0] == '\0'))
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12017, "��Ϣ"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == BUSINESS_OF_TRIP)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12018, "����"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == LEAVE_DAY)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12016, "���"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == ABSENCE_FROM_DUTY)   /* ȱ�� */
        {
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
            fill_normal_cell(EMPTY_FILL_CELL, NULL, NULL, NULL);
        }
        else
        {
            for (iSchedule = 0; iSchedule < XLS_MAX_SCHEDULE_NUM; iSchedule++)
            {
                /* �ϰ� */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_RED_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }
                else
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }

                /* �°� */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* ��ѯ�ð�ο���״̬�����ٵ������� */
                if (iStatus)
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_RED_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].offduty); 
                }
                else
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].offduty); 
                }
            }
        }

        end_row();
    }

    fill_user_attendaceinfo(pParticularRecord->fWorkdayNo,
                            pParticularRecord->fAttendanceNo,
                            pParticularRecord->fAbsenceNo,
                            pParticularRecord->iDelayTimes,
                            pParticularRecord->iDelayMinute,
                            pParticularRecord->iLeaveEarlyTimes,
                            pParticularRecord->iLeaveEarlyMinute,
                            pParticularRecord->fOvertimeHours,
                            pParticularRecord->fLeaveDays,
                            pParticularRecord->fTrips);
    fill_user_wageinfo();
    fill_user_tailinfo();

    if (iflag == 2 || iflag == 3)
    {
        end_sheet();
        end_file();
    }

	return 0;
}

/**************************************************************\
** ��������:ExportMonthRecordTable
** ���ܣ�   �����±���
** ������   pMonthRecord:MONTHFORMָ��
            usernum:�û���
** ���أ�   
** �������ߣ�
** �������ڣ�
** �޸����ߣ�
** �޸����ڣ�
\**************************************************************/
int ExportMonthRecordTable(TIMERANGE timeRange, MONTHFORM* pMonthRecord,int usernum,char* StartTime, char* EndTime)
{
	workbook *w;
	worksheet *ws;
    wchar_t *pLabel =NULL;
	int Col = 0;
	int Row = 0;
	char buf[1024] = {0};
    char acName[1024];

	ATTENDANCE_PARAM attendanceParm;

	memset(&attendanceParm, 0, sizeof(ATTENDANCE_PARAM));

	GetConfigAttendance_Ex(&attendanceParm);

	CONFIG_BASE_STR strConfigBase;
	memset(&strConfigBase, 0, sizeof(CONFIG_BASE_STR));
	GetConfigBase(&strConfigBase);

	struct tm strTm;
	struct tm* pTm = &strTm;
	time_t tCurTime = time(NULL);
	tCurTime = time(NULL);
	localtime_r(&tCurTime, pTm);	 

	w = xlsNewWorkbook();

    /* ��ʼ������ */
	initParticulXFTFont(w);

    pLabel = ZhcnToUincode((char *)GetLangText(12143, "��ͳ�Ʊ�"));
	ws = xlsWorkbookSheetW(w, pLabel);

    /* д���±����ͷ */
	MonthRecordHadInfo(ws,timeRange);
	
    /* ��ʼд���û��¼�¼ */
	Row = 3;

	while(pMonthRecord != NULL && Row < usernum + 3)
	{
		memset(buf, 0, sizeof(buf));
		
		Col = 0; 
		/* ���� */
        xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->nUserNo,  g_Xf_t[BORDER_LEFT_10]);

        Col++;
        if(pMonthRecord->szUserName[0] != '\0')
		{
			pLabel = ZhcnToUincode(pMonthRecord->szUserName);
    		xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);
		}
		else
		{
			 xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
		}

        /* �ݲ�֧�ֲ��ţ�����д���� */
        Col++;
        xlsWorksheetLabel(ws, Row, Col, "",g_Xf_t[BORDER_LEFT_10]);

        Col++;
    	if((pMonthRecord->iSchedule > 0)&&(pMonthRecord->iSchedule < 6))
    	{
    		strcpy(buf,attendanceParm.Schedules[pMonthRecord->iSchedule - 1].caScheduleName);
    	}
    	else
    	{
    		strcpy(buf,(char *)GetLangText(12155, "��"));
    	}
    	pLabel = ZhcnToUincode(buf);
    	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);

        Col++; 
		if(pMonthRecord->fWorkdayNo)
    	{
    		xlsWorksheetNumberDbl(ws, Row, Col, pMonthRecord->fWorkdayNo, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

    	Col++; 
    	if(pMonthRecord->fAttendanceNo)
    	{
    		sprintf(buf, "%.1lf",pMonthRecord->fAttendanceNo);
    		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

    	Col++; 
    	if(pMonthRecord->fAbsenceNo)
    	{
    		sprintf(buf, "%.1lf",pMonthRecord->fAbsenceNo);
    		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}
    	
    	Col++; 
    	if(pMonthRecord->iDelayminute)
    	{
    		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iDelayminute, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

    	Col++;
    	if(pMonthRecord->iDelaytimes)
    	{
    		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iDelaytimes, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}
    	
    	Col++;

    	if(pMonthRecord->iLeaveEarlyMinute)
    	{
    		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iLeaveEarlyMinute, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}
    	
    	Col++;

    	if(pMonthRecord->iLeaveEarlyTimes)
    	{
    		xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->iLeaveEarlyTimes, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

    	Col++;

    	if(pMonthRecord->fOvertimeHours)
    	{
    		sprintf(buf, "%.1f", pMonthRecord->fOvertimeHours);
    		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

    	Col++;

    	if(pMonthRecord->fLeaveDays)    /* ������� */
    	{
    		sprintf(buf, "%.1f", pMonthRecord->fLeaveDays);
    		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

    	Col++;

    	if(pMonthRecord->fTrips)    /* �������� */
    	{
    		sprintf(buf, "%.1f", pMonthRecord->fTrips);
    		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

		Col++;
		Row++;
		
		pMonthRecord = pMonthRecord->pMonthForm;
        if (Row < usernum + 4)
        {
            AddRecordNum(1);
        }
	}
	
    //sprintf(buf, "/mnt/usb/%s-%s %s-%s.xls", (char*)GetLangText(11943, "��ͳ�Ʊ���"),
	//	GetSerailStr(),StartTime,EndTime);

	sprintf(buf,"/mnt/usb/%d_%d_%s.xls",pTm->tm_year + 1900, pTm->tm_mon + 1, (char*)GetLangText(11943, "��ͳ�Ʊ���"));

	//GB2312ToUTF_8(acName, buf, strlen(buf));
	GBKBufToUTF8Buf(buf, strlen(buf), acName);

    unlink(acName);    /* ɾ��ԭ���ļ� */
	if (GetUsbStatus() == TRUE)
	{
    	memset(buf, 0, sizeof(buf));
        memset(acName, 0, sizeof(acName));
        
	    //sprintf(buf, "/mnt/usb/%s-%s %s-%s.xls", (char*)GetLangText(11943, "��ͳ�Ʊ���"),
		//	GetSerailStr(),StartTime,EndTime);

		//sprintf(buf,"/mnt/usb/%03d_%d_%d_%s.xls",strConfigBase.machineID,
	    //pTm->tm_year + 1900, pTm->tm_mon + 1, (char*)GetLangText(11943, "��ͳ�Ʊ���"));
		sprintf(buf,"/mnt/usb/%d_%d_%s.xls", timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11943, "��ͳ�Ʊ���"));
        //GB2312ToUTF_8(acName, buf, strlen(buf));
    	GBKBufToUTF8Buf(buf, strlen(buf), acName);

		xlsWorkbookDump(w, acName);
	}

	sync();

	xlsDeleteWorkbook(w);

    AddRecordNum(1);
    
	return 0;
}

/**************************************************************\
** �������ƣ� ExportSchedules
** ���ܣ�     ������ٳ����
** ������     pUserLeaveTripInfo:�û���ٳ�����Ϣ
** ���أ�     �Ű��4-LeaveTrip.xls
** �������ߣ�
** �������ڣ�2013-4-13
** �޸����ߣ����ڻ�
** �޸����ڣ�
\**************************************************************/
int  ImportUserLeaveTrip(LP_ALL_USER_LEAVE_TRIP* pUserLeaveTripInfo)
{
	xlsWorkBook* pWB  = NULL;
	xlsWorkSheet* pWS = NULL;
	LP_ALL_USER_LEAVE_TRIP pUserLeaveTrip = NULL;
	LP_ALL_USER_LEAVE_TRIP pUserLeaveTripHead = NULL;
	LP_ALL_USER_LEAVE_TRIP pUserLeaveTripTmp = NULL;
	char szStartTime[16] = {0};
	struct stat st;
    char acFilePath[128];
    char caName[128];
    int iUserNum = 0;
    
#if 0
	struct st_row_data* row;
#else
	xls::st_row::st_row_data *row;
#endif

	WORD t;
	WORD tt;
	int i = 0;

    memset(acFilePath, 0, sizeof(acFilePath));
    memset(caName, 0, sizeof(caName));
    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12126, "��ٳ����"));

    //GB2312ToUTF_8(acFilePath, caName, strlen(caName));
	GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);

    if(stat(acFilePath, &st) != 0)
    {
		TRACE("%s not exist! %s %d\r\n", acFilePath, __FUNCTION__, __LINE__);

        goto IMPORTERR;
    }

	pWB = xls_open(acFilePath, "UTF-8");

	if (pWB == NULL)
	{
		TRACE("Open xls failed! %s %d\r\n", __FUNCTION__, __LINE__);
		
        goto IMPORTERR;
	}
	else
	{
		/* ���ȡ�ı��ĵ�Ԫ����䲻��������ʹ��������������ʱ����ִ��� */
		pWS = xls_getWorkSheet(pWB, 0);	
		
		xls_parseWorkSheet(pWS);

		if(pWS->rows.lastrow < 4)		
		{	
			TRACE("The xls is NULL! %s %d\r\n", __FUNCTION__, __LINE__);

            goto IMPORTERR;
		}

		/* ��ʼ���� */
		row = &pWS->rows.row[2];
		if((char *)(&row->cells.cell[0])->str != NULL)
		{
			//strncpy(szStartTime, (char *)(&row->cells.cell[3])->str, 15);
			rightStr(szStartTime, (char *)(&row->cells.cell[0])->str, 10);
		}

		//printf("szStartTime %s %s %d\r\n", szStartTime, __FUNCTION__, __LINE__);
		
		/* ѭ�������û���Ϣ������ṹ�����飬�ӵ����п�ʼ�� */			
		for (t = 4; t <= pWS->rows.lastrow; t++)		
		{
			/* ����ռ� ���Դ������� */
	        pUserLeaveTrip = (LP_ALL_USER_LEAVE_TRIP)Malloc(sizeof(ALL_USER_LEAVE_TRIP));
			memset(pUserLeaveTrip, 0, sizeof(ALL_USER_LEAVE_TRIP));
			pUserLeaveTrip->pNext = NULL;
            iUserNum++;

			//if(4 == t)
			if (pUserLeaveTripHead == NULL)
			{
				pUserLeaveTripHead = pUserLeaveTrip;
				pUserLeaveTripTmp = pUserLeaveTrip;
			}
			else
			{
				pUserLeaveTripTmp->pNext = pUserLeaveTrip;
				pUserLeaveTripTmp = pUserLeaveTrip;
			}

			strncpy(pUserLeaveTrip->szStartTime, szStartTime, 31);
			
			row = &pWS->rows.row[t];
      		tt = 0;

			/* ������ ���� */
			if((&row->cells.cell[tt])->str != NULL 
				&& (&row->cells.cell[tt])->str[0] != '\0')
			{				
				/* ���� */
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					pUserLeaveTrip->UserNo = atoi((char *)(&row->cells.cell[tt])->str);
				}

                tt++;
                /* ���� */
				memset(pUserLeaveTrip->szUserName, 0 , 32);
				
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					strncpy(pUserLeaveTrip->szUserName, (char *)(&row->cells.cell[tt])->str, 31);
				}
				
				tt++;
                memset(pUserLeaveTrip->szDepName, 0 , 32);
				/* �������� */
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					strncpy(pUserLeaveTrip->szDepName, (char *)(&row->cells.cell[tt])->str, 31);
				}
				
				for( i = 0; i < 31; i++ )
				{
					tt++;
					if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
					{
						pUserLeaveTrip->data[i]= atoi((char *)(&row->cells.cell[tt])->str);
                        if ((pUserLeaveTrip->data[i] < 0) || (pUserLeaveTrip->data[i] > 3))
                        {
                            TRACE("pUserLeaveTrip->szUserName %s data[%d] error (%d) %s %d\r\n", 
								pUserLeaveTrip->szUserName, i, (int)pUserLeaveTrip->data[i], __FUNCTION__, __LINE__);

                            goto IMPORTERR;
                        }
					}
					else
					{
						/* ���δ����Ĭ��Ϊ�������� */
						pUserLeaveTrip->data[i] = 1;
					}
				}

                TRACE("UserNo %d szUserName %s %s %d\r\n", (int)pUserLeaveTrip->UserNo, pUserLeaveTrip->szUserName, __FUNCTION__, __LINE__);
			}
		}

		xls_close_WS(pWS);
	}

	*pUserLeaveTripInfo= pUserLeaveTripHead;

	xls_close_WB(pWB);

	return iUserNum; 

IMPORTERR:

    while (pUserLeaveTripHead)
    {
        pUserLeaveTripTmp = pUserLeaveTripHead->pNext;
        Free(pUserLeaveTripHead);
        pUserLeaveTripHead = pUserLeaveTripTmp;
    }

    if (pWS != NULL)
    {
        xls_close_WS(pWS);
    }

    if (pWB)
    {
        xls_close_WB(pWB);
    }

    *pUserLeaveTripInfo = NULL;
    
    return -1;    
}

int GetDayNum( char * szStartTime, int iDay, char * caDate)
{
	int year = 0;
	int month = 0;
	int day = 0;
	char* pTime = NULL;
	unsigned long sec = 0;

	struct tm   strTime;
    time_t tRefTime;

	TRACE(" szStartTime %s %s %d\r\n", szStartTime, __FUNCTION__, __LINE__);

	pTime = szStartTime;
	year = atoi(pTime);
	
	pTime = strstr(pTime, "-");
	pTime += 1;
	month = atoi(pTime);

	pTime = strstr(pTime, "-");
	pTime += 1;
	day = atoi(pTime);

	TRACE(" year %d month %d day %d %s %s %d\r\n", year, month, day, szStartTime, __FUNCTION__, __LINE__);

	sec = 24 * 60 * 60 * iDay;

	/* �ж�ʱ����� */
    memset(&strTime, 0, sizeof(tm));
    strTime.tm_year = year -  1900;
    strTime.tm_mon  = month - 1;
    strTime.tm_mday = day;    
    strTime.tm_hour = 0;
    strTime.tm_min = 0;
    strTime.tm_sec = 0;

    tRefTime = mktime(&strTime);    /* ��ȡ������ʱ��ֵ�ʱ���������� */

	TRACE("year %d %d %d %s %d\r\n",strTime.tm_year, strTime.tm_mon, strTime.tm_mday,  __FUNCTION__, __LINE__);

	tRefTime += sec;
	
    gmtime_r(&tRefTime, &strTime);

	TRACE("year %d %d %d %s %d\r\n",strTime.tm_year, strTime.tm_mon, strTime.tm_mday,  __FUNCTION__, __LINE__);
	
	//sprintf(caDate, "%02d-%02d", strTime.tm_mon + 1, strTime.tm_mday);
	sprintf(caDate, "%02d", strTime.tm_mday);

	return 0;
}

/**************************************************************\
** �������ƣ� ExportUserLeaveTrip
** ���ܣ�     ������ٳ����
** ������     pUserLeaveTrip:�û���ٳ�����Ϣ
** ���أ�     �Ű��4-LeaveTrip.xls
** �������ߣ�
** �������ڣ�2013-4-13
** �޸����ߣ����ڻ�
** �޸����ڣ�
\**************************************************************/
int ExportUserLeaveTrip(LP_ALL_USER_LEAVE_TRIP pUserLeaveTrip)
{	
	workbook *w;
	worksheet *ws;
    wchar_t *pLabel = NULL;
	cell_t *c=NULL;
	int Col=0;
	int Row=0;
	int iTmpCol = 0;
	LP_ALL_USER_LEAVE_TRIP pUserLeaveTripTmp = NULL;
	int i = 0;
	char caDate[12] = {0};
    char acFilePath[128];
    char caName[128];
	int iYear = 0;
	int iMonth = 0;
	int iDay = 0;
	int iMonthDayNum = 0;
	int iLeftDayNum = 0;
	char aTmpBuf[64] = {0};

	pUserLeaveTripTmp = pUserLeaveTrip;

	w = xlsNewWorkbook();

    /* ��ʼ������ */
	initParticulXFTFont(w);

	pLabel = ZhcnToUincode((char*)GetLangText(12126, "��ٳ����"));
	ws = xlsWorkbookSheetW(w, pLabel);	

	xlsWorksheetMerge(ws, 0, 0, 0, 33);
	
    pLabel = ZhcnToUincode((char*)GetLangText(12126, "��ٳ����"));
    xlsWorksheetRowheight(ws,0,(unsigned16_t)60*20, NULL);
    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

    ws->defaultColwidth(8); /* ����Ĭ�Ͽ�ȣ���������Ϊ8��xlsWorksheetColwidth�Ż������� */
    for (iTmpCol = 0; iTmpCol < 3; iTmpCol++)
    {
        xlsWorksheetColwidth(ws, iTmpCol, XLS_COL_WIDTH, NULL);
    }   

	for (iTmpCol = 3; iTmpCol <= 33; iTmpCol++)
    {
        xlsWorksheetColwidth(ws, iTmpCol, XLS_LEAVE_COL_WIDTH, NULL);
    }

	for(iTmpCol = 0; iTmpCol <= 33; iTmpCol++)
	{
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}

	xlsWorksheetMerge(ws, 1, 0, 1, 33);
    pLabel = ZhcnToUincode((char *)GetLangText(12176, "�û���ٳ�����Ϣ:1-�������� 2-���� 3-���"));
    xlsWorksheetRowheight(ws, 1, (unsigned16_t)30*20, NULL);
    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);

	//xlsWorksheetMerge(ws, 2, 0, 2, 2);
	xlsCellMerge(ws,2,0, 2, 2, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
	sprintf(aTmpBuf, "%s: %s", (char *)GetLangText(12163, "��ʼ����"), pUserLeaveTripTmp->szStartTime);
	pLabel = ZhcnToUincode(aTmpBuf);
	xlsWorksheetLabelW(ws, 2, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);

	memset(aTmpBuf, 0, sizeof(aTmpBuf));
	strncpy(aTmpBuf, pUserLeaveTripTmp->szStartTime, 4);
	iYear = atoi(aTmpBuf);

	memset(aTmpBuf, 0, sizeof(aTmpBuf));
	strncpy(aTmpBuf, pUserLeaveTripTmp->szStartTime + 5, 2);
	iMonth = atoi(aTmpBuf);

	memset(aTmpBuf, 0, sizeof(aTmpBuf));
	strncpy(aTmpBuf, pUserLeaveTripTmp->szStartTime + 8, 2);
	iDay = atoi(aTmpBuf);

	printf("iYear %04d iMonth %02d iDay %02d %s %d\r\n", iYear,
		iMonth, iDay, __FUNCTION__, __LINE__);

	iMonthDayNum = GetMonthDay(iYear, iMonth) - iDay + 1;

	iLeftDayNum = 31 - iMonthDayNum;

	sprintf(aTmpBuf, "%02d%s", iMonth, (char *)GetLangText(11310, "��"));
    xlsCellMerge(ws,2, 3, 2, 3 + iMonthDayNum - 1, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
	pLabel = ZhcnToUincode(aTmpBuf);
	xlsWorksheetLabelW(ws, 2, 3, pLabel, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);

	if (iLeftDayNum > 0)
	{
		sprintf(aTmpBuf, "%02d%s", iMonth + 1, (char *)GetLangText(11310, "��"));
        xlsCellMerge(ws, 2, 3 + iMonthDayNum, 2, 33, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
		pLabel = ZhcnToUincode(aTmpBuf);
		xlsWorksheetLabelW(ws, 2, 3 + iMonthDayNum, pLabel, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
	}

	pLabel = ZhcnToUincode((char *)GetLangText(11295, "����"));
	xlsWorksheetLabelW(ws, 3, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

    pLabel = ZhcnToUincode((char *)GetLangText(10006, "����"));
   	xlsWorksheetLabelW(ws, 3, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	pLabel = ZhcnToUincode((char *)GetLangText(10012, "����"));
   	xlsWorksheetLabelW(ws, 3, 2, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	/* 31������ */
	for( i=0; i<31; i++ )
	{
		GetDayNum(pUserLeaveTripTmp->szStartTime, i, caDate);
		
		xlsWorksheetLabel(ws, 3, i+3, caDate, g_Xf_t[BORDER_LEFT_Text_10]);
	}
	
	Row = 4;
	while(NULL != pUserLeaveTripTmp)
	{
		Col = 0;

		/* ���� */
		xlsWorksheetNumberInt(ws, Row, Col, pUserLeaveTripTmp->UserNo, g_Xf_t[BORDER_LEFT_Text_10]); 

		Col++;
		/* �û��� */
		if(pUserLeaveTripTmp->szUserName[0] != '\0')
		{
			pLabel = ZhcnToUincode(pUserLeaveTripTmp->szUserName);
			xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_Text_10]);
		}

        Col++;
        xlsWorksheetLabel(ws, Row, Col, "",  g_Xf_t[BORDER_LEFT_Text_10]);

		for( i = 0; i < 31; i++ )
		{
			Col++;
			xlsWorksheetNumberInt(ws, Row, Col, pUserLeaveTripTmp->data[i], g_Xf_t[BORDER_LEFT_Text_10]); 
		}

		Row++;
		pUserLeaveTripTmp = pUserLeaveTripTmp->pNext;
        AddRecordNum(1);
	}	

    if (GetUsbStatus() == TRUE)
    {
        memset(acFilePath, 0, sizeof(acFilePath));
        memset(caName, 0, sizeof(caName));
    	sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12126, "��ٳ����"));

		GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);
    	xlsWorkbookDump(w, acFilePath);

    	sync();

    	xlsDeleteWorkbook(w);
    }
	return 0;
}


//--------------------------------------
int create_file(char * file_name)
{
	release();
    
    unlink(file_name);
    
	m_xls = fopen(file_name, "w+");

    //printf("m_xls %p file_name %s %s %d.\r\n", m_xls, file_name, __FUNCTION__, __LINE__);

	if (!m_xls)
		return __LINE__;

	//fprintf(m_xls, FILE_HTML_TITLE);

	if (load_xls_head())
		return __LINE__;
	
	//fprintf(m_xls, FILE_BODY_TITLE);
	//fprintf(m_xls, FILE_TABLE_TITLE);

	return 0;
}

int load_xls_head()
{
    char *pcHead;
    int indexFd;
	struct stat st;
    
	//CHECK_BUFFER;
	//FILE *p_head = fopen(XML_HEAD_TEXT_FILE, "r");

	//if (!p_head)
	//	return ERR_WRITE_HEAD_FAILED;

	//copy_text(p_head, m_xls);
	//fclose(p_head);

        if (stat(XML_HEAD_TEXT_FILE, &st) == 0) /* �ļ������ڣ�ֱ�ӷ��� */
        {
            pcHead = (char *)malloc(st.st_size + 1);
            memset(pcHead, 0, (st.st_size + 1));
        
            indexFd = open(XML_HEAD_TEXT_FILE, O_RDONLY | O_NOATIME, 0666);
            if (indexFd < 0)
            {
                printf(" %s %d\r\n", __FUNCTION__, __LINE__);
            }
            else
            {
                read(indexFd, pcHead, st.st_size);
            	fprintf(m_xls, "%s", pcHead);
            }

            printf("%s st.st_size %d %s %d.\r\n", XML_HEAD_TEXT_FILE, (int)st.st_size, __FUNCTION__, __LINE__);

            free(pcHead);
            close(indexFd);
        }
        else
        {
            printf(" %s %d\r\n", __FUNCTION__, __LINE__);

    		return __LINE__;
        }

	return 0;
}

int end_file()
{
	//CHECK_BUFFER;
	//fprintf(m_xls, "</table>\n");
	//fprintf(m_xls, "</body>\n");
	//fprintf(m_xls, "</html>\n");

	//FILE *p_head = fopen(XML_END_SHEET_FILE, "r");

	//if (!p_head)
	//	return ERR_WRITE_HEAD_FAILED;

	//copy_text(p_head, m_xls);
    
	//fclose(p_head);

	CHECK_BUFFER -1;
	
	fprintf(m_xls, "%s", XML_WORKBOOK_END);

	release();

	return 0;
}

void release()
{
	if (m_xls)
	{
		fclose(m_xls);
		m_xls = NULL;
	}

    if (pcUserInfo)
    {
        free(pcUserInfo);
        pcUserInfo = NULL;
    }
}
void start_new_sheet(char* val)
{
    char acInfo[128];

	CHECK_BUFFER;

    memset(acInfo, 0, sizeof(acInfo));
    sprintf(acInfo, "\n<Worksheet ss:Name=\"%s\">\n", val);
        
	fprintf(m_xls, "%s", acInfo);
    
    start_new_table(TABLE_START);
}

void end_sheet()
{
    char *pcHead;
    int indexFd;
	struct stat st;
    
	CHECK_BUFFER;
    if (stat(XML_END_SHEET_FILE, &st) == 0) /* �ļ������ڣ�ֱ�ӷ��� */
    {
        pcHead = (char *)malloc(st.st_size + 1);
        memset(pcHead, 0, (st.st_size + 1));
    
        indexFd = open(XML_END_SHEET_FILE, O_RDONLY | O_NOATIME, 0666);
        if (indexFd < 0)
        {
            printf(" %s %d\r\n", __FUNCTION__, __LINE__);
        }
        else
        {
            read(indexFd, pcHead, st.st_size);
        	fprintf(m_xls, "%s", pcHead);
        }

        printf("%s st.st_size %d %s %d.\r\n", XML_END_SHEET_FILE, (int)st.st_size, __FUNCTION__, __LINE__);

        free(pcHead);
        close(indexFd);

    }
    else
    {
        printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		return ;
    }

	return ;
}

void start_new_table(const char* val)
{
	CHECK_BUFFER;
	fprintf(m_xls, "%s", val);
}

void start_new_row(void)
{
	CHECK_BUFFER;
	fprintf(m_xls, "%s", STARTROW);
}

void add_normal_attribute_of_row(void)
{
	CHECK_BUFFER;
	fprintf(m_xls, "%s", ADDNORMALROW);
}

void end_row(void)
{
	CHECK_BUFFER;
	fprintf(m_xls, "%s", ENDROW);
}

void fill_user_headinfo(int userno, char* name, char* dep, int isch, char* datestart, char* dateend)
{
    char acInfo[1024];
    char tmpbuf[128];    
    int iLength = 0;
    char *pcTmp;

    memset(tmpbuf, 0, sizeof(tmpbuf));

    /* ������� */
    pcTmp = GetScheduleNameByNo(isch);
    if (pcTmp)
    {
        memcpy(tmpbuf, pcTmp, 32);
    }
    else
    {
		sprintf(tmpbuf,"%s",(char *)GetLangText(12155, "��"));
	}

	CHECK_BUFFER;

    memset(acInfo, 0, sizeof(acInfo));
    iLength  = sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%d</Data></Cell>\n", (char *)GetLangText(11295, "����"), userno);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s</Data></Cell>\n", (char *)GetLangText(10006, "����"), name);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s</Data></Cell>\n", (char *)GetLangText(10012, "����"), dep);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s</Data></Cell>\n", (char *)GetLangText(12144, "���"), tmpbuf);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"3\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s~%s</Data></Cell>", (char *)GetLangText(10013, "����"), datestart,dateend);

	fprintf(m_xls, "%s%s", ADDUSERINFO_ATT, acInfo);
    end_row();
}

//void fill_user_headinfo(const char* val)
//{
//	CHECK_BUFFER;
//	fprintf(m_xls, "%s", val);
//}

void fill_user_table_headinfo(void)
{
#if 0
    int indexFd;
	struct stat st;
    
	CHECK_BUFFER;

    if (pcUserInfo == NULL)
    {
        if (stat(XML_USER_TABLE_FILE, &st) == 0) /* �ļ������ڣ�ֱ�ӷ��� */
        {
            pcUserInfo = (char *)malloc(st.st_size + 1);
            memset(pcUserInfo, 0, (st.st_size + 1));
        
            indexFd = open(XML_USER_TABLE_FILE, O_RDONLY | O_NOATIME, 0666);
            if (indexFd < 0)
            {
                printf(" %s %d\r\n", __FUNCTION__, __LINE__);
            }
            else
            {
                read(indexFd, pcUserInfo, st.st_size);
            }

            printf("%s st.st_size %d %s %d.\r\n", XML_USER_TABLE_FILE, (int)st.st_size, __FUNCTION__, __LINE__);
            close(indexFd);
        }
    }
    
    printf("pcUserInfo %p %s %d.\r\n", pcUserInfo, __FUNCTION__, __LINE__);
    
	fprintf(m_xls, "%s", pcUserInfo);
#endif

	char acInfo[2048];

	CHECK_BUFFER;
    memset(acInfo, 0, sizeof(acInfo));
    sprintf(acInfo, "<Row ss:AutoFitHeight=\"0\">"
					"<Cell ss:StyleID=\"m26293234\"/>"
					"<Cell ss:StyleID=\"m26293234\"/>");
	fprintf(m_xls, "%s", acInfo);
	
	memset(acInfo, 0, sizeof(acInfo));
	sprintf(acInfo, "<Cell ss:MergeAcross=\"1\" ss:StyleID=\"m26293234\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:MergeAcross=\"1\" ss:StyleID=\"m26293234\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:MergeAcross=\"1\" ss:StyleID=\"m26293234\"><Data ss:Type=\"String\">%s</Data></Cell>",
					(char *)GetLangText(11928, "���һ"), (char *)GetLangText(11929, "��ζ�"), (char *)GetLangText(11930, "�����"));
	fprintf(m_xls, "%s", acInfo);
	
	memset(acInfo, 0, sizeof(acInfo));
	sprintf(acInfo, "<Cell ss:StyleID=\"m26293234\"/>"
					"<Cell ss:StyleID=\"m26293234\"/>");
	fprintf(m_xls, "%s", acInfo);

	memset(acInfo, 0, sizeof(acInfo));
	sprintf(acInfo, "<Cell ss:MergeAcross=\"1\" ss:StyleID=\"m26293234\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:MergeAcross=\"1\" ss:StyleID=\"m26293234\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:MergeAcross=\"1\" ss:StyleID=\"m26293234\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"</Row>"
					"<Row ss:AutoFitHeight=\"0\" ss:Height=\"24\">",
					(char *)GetLangText(11928, "���һ"), (char *)GetLangText(11929, "��ζ�"), (char *)GetLangText(11930, "�����"));
	fprintf(m_xls, "%s", acInfo);

	memset(acInfo, 0, sizeof(acInfo));
	sprintf(acInfo, "<Cell ss:StyleID=\"s23\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:StyleID=\"s24\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(IN)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(OUT)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(IN)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(OUT)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(IN)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(OUT)</Data></Cell>",
					(char *)GetLangText(10013, "����"), (char *)GetLangText(12175, "����"),
					(char *)GetLangText(11403, "�ϰ�"), (char *)GetLangText(11404, "�°�"),
					(char *)GetLangText(11403, "�ϰ�"), (char *)GetLangText(11404, "�°�"),
					(char *)GetLangText(11403, "�ϰ�"), (char *)GetLangText(11404, "�°�"));
	fprintf(m_xls, "%s", acInfo);

	memset(acInfo, 0, sizeof(acInfo));
	sprintf(acInfo, "<Cell ss:StyleID=\"s23\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:StyleID=\"s24\"><Data ss:Type=\"String\">%s</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(IN)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(OUT)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(IN)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(OUT)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(IN)</Data></Cell>"
					"<Cell ss:StyleID=\"s25\"><Data ss:Type=\"String\">%s(OUT)</Data></Cell>"
					"</Row>",
					(char *)GetLangText(10013, "����"), (char *)GetLangText(12175, "����"),
					(char *)GetLangText(11403, "�ϰ�"), (char *)GetLangText(11404, "�°�"),
					(char *)GetLangText(11403, "�ϰ�"), (char *)GetLangText(11404, "�°�"),
					(char *)GetLangText(11403, "�ϰ�"), (char *)GetLangText(11404, "�°�"));
	fprintf(m_xls, "%s", acInfo);
}

void fill_normal_cell(int itype, char* cells, char* StyleID, char *info)
{
    char acInfo[1024];
	CHECK_BUFFER;

    memset(acInfo, 0, sizeof(acInfo));
    
    if (itype == NORMAL_FILL_CELL)
    {
        sprintf(acInfo, "<Cell ss:StyleID=\"%s\"><Data ss:Type=\"String\">%s</Data></Cell>\n", StyleID, info);
    }
    else if (itype == MERGE_FILL_CELL)
    {
        sprintf(acInfo, "<Cell ss:MergeAcross=\"%s\" ss:StyleID=\"%s\"><Data ss:Type=\"String\">%s</Data></Cell>\n", cells, StyleID, info);
    }
    else
    {
        sprintf(acInfo, "%s", ADDEMPTYCELL);
    }
    
	fprintf(m_xls, "%s", acInfo);
}

void fill_title_info(char* StartTime, char* EndTime)
{
    char acInfo[1024];

	CHECK_BUFFER;
    memset(acInfo, 0, sizeof(acInfo));
    sprintf(acInfo, "<Cell ss:MergeAcross=\"15\" ss:StyleID=\"s31\"><Data ss:Type="
		"\"String\">%s</Data></Cell>\n", (char *)GetLangText(11942, "������ϸ��"));

	fprintf(m_xls, "%s%s", ADDUSERINFO_ATT, acInfo);

    end_row();
}

void fill_user_attendaceinfo(float days, float workdays, float absence, int latenum,
	int lateMin, int earlyNum, int earlyMin, float overtime, float leaveDays, float trips)
{
    char acInfo[1024];

	CHECK_BUFFER;
    memset(acInfo, 0, sizeof(acInfo));
    sprintf(acInfo, "<Cell ss:MergeAcross=\"15\" ss:StyleID=\"m26293204\"><Data ss:Type="
		"\"String\">%s:  %.1lf  %s:  %.1lf   %s:  %.1lf   %s/%s:"
		"  %d/%d   %s/%s:%d/%d   %s:  %.1lf   %s:  %.1lf   %s:  %.1lf</Data></Cell>\n", 
        (char *)GetLangText(12145, "��������"), days, (char *)GetLangText(12146, "��������"), workdays,
        (char *)GetLangText(12147, "ȱ������"), absence, (char *)GetLangText(12149, "�ٵ�����"), 
          (char *)GetLangText(11444, "����"), latenum, lateMin, (char *)GetLangText(12151, "���˴���"), 
          (char *)GetLangText(11444, "����"), earlyNum, earlyMin, (char *)GetLangText(12165, "�Ӱ�ʱ��"), 
          overtime, (char *)GetLangText(12153, "�������"), leaveDays, (char *)GetLangText(12154, "��������"), trips);
    
	fprintf(m_xls, "%s%s", ADDUSERTABLE, acInfo);
    end_row();
}

void fill_company_attendaceinfo(TIMERANGE timeRange)
{
				 
	CONFIG_BASE_STR strConfigBase;
	memset(&strConfigBase, 0, sizeof(CONFIG_BASE_STR));
	GetConfigBase(&strConfigBase);
	
    char acInfo[1024];

	CHECK_BUFFER;
    memset(acInfo, 0, sizeof(acInfo));
    sprintf(acInfo, "<Cell ss:MergeAcross=\"15\"><Data ss:Type="
		"\"String\">%s:%04d-%02d-%02d %s %04d-%02d-%02d</Data></Cell>\n",(char *)GetLangText(10013, "����"), timeRange.iStartYear, timeRange.iStartMonth,
	timeRange.iStartDay, (char *)GetLangText(12140, "��"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
    
	fprintf(m_xls, "%s%s", ADDNORMALROW, acInfo);
    end_row();
}

void fill_user_wageinfo(void)
{
    char acInfo[1024];
    
	CHECK_BUFFER;
    
    memset(acInfo, 0, sizeof(acInfo));
    sprintf(acInfo, "<Cell ss:MergeAcross=\"15\" ss:StyleID=\"m26293204\"><Data ss:Type=\"String\">%s��    %s:     %s��    %s��     %s��    %s��     %s��    </Data></Cell>\n",
		(char *)GetLangText(12166, "����ʱ��"),
		(char *)GetLangText(12167, "�¼�ʱ��"),
		(char *)GetLangText(12168, "��н����"),
		(char *)GetLangText(12169, "�Ӱ๤��"),
		(char *)GetLangText(12170, "��������"),
		(char *)GetLangText(12171, "�����ۿ�"),
		(char *)GetLangText(12172, "ʵ�ù���"));

	if (GetLangID() == 1)
	{
		fprintf(m_xls, "%s%s", ADDUSERTABLE, acInfo);
	}
	else
	{
		fprintf(m_xls, "%s%s", ADDUSERTABLE2, acInfo);
	}
    end_row();
}

void fill_user_tailinfo(void)
{
	CHECK_BUFFER;
	fprintf(m_xls, "%s<Cell ss:MergeAcross=\"15\" ss:StyleID=\"m26293204\">"
		"<Data ss:Type=\"String\">%s:                           %s:</Data></Cell>\n",
		ADDUSERTABLE3, (char *)GetLangText(12173, "ȷ��"), (char *)GetLangText(12174, "���"));
    end_row();
}
