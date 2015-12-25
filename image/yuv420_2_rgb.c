static void YUV420p_to_RGB24(unsigned char *yuv420[3], unsigned char *rgb24, int width, int height) 
{
    int begin = GetTickCount();
    int R,G,B,Y,U,V;
 int x,y;
 int nWidth = width>>1; //色度信号宽度
 for (y=0;y<height;y++)
 {
  for (x=0;x<width;x++)
  {
   Y = *(yuv420[0] + y*width + x);
   U = *(yuv420[1] + ((y>>1)*nWidth) + (x>>1));
   V = *(yuv420[2] + ((y>>1)*nWidth) + (x>>1));
   R = Y + 1.402*(V-128);
   G = Y - 0.34414*(U-128) - 0.71414*(V-128);
   B = Y + 1.772*(U-128);

   //防止越界
   if (R>255)R=255;
   if (R<0)R=0;
   if (G>255)G=255;
   if (G<0)G=0;
   if (B>255)B=255;
   if (B<0)B=0;
   
   *(rgb24 + ((height-y-1)*width + x)*3) = B;
   *(rgb24 + ((height-y-1)*width + x)*3 + 1) = G;
   *(rgb24 + ((height-y-1)*width + x)*3 + 2) = R;
    *(rgb24 + (y*width + x)*3) = B;
    *(rgb24 + (y*width + x)*3 + 1) = G;
    *(rgb24 + (y*width + x)*3 + 2) = R;   
  }
 }
}
