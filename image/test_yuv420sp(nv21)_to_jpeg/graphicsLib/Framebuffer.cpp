
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "jpeg_if.h"
#include "graphicsLib.h"

#define Malloc malloc
#define Free free
#define TRACE printf
#define MDL __FUNCTION__, __LINE__

#define UYVY_MODE 1
#define YUYV_MODE 2
#define YUV_MODE UYVY_MODE

#define YCbCrtoR(Y,Cb,Cr)       (1000*Y + 1371*(Cr-128))/1000
#define YCbCrtoG(Y,Cb,Cr)       (1000*Y - 336*(Cb-128) - 698*(Cr-128))/1000
#define YCbCrtoB(Y,Cb,Cr)       (1000*Y + 1732*(Cb-128))/1000
#define MAKERGB(r,g,b) (RGB)( ((r)<<16) | ((g)<<8) | (b) )
#define MAKEBGR(r,g,b) (RGB)( ((b)<<16) | ((g)<<8) | (r) )
#define MAKERGB565(r,g,b) (RGB16)( (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3) )
#define MAKEBGR565(r,g,b) (RGB16)( (((b)>>3)<<11) | (((g)>>2)<<5) | ((r)>>3) )
#define RGB_TO_RGB565(c) (RGB16)( ((((c)&0xff0000)>>19)<<11) | ((((c)&0xff00)>>10)<<5) | (((c)&0xff)>>3) )
#define RGB565_TO_RGB(c) (RGB)( (((c)&0x1f)<<3) | ((((c)>>5)&0x3f)<<10) | (((c)>>11)<<19)  | 0x070307)
#define GETRVALUE(rgb) 	((rgb >> 16) & 0xff)
#define GETGVALUE(rgb) 	((rgb >> 8) & 0xff)
#define GETBVALUE(rgb) 	((rgb) & 0xff)

BOOL CFramebuffer::m_bYuvMode = FALSE;
int CFramebuffer::m_nBytesPerPixel = 2;
int CFramebuffer::m_nDepth = 16;
COLORFMT CFramebuffer::m_palette[256];

int isAlpha(unsigned short ch)
{
	return ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'));
}

int isDigit(unsigned short ch)
{
	return (ch>='0' && ch<='9');
}

RGB yuv2rgb(BYTE y, BYTE u, BYTE v)
{
	int r, g, b;

	r = ( (y<<8)               + 359*(v-128) ) >> 8;
	g = ( (y<<8) -  88*(u-128) - 183*(v-128) ) >> 8;
	b = ( (y<<8) + 454*(u-128)               ) >> 8;

	if (r < 0)
		r = 0;
	else if (r > 255)
		r = 255;

	if (g < 0)
		g = 0;
	else if (g > 255)
		g = 255;

	if (b < 0)
		b = 0;
	else if (b > 255)
		b = 255;

	return MAKERGB(r,g,b);
}

RGB yuv2rgb(YUV yuv)
{
#if YUV_MODE==UYVY_MODE
	return yuv2rgb((yuv>>8) & 0xff, yuv & 0xff, (yuv>>16) & 0xff);
#else
	return yuv2rgb(yuv & 0xff, (yuv>>8) & 0xff, yuv>>24);
#endif
}

YUV rgb2yuv(BYTE r, BYTE g, BYTE b)
{
	int y, u, v;

	y = (299*r+587*g+114*b)/1000;
	u = (128000-169*r-331*g+500*b)/1000;
	v = (128000+500*r-419*g-81*b)/1000;

	if(y < 16)
		y = 16;
	else if(y>235)
		y = 235;

	if(u < 16)
		u = 16;
	else if(u>240)
		u = 240;

	if(v < 16)
		v = 16;
	else if(v>240)
		v = 240;

#if YUV_MODE==UYVY_MODE
	return (y<<24)+(v<<16)+(y<<8)+u;
#else
	return (v<<24)+(y<<16)+(u<<8)+y;
#endif
}

YUV rgb2yuv(RGB color)
{
	return rgb2yuv((color>>16) & 0xff, (color>>8) & 0xff, color & 0xff);
}

#define SATURATE8(x) ((unsigned int) x <= 255 ? x : (x < 0 ? 0: 255))

BMP::BMP()
{
	pBits=NULL;
}

BMP::~BMP()
{
	if(pBits) Free(pBits); pBits=NULL;
}

CFramebuffer::CFramebuffer()
{
    ;
}


CFramebuffer::~CFramebuffer()
{
    ;
}

/******************************************************************************
 * 函数名称： CFramebuffer.MatchColor
 * 功能： 取当前RGB值对应的调色板的序号
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 2012-6-18
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
BYTE CFramebuffer::MatchColor(BYTE r, BYTE g, BYTE b) 
{
	static BYTE last_r=0, last_g=0, last_b=0, last_ret=0;
	int i, n=1;
	if(r==last_r && g==last_g && b==last_b)
		return last_ret;

	for(i=0;m_nDepth<=8 && i<m_nDepth;i++)
		n<<=1;

	for(i=0;i<n && i<16; i++)
	{
		if( r==m_palette[i].val.r && g==m_palette[i].val.g && b==m_palette[i].val.b )
			break;
	}

	if(i==n || i==16)
	{
		if(m_nDepth==8)
			i=r/51*36+g/51*6+b/51+16;
		else
		{
#define _ERROR	42
			for(i=0;i<n;i++)
			{
				if(abs(r-m_palette[i].val.r)<=_ERROR && abs(g-m_palette[i].val.g)<=_ERROR && abs(b-m_palette[i].val.b)<=_ERROR)
					break;
			}
			if(i==n) i=0;
		}
	}
	last_r=r; last_g=g; last_b=b; last_ret=(BYTE)i;
	return last_ret;
}

BYTE CFramebuffer::MatchColor(RGB color) // match color from color-map
{
	return MatchColor( (color>>16) & 0xff, (color>>8) & 0xff, color & 0xff);
}


/******************************************************************************
 * 函数名称： CFramebuffer.MapColor
 * 功能： 取当前调色板对应的RGB值
 * 参数： 
 * 返回： 
 * 创建作者： 
 * 创建日期： 2012-6-18
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
RGB CFramebuffer::MapColor(BYTE nMapIndex)
{
	int i, n=1;
	for(i=0;m_nDepth<=8 && i<m_nDepth;i++) n<<=1;
	//if(nMapIndex < 0 || nMapIndex >= n)
	if(nMapIndex >= n)
	{
		return 0;
	}
	return MAKERGB(m_palette[nMapIndex].val.r, m_palette[nMapIndex].val.g, m_palette[nMapIndex].val.b);
}

LPBMP CFramebuffer::CreateBmpFromYUV420PBuf(int nWidth, int nHeight, char* pYuv420pBuf)
{
	LPBMP pBmp = new BMP;
	if(pBmp==NULL)
		return NULL;
	memset(pBmp, 0, sizeof(BMP));

	pBmp->nWidth=nWidth;
	pBmp->nHeight=nHeight;
	pBmp->nBitsPerPixel=m_nBytesPerPixel*8;
	pBmp->nWidthBytes=pBmp->nWidth*m_nBytesPerPixel;

	pBmp->pBits = (char*)Malloc(pBmp->nWidthBytes*pBmp->nHeight);
	if(pBmp->pBits==NULL)
	{
		delete pBmp;
		pBmp=NULL;
	}
	else
	{
		int i, j;
		unsigned char *pDst = (unsigned char*)pBmp->pBits; 
        unsigned char *pY = (unsigned char*)pYuv420pBuf;
        unsigned char *pU = pY+nWidth*nHeight;
        unsigned char *pV = pU+nWidth*nHeight / 4;
		if (m_bYuvMode)
		{
			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					if(*pY<0x10)
						*pY = 0x10;
					else if(*pY>0xEB)
						*pY = 0xEB;
					*pDst++=*pU;
					*pDst++=*pY;
					*pDst++=*pV;
					*pDst++=*pY;
					pY++;
					if(i%2)
					{
						pU++;
						pV++;
					}
				}
				if(j%2==0)
				{
					pU-=nWidth/2;
					pV-=nWidth/2;
				}
			}
		}
		else
		{
			RGB rgb;
			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					rgb = yuv2rgb(*pY, *pU, *pV);
                    
					if(m_nDepth==8)
						*pDst = MatchColor(rgb);
					else if(m_nDepth==16)
						*((RGB16*)pDst)=RGB_TO_RGB565(rgb);
					else if(m_nDepth==24 || m_nDepth==32)
						memcpy(pDst, &rgb, m_nDepth/8);
					else
						memset(pDst, 0, m_nDepth/8);
					pDst+=m_nDepth/8;

					pY++;

                    if(i%2)
					{
						pU++;
						pV++;
					}
				}
                
				if(j%2==0)
				{
					pU-=nWidth/2;
					pV-=nWidth/2;
				}
			}
		}
	}
    
	return pBmp;
}

LPBMP CFramebuffer::CreateBmpFromYUV420Buf(int nWidth, int nHeight, char* pYuv420Buf)
{
	LPBMP pBmp = new BMP;
	if(pBmp==NULL)
		return NULL;
	memset(pBmp, 0, sizeof(BMP));
     
	pBmp->nWidth=nWidth;
	pBmp->nHeight=nHeight;
	pBmp->nBitsPerPixel=m_nBytesPerPixel*8;
	pBmp->nWidthBytes=pBmp->nWidth*m_nBytesPerPixel;

	pBmp->pBits = (char*)Malloc(pBmp->nWidthBytes*pBmp->nHeight);
	if(pBmp->pBits==NULL)
	{
		delete pBmp;
		pBmp=NULL;
	}
	else
	{
		int i, j;
		unsigned char *pDst = (unsigned char*)pBmp->pBits; 
        unsigned char *pY = (unsigned char*)pYuv420Buf;
        unsigned char *pU = pY + nWidth * nHeight;
        unsigned char *pV = pU + nWidth * nHeight / 4;
        
		if (m_bYuvMode)
		{   
			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					if(*pY<0x10)
						*pY = 0x10;
					else if(*pY>0xEB)
						*pY = 0xEB;
					*pDst++=*pU;
					*pDst++=*pY;
					*pDst++=*pV;
					*pDst++=*pY;
					pY++;
					if(i%2)
					{
						pU++;
						pV++;
					}
				}
				if(j%2==0)
				{
					pU-=nWidth/2;
					pV-=nWidth/2;
				}
			}
		}
		else
		{
			RGB rgb;
			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					rgb = yuv2rgb(*pY, *pU, *pV);
                    
					if(m_nDepth==8)
						*pDst = MatchColor(rgb);
					else if(m_nDepth==16)
						*((RGB16*)pDst)=RGB_TO_RGB565(rgb);
					else if(m_nDepth==24 || m_nDepth==32)
						memcpy(pDst, &rgb, m_nDepth/8);
					else
						memset(pDst, 0, m_nDepth/8);
                    
					pDst+=m_nDepth/8;

					pY++;

                    if(i%2)
					{
						pU++;
						pV++;
					}
				}
                
				if(j%2==0)
				{
					pU-=nWidth/2;
			    	pV-=nWidth/2;
				}
			}
		}
	}
    
	return pBmp;
}

LPBMP CFramebuffer::CreateBmpFromRgbBuf(int nWidth, int nHeight, int nBitsPerPixel, char* pRgbBuf)
{
    if (pRgbBuf==NULL) 
        return NULL;

	LPBMP pBmp = new BMP;
	if(pBmp==NULL) 
        return NULL;
	memset(pBmp, 0, sizeof(BMP));

	pBmp->nWidth=nWidth;
	pBmp->nHeight=nHeight;
	pBmp->nBitsPerPixel=m_nBytesPerPixel*8;
	pBmp->nWidthBytes=pBmp->nWidth*m_nBytesPerPixel;

	pBmp->pBits = (char*)Malloc(pBmp->nWidthBytes*pBmp->nHeight);
	if(pBmp->pBits==NULL)
	{
		delete pBmp;
		pBmp=NULL;
	}
	else
	{
		if(nBitsPerPixel==m_nDepth)
			memcpy(pBmp->pBits, pRgbBuf, pBmp->nWidthBytes*nHeight);
		else
		{
			RGB rgb;
			int i, j;
			unsigned char *pDst = (unsigned char*)pBmp->pBits, *pRgb = (unsigned char*)pRgbBuf;
			RGB16 *pDst16 = (RGB16*)pDst, *pRgb16 = (RGB16*)pRgb;
			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					if(nBitsPerPixel==8)
						rgb = MapColor(*pRgb++);
					else if(nBitsPerPixel==16)
					{
						RGB16 rgb16 = *pRgb16++;
						rgb = RGB565_TO_RGB(rgb16);
					}
					else
					{
						rgb = 0;
						memcpy(&rgb, pRgb, 3);
						pRgb+=3;
						if(nBitsPerPixel==32)
							pRgb++;
					}
					if(m_nDepth==8)
						*pDst++ = MatchColor(rgb);
					if(m_nDepth==16)
						*pDst16++ = RGB_TO_RGB565(rgb);
					else
					{
						if(m_nDepth==24 || m_nDepth==32)
							memcpy(pDst, &rgb, m_nDepth/8);
						pDst+=m_nDepth/8;
					}
				}
			}
		}
	}
	
	return pBmp; 
}


BOOL CFramebuffer::SaveJpg(char* pFileName, LPBMP pBmp, BOOL bColor, int nQuality)
{
    BOOL bRet = FALSE;
	if(pBmp==NULL)
		return bRet;

	if(pBmp->pBits==NULL)
		return bRet;

	RGB rgb;
	unsigned char r = 0, g = 0, b = 0;
    unsigned char *pDataBuf = (unsigned char*)Malloc(pBmp->nWidth*pBmp->nHeight*3);
	if(pDataBuf==NULL)
	{
		TRACE("CFramebuffer::SaveJpg: Save JPEG Out Of Memory (%ld,%ld,%ld,%d)!\r\n",
            pBmp->nWidth*pBmp->nHeight*3,pBmp->nWidth,pBmp->nHeight,m_nBytesPerPixel);
		return bRet;
	}

	unsigned char *pSrc = (unsigned char*)pBmp->pBits, *pDst = pDataBuf;
	for(int row=0; row<pBmp->nHeight; row++)
	{
		for(int col=0; col<pBmp->nWidth; col++)
		{
			switch(m_nDepth)
			{
				case 8:
					rgb = MapColor(*pSrc++);
					r = (rgb>>16) & 0xFF;
					g = (rgb>>8) & 0xFF;
					b = rgb & 0xFF;
					break;
				case 16:
					if(m_bYuvMode) {
						rgb = yuv2rgb(*((YUV*)pSrc));
						pSrc+=sizeof(YUV);
					} else {
						rgb = RGB565_TO_RGB(*((RGB16*)pSrc));
						pSrc+=sizeof(RGB16);
					}
					r = (rgb>>16) & 0xFF;
					g = (rgb>>8) & 0xFF;
					b = rgb & 0xFF;
					break;
				case 24:
				case 32:
					b = *pSrc++;
					g = *pSrc++;
					r = *pSrc++;
					if(m_nDepth==32) pSrc++;
					break;
				default:
					break;
			}
			if(bColor)
			{
				*pDst++ = r;
				*pDst++ = g;
				*pDst++ = b;
			}
			else
			{
				*pDst++ = (299*r+587*g+114*b)/1000;
			}
		}
	}

	bRet = (jpeg_compress(pFileName, pDataBuf, pBmp->nWidth, pBmp->nHeight, bColor, nQuality) == 0);

	Free(pDataBuf);

	return bRet;
}


int CFramebuffer::BmpToJpg(char* pJpgBuf, LPBMP pBmp, BOOL bColor, int nQuality)
{
	int bRet = FALSE;

    if (pBmp == NULL) 
        return bRet;

    if (pBmp->pBits == NULL)
    {
        return bRet;
    }
    
	RGB rgb;
	unsigned char r = 0, g = 0, b = 0;
    unsigned char *pDataBuf = (unsigned char*)Malloc(pBmp->nWidth*pBmp->nHeight*3);
	if(pDataBuf==NULL) 
    {
		TRACE("CFramebuffer::SaveJpg: Save JPEG Out Of Memory (%ld,%ld,%ld,%d)!\r\n", \
            pBmp->nWidth*pBmp->nHeight*3,pBmp->nWidth,pBmp->nHeight,m_nBytesPerPixel);
		return bRet;
	}

	unsigned char *pSrc = (unsigned char*)pBmp->pBits, *pDst = pDataBuf;
	for(int row=0; row<pBmp->nHeight; row++) {
		for(int col=0; col<pBmp->nWidth; col++) {
			switch(m_nDepth) {
				case 16:
					if(m_bYuvMode) {
						rgb = yuv2rgb(*((YUV*)pSrc));
						pSrc+=sizeof(YUV);
					} else {
						rgb = RGB565_TO_RGB(*((RGB16*)pSrc));
						pSrc+=sizeof(RGB16);
					}
					r = (rgb>>16) & 0xFF;
					g = (rgb>>8) & 0xFF;
					b = rgb & 0xFF;
					break;
				case 24:
				case 32:
					b = *pSrc++;
					g = *pSrc++;
					r = *pSrc++;
					if(m_nDepth==32) pSrc++;
					break;
				default:
					break;
			}
			if(bColor) {
				*pDst++ = r;
				*pDst++ = g;
				*pDst++ = b;
			} else {
				*pDst++ = (299*r+587*g+114*b)/1000;
			}
		}
	}

	//bRet = jpeg_compress_ex(pJpgBuf, pDataBuf, pBmp->nWidth, pBmp->nHeight, bColor, nQuality);

	Free(pDataBuf);

	return bRet;
}


int SnapCifPicture(char *pFilePath, char *pDataBuff, int iWidth, int iHeight)
{
    int iRet = - 1;
    
    if (!pDataBuff || !pFilePath)
        return -1;

    if (iWidth != CIF_WIDTH)
        iWidth = CIF_WIDTH;

    if (iHeight != CIF_HEIGHT)
        iHeight = CIF_HEIGHT;
    
    CFramebuffer stFb;
    LPBMP pBmp = NULL;
     
    pBmp = stFb.CreateBmpFromYUV420Buf(iWidth, iHeight, pDataBuff);
    if (pBmp != NULL)
    {
        if (stFb.SaveJpg(pFilePath, pBmp, FALSE, 90))
        {
            iRet = 0;
        }
        
        delete pBmp;
    }
        
    return iRet;
}

int SnapCifPictureEx(char *pFilePath, char *pDataBuff, int iWidth, int iHeight)
{
    int iRet = - 1;
    
    if (!pDataBuff || !pFilePath)
        return -1;

    if (iWidth != CIF_WIDTH)
        iWidth = CIF_WIDTH;

    if (iHeight != CIF_HEIGHT)
        iHeight = CIF_HEIGHT;
    
    CFramebuffer stFb;
    LPBMP pBmp = NULL;
     
    pBmp = stFb.CreateBmpFromYUV420PBuf(iWidth, iHeight, pDataBuff);
    if (pBmp != NULL)
    {
        if (stFb.SaveJpg(pFilePath, pBmp, FALSE, 90))
        {
            iRet = 0;
        }
        
        delete pBmp;
    }
        
    return iRet;
}

void yuv420p_to_rgb(LPBMP pBmp, int newwidth, int newheight, int nBitsPerPixel, void* psrc, int srcwidth, int srcheight, BOOL bUV)
{
	//static char D1BUF[D1_WIDTH*D1_HEIGHT*2];
	int line, col, width, height, half_width;
	int y=0x10, u=0x80, v=0x80, yy, uu, vv, vr=0, ug=0, vg=0, ub=0;
	int r, g, b;
	unsigned char *py, *pu, *pv;
	unsigned char* pDst;
	RGB16* pDst16;

	if(psrc==NULL || srcwidth==0 || srcheight==0) return;
	if(newwidth==0) newwidth=srcwidth;
	if(newheight==0) newheight=srcheight;

#if 0
    if(newwidth!=srcwidth && newheight!=srcheight) {
		if(srcwidth==CIF_WIDTH && srcheight==CIF_HEIGHT && newwidth==SQCIF_WIDTH && newheight==SQCIF_HEIGHT) {
			yuv420p_cif2sqcif(psrc, bUV);
			srcwidth=SQCIF_WIDTH;
			srcheight=SQCIF_HEIGHT;
		} else if(srcwidth==QCIF_WIDTH && srcheight==QCIF_HEIGHT && newwidth==SQCIF_WIDTH && newheight==SQCIF_HEIGHT) {
			yuv420p_qcif2sqcif(psrc, bUV);
			srcwidth=SQCIF_WIDTH;
			srcheight=SQCIF_HEIGHT;
		} else {
			int zoom = srcwidth/newwidth;
			if(zoom && srcheight/newheight==zoom && srcwidth%newwidth==0  && srcheight%newheight==0) {
				yuv420p_zoom(psrc, srcwidth, srcheight, 100/zoom, D1BUF, bUV);
				psrc = D1BUF;
				srcwidth/=zoom;
				srcheight/=zoom;
			} else {
				zoom = newwidth/srcwidth;
				if(zoom && newheight/srcheight==zoom && newwidth%srcwidth==0 && newheight%srcheight==0) {
					yuv420p_zoom(psrc, srcwidth, srcheight, zoom*100, D1BUF, bUV);
					psrc = D1BUF;
					srcwidth*=zoom;
					srcheight*=zoom;
				} else {
					int zoom1 = srcwidth, zoom2 = newwidth;
					while(zoom1/2*2==zoom1 && zoom2/2*2==zoom2) { zoom1/=2; zoom2/=2; }
					while(zoom1/3*3==zoom1 && zoom2/3*3==zoom2) { zoom1/=3; zoom2/=3; }
					while(zoom1/5*5==zoom1 && zoom2/5*5==zoom2) { zoom1/=5; zoom2/=5; }
					while(zoom1/7*7==zoom1 && zoom2/7*7==zoom2) { zoom1/=7; zoom2/=7; }
					if(srcheight/zoom1*zoom1==srcheight && newheight/zoom2*zoom2==newheight);
					else {
						zoom1 = srcheight, zoom2 = newheight;
						while(zoom1/2*2==zoom1 && zoom2/2*2==zoom2) { zoom1/=2; zoom2/=2; }
						while(zoom1/3*3==zoom1 && zoom2/3*3==zoom2) { zoom1/=3; zoom2/=3; }
						while(zoom1/5*5==zoom1 && zoom2/5*5==zoom2) { zoom1/=5; zoom2/=5; }
						while(zoom1/7*7==zoom1 && zoom2/7*7==zoom2) { zoom1/=7; zoom2/=7; }
						if(srcwidth/zoom1*zoom1==srcwidth && newwidth/zoom2*zoom2==newwidth);
						else zoom1 = zoom2 = 0;
					}
					if(zoom1 && zoom2) {
						yuv420p_zoom_ex(psrc, srcwidth, srcheight, zoom1, zoom2, D1BUF, bUV);

						psrc = D1BUF;
						srcwidth = newwidth;
						srcheight = newheight;
					}
				}
			}
		}
	}
#endif

	pBmp->nWidth=width=srcwidth;
	pBmp->nHeight=height=srcheight;
	pBmp->nBitsPerPixel = nBitsPerPixel;
	pBmp->nWidthBytes=srcwidth*pBmp->nBitsPerPixel/8;
	pBmp->pBits=(char*)Malloc(pBmp->nWidthBytes*pBmp->nHeight);
	if(pBmp->pBits==NULL) return;

	half_width=width>>1;
	py=(unsigned char*)psrc;
	pu=py+width*height;
	pv=pu+width*height/4;

	pDst = (unsigned char*)pBmp->pBits;
	pDst16=(RGB16*)pDst;

	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col++) {
			y = *py++;
			if(bUV && !(col%2)) {
				u = *pu++;
				v = *pv++;
			}
#if 1
			if(y<16) y=16;
			else if(y>235) y=235;
#endif
			yy = y<<8;
			if(!(col%2)) {
				uu = u-128;
				ug = 88 * uu;
				ub = 454 * uu;
				vv = v - 128;
				vg = 183 * vv;
				vr = 359 * vv;
			}
			
			r = ( yy + vr ) >> 8;
			g = ( yy - ug - vg ) >> 8;
			b = ( yy + ub ) >> 8;

			if(r<0) r=0;
			else if(r>0xff) r=0xff;
			if(g<0) g=0;
			else if(g>0xff) g=0xff;
			if(b<0) b=0;
			else if(b>0xff) b=0xff;

			if(nBitsPerPixel==16) *pDst16++=MAKERGB565(r, g, b);
			else if(nBitsPerPixel==24 || nBitsPerPixel==32) {
				*pDst++=b;
				*pDst++=g;
				*pDst++=r;

				if(nBitsPerPixel==32) *pDst++=0;
			} else *pDst++ = 0;
		}
		if(bUV && !(line%2)) {
			pu -= half_width;
			pv -= half_width;
		}
	}
}

int CreateJpgByYuv420P(char *pYuv420PBuf, char *pJpgFilePath)
{
	CFramebuffer	fb;
	BMP				bmp;
	int 			nRet = -1;

	yuv420p_to_rgb( &bmp, CIF_WIDTH, CIF_HEIGHT, 16, pYuv420PBuf, CIF_WIDTH, CIF_HEIGHT, TRUE);
	
	if ( bmp.pBits != NULL )
	{
		if (fb.SaveJpg(pJpgFilePath, &bmp, TRUE, 90))
		{
			nRet = 0;
		}
        
		usleep( 20*1000 );
	}
	
	return nRet;
}

