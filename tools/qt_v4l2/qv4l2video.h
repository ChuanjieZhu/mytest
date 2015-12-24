#ifndef QV4L2VIDEO_H
#define QV4L2VIDEO_H

#include <QWidget>
#include "image.h"
#include "v4l2.h"
#include "sendvideo.h"

#define         VIDEO_DEVICE    "/dev/video0"

class QPushButton;
class QLabel;
class QTimer;
class QPixmap;
class QPaintEvent;
class QString;
class QImage;
class QComboBox;
class QLineEdit;

class qv4l2video : public QWidget
{
    Q_OBJECT

public:
    qv4l2video(QWidget *parent = 0);
    ~qv4l2video();
    void showPic(QString pic);
    void showPicData(unsigned char *);

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void startButtonClick();
    void stopButtonClick();
    void snapButtonClick();
    void flush();

private:
    v4l2 *cv4l2;
    image *cimage;
    QImage *frame;
    sendVideo *sender;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *snapButton;
    QLabel *videoLabel;
    QLabel *frameLable;
    QLabel *fmtLabel;
    QLabel *fpsLabel;
    QLabel *ipLabel;
    QLabel *portLabel;
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QComboBox *fmtBox;
    QTimer *timer;
    QPixmap *pixmap;
    char start_fmt[TYPE_LEN];
    int framecount;
    bool start;
    bool first_start;
    int width;
    int height;
    unsigned char *rgb_buffer;
    int buffer_len;
};

#endif // QV4L2VIDEO_H
