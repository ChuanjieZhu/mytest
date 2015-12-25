#ifndef __GRAPHICSLIB_H__
#define __GRAPHICSLIB_H__

#include <assert.h>
#include <linux/fb.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define CIF_WIDTH   352
#define CIF_HEIGHT  288

#define BOOL int
#define HANDLE int
#define BYTE unsigned char
#define WORD unsigned short
#define LONG long
#define DWORD unsigned long
#define DDWORD unsigned long long
#define LONGLONG long long
#define FLOAT float
#define RGB unsigned long
#define RGB16 unsigned short
#define YUV unsigned long

#define MAKERGB(r,g,b) (RGB)( ((r)<<16) | ((g)<<8) | (b) )
#define MAKEBGR(r,g,b) (RGB)( ((b)<<16) | ((g)<<8) | (r) )
#define MAKERGB565(r,g,b) (RGB16)( (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3) )
#define MAKEBGR565(r,g,b) (RGB16)( (((b)>>3)<<11) | (((g)>>2)<<5) | ((r)>>3) )
#define RGB_TO_RGB565(c) (RGB16)( ((((c)&0xff0000)>>19)<<11) | ((((c)&0xff00)>>10)<<5) | (((c)&0xff)>>3) )
#define RGB565_TO_RGB(c) (RGB)( (((c)&0x1f)<<3) | ((((c)>>5)&0x3f)<<10) | (((c)>>11)<<19)  | 0x070307)
#define GETRVALUE(rgb) 	((rgb >> 16) & 0xff)
#define GETGVALUE(rgb) 	((rgb >> 8) & 0xff)
#define GETBVALUE(rgb) 	((rgb) & 0xff)

#ifdef WIN32
#pragma pack(1) 
#define PACKED_ALIGN
#else
#define PACKED_ALIGN	__attribute__((packed))
#endif

typedef union 
{
	RGB rgb;
	
	struct 
	{
		BYTE b, g, r, t;
	} PACKED_ALIGN val;

} PACKED_ALIGN COLORFMT;

#ifdef WIN32
#pragma pack()
#endif
#undef PACKED_ALIGN

typedef struct BMP 
{ 
	long nWidth;
	long nHeight;
	long nWidthBytes;
	short nBitsPerPixel;
	int nPal;
	RGB pal[256];
	char* pBits;
	
public:
	BMP();
	~BMP();
	
} BMP, * LPBMP;


int isDigit(unsigned short ch);
int isAlpha(unsigned short ch);

class CFramebuffer
{   
    static BOOL m_bYuvMode;				/* 是否为YUV模式 */
    static int m_nDepth;                    /* 色深 */
    static int m_nBytesPerPixel;			/* 每个像素字节数 */ 
    static COLORFMT m_palette[256];			/* 颜色板 */

    BYTE MatchColor(BYTE r, BYTE g, BYTE b); // match color from color-map
	BYTE MatchColor(RGB rgb);
    RGB MapColor(BYTE nMapIndex);           // match color from color-map


public:
	CFramebuffer();
	~CFramebuffer();
    
	LPBMP CreateBmpFromRgbBuf(int nWidth, int nHeight, int nBitsPerPixel, char* pRgbBuf);

    LPBMP CreateBmpFromYUV420PBuf(int nWidth, int nHeight, char* pYuv420pBuf);

    LPBMP CreateBmpFromYUV420Buf(int nWidth, int nHeight, char* pYuv420Buf);
        
    BOOL SaveJpg(char* pFileName, LPBMP pBmp, BOOL bColor=TRUE, int nQuality=75);

    int BmpToJpg(char* pJpgBuf, LPBMP pBmp, BOOL bColor, int nQuality);
};

int CreateJpgByYuv420P(char *pYuv420PBuf, char *pJpgFilePath);

int SnapCifPicture(char *pFilePath, char *pDataBuff, int iWidth, int iHeight);
int SnapCifPictureEx(char *pFilePath, char *pDataBuff, int iWidth, int iHeight);


#endif  /* __GRAPHICSLIB_H__ */

