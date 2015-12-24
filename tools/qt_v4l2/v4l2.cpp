
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "v4l2.h"

v4l2::v4l2(QObject *parent) :
    QObject(parent)
{
    fd = -1;
    user_buf = NULL;
    n_buffer = 0;
    n_index = 0;
    fmt_index = 0;
    memset(&fmt_desc, 0, MAX_TYPE * TYPE_LEN * sizeof(char));
}

int v4l2::v4l2_open(char *device)
{
    if (device == NULL)
    {
        return -1;
    }

    int fd;

    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        perror("open error.");
        return -1;
    }

    return fd;
}

int v4l2::v4l2_close(int open_fd)
{
    if (-1 == close(open_fd))
    {
        perror("Fail to close fd");
        return -1;
    }

    fd = -1;

    return 0;
}

unsigned int v4l2::v4l2_get_streamparam(int fd)
{
    int ret = -1;
    unsigned int fps = 0;
    struct v4l2_streamparm param;

    memset(&param, 0, sizeof(param));
    param.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl(fd, VIDIOC_G_PARM, &param);
    if (ret < 0)
    {
        perror("Fail to VIDIOC_G_PARM");
        return -1;
    }

    fps = (param.parm.capture.timeperframe.denominator)/(param.parm.capture.timeperframe.numerator);
    return fps;
}

void v4l2::v4l2_enum_fmt(int fd)
{
    int ret = -1;
    struct v4l2_fmtdesc fmt;

    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == 0)
    {
        fmt.index++;
        printf("{pixelformat = %c%c%c%c},description = '%s'\n",
            fmt.pixelformat & 0xff,
            (fmt.pixelformat >> 8)  & 0xff,
            (fmt.pixelformat >> 16) & 0xff,
            (fmt.pixelformat >> 24) & 0xff,
            fmt.description);

        memcpy(fmt_desc[fmt_index++], fmt.description, TYPE_LEN - 1);
    }
}

int v4l2::v4l2_set_fmt(int fd, const char *fmt)
{
    struct v4l2_format stream_fmt;

    //ÉèÖÃÉãÏñÍ·ÊÓÆµ²É¼¯¸ñÊ½£¬ÈçÉèÖÃ²É¼¯Êý¾ÝµÄ³¤¿í£¬Í¼Ïñ¸ñÊ½(JPEG, YUYV, MJPEGµÈ¸ñÊ½)
    stream_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_fmt.fmt.pix.width = 640;
    stream_fmt.fmt.pix.height = 480;

    if (strcmp(fmt, YUV422) == 0)
    {
        stream_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;     //thinkpad t400
    }
    else if (strcmp(fmt, MJPEG) == 0)
    {
        stream_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;     //thinkpad t420
    }
    else
    {
        stream_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;     //thinkpad t400
    }

    stream_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if(-1 == ioctl(fd, VIDIOC_S_FMT, &stream_fmt))
    {
        perror("VIDIOC_S_FMT ioctl fail.");
        return -1;
    }

    return 0;
}

int v4l2::v4l2_query_capability(int fd)
{
    struct v4l2_capability cap;
    int ret = -1;

    //ÊÓÆµÉè±¸Çý¶¯¹¦ÄÜ
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if(ret < 0)
    {
        perror("FAIL to ioctl VIDIOC_QUERYCAP");
        return -1;
    }

    //ÅÐ¶ÏÊÇ·ñÎªÒ»¸öÊÓÆµ²¶»ñÉè±¸
    if(!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE))
    {
        printf("The Current device is not a video capture device\n");
        return -1;
    }

    //ÅÐ¶ÏÊÇ·ñÖ§³ÖÊÓÆµÁ÷¸ñÊ½
    if(!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        printf("The Current device does not support streaming i/o\n");
        return -1;
    }

    return 0;
}

int v4l2::v4l2_init_mmap(int fd)
{
    int i = 0;
    struct v4l2_requestbuffers reqbuf;

    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.count = 4;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    /* ÉêÇëÊÓÆµ»º³åÇø */
    if(-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbuf))
    {
        perror("Fail to ioctl 'VIDIOC_REQBUFS'");
        return -1;
    }

    n_buffer = reqbuf.count;
    printf("n_buffer = %d\n", n_buffer);

    user_buf = (buffer *)calloc(reqbuf.count, sizeof(*user_buf));
    if(user_buf == NULL)
    {
        fprintf(stderr,"Out of memory\n");
        return -1;
    }

    //½«ÄÚºË»º³åÇøÓ³Éäµ½ÓÃ»§½ø³Ì¿Õ¼ä
    for(i = 0; i < reqbuf.count; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        //²éÑ¯ÉêÇëµ½µÄÄÚºË»º³åÇøÐÅÏ¢
        if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            perror("Fail to ioctl : VIDIOC_QUERYBUF");
            return -1;
        }

        user_buf[i].length = buf.length;
        user_buf[i].start = mmap(
                    NULL,/*start anywhere*/
                    buf.length,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    fd,
                    buf.m.offset);

        if(MAP_FAILED == user_buf[i].start)
        {
            perror("Fail to mmap");
            return -1;
        }
    }

    return 0;
}

int v4l2::v4l2_start_capturing(int fd)
{
    unsigned int i;
    enum v4l2_buf_type type;

    //½«ÉêÇëµÄÄÚºË»º³åÇø·ÅÈë¶ÓÁÐ
    for(i = 0; i < n_buffer; i ++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            perror("Fail to ioctl 'VIDIOC_QBUF'");
            return -1;
        }
    }

    //¿ªÊ¼Êý¾Ý²É¼¯
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == ioctl(fd, VIDIOC_STREAMON, &type))
    {
        printf("i = %d.\n", i);
        perror("Fail to ioctl 'VIDIOC_STREAMON'");
        return -1;
    }

    return 0;
}

int v4l2::v4l2_read_frame(int fd)
{
    struct v4l2_buffer buf;

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    //´Ó¶ÓÁÐÖÐÈ¡»º³å
    if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("Fail to ioctl 'VIDIOC_DQBUF'");
        return -1;
    }

    assert(buf.index < n_buffer);

    n_index = buf.index;

    //¶ÁÈ¡½ø³Ì¿Õ¼äÖÐÊý¾Ýµ½ÎÄ¼þ
    //v4l2_process_image(user_buf[buf.index].start, user_buf[buf.index].length, name, V4L2_PIX_FMT_YUYV);

    //½«»º´æÔÚ´Î·ÅÈë¶ÓÁÐ
    if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Fail to ioctl 'VIDIOC_QBUF'");
        return -1;
    }

    return 0;
}

//
void v4l2::v4l2_stop_capturing(int fd)
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
        return;
    }

    return;
}

void v4l2::v4l2_uninit_device(int fd)
{
    unsigned int i;

    for(i = 0; i < n_buffer; i++)
    {
        if(-1 == munmap(user_buf[i].start, user_buf[i].length))
        {
            return;
        }
    }

    free(user_buf);

    return;
}

int v4l2::v4l2_save_image(void *buffer, int length, char *name)
{
    FILE *fp;

    if((fp = fopen(name, "w")) == NULL)
    {
        perror("Fail to fopen");
        return -1;
    }

    fwrite(buffer, length, 1, fp);

    usleep(500);
    fclose(fp);

    return 0;
}
