
#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "../jpgLib/jpeglib.h"
#include "graphicsLib.h"

#define TRACE printf
#define MDL __FUNCTION__, __LINE__

#define JPEG_QUALITY    80      // 图片质量  

typedef struct {  
        WORD    bfType;  
        DWORD   bfSize;  
        WORD    bfReserved1;  
        WORD    bfReserved2;  
        DWORD   bfOffBits;  
} BMPFILEHEADER_T; 

typedef struct{  
        DWORD      biSize;  
        LONG       biWidth;  
        LONG       biHeight;  
        WORD       biPlanes;  
        WORD       biBitCount;  
        DWORD      biCompression;  
        DWORD      biSizeImage;  
        LONG       biXPelsPerMeter;  
        LONG       biYPelsPerMeter;  
        DWORD      biClrUsed;  
        DWORD      biClrImportant;  
} BMPINFOHEADER_T;

typedef struct yuv2rgb_rgb_t {  
    int r, g, b;  
} yuv2rgb_rgb_t;  
  
static inline void rgb_calc(yuv2rgb_rgb_t* rgb, int Y, int Cr, int Cb) {  
    rgb->r = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);  
    if (rgb->r < 0)  
        rgb->r = 0;  
    else if (rgb->r > 255)  
        rgb->r = 255;  
    rgb->g = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);  
    if (rgb->g < 0)  
        rgb->g = 0;  
    else if (rgb->g > 255)  
        rgb->g = 255;  
    rgb->b = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);  
    if (rgb->b < 0)  
        rgb->b = 0;  
    else if (rgb->b > 255)  
        rgb->b = 255;  
}  
  
#define YUV2RGB_SET_RGB(p, rgb) *p++ = (unsigned char)rgb.r; *p++ = (unsigned char)rgb.g; *p++ = (unsigned char)rgb.b; *p++ = 0xff  
  
static void yuv420sp_to_rgba(unsigned char const* yuv420sp, int width, int height, unsigned char* rgba) 
{  
    const int width4 = width * 4;  
    unsigned char const* y0_ptr = yuv420sp;  
    unsigned char const* y1_ptr = yuv420sp + width;  
    unsigned char const* cb_ptr = yuv420sp + (width * height);  
    unsigned char const* cr_ptr = cb_ptr + 1;  
    unsigned char* rgba0 = rgba;  
    unsigned char* rgba1 = rgba + width4;  
    int Y00, Y01, Y10, Y11;  
    int Cr = 0;  
    int Cb = 0;  
    int r, c;  
    yuv2rgb_rgb_t rgb00, rgb01, rgb10, rgb11;  
    for (r = 0; r < height / 2; ++r) {  
        for (c = 0; c < width / 2; ++c, cr_ptr += 2, cb_ptr += 2) {  
            Cr = *cr_ptr;  
            Cb = *cb_ptr;  
            if (Cb < 0)  
                Cb += 127;  
            else  
                Cb -= 128;  
            if (Cr < 0)  
                Cr += 127;  
            else  
                Cr -= 128;  
            Y00 = *y0_ptr++;  
            Y01 = *y0_ptr++;  
            Y10 = *y1_ptr++;  
            Y11 = *y1_ptr++;  
            rgb_calc(&rgb00, Y00, Cr, Cb);  
            rgb_calc(&rgb01, Y01, Cr, Cb);  
            rgb_calc(&rgb10, Y10, Cr, Cb);  
            rgb_calc(&rgb11, Y11, Cr, Cb);  
            YUV2RGB_SET_RGB(rgba0, rgb00);  
            YUV2RGB_SET_RGB(rgba0, rgb01);  
            YUV2RGB_SET_RGB(rgba1, rgb10);  
            YUV2RGB_SET_RGB(rgba1, rgb11);  
        }  
        y0_ptr += width;  
        y1_ptr += width;  
        rgba0 += width4;  
        rgba1 += width4;  
    }  
}  


void yuv422sp_to_yuv422p(unsigned char* yuv422sp, unsigned char* yuv422p, int width, int height)  
{  
    int i, j;  
    int y_size;  
    int uv_size;  
    unsigned char* p_y1;  
    unsigned char* p_uv;  
  
    unsigned char* p_y2;  
    unsigned char* p_u;  
    unsigned char* p_v;  
  
    y_size = uv_size = width * height;  
  
    p_y1 = yuv422sp;  
    p_uv = yuv422sp + y_size;  
  
    p_y2 = yuv422p;  
    p_u  = yuv422p + y_size;  
    p_v  = p_u + width * height / 2;  
  
    memcpy(p_y2, p_y1, y_size);  
  
    for (j = 0, i = 0; j < uv_size; j+=2, i++)  
    {  
        p_u[i] = p_uv[j];  
        p_v[i] = p_uv[j+1];  
    }  
} 

void yuv420sp_to_yuv422p(unsigned char* yuv420sp, unsigned char* yuv422p, int width, int height)  
{   
    int i, j;  
    int y_size;  
    
    unsigned char* p_y1;  
    unsigned char* p_uv;  
  
    unsigned char* p_y2;  
    unsigned char* p_u;  
    unsigned char* p_v; 
    
    y_size = width * height;
    
    p_y1 = yuv420sp;  
    p_uv = yuv420sp + y_size;  
  
    p_y2 = yuv422p;  
    p_u  = yuv422p + y_size;  
    p_v  = p_u + width * height / 2;  

    memcpy(p_y2, p_y1, y_size); 

    for (j = 0, i = 0; j < y_size / 2; j+=2, i++)  
    {  
        p_u[i] = p_uv[j];  
        p_v[i] = p_uv[j+1];  
    }  
}



void yuv420p_to_yuv422(void* psrc, unsigned char *pdst, int srcwidth, int srcheight, int bUV)
{
	unsigned char *py, *pu, *pv;
    unsigned char *pDst;
	int y=16, u=0x80, v=0x80, line, col, half_width;

	half_width=srcwidth>>1;
	py=(unsigned char*)psrc;
	pu=py+srcwidth*srcheight;
	pv=pu+srcwidth*srcheight/4;
	pDst = pdst;

	for (line = 0; line < srcheight; line++)
	{
		for (col = 0; col < srcwidth; col++)
		{
			y = *py++;
			if(bUV && !(col%2))
			{
				u = *pu++;
				v = *pv++;
			}
#if YUV_MODE==UYVY_MODE
			if(col%2==0)
				*pDst++ = u;
			else
				*pDst++ = v;

			*pDst++ = y;
#else
			*pDst++ = y;

			if(col%2==0)
				*pDst++ = u;
			else
				*pDst++ = v;
#endif
		}
        
		if(bUV && !(line%2))
		{
			pu -= half_width;
			pv -= half_width;
		}
	}
}

/* 将NV21转换成YUV420P */
static int YUV420SP_NV21_TO_YUV420P(char *yuv420sp, char *yuv420p, int width, int height)
{
    if (!yuv420sp || !yuv420p)
        return -1;
    
	int framesize = width*height;
	int i = 0, j = 0;

    /* copy y */
	for (i = 0; i < framesize; i++)
	{
	    yuv420p[i] = yuv420sp[i];
	}

    /* copy Cr(V) */
	i = 0;
	for (j = 0; j < framesize / 2; j += 2)
	{
	    yuv420p[i + framesize * 5 / 4] = yuv420sp[j + framesize];
	    i++;
	}

    /* copy Cb(U) */
	i = 0;
	for (j = 1; j < framesize / 2; j += 2)
    {
	    yuv420p[i + framesize] = yuv420sp[j + framesize];
	    i++;
	}

    return 0;
}

/* 将NV12转换成YUV420P */
static int YUV420SP_NV12_TO_YUV420P(char *yuv420sp,  char *yuv420, int width, int height)
{
    int PixelsCount  = width * height;  
    int i = 0, j= 0;

    if (yuv420sp == NULL || yuv420 == NULL)
    {    
        return -1;
    }

    //copy  Y
    for (i = 0; i < PixelsCount; i++)
    {
       *(yuv420 +i) = *(yuv420sp + i);
    }
 
    //copy  Cb(U)
    i = 0;
    for (j = 0;  j < PixelsCount / 2; j += 2)
    {
       *(yuv420 + (i + PixelsCount)) = *(yuv420sp + (j + PixelsCount));
       i++;
    }

    //copy Cr(V)
    i = 0;
    for (j = 1;  j < PixelsCount / 2; j += 2)
    {
       *(yuv420 +(i+PixelsCount * 5 / 4)) = *(yuv420sp + (j + PixelsCount));
       i++;
    }

    return 0;
}

void yuv420sp_to_yuv420p(unsigned char* yuv420sp, unsigned char* yuv420p, int width, int height)
{
    int i, j;
    int y_size = width * height;

    unsigned char* y = yuv420sp;
    unsigned char* uv = yuv420sp + y_size;

    unsigned char* y_tmp = yuv420p;
    unsigned char* v_tmp = yuv420p + y_size;
    unsigned char* u_tmp = yuv420p + y_size * 5 / 4;

    // y
    memcpy(y_tmp, y, y_size);

    // u
    for (j = 0, i = 0; j < y_size/2; j+=2, i++)
    {
        v_tmp[i] = uv[j];
        u_tmp[i] = uv[j+1];
    }
}


static FILE * OpenYuv420File()
{
    FILE *pfd;
    char aszFileName[128];

    sprintf(aszFileName, "1.%s", "yuv");
    pfd = fopen(aszFileName, "w+");
    if (NULL == pfd)
    {
        TRACE("> Open file %s failed. %s %d\r\n", aszFileName, MDL);
        return NULL;
    }
    
    TRACE("* open image file:\"%s\" ok\n", aszFileName);
    return pfd;
}

static void SaveYuv420File(FILE *fp, char *data, unsigned int size)
{
    fwrite(data, 1, size, fp);
}

int CopyFileToBuf(char *buf, int n, char *file)
{
	FILE *fp = NULL;
	int retVal = -1;
	unsigned int len = 0;

	if (buf != NULL && n > 0 && file != NULL)
	{
		len = (unsigned int)n;

		if ((fp = fopen(file, "r")) != NULL)
		{
			if (fread(buf, sizeof(char), len, fp) == len)
			{
				retVal = 0;
			}

			fclose(fp);
		}
	}

	return retVal;
}


int CopyBufToFile(char *buf, int n, char *file)
{
    FILE *fp = NULL;
    int retVal = -1;
    unsigned int len = 0;

    if (buf != NULL && n > 0 && file != NULL)
    {
        len = (unsigned int)n;
        if ((fp = fopen(file, "w+")) != NULL)
        {
            if (fwrite(buf, sizeof(char), len, fp) == len)
            {
                retVal = 0;
            }

            fflush(fp);
            fclose(fp);
        }
        else
        {
            printf("fopen error.(%s) %s %d\r\n", strerror(errno), MDL);       
        }
    }
    
    return retVal;
}

static int R = 0;  
static int G = 1;  
static int B = 2;

typedef struct RGB_EX
{  
    int r, g, b;  
} RGB_EX;
      
void yuvTorgb(RGB_EX *rgb,char Y, char U, char V)
{  
    rgb->r = (int)((Y&0xff) + 1.4075 * ((V&0xff)-128));  
    rgb->g = (int)((Y&0xff) - 0.3455 * ((U&0xff)-128) - 0.7169*((V&0xff)-128));  
    rgb->b = (int)((Y&0xff) + 1.779 * ((U&0xff)-128));  
    rgb->r =(rgb->r<0? 0: rgb->r>255? 255 : rgb->r);  
    rgb->g =(rgb->g<0? 0: rgb->g>255? 255 : rgb->g);  
    rgb->b =(rgb->b<0? 0: rgb->b>255? 255 : rgb->b);    
}  

//NV21是YUV420格式，排列是(Y), (VU)，是2 plane  
void NV21ToRGB(unsigned char *rgb, unsigned char *src, int width, int height)
{  
    int numOfPixel = width * height;  
    int positionOfV = numOfPixel;  
    //int[] rgb = new int[numOfPixel*3];  

    RGB_EX tmp;
    
    for(int i=0; i<height; i++)
    {  
        int startY = i*width;  
        int step = i/2*width;  
        int startV = positionOfV + step;  

        for(int j = 0; j < width; j++)
        {  
            int Y = startY + j;  
            int V = startV + j/2;  
            int U = V + 1;  
            int index = Y*3;  
            yuvTorgb(&tmp, src[Y], src[U], src[V]);  
            rgb[index+R] = tmp.r;  
            rgb[index+G] = tmp.g;  
            rgb[index+B] = tmp.b;  
        }  
    }
}  


int Bmp2Jpg(unsigned char *pBmpBuf,int bmpSize,char *jpgFilePath,int width,int height)
{
    int i = 0;
    int depth = 3;
    JSAMPROW row_pointer[1];
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;   
    int row_stride = 0;
    unsigned char tmp;
    int n = 0;
    
    //Convert BMP to JPG
    cinfo.err = jpeg_std_error(&jerr); 
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(jpgFilePath, "wb")) == NULL) 
    {
        fprintf(stderr, "can't open %s\n", jpgFilePath);
        
        return -1;
    }
    
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;             
    cinfo.image_height = height;
    cinfo.input_components = depth;    
    cinfo.in_color_space = JCS_RGB;   
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE );
    jpeg_start_compress(&cinfo, TRUE);
    
    i = bmpSize-3;
    row_stride = width*3;   
    while(i>=0)
    {
    	tmp = pBmpBuf[i];
    	pBmpBuf[i] = pBmpBuf[i+2];
    	pBmpBuf[i+2] = tmp;
    	i-=3;
    }
    
	n = height-1;
	while(n>=0)
	{
		row_pointer[0] = &pBmpBuf[n*row_stride];
		jpeg_write_scanlines(&cinfo,row_pointer,1);
		n--;
	}
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(outfile);

    return 0;
} 

static void cvt_420p_to_rgb565_2(const unsigned char *src, unsigned char *dst, int width, int height)
{
  int line, col, linewidth;
  int y, u, v, yy, vr, ug, vg, ub;
  int r, g, b;
  const unsigned char *py, *pu, *pv;

  linewidth = width >> 1;
  py = src;
  pu = py + (width * height);
  pv = pu + (width * height) / 4;

  y = *py++;
  yy = y << 8;
  u = *pu - 128;
  ug = 88 * u;
  ub = 454 * u;
  v = *pv - 128;
  vg = 183 * v;
  vr = 359 * v;

  for (line = 0; line < height; line++) {
    for (col = 0; col < width; col++) {
      r = (yy + vr) >> 8;
      g = (yy - ug - vg) >> 8;
      b = (yy + ub ) >> 8;

      if (r < 0) r = 0;
      if (r > 255) r = 255;
      if (g < 0) g = 0;
      if (g > 255) g = 255;
      if (b < 0) b = 0;
      if (b > 255) b = 255;
      *((unsigned short *)dst) = (((unsigned short)r>>3)<<11) | (((unsigned short)g>>2)<<5) | (((unsigned short)b>>3)<<0);
      dst+=2;
      y = *py++;
      yy = y << 8;
      if (col & 1) {
    pu++;
    pv++;

    u = *pu - 128;
    ug = 88 * u;
    ub = 454 * u;
    v = *pv - 128;
    vg = 183 * v;
    vr = 359 * v;
      }
    }
    if ((line & 1) == 0) {
      pu -= linewidth;
      pv -= linewidth;
    }
  }
}

unsigned short ConvertRGB32To16(unsigned int nSrcRGB)  
{  
    unsigned short Red = (nSrcRGB & 0x00F80000) >> 8 ;  
    unsigned short Green = (nSrcRGB & 0x0000FC00) >> 5 ;  
    unsigned short Blue = (nSrcRGB & 0x000000F8) >> 3;  
    return Red | Green | Blue;  
}  


static void decodeYUV420SP(unsigned char *rgb, unsigned char * yuv420sp, int width, int height) 
{
    int frameSize = width * height;    
    int j, yp, i;

    for (j = 0, yp = 0; j < height; j++) 
    {
        int uvp = frameSize + (j >> 1) * width;
        int u = 0, v = 0;
        for (i = 0; i < width; i++, yp++) 
        {
            int y = (0xff & ((int) yuv420sp[yp])) - 16;
            if (y < 0) y = 0;
            if ((i & 1) == 0) {
                v = (0xff & yuv420sp[uvp++]) - 128;
                u = (0xff & yuv420sp[uvp++]) - 128;
            }
   
            int y1192 = 1192 * y;
            int r = (y1192 + 1634 * v);
            int g = (y1192 - 833 * v - 400 * u);
            int b = (y1192 + 2066 * u);
       
            if (r < 0) r = 0; else if (r > 262143) r = 262143;
            if (g < 0) g = 0; else if (g > 262143) g = 262143;
            if (b < 0) b = 0; else if (b > 262143) b = 262143;

            unsigned int tmp = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
            *((unsigned short *)rgb) = ConvertRGB32To16(tmp);
            rgb += 2;
        }
    }
}

static void decodeYUV420SPrgb565(unsigned char *rgb, char *yuv420sp, int width, int height) 
{
        int frameSize = width * height;
        int j, yp, i;
        
		for (j = 0, yp = 0; j < height; j++) 
        {
			int uvp = frameSize + (j >> 1) * width;
            int u = 0, v = 0;
			for (i = 0; i < width; i++, yp++) 
            {
				int y = (0xff & ((int) yuv420sp[yp])) - 16;

                if (y < 0)
					y = 0;

                if ((i & 1) == 0) 
                {
					v = (0xff & yuv420sp[uvp++]) - 128;
					u = (0xff & yuv420sp[uvp++]) - 128;
				}
				int y1192 = 1192 * y;
				int r = (y1192 + 1634 * v);
				int g = (y1192 - 833 * v - 400 * u);
				int b = (y1192 + 2066 * u);
				if (r < 0)
					r = 0;
				else if (r > 262143)
					r = 262143;
				if (g < 0)
					g = 0;
				else if (g > 262143)
					g = 262143;
				if (b < 0)
					b = 0;
				else if (b > 262143)
					b = 262143;
				*((unsigned int *)rgb) = 0xff000000 | ((r << 6) & 0xff0000)
						| ((g >> 2) & 0xff00) | ((b >> 10) & 0xff); 
                rgb += 4;
			}
		}
	}

int Yuv420pToJpeg(char *yuv420spBuf, int imageWidth, int imageHeigth, char *jpgFilePath)
{
    unsigned char *bmpBuf = NULL;

    if((NULL == yuv420spBuf)||(NULL == jpgFilePath))
    {
        return -1;    
    }
    
    bmpBuf = (unsigned char*)malloc(sizeof(unsigned char)*imageWidth*imageHeigth*3 *4);
    if (NULL == bmpBuf)
    {
        return -2;
    }

    decodeYUV420SPrgb565(bmpBuf, yuv420spBuf, imageWidth, imageHeigth);

    //yuv420sp_to_rgba((unsigned char *)yuv420spBuf, imageWidth, imageHeigth, bmpBuf);
    //cvt_420p_to_rgb565_2((unsigned char *)yuv420pBuf, bmpBuf, imageWidth, imageHeigth);
    
	Bmp2Jpg(bmpBuf, imageWidth*imageHeigth*3*4, jpgFilePath, imageWidth, imageHeigth);

    free(bmpBuf);
		
	return 0;
}

#define FILE_NAME   "1.yuv"
#define FILE_NEW    "2.yuv"
#define FILE_JPEG   "1.jpg"

int main()
{
    int ret = -1;
    int yuvSize = CIF_HEIGHT * CIF_WIDTH * 3 / 2;

    char *yuv420spBuf = (char *)malloc(yuvSize);
    if (!yuv420spBuf)
    {
         TRACE("> malloc buffer fail. %s %d\r\n", MDL);
         return -1;
    }

    memset(yuv420spBuf, 0, yuvSize);
    ret = CopyFileToBuf(yuv420spBuf, yuvSize, (char *)FILE_NAME);
    if (0 != ret)
    {
        TRACE("> read file fail. %s %d\r\n", MDL);
        free(yuv420spBuf);
        return -1;    
    }

#if 1
    char *yuv420pBuf = (char *)malloc(yuvSize);
    if (!yuv420pBuf)
    {
        free(yuv420spBuf);
        TRACE("> malloc buffer fail. %s %d\r\n", MDL);
        return -1;
    }

    memset(yuv420pBuf, 0, yuvSize);
    YUV420SP_NV21_TO_YUV420P(yuv420spBuf, yuv420pBuf, CIF_WIDTH, CIF_HEIGHT);
    
    free(yuv420spBuf);
    yuv420spBuf = NULL;

#if 0
    ret = CopyBufToFile(yuv420pBuf, yuvSize, (char *)FILE_NEW);
    if (0 != ret)
    {
        TRACE("> save yuv420p to file fail. %s %d\r\n", MDL);
        free(yuv420pBuf);
        yuv420pBuf = NULL;
        return -1;    
    }
#endif
#endif

    CreateJpgByYuv420P(yuv420pBuf, (char *)FILE_JPEG);
    
    free(yuv420pBuf);
    
    return 0;
}

