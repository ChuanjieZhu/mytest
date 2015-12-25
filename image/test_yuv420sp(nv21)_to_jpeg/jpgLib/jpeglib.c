
#include <stdio.h>

#include "jpeglib.h"

#ifndef YUV
#define YUV unsigned long
#endif
#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef RGB16
#define RGB16 unsigned short
#endif
#ifndef MAKERGB565
#define MAKERGB565(r,g,b) (RGB16)( (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3) )
#endif

#ifndef YUV_MODE
#define UYVY_MODE 1
#define YUYV_MODE 2
#define YUV_MODE UYVY_MODE
#endif

YUV RGB2YUV(BYTE r, BYTE g, BYTE b)
{
	int y, u, v;

	y = (299*r+587*g+114*b)/1000;
	u = (128000-169*r-331*g+500*b)/1000;
	v = (128000+500*r-419*g-81*b)/1000;

	if(y < 16) y = 16;
	else if(y>235) y = 235;
	if(u < 16) u = 16;
	else if(u>240) u = 240;
	if(v < 16) v = 16;
	else if(v>240) v = 240;

#if YUV_MODE==UYVY_MODE
	return (y<<24)+(v<<16)+(y<<8)+u;
#else
	return (v<<24)+(y<<16)+(u<<8)+y;
#endif
}

void jpeg_error_exit (j_common_ptr cinfo)
{
}

void PutRGBScanLine(unsigned char* jpegline, char* pOutBuf, int width, int components, int depth, int bYuvMode)
{
	int i;
	unsigned char r, g, b;
	YUV* pDst = (YUV*)pOutBuf;
	if((components!=1 && components!=3) || (depth!=16 && depth!=24 && depth!=32)) return;
	for(i=0; i<width; i++) 
	{
		if(components==1) b = g = r = *jpegline++;
		else {
			r = *jpegline++;
			g = *jpegline++;
			b = *jpegline++;
		}
		if(bYuvMode) *pDst++ = RGB2YUV(r, g, b);
		else {
			switch(depth) {
				case 16: // 16BPP
					*((RGB16*)pOutBuf)=MAKERGB565(r,g,b);
					pOutBuf+=sizeof(RGB16);
					break;
				case 24: // 24BPP
				case 32: // 32BPP
					*pOutBuf++ = b;
					*pOutBuf++ = g;
					*pOutBuf++ = r;
					if(depth==32) *pOutBuf++ = 0;
					break;
				default:
					break;
			}
		}
	}
}

int jpeg_compress(char* pFileName, unsigned char* pData, int nWidth, int nHeight, int bColor, int nQuality)
{
	int ret = 0;
	FILE* outfile = fopen(pFileName, "wb"); /* target file */
	if (outfile == NULL) {
		printf("JPEG: Save JPEG Can not open file %s\r\n", pFileName);
		ret = -1;
	} else {
		struct jpeg_compress_struct cinfo;

		struct jpeg_error_mgr emgr;

		/* Step 1: allocate and initialize JPEG compression object */
		cinfo.err = jpeg_std_error(&emgr);
		emgr.error_exit = jpeg_error_exit;

		/* Now we can initialize the JPEG compression object. */
		jpeg_create_compress(&cinfo);

		/* Step 2: specify data destination (eg, a file) */
		jpeg_stdio_dest(&cinfo, outfile);

		/* Step 3: set parameters for compression */
													    
		/* First we supply a description of the input image.
		* Four fields of the cinfo struct must be filled in:
		*/
		cinfo.image_width = nWidth; 	/* image widthPix and height, in pixels */
		cinfo.image_height = nHeight;
		if (bColor) {
			cinfo.input_components = 3;		/* # of color components per pixel */
			cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
		} else {
			cinfo.input_components = 1;		/* # of color components per pixel */
			cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
		}

		/* Now use the library's routine to set default compression parameters.
		* (You must set at least cinfo.in_color_space before calling this,
		* since the defaults depend on the source color space.)
		*/

		jpeg_set_defaults(&cinfo);

		/* Now you can set any non-default parameters you wish to.
		* Here we just illustrate the use of quality (quantization table) scaling:
		*/
		jpeg_set_quality(&cinfo, nQuality, TRUE /* limit to baseline-JPEG values */);

		/* Step 4: Start compressor */

		/* TRUE ensures that we will write a complete interchange-JPEG file.
		* Pass TRUE unless you are very sure of what you're doing.
		*/

		jpeg_start_compress(&cinfo, TRUE);

		/* Step 5: while (scan lines remain to be written) */
		/*           jpeg_write_scanlines(...); */

		/* Here we use the library's state variable cinfo.next_scanline as the
		* loop counter, so that we don't have to keep track ourselves.
		* To keep things simple, we pass one scanline per call; you can pass
		* more if you wish, though.
		*/

		while (cinfo.next_scanline < cinfo.image_height) {
			/* jpeg_write_scanlines expects an array of pointers to scanlines.
			 * Here the array is only one element long, but you could pass
			 * more than one scanline at a time if that's more convenient.
			 */
			jpeg_write_scanlines(&cinfo, &pData, 1);
			if(bColor) pData += nWidth * 3;
			else pData += nWidth;
		}

		/* Step 6: Finish compression */
		jpeg_finish_compress(&cinfo);

		/* Step 7: release JPEG compression object */

		/* This is an important step since it will release a good deal of memory. */
		jpeg_destroy_compress(&cinfo);

		/* After finish_compress, we can close the output file. */
		fflush(outfile);
		fclose(outfile);
	}
	return ret;
}

int jpeg_decompress(char* pFileName, char* pBuffer, int* pnWidth, int* pnHeight, int* pnComponents, int nDepth, int bYuvMode)
{
	int ret = -1;
	FILE* infile;
	if(pFileName==NULL || pnWidth==NULL || pnHeight==NULL) return ret;
	infile=fopen(pFileName, "rb"); /* source file */
	if (infile == NULL) {
		printf("JPEG : Can't open jpeg file %s\r\n", pFileName);
	} else {
		/* This struct contains the JPEG decompression parameters and pointers to
		* working space (which is allocated as needed by the JPEG library).
		*/
		struct jpeg_decompress_struct cinfo;
		/* We use our private extension JPEG error handler.
		* Note that this struct must live as long as the main JPEG parameter
		* struct, to avoid dangling-pointer problems.
		*/
		struct jpeg_error_mgr emgr;

		JSAMPARRAY buffer = NULL;		/* Output row buffer */
		int row_stride;		/* physical row width in output buffer */
		
		/* Step 1: allocate and initialize JPEG decompression object */

		/* We set up the normal JPEG error routines, then override error_exit. */
		
		cinfo.err = jpeg_std_error(&emgr);
		emgr.error_exit = jpeg_error_exit;

		/* Now we can initialize the JPEG decompression object. */
		jpeg_create_decompress(&cinfo);

		/* Step 2: specify data source (eg, a file) */
		jpeg_stdio_src(&cinfo, infile);

		/* Step 3: read file parameters with jpeg_read_header() */
		jpeg_read_header(&cinfo, TRUE);
		/* We can ignore the return value from jpeg_read_header since
		*   (a) suspension is not possible with the stdio data source, and
		*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
		* See libjpeg.doc for more info.
		*/

		/* Step 4: set parameters for decompression */

		/* In this example, we don't need to change any of the defaults set by
		* jpeg_read_header(), so we do nothing here.
		*/

		/* Step 5: Start decompressor */
		jpeg_start_decompress(&cinfo);
		/* We can ignore the return value since suspension is not possible
		* with the stdio data source.
		*/

		if(cinfo.output_width && cinfo.output_height && cinfo.output_components) {
			ret = 0;
			
			*pnWidth = cinfo.output_width;
			*pnHeight = cinfo.output_height;
			if(pnComponents) *pnComponents = cinfo.output_components;

			if(pBuffer) {			
				/* JSAMPLEs per row in output buffer */
				row_stride = cinfo.output_width * cinfo.output_components;

				/* Make a one-row-high sample array that will go away when done with image */
				buffer = (*cinfo.mem->alloc_sarray)
					((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

				if(buffer==NULL) {
					printf("JPEG: alloc_sarray buffer fail\r\n");
				}  else {
					/* We may need to do some setup of our own at this point before reading
					* the data.  After jpeg_start_decompress() we have the correct scaled
					* output image dimensions available, as well as the output colormap
					* if we asked for color quantization.
					* In this example, we need to make an output work buffer of the right size.
					*/ 
					/* Step 6: while (scan lines remain to be read) */
					/*           jpeg_read_scanlines(...); */
					while (cinfo.output_scanline < cinfo.output_height) {
						/* jpeg_read_scanlines expects an array of pointers to scanlines.
						 * Here the array is only one element long, but you could ask for
						 * more than one scanline at a time if that's more convenient.
						 */
						jpeg_read_scanlines(&cinfo, buffer, 1);
						/* Assume put_scanline_someplace wants a pointer and sample count. */

						// asuumer all 3-components are RGBs
						PutRGBScanLine(buffer[0], pBuffer+(cinfo.output_scanline-1)*cinfo.output_width*nDepth/8, cinfo.output_width, cinfo.out_color_components, nDepth, bYuvMode);
					}
				}
			}
		}

		/* Step 7: Finish decompression */

		jpeg_finish_decompress(&cinfo);
		/* We can ignore the return value since suspension is not possible
		* with the stdio data source.
		*/

		/* Step 8: Release JPEG decompression object */

		/* This is an important step since it will release a good deal of memory. */
		jpeg_destroy_decompress(&cinfo);

		/* After finish_decompress, we can close the input file.
		* Here we postpone it until after no more JPEG errors are possible,
		* so as to simplify the setjmp error logic above.  (Actually, I don't
		* think that jpeg_destroy can do an error exit, but why assume anything...)
		*/
		fclose(infile);
	}

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/
	return ret;
}

#if 0
int jpeg_compress_ex(char* pJpgBuf, unsigned char* pData, int nWidth, int nHeight, int bColor, int nQuality)
{
	int ret = 0;
	{
		struct jpeg_compress_struct cinfo;

		struct jpeg_error_mgr emgr;

		/* Step 1: allocate and initialize JPEG compression object */
		cinfo.err = jpeg_std_error(&emgr);
		emgr.error_exit = jpeg_error_exit;

		/* Now we can initialize the JPEG compression object. */
		jpeg_create_compress(&cinfo);

		/* Step 2: specify data destination (eg, a file) */
		jpeg_memory_dest(&cinfo, pJpgBuf);

		/* Step 3: set parameters for compression */
													    
		/* First we supply a description of the input image.
		* Four fields of the cinfo struct must be filled in:
		*/
		cinfo.image_width = nWidth; 	/* image widthPix and height, in pixels */
		cinfo.image_height = nHeight;
		if (bColor) {
			cinfo.input_components = 3;		/* # of color components per pixel */
			cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
		} else {
			cinfo.input_components = 1;		/* # of color components per pixel */
			cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
		}

		/* Now use the library's routine to set default compression parameters.
		* (You must set at least cinfo.in_color_space before calling this,
		* since the defaults depend on the source color space.)
		*/

		jpeg_set_defaults(&cinfo);

		/* Now you can set any non-default parameters you wish to.
		* Here we just illustrate the use of quality (quantization table) scaling:
		*/
		jpeg_set_quality(&cinfo, nQuality, TRUE /* limit to baseline-JPEG values */);

		/* Step 4: Start compressor */

		/* TRUE ensures that we will write a complete interchange-JPEG file.
		* Pass TRUE unless you are very sure of what you're doing.
		*/

		jpeg_start_compress(&cinfo, TRUE);

		/* Step 5: while (scan lines remain to be written) */
		/*           jpeg_write_scanlines(...); */

		/* Here we use the library's state variable cinfo.next_scanline as the
		* loop counter, so that we don't have to keep track ourselves.
		* To keep things simple, we pass one scanline per call; you can pass
		* more if you wish, though.
		*/

		while (cinfo.next_scanline < cinfo.image_height) {
			/* jpeg_write_scanlines expects an array of pointers to scanlines.
			 * Here the array is only one element long, but you could pass
			 * more than one scanline at a time if that's more convenient.
			 */
			jpeg_write_scanlines(&cinfo, &pData, 1);
			if(bColor) pData += nWidth * 3;
			else pData += nWidth;
		}

		/* Step 6: Finish compression */
		jpeg_finish_compress(&cinfo);

		/* Step 7: release JPEG compression object */

		/* This is an important step since it will release a good deal of memory. */
		jpeg_destroy_compress(&cinfo);
		
		ret = jpeg_memory_size();
	}
	return ret;
}
#endif

