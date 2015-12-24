#include "qv4l2video.h"
#include <QtCore>
#include <QtGui>
#include <QDebug>

#include <unistd.h>
#include <stdlib.h>

qv4l2video::qv4l2video(QWidget *parent)
    : QWidget(parent)
{
    startButton = new QPushButton(tr("start"));
    stopButton = new QPushButton(tr("stop"));
    stopButton->setEnabled(false);
    snapButton = new QPushButton(tr("snap"));
    snapButton->setEnabled(false);
    fmtBox = new QComboBox;
    videoLabel = new QLabel(tr(""));
    frameLable = new QLabel(tr("frame: 0"));
    fpsLabel = new QLabel(tr("fps: 0"));
    fmtLabel = new QLabel(tr("fmt: "));
    ipLabel = new QLabel(tr("ip: "));
    portLabel = new QLabel(tr("port: "));
    ipLineEdit = new QLineEdit(tr("127.0.0.1"));
    portLineEdit = new QLineEdit(tr("45000"));

    QHBoxLayout *hTopLayout = new QHBoxLayout;
    hTopLayout->addWidget(startButton);
    //hTopLayout->addSpacing(10);
    hTopLayout->addWidget(stopButton);
    //hTopLayout->addSpacing(10);
    hTopLayout->addWidget(snapButton);
    //hTopLayout->addSpacing(10);
    hTopLayout->addWidget(frameLable);
    //hTopLayout->addSpacing(10);
    hTopLayout->addWidget(fpsLabel);
    //hTopLayout->addSpacing(10);
    hTopLayout->addWidget(fmtLabel);
    //hTopLayout->addSpacing(10);
    hTopLayout->addWidget(fmtBox);
    //hTopLayout->addSpacing(10);
    hTopLayout->addWidget(ipLabel);
    hTopLayout->addWidget(ipLineEdit);
    hTopLayout->addWidget(portLabel);
    hTopLayout->addWidget(portLineEdit);
    hTopLayout->addStretch();

    //hTopLayout->SetFixedSize();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hTopLayout);
    vLayout->addWidget(videoLabel);
    vLayout->setStretch(0, 1);
    vLayout->setStretch(1, 12);

    setLayout(vLayout);

    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClick()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClick()));
    connect(snapButton, SIGNAL(clicked()), this, SLOT(snapButtonClick()));

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(flush()));
    timer->start(50);

    sender = new sendVideo;
    cimage = new image;
    cv4l2 = new v4l2;

    cv4l2->fd = cv4l2->v4l2_open((char *)VIDEO_DEVICE);
    if (cv4l2->fd < 0)
    {
        qDebug() << "fd = " << cv4l2->fd;
        QMessageBox::critical(this, tr("error"),
                              tr("capture device init fail."),
                              QMessageBox::Ok);
        sleep(1);
    }

    cv4l2->fps = cv4l2->v4l2_get_streamparam(cv4l2->fd);
    //qDebug() << "fps: " << cv4l2->fps;

    cv4l2->v4l2_enum_fmt(cv4l2->fd);

    for (int i = 0; i < cv4l2->fmt_index; i++)
    {
        fmtBox->addItem(QString(QLatin1String(cv4l2->fmt_desc[i])));
    }

    cv4l2->v4l2_query_capability(cv4l2->fd);

    int index = fmtBox->currentIndex();
    cv4l2->v4l2_set_fmt(cv4l2->fd, cv4l2->fmt_desc[index]);
    cv4l2->v4l2_init_mmap(cv4l2->fd);

    if (strcmp(cv4l2->fmt_desc[index], MJPEG) == 0)
    {
        snapButton->setEnabled(true);
    }

    first_start = true;
    framecount = 0;
    start = false;
    width = 640;
    height = 480;

    buffer_len = width * height * 3;
    rgb_buffer = (unsigned char *)malloc(buffer_len * sizeof(char));
    if (rgb_buffer == NULL)
    {
        qDebug() << "malloc fail.";
    }

    frame = new QImage(rgb_buffer, 640, 480, QImage::Format_RGB888);

    pixmap = new QPixmap(width, height);
    videoLabel->setPixmap(*pixmap);

    resize(QSize(660, 500));
    setMaximumSize(660, 500);
    setWindowTitle(tr("Video"));
}

qv4l2video::~qv4l2video()
{
    if (rgb_buffer)
        free(rgb_buffer);
    if (cv4l2->fd > 0)
    {
        cv4l2->v4l2_stop_capturing(cv4l2->fd);
        cv4l2->v4l2_uninit_device(cv4l2->fd);
        cv4l2->v4l2_close(cv4l2->fd);
    }
    delete cv4l2;
    delete cimage;
    delete sender;
}

void qv4l2video::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //painter.begin(this);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    //painter.end();
}

void qv4l2video::startButtonClick()
{
    int index = fmtBox->currentIndex();
    if (first_start == true)
    {
        cv4l2->v4l2_set_fmt(cv4l2->fd, cv4l2->fmt_desc[index]);
    }
    else
    {
        cv4l2->fd = cv4l2->v4l2_open((char *)VIDEO_DEVICE);
        cv4l2->v4l2_set_fmt(cv4l2->fd, cv4l2->fmt_desc[index]);
        cv4l2->v4l2_init_mmap(cv4l2->fd);
    }

    memset(start_fmt, 0, sizeof(start_fmt));
    memcpy(start_fmt, cv4l2->fmt_desc[index], sizeof(start_fmt) - 1);

    sender->setHostAddr(ipLineEdit->text().toLocal8Bit().data());
    sender->setPort(portLineEdit->text().toInt());

    if (0 == cv4l2->v4l2_start_capturing(cv4l2->fd))
    {
        fmtBox->setEnabled(false);
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        ipLineEdit->setEnabled(false);
        portLineEdit->setEnabled(false);

        if (strcmp(cv4l2->fmt_desc[index], MJPEG) == 0)
        {
            snapButton->setEnabled(true);
        }
        else
        {
            snapButton->setEnabled(false);
        }

        start = true;
    }
}

void qv4l2video::stopButtonClick()
{
    if (cv4l2->fd > 0)
    {
        cv4l2->v4l2_stop_capturing(cv4l2->fd);
        usleep(200);
        cv4l2->v4l2_uninit_device(cv4l2->fd);
        usleep(200);
        cv4l2->v4l2_close(cv4l2->fd);
    }

    startButton->setEnabled(true);
    fmtBox->setEnabled(true);
    stopButton->setEnabled(false);
    snapButton->setEnabled(false);
    ipLineEdit->setEnabled(true);
    portLineEdit->setEnabled(true);

    start = false;
    first_start = false;
}

void qv4l2video::snapButtonClick()
{
    if (start == false)
    {
        QMessageBox::critical(this, tr("error"),
                              tr("error, camera device not open"),
                              QMessageBox::Ok);
        return;
    }

    int ret = -1;
    char name[32] = {0};
    time_t tt = time(NULL);
    snprintf(name, sizeof(name) - 1, "snap_%u", tt);
    if (0 == cv4l2->v4l2_read_frame(cv4l2->fd))
    {
        ret = cv4l2->v4l2_save_image((unsigned char *)cv4l2->user_buf[cv4l2->n_index].start,
                cv4l2->user_buf[cv4l2->n_index].length, name);
        if (ret != 0)
        {
            qDebug() << "v4l2_save_image fail.";
        }
    }
}

void qv4l2video::flush()
{
    if (start == false)
    {
        sleep(1);
        return;
    }

    cv4l2->fps = cv4l2->v4l2_get_streamparam(cv4l2->fd);
    //qDebug() << "fps: " << cv4l2->fps;
    QString fps = "fps: " + QString::number(cv4l2->fps);
    fpsLabel->setText(fps);

    if (0 == cv4l2->v4l2_read_frame(cv4l2->fd))
    {
        framecount++;
        QString text = "frame: " + QString::number(framecount);
        frameLable->setText(text);

        //qDebug() << "start_fmt " << start_fmt;

        if (strcmp(start_fmt, YUV422) == 0)
        {
            memset(rgb_buffer, 0, buffer_len);

            //cimage->yuv_to_rgb_buffer((unsigned char *)cv4l2->user_buf[cv4l2->n_index].start,
            //        rgb_buffer, width, height);

            /* 查表法快速yuv转rgb */
            cimage->yuv2rgb((unsigned char *)cv4l2->user_buf[cv4l2->n_index].start,
                            rgb_buffer, width * height * 2);

            /* 水平方向图象镜像 */
            cimage->mirrorRGB(rgb_buffer, width, height, 24, 1);

            //sender->sendDatagram(QString(QLatin1String((char *)rgb_buffer)), buffer_len);

            frame->loadFromData((unsigned char *)rgb_buffer, buffer_len);
            videoLabel->setPixmap(QPixmap::fromImage(*frame, Qt::AutoColor));
        }
        else if (strcmp(start_fmt, MJPEG) == 0)
        {
            sender->sendDatagram(QString(QLatin1String((char *)cv4l2->user_buf[cv4l2->n_index].start)),
                    cv4l2->user_buf[cv4l2->n_index].length);
            frame->loadFromData((unsigned char *)cv4l2->user_buf[cv4l2->n_index].start,
                    cv4l2->user_buf[cv4l2->n_index].length);
            videoLabel->setPixmap(QPixmap::fromImage(*frame, Qt::AutoColor));
        }
    }
}

void qv4l2video::showPic(QString pic)
{
    QImage image(pic);
    videoLabel->setPixmap(QPixmap::fromImage(image));
}

void qv4l2video::showPicData(unsigned char *buffer)
{
    pixmap->loadFromData(buffer,7000,"jpg");
}
