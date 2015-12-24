#ifndef SENDVIDEO_H
#define SENDVIDEO_H

#include <QObject>
#include <QUdpSocket>

class QLabel;
class QLineEidt;
class QString;

class sendVideo : public QObject
{
    Q_OBJECT
public:
    explicit sendVideo(QObject *parent = 0);
    ~sendVideo();
    void setHostAddr(char *hostIp);
    void setPort(int port);
signals:

public slots:
    void sendDatagram(QString data, int length);

private:
    QUdpSocket udpSocket;
    QHostAddress *hostAddr;
    int hostPort;
};

#endif // SENDVIDEO_H
