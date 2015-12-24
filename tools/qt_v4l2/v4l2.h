#ifndef V4L2_H
#define V4L2_H

#include <QObject>

#define     MAX_TYPE        10
#define     TYPE_LEN        32
#define     YUV422          "YUV 4:2:2 (YUYV)"
#define     MJPEG           "MJPEG"


class v4l2 : public QObject
{
    Q_OBJECT
public:
    explicit v4l2(QObject *parent = 0);

signals:

public slots:

public:
    int v4l2_open(char *device);
    int v4l2_close(int fd);
    int v4l2_query_capability(int fd);
    unsigned int v4l2_get_streamparam(int fd);
    void v4l2_enum_fmt(int fd);
    int v4l2_set_fmt(int fd, const char *fmt);
    int v4l2_init_mmap(int fd);
    int v4l2_start_capturing(int fd);
    int v4l2_process_image(void *addr, int length, char *name, unsigned int pixelformat);
    int v4l2_read_frame(int fd);
    void v4l2_stop_capturing(int fd);
    void v4l2_uninit_device(int fd);
    int v4l2_save_image(void *buffer, int length, char *name);

public:
    int fd;
    struct buffer
    {
        void *start;
        int length;
    };

    buffer *user_buf;
    unsigned int n_buffer;
    int n_index;
    char fmt_desc[MAX_TYPE][TYPE_LEN];
    int fmt_index;
    unsigned int fps;
};

#endif // V4L2_H
