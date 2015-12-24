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

/* 字体 */
typedef enum
{
  NOBORDER_BOLD_14 = 0,	/* 字体14号加粗，无边框，横竖居中  */
  NOBORDER_12, 			/* 字体12号不加粗，无边框，横居左竖居中 */
  BORDER_FILL_12,		/* 字体12号加粗，4边框，灰色背景，横左竖居中 */
  BORDER_RED_9,			/* 字体9号红色，4边框，横竖居中 */
  BORDER_RED_LEFT_9,	/* 字体9号红色，4边框，横左居中 */
  BORDER_9,				/* 字体9号黑色，4边框，横竖居中 */
  BORDER_RACCROSS_9,	/* 字体9号黑色，4边框，横两端对齐竖居中 */
  BORDER_10,			/* 字体10号黑色，4边框，横竖居中 */
  BORDER_LEFT_10,		/* 字体10号黑色，4边框，横左竖居中 */
  BORDER_CENTERACCROSS_FILL_10,	/* 字体10号黑色，4边框，灰色背景，横跨行居中竖居中 */
  NOBORDER_LEFT_10,		/* 字体10号黑色，无边框，横左竖居中 */
  BORDER_LEFT_FILL_10,	/* 字体10号黑色，4边框，灰色背景，横左竖居中 */
  NOBORDER_BOLD_20,		/* 字体20号加粗，无边框，横竖居中  */
  BORDER_LEFT_Text_10,	/* 字体10号黑色，4边框，横左竖居中 文本 */
  BORDER_RED_FILL_LEFT_10,	/* 字体10号红色，4边框，横左竖居中 灰色背景， */
  BORDER_RED_FILL_LEFT_Text_10,	/* 字体10号红色，4边框，文本横左竖居中 灰色背景， */
  BORDER_RED_LEFT_TEXT_10,	/* 字体10号红色，4边框，横左竖居中 文本*/
  BORDER_LEFT_FILL_NOTRAP_10, /* 字体10号黑色，4边框，灰色背景，横左竖居中 */
} Xft_Font;


#define XLS_FORMAT

/* 单元格设置宏定义 */
#define XLS_LABEL_DEFAULT_INFO(ws, row, col, pLabel, c, xf)     \
   	xlsWorksheetLabelW(ws, row, col, pLabel, NULL);             \
	c = xlsWorksheetFindCell(ws, row, col);/* 查找单元格 */     \
	xlsCellFillstyle(c, FILL_SOLID);                        \
    xlsCellFillfgcolor(c, CLR_GRAY25);/* 填充 */            \
	xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);      \
	xlsCellBorderstyle(c, BORDER_TOP, BORDER_THIN);	        \
    xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);       \
    xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);        \
    xlsCellWrap(c, true);   /* 设置自动换行 */              \
 	xlsCellHalign(c, HALIGN_LEFT);                        \
    xlsCellValign(c, VALIGN_CENTER);                        \

#define XLS_CELL_DEFAULT_FORMAT(ws, Row, Col, c, xf)        \
		c = xlsWorksheetFindCell(ws, Row, Col);             \
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_THIN);  \
   		xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);   \
   		xlsCellBorderstyle(c, BORDER_LEFT, BORDER_THIN);        \
        xlsCellWrap(c, true);   /* 设置自动换行 */              \
     	xlsCellHalign(c, HALIGN_LEFT);                        \
        xlsCellValign(c, VALIGN_CENTER);                        \
		
extern void GB2312ToUTF_8(char *pOut,char *pText, int pLen);

xf_t *xlsWorkbookxFormatDefaultEx(workbook *w, int n)
{
	return NULL;
}

/**************************************************************\
** 函数名称： xlsCellMerge
** 功能：     合并单元格
** 参数：     
** 返回：    
** 创建作者：胡宗华
** 创建日期：2013-10-21
** 修改作者：
** 修改日期：
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

/* 添加字体:粗体 */
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
	/* 粗体 */
	xlsFontSetBoldStyle(pFont_t, BOLDNESS_BOLD);

	return pFont_t;
}

/* 添加字体:默认非粗体黑色 */
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
	/* 粗体 */
	//xlsFontSetBoldStyle(pFont_t, BOLDNESS_BOLD);

	return pFont_t;
}

/* 添加字体:红色非粗体 */
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
	/* 粗体 */
	//xlsFontSetBoldStyle(pFont_t, BOLDNESS_BOLD);

	return pFont_t;
}

/* 增加单元格格式，无边框，横竖居中,字体为NULL时，使用默认字体 */
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
	/* 居中 */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);
	xlsXformatSetWrap(pXf_t, true);   /* 设置自动换行 */

	return pXf_t;
}

/* 增加单元格格式，边框，横左竖居中, 灰色背景 字体为NULL时，使用默认字体 */
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
	/* 居中 */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);

	/* 边框 */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
		/* 填冲 */
	xlsXformatSetFillStyle(pXf_t, FILL_SOLID);
	xlsXformatSetFillFGColor(pXf_t, CLR_GRAY25);
	xlsXformatSetWrap(pXf_t, true);   /* 设置自动换行 */

	return pXf_t;
}
/* 增加单元格格式，横竖居中, 边框， 字体为NULL时，使用默认10号字体 */
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
	/* 边框 */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
	/* 文本格式 */
    //xlsXformatSetFormat(pXf_t, FMT_TEXT);

	xlsXformatSetWrap(pXf_t, true);   /* 设置自动换行 */

	return pXf_t;
}

/* 增加单元格格式，横竖居中, 边框，文本 字体为NULL时，使用默认10号字体 */
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
	/* 边框 */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
	/* 文本格式 */
    xlsXformatSetFormat(pXf_t, FMT_TEXT);

	xlsXformatSetWrap(pXf_t, true);   /* 设置自动换行 */

	return pXf_t;
}


/* 增加单元格格式，边框，横左竖居中, 灰色背景 字体为NULL时，使用默认字体 文本 */
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
	/* 居中 */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);

	/* 边框 */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
		/* 填冲 */
	xlsXformatSetFillStyle(pXf_t, FILL_SOLID);
	xlsXformatSetFillFGColor(pXf_t, CLR_GRAY25);
	xlsXformatSetWrap(pXf_t, true);   /* 设置自动换行 */

	/* 文本格式 */
    xlsXformatSetFormat(pXf_t, FMT_TEXT);

	return pXf_t;
}

/* 增加单元格格式，边框，横左竖居中, 灰色背景 字体为NULL时，使用默认字体 不使用自动换行 */
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
	/* 居中 */
	xlsXformatSetHAlign(pXf_t, ha_option);
	xlsXformatSetVAlign(pXf_t, VALIGN_CENTER);

	/* 边框 */
	xlsXformatSetBorderStyle(pXf_t, BORDER_BOTTOM, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_RIGHT, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_TOP, BORDER_THIN);
	xlsXformatSetBorderStyle(pXf_t, BORDER_LEFT, BORDER_THIN);
		/* 填冲 */
	xlsXformatSetFillStyle(pXf_t, FILL_SOLID);
	xlsXformatSetFillFGColor(pXf_t, CLR_GRAY25);
	//xlsXformatSetWrap(pXf_t, true);   /* 设置自动换行 */

	return pXf_t;
}

int initParticulXFTFont(workbook *w)
{
	font_t* pFont_t = NULL;
	
	xf_t * pXf_t = NULL;

	memset(g_Xf_t, 0, sizeof(g_Xf_t));

	/* 增加单元格式:字体14号加粗，无边框，横竖居中 */
	pFont_t = AddFont_Bold(w, 14);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t,HALIGN_CENTER);
	g_Xf_t[NOBORDER_BOLD_14] =  pXf_t;

	/* 增加单元格式:字体12号不加粗，无边框，横居左竖居中 */
	pFont_t = AddFont_Default(w, 12);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t, HALIGN_LEFT);
	g_Xf_t[NOBORDER_12] =  pXf_t;

	/* 增加单元格式:字体12号加粗，4边框，灰色背景，横竖居中 */
	pFont_t = AddFont_Bold(w, 12);
	pXf_t = AddCellXFT_Center_Fill(w,pFont_t,HALIGN_LEFT);
	g_Xf_t[BORDER_FILL_12] =  pXf_t;

	/* 增加单元格式:字体9号红色，4边框，横竖居中 */
	pFont_t = AddFont_Red(w, 9);
	pXf_t = AddCellXFT_Border(w,pFont_t,HALIGN_CENTER);
	g_Xf_t[BORDER_RED_9] =  pXf_t;

	/* 增加单元格式:字体红色9号，4边框，横左竖居中 */
	pXf_t = AddCellXFT_Border(w,pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_LEFT_9] =  pXf_t;
	/* 增加单元格式:字体14号加粗，无边框，横竖居中 */
	pFont_t = AddFont_Bold(w, 14);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t, HALIGN_CENTER);
	g_Xf_t[NOBORDER_BOLD_14] =  pXf_t;
	/* 增加单元格式:字体黑色9号，4边框，横竖居中 */
	pFont_t = AddFont_Default(w, 9);
	pXf_t = AddCellXFT_Border(w,pFont_t, HALIGN_CENTER);
	g_Xf_t[BORDER_9] =  pXf_t;
	
	/* 增加单元格式:字体黑色9号，4边框，横两边对齐竖居中 */
	pXf_t = AddCellXFT_Border(w,pFont_t, HALIGN_JUSTIFY);
	g_Xf_t[BORDER_RACCROSS_9] =  pXf_t;

	/* 增加单元格式:字体10，4边框，横竖居中 */
	pXf_t = AddCellXFT_Border(w, NULL,HALIGN_CENTER);
	g_Xf_t[BORDER_10] =  pXf_t;
	
	/* 增加单元格式:字体10，4边框，横左竖居中 */
	pXf_t = AddCellXFT_Border(w, NULL,HALIGN_LEFT);
	g_Xf_t[BORDER_LEFT_10] =  pXf_t;

	/*增加单元格式:字体10，4边框，横跨列居中竖居中*/
	pXf_t = AddCellXFT_Center_Fill(w, NULL, HALIGN_CENTERACCROSS);
	g_Xf_t[BORDER_CENTERACCROSS_FILL_10] =  pXf_t;

	/*增加单元格式:字体10，无边框，横左竖居中*/
	pXf_t = AddCellXFT_NoBorder(w, NULL, HALIGN_LEFT);
	g_Xf_t[NOBORDER_LEFT_10] =  pXf_t;

	/* 增加单元格式:字体20号加粗，无边框，横竖居中 */
	pFont_t = AddFont_Bold(w, 20);
	pXf_t = AddCellXFT_NoBorder(w,pFont_t, HALIGN_CENTER);
	g_Xf_t[NOBORDER_BOLD_20] =  pXf_t;

	/*增加单元格式:字体10，无边框，灰色背景，横左竖居中*/
	pXf_t = AddCellXFT_Center_Fill(w, NULL, HALIGN_LEFT);
	g_Xf_t[BORDER_LEFT_FILL_10] =  pXf_t;

    /*增加单元格式:字体10，无边框，灰色背景，横竖居中,不自动换行*/
	pXf_t = AddCellXFT_Center_NoTrap_Fill(w, NULL, HALIGN_CENTER);
	g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10] =  pXf_t;

	/*增加单元格式:字体10，4边框，横左竖居中 文本 */
	pXf_t = AddCellXFT_Border_Text(w, NULL, HALIGN_LEFT);
	g_Xf_t[BORDER_LEFT_Text_10] =  pXf_t;

	/*增加单元格式:字体10，4边框，横左竖居中 文本 */
	pFont_t = AddFont_Red(w, 10);
	pXf_t = AddCellXFT_Center_Fill(w, pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_FILL_LEFT_10] =  pXf_t;

	/*增加单元格式:字体10，4边框，横左竖居中 文本 */
	pFont_t = AddFont_Red(w, 10);
	pXf_t = AddCellXFT_Center_FillText(w, pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_FILL_LEFT_Text_10] =  pXf_t;

	/*增加单元格式:字体10，4边框，横左竖居中 文本 */
	pFont_t = AddFont_Red(w, 10);
	pXf_t = AddCellXFT_Border_Text(w, pFont_t, HALIGN_LEFT);
	g_Xf_t[BORDER_RED_LEFT_TEXT_10] =  pXf_t;
	return 0;
}

/**************************************************************\
** 函数名称： ParticulUserHaedInfo
** 功能：     考勤明细表通用表头
** 参数：     
** 返回：    
** 创建作者：胡宗华
** 创建日期：2013-10-21
** 修改作者：
** 修改日期：
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

	sprintf(buf,"%s", (char *)GetLangText(11942, "考勤明细表"));

	//xf = xlsWorkbookxFormatDefaultEx(w, 5);

	/* 查询的天数合并 写入表头 */
	xlsWorksheetMerge(wsRecord, 0, 0, 0, (XLS_ORI_RECORD_TAB_COLS - 2));
	/* 设置行高25像素 */
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
	sprintf(buf,"%s:%04d-%02d-%02d %s %04d-%02d-%02d", (char *)GetLangText(10013, "日期"), timeRange.iStartYear, timeRange.iStartMonth,
		timeRange.iStartDay, (char *)GetLangText(12140, "到"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
	pLabel = ZhcnToUincode(buf);
    xlsWorksheetLabelW(wsRecord, 1, 0, pLabel, g_Xf_t[NOBORDER_12]);

	Row = 2;
	Col = 0;
	xlsWorksheetRowheight(wsRecord, 3, (unsigned16_t)20*20, NULL);
	/* 工号合并与填写 */
	sprintf(buf, "%s:%d", (char *)GetLangText(11295, "工号"), iUserNo);
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* 姓名合并与填写 */
	sprintf(buf, "%s:%s", (char *)GetLangText(10006, "姓名"), szUserName);
	Col = 3;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* 部门合并与填写 */
	sprintf(buf, "%s:%s",(char *)GetLangText(10012, "部门"), "");
	Col = 6;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* 班次合并与填写 */
	memset(tmpbuf, 0, sizeof(tmpbuf));
	 /* 班次名称 */
	pcTmp = GetScheduleNameByNo(iSh);
	if (pcTmp)
	{
	    memcpy(tmpbuf, pcTmp, 32);
	}
	else
	{
		sprintf(tmpbuf,"%s",(char *)GetLangText(12155, "无"));
	}

	sprintf(buf, "%s:%s",(char *)GetLangText(12144, "班次"), tmpbuf);

	Col = 9;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+2, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	/* 日期合并与填写 */
	sprintf(buf, "%s:%d.%d~%d.%d",(char *)GetLangText(10013, "日期"), timeRange.iStartMonth,timeRange.iStartDay,
				timeRange.iEndMonth,timeRange.iEndDay);

	Col = 12;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+3, g_Xf_t[BORDER_FILL_12]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_FILL_12]);

	Row = 4;
	xlsWorksheetLabel(wsRecord, Row, 0, "", g_Xf_t[BORDER_10]);
	xlsWorksheetLabel(wsRecord, Row, 1, "", g_Xf_t[BORDER_10]);

	Col = 2;
	/* 左边班段一 */	
	pLabel = ZhcnToUincode((char *)GetLangText(11928, "班段一"));
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col = 10;
	/* 右边班段一 */
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);
	
	/* 左边班段二 */
	Col = 4;
	pLabel = ZhcnToUincode((char *)GetLangText(11929, "班段二"));
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	/* 右边班段二 */
	Col = 12;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	/* 左边班段三 */
	Col = 6;
	pLabel = ZhcnToUincode((char *)GetLangText(11930, "班段三"));
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	/* 右边班段三 */
	Col = 14;
	xlsCellMerge(wsRecord,Row,Col, Row, Col+1, g_Xf_t[BORDER_10]);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	xlsWorksheetLabel(wsRecord, Row, 8, "", g_Xf_t[BORDER_10]);
	xlsWorksheetLabel(wsRecord, Row, 9, "", g_Xf_t[BORDER_10]);

	Row++;
	Col = 0;
	pLabel = ZhcnToUincode((char *)GetLangText(10013, "日期"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12175, "星期"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col = 8;
	pLabel = ZhcnToUincode((char *)GetLangText(10013, "日期"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12175, "星期"));
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_10]);

	Col = 2;
	sprintf(tmpbuf, "%s(IN)",(char *)GetLangText(11403, "上班"));
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
	sprintf(tmpbuf, "%s(OUT)",(char *)GetLangText(11404, "下班"));
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

	sprintf(buf, (char *)GetLangText(12143, "月统计表"));

    ws->defaultColwidth(8); /* 设置默认宽度，必须设置为8，xlsWorksheetColwidth才会起作用 */

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
	sprintf(buf,"%s:%04d-%02d-%02d %s %04d-%02d-%02d", (char *)GetLangText(10013, "日期"), timeRange.iStartYear, timeRange.iStartMonth,
		timeRange.iStartDay, (char *)GetLangText(12140, "到"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
	pLabel = ZhcnToUincode(buf);
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[NOBORDER_12]);

	/* 写入表头 */
	Row = 2;
	Col = 0;

	if (GetLangID() ==  1)
	{
		xlsWorksheetRowheight(ws, Row, (unsigned16_t)60*20, NULL);   /* 设置行高 */
	}
	else
	{
		xlsWorksheetRowheight(ws, Row, (unsigned16_t)30*20, NULL);   /* 设置行高 */
	}

	//xlsWorksheetRowheight(ws, Row, (unsigned16_t)15 * 20, NULL);
	pLabel = ZhcnToUincode((char *)GetLangText(11295, "工号"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);  

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(10006, "姓名"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]); 

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(10012, "部门"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]); 

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12144, "班次"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12145, "工作日数"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12146, "出勤天数"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12147, "缺勤天数"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12148, "迟到分钟"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12149, "迟到次数"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12150, "早退分钟"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12151, "早退次数"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12152, "加班小时"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12153, "请假天数"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	Col++;
	pLabel = ZhcnToUincode((char *)GetLangText(12154, "出差天数"));
    xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_CENTERACCROSS_FILL_10]);

	TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

	return 0;
}

/**************************************************************\
** 函数名称： ExportUserInfo
** 功能：     导出人员信息表
** 参数：     pUser:人员信息链表头指针
              UserNum:人员总数
** 返回：     0:导出成功
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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

	/* 初始化字体 */
	initParticulXFTFont(w);
	
    pLabel = ZhcnToUincode((char *)GetLangText(12123, "人员信息表"));
	ws = xlsWorkbookSheetW(w, pLabel);

    //xlsCellMerge(ws,0,0, 0, 4, g_Xf_t[NOBORDER_BOLD_20]);

	xlsWorksheetMerge(ws, 0, 0, 0, 4);
	xlsWorksheetRowheight(ws, 0, (unsigned16_t)60*20, NULL);
    pLabel = ZhcnToUincode((char *)GetLangText(12123, "人员信息表"));
    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

	for(iTmpCol = 0; iTmpCol <= 4; iTmpCol++)
	{
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}
	
	xlsWorksheetMerge(ws, 1, 0, 1, 4);	
	xlsWorksheetRowheight(ws, 1, (unsigned16_t)60*20, NULL);
	pLabel = ZhcnToUincode((char *)GetLangText(12156, "提示：1.工号最大为9位（必填）,2.姓名中文最多7个字，英文最多15个字（必填）,3.部门编号输入时填1~16（必填）,4.班次输入时有班次填1~5,无班次填0（必填）,5.卡号最大10位（选填）。"));
	xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);

	pLabel = ZhcnToUincode((char *)GetLangText(11295, "工号"));
	xlsWorksheetLabelW(ws, 2, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

    pLabel = ZhcnToUincode((char *)GetLangText(10006, "姓名"));
   	xlsWorksheetLabelW(ws, 2, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	   
	pLabel = ZhcnToUincode((char *)GetLangText(10007, "部门编号"));
	xlsWorksheetLabelW(ws, 2, 2, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	pLabel = ZhcnToUincode((char *)GetLangText(12144, "班次"));
	xlsWorksheetLabelW(ws, 2, 3, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	
	pLabel = ZhcnToUincode((char *)GetLangText(10004, "卡号"));
	xlsWorksheetLabelW(ws, 2, 4, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	/*循环写入每个人员信息*/
	Row = 3;

	while(pUser != NULL && Row < (UserNum + 3))
	{
		TRACE("pUser->userno %d pUser->sUserName %s pUser->sDepName %s pUser->schedule %d pUser->card %d %s %d\r\n", 
			(int)pUser->userno , pUser->sUserName, pUser->sDepName, pUser->schedule, (int)pUser->card, __FUNCTION__,__LINE__);

		Col = 0;

		/* 工号 */
		if(pUser->userno != 0)
		{
			xlsWorksheetNumberInt(ws, Row, Col, pUser->userno, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}
		
		Col++;
		
        /* 用户名 */
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

        /* 部门名 */
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

		/* 班次 */
		if(pUser->schedule >= 0)
		{
			xlsWorksheetNumberDbl(ws, Row, Col, pUser->schedule, g_Xf_t[BORDER_LEFT_Text_10]);
		}
		else
		{
			xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_Text_10]);
		}

		Col++;

		 /* 卡号 */
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
        
        AddRecordNum(3);    /* 更新已处理人数份额 */
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

        sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12123, "人员信息表"));

        GBKBufToUTF8Buf(caName, strlen(caName), caTmpName);

		xlsWorkbookDump(w, caTmpName);

        TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
	}

	sync();

	xlsDeleteWorkbook(w);
	
	return 0;
}

/******************************************************************************
 * 函数名称： CheckImportUserNo
 * 功能： 检查导入的工号有效性
 * 参数： pcUserNo:工号字符串类型
 * 返回： 0 表示有效 －1表示无效
 * 创建作者： Jason
 * 创建日期： 2013-5-23
 * 修改作者：
 * 修改日期：
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

	/* 判断是否有不在0－9之间的数据 */
	for (i = 0; i < (int)strlen(pcUserNo); i++)
	{
		if ((pcUserNo[i] < 0x30 || pcUserNo[i] > 0x39)
			&& pcUserNo[i] != 0x2e)
		{
			return -1;
		}
	}

	/* 工号不能为0 */
	if (i == 1 && pcUserNo[0] == 0x30)
	{
		return -1;
	}
	
	return 0;
}

/**************************************************************\
** 函数名称:ImportUserInfo
** 功能：   导入用户信息
** 参数：   pstrUserList:LPUSER_LIST_ATT型指针
** 返回：   用户信息链表头指针LPUSER_LIST_ATT
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
\**************************************************************/
int ImportUserInfo(LPUSER_LIST_ATT * pstrUserList)
{
	struct stat st;
    char acFilePath[128];
    char caName[128];
	int usernum = 0;	
	/* 链表头指针 */
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

    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12123, "人员信息表"));

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

		/* 如读取的表格的单元格填充不完整，在使用以下两个函数时会出现错误 */
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

            /* 如果该行内容不完整，则跳过该行 */
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
			
			/* 继续创建下一个节点 */
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
			
            /* 工号 */ 
			if ((&row->cells.cell[usUserIndex])->str != NULL && (&row->cells.cell[usUserIndex])->str[0] != '\0')
			{
				pUserTmp->userno = strtoul((const char *)(&row->cells.cell[usUserIndex])->str, NULL, 10);
			}
			
			usUserIndex++;
			
			/* 用户名 */
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
			
			/* 部门名 */
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
			
			/* 班次 */
			if ((&row->cells.cell[usUserIndex])->str != NULL && (&row->cells.cell[usUserIndex])->str[0] != '\0')
			{
				pUserTmp->schedule = atoi((const char *)(&row->cells.cell[usUserIndex])->str);
			}
			else
			{
				/* 未定义班次时，默认为０，无班次 */
				pUserTmp->schedule = 0;
			}
			
			TRACE("pUserTmp->schedule %d %s %d\r\n", pUserTmp->schedule, __FUNCTION__, __LINE__);
			
			usUserIndex++;

			/* 卡号 */
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

	return usernum; //返回头指针

IMPORTERR:   /* 错误处理分支 */
    
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
** 函数名称： ExportHoliday
** 功能：     导出节假日表
** 参数：     节假日链表 LP_ALL_HOLIDAY，节假日总数Hodaynum
** 返回：     Holiday.xls
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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

	/* 初始化字体 */
	initParticulXFTFont(w);

	pLabel = ZhcnToUincode((char *)GetLangText(12125, "节假信息表"));
	ws = xlsWorkbookSheetW(w, pLabel);	

	xlsWorksheetMerge(ws, 0, 0, 0, 3);
    pLabel = ZhcnToUincode((char *)GetLangText(12125, "节假信息表"));
	xlsWorksheetRowheight(ws,0,(unsigned16_t)60*20, NULL);
    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

	for(iTmpCol = 0; iTmpCol <= 3; iTmpCol++)
	{
		c = xlsWorksheetFindCell(ws, 0, iTmpCol);
		xlsCellBorderstyle(c, BORDER_BOTTOM, BORDER_DOUBLE);
	}

	xlsWorksheetMerge(ws, 1, 0, 1, 3);
	xlsWorksheetRowheight(ws, 1, (unsigned16_t)60*20, NULL);
	pLabel = ZhcnToUincode((char *)GetLangText(12161, "提示： 1、节假日中文最多7个字或英文最多15个字（必填） 2、最多24个节假日 3、时间输入时格式为YYYY-MM-DD"));
    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);
	
	//c = xlsWorksheetFindCell(ws, 1, 3);
	//xlsCellBorderstyle(c, BORDER_RIGHT, BORDER_THIN);
		
	xlsWorksheetLabel(ws, 2, 0, "ID", g_Xf_t[BORDER_LEFT_FILL_10]);
	
	pLabel = ZhcnToUincode((char *)GetLangText(12162, "节假日名称"));
	xlsWorksheetLabelW(ws, 2, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	
    pLabel = ZhcnToUincode((char *)GetLangText(12163, "开始日期"));
   	xlsWorksheetLabelW(ws, 2, 2, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	

	pLabel = ZhcnToUincode((char *)GetLangText(12164, "结束日期"));
   	xlsWorksheetLabelW(ws, 2, 3, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	/* 循环写入每个节假日 */	
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

    	sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12125, "节假信息表"));

        GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);

		xlsWorkbookDump(w, acFilePath);
	}
	
	sync();
	
	xlsDeleteWorkbook(w);

	return 0;
}

/**************************************************************\
** 函数名称:ImportHoday
** 功能：   导入节假日
** 参数：   pHoliday :节假日数组
** 返回：   -1:读取xls文件失败; >=0:节假日数目
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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

    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12125, "节假信息表"));

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
		/* 如读取的表格的单元格填充不完整，在使用以下两个函数时会出现错误 */
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

		/* 分配空间 才可以存入内容 */
        pHoliday = (LP_ALL_HOLIDAY)Malloc(sizeof(ALL_HOLIDAY));
        if (pHoliday == NULL)
        {	
			TRACE("pHoliday Malloc fail! %s %d\r\n", __FUNCTION__, __LINE__);

			goto IMPORTERR;
		}
        
		memset(pHoliday, 0, sizeof(ALL_HOLIDAY));

        iHoliday = 0;

        TRACE("pWS->rows.lastrow %u %s %d\r\n", pWS->rows.lastrow, __FUNCTION__, __LINE__);
        
		/* 循环读出用户信息并填入结构体数组 */			
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

            /* 如果该行内容不完整，则跳过该行 */
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

			/* 有数据 读入 */
			if((&row->cells.cell[tt])->str != NULL 
				&& (&row->cells.cell[tt])->str[0] != '\0')
			{				
	            tt++;

				/* 保存已读取的该条数据 */
				/* 节假日名称 */				
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
				/* 开始时间 */
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					strncpy(pHoliday->HoliDay[iHoliday].szStartTime, (char *)(&row->cells.cell[tt])->str, 31);
                    pHoliday->HoliDay[iHoliday].szStartTime[31] = 0;
				}
				
				tt++;
				/* 结束时间 */
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
** 函数名称： ExportSchedules
** 功能：     生成排班表
** 参数：     pshedule:班次数组
** 返回：     排班表ScheduleTable.xls
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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

	/* 初始化字体 */
	initParticulXFTFont(w);

    pLabel = ZhcnToUincode((char *)GetLangText(12124, "排班信息表"));
    
	ws = xlsWorkbookSheetW(w, pLabel);

    ws->defaultColwidth(8); /* 设置默认宽度，必须设置为8，xlsWorksheetColwidth才会起作用 */
    for (Col = 0; Col < 10; Col++)
    {
        xlsWorksheetColwidth(ws, Col, aiColWidth[Col], NULL);
    }       

	xlsWorksheetMerge(ws, 0, 0, 0, 9);
	xlsWorksheetRowheight(ws,0,(unsigned16_t)60*20, NULL);

    pLabel = ZhcnToUincode((char *)GetLangText(12124, "排班信息表"));
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

    pLabel = ZhcnToUincode((char *)GetLangText(12157, "说明：红色部分为可编辑区域；类型（0-正常考勤; 1-加班）时间格式:HH:MM 班次名最长7个汉字, 跨日设置:在跨天时间之前的打卡算为前一天打卡"));
    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);

    /* 循环写入每个班次信息*/			
	for (RowSchedule = 1; RowSchedule <= 55; RowSchedule += 11)		
	{	
		/* 班次  */
		iScheduleNo++;

		/* 当前行 */
		Row = RowSchedule + 1;
		     	
		/* 写入表头 */
		sprintf(tmp,"%s%d",(char *)GetLangText(12158, "序号："), iScheduleNo);
		pLabel = ZhcnToUincode(tmp);
		xlsWorksheetLabelW(ws, Row, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(12159, "名称"));
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
					strcpy(tmp,(char *)GetLangText(11922, "班次一"));
					
					break;
				}
				case 2:
				{
					strcpy(tmp,(char *)GetLangText(11923, "班次二"));
					
					break;
				}
				case 3:
				{
					strcpy(tmp,(char *)GetLangText(11924, "班次三"));
					
					break;	
				}
				case 4:
				{
					strcpy(tmp,(char *)GetLangText(11925, "班次四"));
					
					break;
				}
				case 5:
				{
					strcpy(tmp,(char *)GetLangText(11926, "班次五"));
					
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
		pLabel = ZhcnToUincode((char *)GetLangText(12160, "跨日设置："));
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
		pLabel = ZhcnToUincode((char *)GetLangText(11928, "班段一"));
		xlsWorksheetLabelW(ws, Row, 1, pLabel, g_Xf_t[BORDER_LEFT_10]);

		xlsCellMerge(ws, Row, 4, Row, 6, g_Xf_t[BORDER_LEFT_10]);
		pLabel = ZhcnToUincode((char *)GetLangText(11929, "班段二"));
		xlsWorksheetLabelW(ws, Row, 4, pLabel, g_Xf_t[BORDER_LEFT_10]);

		xlsCellMerge(ws, Row, 7, Row, 9, g_Xf_t[BORDER_LEFT_10]);
		pLabel = ZhcnToUincode((char *)GetLangText(11930, "班段三"));
		xlsWorksheetLabelW(ws, Row, 7, pLabel, g_Xf_t[BORDER_LEFT_10]);

		Row++;
		
		xlsWorksheetLabel(ws, Row, 0, "",  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(11403, "上班"));
		xlsWorksheetLabelW(ws, Row, 1, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(11404, "下班"));
		xlsWorksheetLabelW(ws, Row, 2, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(10002, "类型"));
		xlsWorksheetLabelW(ws, Row, 3, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		
		pLabel = ZhcnToUincode((char *)GetLangText(11403, "上班"));
		xlsWorksheetLabelW(ws, Row, 4, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(11404, "下班"));
		xlsWorksheetLabelW(ws, Row, 5, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(10002, "类型"));
		xlsWorksheetLabelW(ws, Row, 6, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(11403, "上班"));
		xlsWorksheetLabelW(ws, Row, 7, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(11404, "下班"));
		xlsWorksheetLabelW(ws, Row, 8, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		pLabel = ZhcnToUincode((char *)GetLangText(10002, "类型"));
		xlsWorksheetLabelW(ws, Row, 9, pLabel,  g_Xf_t[BORDER_LEFT_10]);

		Row++;

		tmpRow = Row;
		pLabel = ZhcnToUincode((char *)GetLangText(10046, "周一"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10047, "周二"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10048, "周三"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10049, "周四"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10050, "周五"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10051, "周六"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
		tmpRow++;
		
		pLabel = ZhcnToUincode((char *)GetLangText(10045, "周日"));
		xlsWorksheetLabelW(ws, tmpRow, 0, pLabel,  g_Xf_t[BORDER_LEFT_10]);
				
		iDutytime = 0;
		
        /* 保存一个班次的七个工作日考勤时间 */
		for(tmpRow = Row; tmpRow < Row + 7; tmpRow++)
		{	
			Col = 1; 

			/* 循环保存每个班次的三个班段 */
			for(iInterval = 0; iInterval < 3; iInterval++)
			{
				/* 类型 -1非法*/
				iType = pschedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType;

				/* 上班 */
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

				/* 下班 */
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
				
				/* 类型 */	
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

		    /* 下一个工作日 */
		    iDutytime++;
		}
		
		iTmpSchedule++;
		Row = Row + 7;
	}

	if (GetUsbStatus() == TRUE)
	{
        memset(acFilePath, 0, sizeof(acFilePath));
        memset(caName, 0, sizeof(caName));

    	sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12124, "排班信息表"));

        GBKBufToUTF8Buf(caName, strlen(caName), acFilePath);
      
		xlsWorkbookDump(w, acFilePath);	
	}

	sync();

	xlsDeleteWorkbook(w);
	
	return 0;
}	

/**************************************************************\
** 函数名称:ImportSchedules
** 功能：   导入排班表
** 参数：   pShedule :班次数组
** 返回：   -1:读取xls文件失败; >=0:节假日数目
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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
	/* 行循环序号 */
	WORD usRowIndex;
	/* cell单元格下标，即列序号 */
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
    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12124, "排班信息表"));

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
		/* 如读取的表格的单元格填充不完整，在使用以下两个函数时会出现错误 */
		pWS = xls_getWorkSheet(pWB, 0);	
        if (pWS == NULL)
        {
    		TRACE("xls_getWorkSheet xls failed! %s %d\r\n", __FUNCTION__, __LINE__);
    		
    		goto IMPORTERR;
	    }
		
		xls_parseWorkSheet(pWS);

		/* 循环读出班次信息并填入结构体数组 */			
		for (usRowIndex = 7; usRowIndex <= pWS->rows.lastrow; usRowIndex += 11)		
		{	
			iDutytime = 0;

			iSchedule++;

			/* 跨天时间所在行 */
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

				sprintf(caScheduleName, "%s", GetLangText(iTmpSchedule+11922,(char *)"班次一"));

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

			/* 每个班次的七个工作日 */	
			for (iTmpRow = usRowIndex; iTmpRow < (usRowIndex + 7); iTmpRow++)
			{				
				/* 当前行 */
				row = &pWS->rows.row[iTmpRow];

				/* 因为第1列为星期几列，不需要读 */
				usColIndex = 1;

				regDayFlag = 0;

				/* 循环保存每个班次的三个班段 */
				for (iInterval = 0; iInterval < 3; iInterval++)
				{
					/* 上班标志  */
					iOnFlag = 0;
					/* 下班标志  */
					iOffFlag = 0;								

					/* 上班必填 */
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

					/* 下班 */
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

					/* 只填写上班或者下班 说明表数据非法 返回 */
					if ((iOnFlag == 1 || iOffFlag == 1) 
						&& (iOnFlag == 0 || iOffFlag == 0))
					{
                		TRACE("iOnFlag %d iOffFlag %d %s %d\r\n", iOnFlag, iOffFlag, __FUNCTION__, __LINE__);

                        goto IMPORTERR;
					}

					/* 上下班都不填入 type = -1 */	
					if(iOnFlag == 1 
						&& iOffFlag == 1)
					{
						pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iType = -1;

						usColIndex++;

						regDayFlag += 1;

						continue;
					}

					/* 上下班都填入时 读取类型 判是否合法 */
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

							/* 数据非法 */
                            goto IMPORTERR;
						}

						usColIndex++;
					}					

					/* 班段 */
					pShedule[iTmpSchedule].ScheduleTime[iDutytime].DayTime[iInterval].iInterval = iInterval + 1;
				}				

				/* 如果regDayFlag为3，则表明本班次的本周几的考勤不设置 */
				if(3 == regDayFlag)
				{
					pShedule[iTmpSchedule].ScheduleTime[iDutytime].iDay = 0;
					regNoFlag += 1;
				}
				else
				{
					pShedule[iTmpSchedule].ScheduleTime[iDutytime].iDay = iDutytime + 1;
				}

				/* 下一个工作日 */
				iDutytime++;								
			}

			/* 如果regNoFlag为7，则表明该本班次考勤不设置 */
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
** 函数名称:ExportsOriginalRecord
** 功能：   每次写入一个用户原始记录并生成原始记录表
** 参数：   pOriginalRecord:原始记录指针
            StartTime:开始时间
            EndTime:结束时间
            iUser:要写入第几个用户
** 返回：   原始记录表OriginalTable.xls
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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

		/* 初始化字体 */
		initParticulXFTFont(w);

        pLabel = ZhcnToUincode((char *)GetLangText(11941, "考勤卡式报表"));
      
		ws = xlsWorkbookSheetW(w, pLabel);

        ws->defaultColwidth(8); /* 设置默认宽度，必须设置为8，xlsWorksheetColwidth才会起作用 */
        for (Col = 0; Col < iColFlag; Col++)
        {
			if(Col == 0)
			{
				xlsWorksheetColwidth(ws, Col, (16*32), NULL);
			}
			else
			{
				/* 32一个像素,列宽37像素 */
            	xlsWorksheetColwidth(ws, Col, 37*32, NULL);
			}
        }

		sprintf(buf,"%s", (char *)GetLangText(11941, "卡式报表"));

		/* 查询的天数合并 写入表头 */
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
		sprintf(buf,"%s:%04d-%02d-%02d %s %04d-%02d-%02d", (char *)GetLangText(10013, "日期"), timeRange.iStartYear, timeRange.iStartMonth,
			timeRange.iStartDay, (char *)GetLangText(12140, "到"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
		pLabel = ZhcnToUincode(buf);
	    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);
	}
	
    Row = 2+iRowFlag;
    Col = 0;

	/* 填写表头信息 */
	pLabel = ZhcnToUincode((char *)GetLangText(12182, "员工"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

	sprintf(buf, "%s:%d  %s:%s ", (char *)GetLangText(11295, "工号"), (int)OriginalRecord.UserNo,
										(char *)GetLangText(10006, "姓名"), OriginalRecord.szUserName);
	Col++;
	xlsCellMerge(ws,Row,Col, Row, iColFlag-1, g_Xf_t[BORDER_LEFT_FILL_10]);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(ws, Row, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

    Row++;
    Col = 0;
	pLabel = ZhcnToUincode((char *)GetLangText(10013, "日期"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

    Row++;
    Col = 0;
	pLabel = ZhcnToUincode((char *)GetLangText(12183, "考勤"));
	xlsWorksheetLabelW(ws, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]); 

	if(OriginalRecord.iDayNum > 16)
	{
        Row++;
		pLabel = ZhcnToUincode((char *)GetLangText(10013, "日期"));
		xlsWorksheetLabelW(ws,  Row, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

        Row++;
		pLabel = ZhcnToUincode((char *)GetLangText(12183, "考勤"));
		xlsWorksheetLabelW(ws,  Row, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);
	}

	for(iTmpUser = 0; iTmpUser < OriginalRecord.iDayNum; iTmpUser++)
	{
		/* 开始写入的行号 */
		Row = 3 + iRowFlag + 2*(iTmpUser/16);

        Col = iTmpUser%16 + 1;

		/* 考勤日期 */
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


		/* 导出该用户该天的打开记录 */			
		/* 有打卡记录 */
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

        sprintf(tmpbuf,"/mnt/usb/%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11941, "卡式报表"));
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
** 函数名称:ExportMonthParticularRecord
** 功能：   导出明细表与月报表合并
** 参数：   pParticularRecord:LP_PARTICULAR_RECORD型指针
            StartTime:开始时间
            EndTime:结束时间
            daynum:天数
            iUser:第几个用户
            iflag:写入标志 
            0,第一个用户但不是最后一个用户
            1,不是第一个用户也不是最后一个用户
            2,不是第一个用户但是最后一个用户
            3,既是第一个用户也是最后一个用户
            
** 返回：   
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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
		/* 初始化明细表单元格格式 */
		initParticulXFTFont(w);

        pLabel = ZhcnToUincode((char *)GetLangText(12143, "月统计表"));

		/* 先为月统计表预留 */
		ws = xlsWorkbookSheetW(w, pLabel);
        /* 写入月报表表头 */
		MonthRecordHadInfo(ws,timeRange);
	}

	sprintf(buf, "%d%s",iUser, pParticularRecord->szUserName);
	pLabel = ZhcnToUincode(buf);

	/* 一个用户一个sheet */
	wsRecord = xlsWorkbookSheetW(w, pLabel);
	/* 设置默认宽度，必须设置为8，xlsWorksheetColwidth才会起作用 */
    wsRecord->defaultColwidth(8); 
    for (Col = 0; Col < XLS_ORI_RECORD_TAB_COLS; Col++)
    {
		if((Col == 1)||(Col == 9))
		{
			xlsWorksheetColwidth(wsRecord, Col, 33*32, NULL);
		}
		else
		{
			/* 32一个像素,列宽39像素 */
        	xlsWorksheetColwidth(wsRecord, Col, 39*32, NULL);
		}
    }

    /* 写入用户明细表表头 */
	ParticulUserHaedInfo(wsRecord, pParticularRecord->UserNo, pParticularRecord->szUserName, pParticularRecord->iSchedule, 
						timeRange );

	/* totalRow 控制写入的行数 */
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
	/* 按行依次写入每一列 */
    for (iLoop = 0; iLoop < iRowNum; iLoop++)
    {
        /* 写左侧信息 */
        /* 设置该row的所有col */
		Col = 0;
		Row ++;
        sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop].szData);    /* 日期 */
		xlsWorksheetLabel(wsRecord, Row, Col, buf, g_Xf_t[BORDER_9]);

		Col++;
        sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop].weekDay);   /* 星期 */
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
			sprintf(buf,"%s", (char *)GetLangText(12017, "休息"));   /* 星期 */
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);

			Col+=6;
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == BUSINESS_OF_TRIP)
        {
			sprintf(buf,"%s", (char *)GetLangText(12018, "出差"));
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);

			Col+=6;
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == LEAVE_DAY)
        {
			sprintf(buf,"%s", (char *)GetLangText(12016, "请假")); 
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);

			Col+=6;
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == ABSENCE_FROM_DUTY)   /* 缺勤 */
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
                /* 上班 */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
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
                /* 下班 */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
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

        if ((iLoop + iRowNum) >= iDaynum)  /* 右侧信息已经写完 */
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

        /* 写右侧信息 */
		sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop + iRowNum].szData);    /* 日期 */
		xlsWorksheetLabel(wsRecord, Row, Col, buf, g_Xf_t[BORDER_9]);

		Col++;
        sprintf(buf,"%s", pParticularRecord->DayAttendanceTime[iLoop + iRowNum].weekDay);   /* 星期 */
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
			sprintf(buf,"%s", (char *)GetLangText(12017, "休息"));   /* 星期 */
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]); 
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == BUSINESS_OF_TRIP)
        {
			sprintf(buf,"%s", (char *)GetLangText(12018, "出差"));
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]); 
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == LEAVE_DAY)
        {
			sprintf(buf,"%s", (char *)GetLangText(12016, "请假")); 
			pLabel = ZhcnToUincode(buf);
			xlsCellMerge(wsRecord, Row, Col, Row, Col+5, g_Xf_t[BORDER_RED_LEFT_9]);
			xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_RED_LEFT_9]);
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == ABSENCE_FROM_DUTY)   /* 缺勤 */
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
                /* 上班 */
				
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
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
                /* 下班 */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
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
        (char *)GetLangText(12145, "工作日数"), pParticularRecord->fWorkdayNo, 
        (char *)GetLangText(12146, "出勤天数"), pParticularRecord->fAttendanceNo,
        (char *)GetLangText(12147, "缺勤天数"), pParticularRecord->fAbsenceNo, 
        (char *)GetLangText(12149, "迟到次数"), (char *)GetLangText(11444, "分钟"), 
        pParticularRecord->iDelayTimes, pParticularRecord->iDelayMinute, 
        (char *)GetLangText(12151, "早退次数"), (char *)GetLangText(11444, "分钟"), 
        pParticularRecord->iLeaveEarlyTimes, pParticularRecord->iLeaveEarlyMinute, 
        (char *)GetLangText(12165, "加班时数"), pParticularRecord->fOvertimeHours, 
        (char *)GetLangText(12153, "请假天数"), pParticularRecord->fLeaveDays, 
        (char *)GetLangText(12154, "出差天数"), pParticularRecord->fTrips);

	xlsCellMerge(wsRecord, Row, Col, Row, Col+15, g_Xf_t[BORDER_LEFT_10]);
	xlsWorksheetRowheight(wsRecord, Row, (unsigned16_t)25*20, NULL);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);

	Row++;
	Col = 0;
	memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s：    %s:     %s：    %s：     %s：    %s：     %s：    ",
		(char *)GetLangText(12166, "病假时数"),
		(char *)GetLangText(12167, "事假时数"),
		(char *)GetLangText(12168, "日薪工资"),
		(char *)GetLangText(12169, "加班工资"),
		(char *)GetLangText(12170, "其它津贴"),
		(char *)GetLangText(12171, "其他扣款"),
		(char *)GetLangText(12172, "实得工资"));

	xlsCellMerge(wsRecord, Row, Col, Row, Col+15, g_Xf_t[BORDER_LEFT_10]);
	xlsWorksheetRowheight(wsRecord, Row, (unsigned16_t)25*20, NULL);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);

	Row++;
	Col = 0;
	memset(buf, 0, sizeof(buf));
 	sprintf(buf, "%s:                           %s:",
		 (char *)GetLangText(12173, "确认"), (char *)GetLangText(12174, "审核"));

	xlsCellMerge(wsRecord, Row, Col, Row, Col+15, g_Xf_t[BORDER_LEFT_10]);
	xlsWorksheetRowheight(wsRecord, Row, (unsigned16_t)25*20, NULL);
	pLabel = ZhcnToUincode(buf);
	xlsWorksheetLabelW(wsRecord, Row, Col, pLabel, g_Xf_t[BORDER_LEFT_10]);

    TRACE(" %s %d\r\n", __FUNCTION__, __LINE__);
    
	/* 写月报表记录 */
	Row = 3+iUser-1;
	Col = 0;
	/* 工号 */
	xlsWorksheetNumberInt(ws, Row, Col, pMonthRecord->nUserNo,  g_Xf_t[BORDER_LEFT_10]);

	Col++;
	
	memset(tmpbuf, 0, sizeof(tmpbuf));
	memset(buf, 0, sizeof(tmpbuf));

    //xlsWorksheetLabel(ws, Row, Col, "",g_Xf_t[BORDER_10]);

	sprintf(tmpbuf,"%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11942, "考勤明细表"));
	sprintf(buf, "[%s]%d%s!A1", tmpbuf, iUser,pMonthRecord->szUserName);

	pLabel = ZhcnToUincode(buf);
	memcpy(LabelVal ,pLabel, 1024);

	pLabel = ZhcnToUincode(pMonthRecord->szUserName);    

	expression_node_factory_t& maker = w->GetFormulaFactory();

	expression_node_t *binary_root = maker.f(FUNC_HYPERLINK, maker.text(LabelVal), maker.text(pLabel));
    
	c = ws->formula(Row, Col, binary_root, true);
	xlsCellSetXF(c, g_Xf_t[BORDER_LEFT_10]);

    /* 暂不支持部门，不填写内容 */
    Col++;
    xlsWorksheetLabel(ws, Row, Col, "",g_Xf_t[BORDER_LEFT_10]);
	
	Col++;
	if((pMonthRecord->iSchedule > 0)&&(pMonthRecord->iSchedule < 6))
	{
		strcpy(buf,AttendanceParam.Schedules[pMonthRecord->iSchedule - 1].caScheduleName);
	}
	else
	{
		strcpy(buf,(char *)GetLangText(12155, "无"));
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

	if(pMonthRecord->fLeaveDays)    /* 请假天数 */
	{
		sprintf(buf, "%.1f", pMonthRecord->fLeaveDays);
		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
	}
	else
	{
		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
	}

	Col++;

	if(pMonthRecord->fTrips)    /* 出差天数 */
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

		sprintf(tmpbuf,"/mnt/usb/%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11942, "考勤明细表"));

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
** 函数名称:ExportParticularRecord
** 功能：   导出明细表
** 参数：   pParticularRecord:LP_PARTICULAR_RECORD型指针
            StartTime:开始时间
            EndTime:结束时间
            daynum:天数
            iUser:第几个用户
            iflag:写入标志 
            0,第一个用户但不是最后一个用户
            1,不是第一个用户也不是最后一个用户
            2,不是第一个用户但是最后一个用户
            3,既是第一个用户也是最后一个用户
            
** 返回：   
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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

    	//sprintf(acTmpName, "/mnt/usb/%s-%s-%s.xls", (char*)GetLangText(11942, "考勤明细表"),
    	//	pParticularRecord->szStartTime, pParticularRecord->szEndTime);

		//sprintf(acTmpName,"/mnt/usb/%03d_%d_%d_%s.xls",strConfigBase.machineID,
	    //pTm->tm_year + 1900, pTm->tm_mon + 1, (char*)GetLangText(11942, "考勤明细表"));
		sprintf(acTmpName,"/mnt/usb/%d_%d_%s.xls",timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11942, "考勤明细表"));
        //GB2312ToUTF_8(acName, acTmpName, strlen(acTmpName));
	    GBKBufToUTF8Buf(acTmpName, strlen(acTmpName), acName);

        unlink(acName);

        /* 创建文档 */
        create_file(acName);
	}

    /* 判断是否需要创建sheet */
    if ((iUser - 1) % 1 == 0)
    {
        /* 若不是第一个sheet，则关闭上一个sheet */
        if (iUser != 1)
        {
            end_sheet();
        }

        memset(acPara, 0, sizeof(acPara));
        sprintf(acPara, "%s", pParticularRecord->szUserName);
        
        /* 创建sheet */
        start_new_sheet(acPara);

		fill_title_info(pParticularRecord->szStartTime, pParticularRecord->szEndTime);

		fill_company_attendaceinfo(timeRange);
    }
	else
	{
		start_new_row();
	}

     /* totalRow 控制写入的行数 */
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
    /* 增加工号信息 */
    fill_user_headinfo((int)pParticularRecord->UserNo, pParticularRecord->szUserName, 
                        pParticularRecord->szDepName, pParticularRecord->iSchedule, 
                        pParticularRecord->szStartTime, pParticularRecord->szEndTime);

    /* 增加通用表头信息 */
    fill_user_table_headinfo(); 

    /* 按行依次写入每一列 */
    for (iLoop = 0; iLoop < iRowNum; iLoop++)
    {
        /* 创建row并设置该行属性 */
        add_normal_attribute_of_row();

        /* 写左侧信息 */
        /* 设置该row的所有col */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].szData);    /* 日期 */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].weekDay);   /* 星期 */

        if ((pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == REST_DAY)
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[0].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[0].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[1].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[1].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[2].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[2].offduty[0] == '\0'))
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12017, "休息"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == BUSINESS_OF_TRIP)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12018, "出差"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == LEAVE_DAY)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12016, "请假"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop].attendanceNo == ABSENCE_FROM_DUTY)   /* 缺勤 */
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
                /* 上班 */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
                if (iStatus)
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_RED_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }
                else
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }

                /* 下班 */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
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

        if ((iLoop + iRowNum) >= iDaynum)  /* 右侧信息已经写完 */
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

        /* 写右侧信息 */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].szData);    /* 日期 */
        fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].weekDay);   /* 星期 */

        if ((pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == REST_DAY)
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[0].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[0].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[1].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[1].offduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[2].onduty[0] == '\0')
            && (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[2].offduty[0] == '\0'))
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12017, "休息"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == BUSINESS_OF_TRIP)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12018, "出差"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == LEAVE_DAY)
        {
            fill_normal_cell(MERGE_FILL_CELL, NORMAL_MERGE_CELLS, MERGE_RED_STYLEID, (char *)GetLangText(12016, "请假"));
        }
        else if (pParticularRecord->DayAttendanceTime[iLoop + iRowNum].attendanceNo == ABSENCE_FROM_DUTY)   /* 缺勤 */
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
                /* 上班 */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> (2 * iSchedule)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
                if (iStatus)
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_RED_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }
                else
                {
                    fill_normal_cell(NORMAL_FILL_CELL, 0, NORMAL_CELL_BLACK_STYLEID, pParticularRecord->DayAttendanceTime[iLoop + iRowNum].StrudayAttndnctime.daytime[iSchedule].onduty); 
                }

                /* 下班 */
                iStatus = pParticularRecord->DayAttendanceTime[iLoop + iRowNum].iAttendanceStatus;
                iStatus = (iStatus >> ((2 * iSchedule) + 1)) & 1; /* 查询该班次考勤状态，若迟到，则标红 */
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
** 函数名称:ExportMonthRecordTable
** 功能：   生成月报表
** 参数：   pMonthRecord:MONTHFORM指针
            usernum:用户数
** 返回：   
** 创建作者：
** 创建日期：
** 修改作者：
** 修改日期：
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

    /* 初始化字体 */
	initParticulXFTFont(w);

    pLabel = ZhcnToUincode((char *)GetLangText(12143, "月统计表"));
	ws = xlsWorkbookSheetW(w, pLabel);

    /* 写入月报表表头 */
	MonthRecordHadInfo(ws,timeRange);
	
    /* 开始写入用户月记录 */
	Row = 3;

	while(pMonthRecord != NULL && Row < usernum + 3)
	{
		memset(buf, 0, sizeof(buf));
		
		Col = 0; 
		/* 工号 */
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

        /* 暂不支持部门，不填写内容 */
        Col++;
        xlsWorksheetLabel(ws, Row, Col, "",g_Xf_t[BORDER_LEFT_10]);

        Col++;
    	if((pMonthRecord->iSchedule > 0)&&(pMonthRecord->iSchedule < 6))
    	{
    		strcpy(buf,attendanceParm.Schedules[pMonthRecord->iSchedule - 1].caScheduleName);
    	}
    	else
    	{
    		strcpy(buf,(char *)GetLangText(12155, "无"));
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

    	if(pMonthRecord->fLeaveDays)    /* 请假天数 */
    	{
    		sprintf(buf, "%.1f", pMonthRecord->fLeaveDays);
    		xlsWorksheetLabel(ws, Row, Col, buf, g_Xf_t[BORDER_LEFT_10]);
    	}
    	else
    	{
    		xlsWorksheetLabel(ws, Row, Col, "", g_Xf_t[BORDER_LEFT_10]);
    	}

    	Col++;

    	if(pMonthRecord->fTrips)    /* 出差天数 */
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
	
    //sprintf(buf, "/mnt/usb/%s-%s %s-%s.xls", (char*)GetLangText(11943, "月统计报表"),
	//	GetSerailStr(),StartTime,EndTime);

	sprintf(buf,"/mnt/usb/%d_%d_%s.xls",pTm->tm_year + 1900, pTm->tm_mon + 1, (char*)GetLangText(11943, "月统计报表"));

	//GB2312ToUTF_8(acName, buf, strlen(buf));
	GBKBufToUTF8Buf(buf, strlen(buf), acName);

    unlink(acName);    /* 删除原有文件 */
	if (GetUsbStatus() == TRUE)
	{
    	memset(buf, 0, sizeof(buf));
        memset(acName, 0, sizeof(acName));
        
	    //sprintf(buf, "/mnt/usb/%s-%s %s-%s.xls", (char*)GetLangText(11943, "月统计报表"),
		//	GetSerailStr(),StartTime,EndTime);

		//sprintf(buf,"/mnt/usb/%03d_%d_%d_%s.xls",strConfigBase.machineID,
	    //pTm->tm_year + 1900, pTm->tm_mon + 1, (char*)GetLangText(11943, "月统计报表"));
		sprintf(buf,"/mnt/usb/%d_%d_%s.xls", timeRange.iStartYear, timeRange.iStartMonth, (char*)GetLangText(11943, "月统计报表"));
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
** 函数名称： ExportSchedules
** 功能：     导入请假出差表
** 参数：     pUserLeaveTripInfo:用户请假出差信息
** 返回：     排班表4-LeaveTrip.xls
** 创建作者：
** 创建日期：2013-4-13
** 修改作者：胡宗华
** 修改日期：
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
    sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12126, "请假出差表"));

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
		/* 如读取的表格的单元格填充不完整，在使用以下两个函数时会出现错误 */
		pWS = xls_getWorkSheet(pWB, 0);	
		
		xls_parseWorkSheet(pWS);

		if(pWS->rows.lastrow < 4)		
		{	
			TRACE("The xls is NULL! %s %d\r\n", __FUNCTION__, __LINE__);

            goto IMPORTERR;
		}

		/* 开始日期 */
		row = &pWS->rows.row[2];
		if((char *)(&row->cells.cell[0])->str != NULL)
		{
			//strncpy(szStartTime, (char *)(&row->cells.cell[3])->str, 15);
			rightStr(szStartTime, (char *)(&row->cells.cell[0])->str, 10);
		}

		//printf("szStartTime %s %s %d\r\n", szStartTime, __FUNCTION__, __LINE__);
		
		/* 循环读出用户信息并填入结构体数组，从第四行开始读 */			
		for (t = 4; t <= pWS->rows.lastrow; t++)		
		{
			/* 分配空间 可以存入内容 */
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

			/* 有数据 读入 */
			if((&row->cells.cell[tt])->str != NULL 
				&& (&row->cells.cell[tt])->str[0] != '\0')
			{				
				/* 工号 */
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					pUserLeaveTrip->UserNo = atoi((char *)(&row->cells.cell[tt])->str);
				}

                tt++;
                /* 姓名 */
				memset(pUserLeaveTrip->szUserName, 0 , 32);
				
				if ((&row->cells.cell[tt])->str != NULL && (&row->cells.cell[tt])->str[0] != '\0')
				{
					strncpy(pUserLeaveTrip->szUserName, (char *)(&row->cells.cell[tt])->str, 31);
				}
				
				tt++;
                memset(pUserLeaveTrip->szDepName, 0 , 32);
				/* 部门名称 */
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
						/* 如果未填则默认为正常考勤 */
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

	/* 判断时间参数 */
    memset(&strTime, 0, sizeof(tm));
    strTime.tm_year = year -  1900;
    strTime.tm_mon  = month - 1;
    strTime.tm_mday = day;    
    strTime.tm_hour = 0;
    strTime.tm_min = 0;
    strTime.tm_sec = 0;

    tRefTime = mktime(&strTime);    /* 获取当天零时零分的时间秒数数据 */

	TRACE("year %d %d %d %s %d\r\n",strTime.tm_year, strTime.tm_mon, strTime.tm_mday,  __FUNCTION__, __LINE__);

	tRefTime += sec;
	
    gmtime_r(&tRefTime, &strTime);

	TRACE("year %d %d %d %s %d\r\n",strTime.tm_year, strTime.tm_mon, strTime.tm_mday,  __FUNCTION__, __LINE__);
	
	//sprintf(caDate, "%02d-%02d", strTime.tm_mon + 1, strTime.tm_mday);
	sprintf(caDate, "%02d", strTime.tm_mday);

	return 0;
}

/**************************************************************\
** 函数名称： ExportUserLeaveTrip
** 功能：     生成请假出差表
** 参数：     pUserLeaveTrip:用户请假出差信息
** 返回：     排班表4-LeaveTrip.xls
** 创建作者：
** 创建日期：2013-4-13
** 修改作者：胡宗华
** 修改日期：
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

    /* 初始化字体 */
	initParticulXFTFont(w);

	pLabel = ZhcnToUincode((char*)GetLangText(12126, "请假出差表"));
	ws = xlsWorkbookSheetW(w, pLabel);	

	xlsWorksheetMerge(ws, 0, 0, 0, 33);
	
    pLabel = ZhcnToUincode((char*)GetLangText(12126, "请假出差表"));
    xlsWorksheetRowheight(ws,0,(unsigned16_t)60*20, NULL);
    xlsWorksheetLabelW(ws, 0, 0, pLabel, g_Xf_t[NOBORDER_BOLD_20]);

    ws->defaultColwidth(8); /* 设置默认宽度，必须设置为8，xlsWorksheetColwidth才会起作用 */
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
    pLabel = ZhcnToUincode((char *)GetLangText(12176, "用户请假出差信息:1-正常考勤 2-出差 3-请假"));
    xlsWorksheetRowheight(ws, 1, (unsigned16_t)30*20, NULL);
    xlsWorksheetLabelW(ws, 1, 0, pLabel, g_Xf_t[NOBORDER_LEFT_10]);

	//xlsWorksheetMerge(ws, 2, 0, 2, 2);
	xlsCellMerge(ws,2,0, 2, 2, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
	sprintf(aTmpBuf, "%s: %s", (char *)GetLangText(12163, "开始日期"), pUserLeaveTripTmp->szStartTime);
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

	sprintf(aTmpBuf, "%02d%s", iMonth, (char *)GetLangText(11310, "月"));
    xlsCellMerge(ws,2, 3, 2, 3 + iMonthDayNum - 1, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
	pLabel = ZhcnToUincode(aTmpBuf);
	xlsWorksheetLabelW(ws, 2, 3, pLabel, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);

	if (iLeftDayNum > 0)
	{
		sprintf(aTmpBuf, "%02d%s", iMonth + 1, (char *)GetLangText(11310, "月"));
        xlsCellMerge(ws, 2, 3 + iMonthDayNum, 2, 33, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
		pLabel = ZhcnToUincode(aTmpBuf);
		xlsWorksheetLabelW(ws, 2, 3 + iMonthDayNum, pLabel, g_Xf_t[BORDER_LEFT_FILL_NOTRAP_10]);
	}

	pLabel = ZhcnToUincode((char *)GetLangText(11295, "工号"));
	xlsWorksheetLabelW(ws, 3, 0, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

    pLabel = ZhcnToUincode((char *)GetLangText(10006, "姓名"));
   	xlsWorksheetLabelW(ws, 3, 1, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	pLabel = ZhcnToUincode((char *)GetLangText(10012, "部门"));
   	xlsWorksheetLabelW(ws, 3, 2, pLabel, g_Xf_t[BORDER_LEFT_FILL_10]);

	/* 31天日期 */
	for( i=0; i<31; i++ )
	{
		GetDayNum(pUserLeaveTripTmp->szStartTime, i, caDate);
		
		xlsWorksheetLabel(ws, 3, i+3, caDate, g_Xf_t[BORDER_LEFT_Text_10]);
	}
	
	Row = 4;
	while(NULL != pUserLeaveTripTmp)
	{
		Col = 0;

		/* 工号 */
		xlsWorksheetNumberInt(ws, Row, Col, pUserLeaveTripTmp->UserNo, g_Xf_t[BORDER_LEFT_Text_10]); 

		Col++;
		/* 用户名 */
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
    	sprintf(caName, "/mnt/usb/%s.xls", (char*)GetLangText(12126, "请假出差表"));

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

        if (stat(XML_HEAD_TEXT_FILE, &st) == 0) /* 文件不存在，直接返回 */
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
    if (stat(XML_END_SHEET_FILE, &st) == 0) /* 文件不存在，直接返回 */
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

    /* 班次名称 */
    pcTmp = GetScheduleNameByNo(isch);
    if (pcTmp)
    {
        memcpy(tmpbuf, pcTmp, 32);
    }
    else
    {
		sprintf(tmpbuf,"%s",(char *)GetLangText(12155, "无"));
	}

	CHECK_BUFFER;

    memset(acInfo, 0, sizeof(acInfo));
    iLength  = sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%d</Data></Cell>\n", (char *)GetLangText(11295, "工号"), userno);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s</Data></Cell>\n", (char *)GetLangText(10006, "姓名"), name);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s</Data></Cell>\n", (char *)GetLangText(10012, "部门"), dep);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"2\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s</Data></Cell>\n", (char *)GetLangText(12144, "班次"), tmpbuf);
    iLength += sprintf((acInfo + iLength), "\t\t<Cell ss:MergeAcross=\"3\" ss:StyleID=\"m26293052\"><Data ss:Type=\"String\">%s:%s~%s</Data></Cell>", (char *)GetLangText(10013, "日期"), datestart,dateend);

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
        if (stat(XML_USER_TABLE_FILE, &st) == 0) /* 文件不存在，直接返回 */
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
					(char *)GetLangText(11928, "班段一"), (char *)GetLangText(11929, "班段二"), (char *)GetLangText(11930, "班段三"));
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
					(char *)GetLangText(11928, "班段一"), (char *)GetLangText(11929, "班段二"), (char *)GetLangText(11930, "班段三"));
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
					(char *)GetLangText(10013, "日期"), (char *)GetLangText(12175, "星期"),
					(char *)GetLangText(11403, "上班"), (char *)GetLangText(11404, "下班"),
					(char *)GetLangText(11403, "上班"), (char *)GetLangText(11404, "下班"),
					(char *)GetLangText(11403, "上班"), (char *)GetLangText(11404, "下班"));
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
					(char *)GetLangText(10013, "日期"), (char *)GetLangText(12175, "星期"),
					(char *)GetLangText(11403, "上班"), (char *)GetLangText(11404, "下班"),
					(char *)GetLangText(11403, "上班"), (char *)GetLangText(11404, "下班"),
					(char *)GetLangText(11403, "上班"), (char *)GetLangText(11404, "下班"));
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
		"\"String\">%s</Data></Cell>\n", (char *)GetLangText(11942, "考勤明细表"));

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
        (char *)GetLangText(12145, "工作日数"), days, (char *)GetLangText(12146, "出勤天数"), workdays,
        (char *)GetLangText(12147, "缺勤天数"), absence, (char *)GetLangText(12149, "迟到次数"), 
          (char *)GetLangText(11444, "分钟"), latenum, lateMin, (char *)GetLangText(12151, "早退次数"), 
          (char *)GetLangText(11444, "分钟"), earlyNum, earlyMin, (char *)GetLangText(12165, "加班时数"), 
          overtime, (char *)GetLangText(12153, "请假天数"), leaveDays, (char *)GetLangText(12154, "出差天数"), trips);
    
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
		"\"String\">%s:%04d-%02d-%02d %s %04d-%02d-%02d</Data></Cell>\n",(char *)GetLangText(10013, "日期"), timeRange.iStartYear, timeRange.iStartMonth,
	timeRange.iStartDay, (char *)GetLangText(12140, "到"),timeRange.iEndYear, timeRange.iEndMonth, timeRange.iEndDay);
    
	fprintf(m_xls, "%s%s", ADDNORMALROW, acInfo);
    end_row();
}

void fill_user_wageinfo(void)
{
    char acInfo[1024];
    
	CHECK_BUFFER;
    
    memset(acInfo, 0, sizeof(acInfo));
    sprintf(acInfo, "<Cell ss:MergeAcross=\"15\" ss:StyleID=\"m26293204\"><Data ss:Type=\"String\">%s：    %s:     %s：    %s：     %s：    %s：     %s：    </Data></Cell>\n",
		(char *)GetLangText(12166, "病假时数"),
		(char *)GetLangText(12167, "事假时数"),
		(char *)GetLangText(12168, "日薪工资"),
		(char *)GetLangText(12169, "加班工资"),
		(char *)GetLangText(12170, "其它津贴"),
		(char *)GetLangText(12171, "其他扣款"),
		(char *)GetLangText(12172, "实得工资"));

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
		ADDUSERTABLE3, (char *)GetLangText(12173, "确认"), (char *)GetLangText(12174, "审核"));
    end_row();
}
