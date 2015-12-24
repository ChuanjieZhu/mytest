#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>

class image : public QObject
{
    Q_OBJECT
public:
    explicit image(QObject *parent = 0);
    int yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
    int yuv_to_rgb_pixel(int y, int u, int v);

    void yuv2rgb(unsigned char *buf_yuv, unsigned char *buf_rgb, int yuv_size);
    void mirrorRGB(unsigned char* pRGB, int nWidth, int nHeight, int nBits, int nHorizontal);

private:
    unsigned char clip(int val);

signals:

public slots:

};

#endif // IMAGE_H
