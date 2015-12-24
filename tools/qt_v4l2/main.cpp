#include "qv4l2video.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qv4l2video w;
    w.show();

    return a.exec();
}
