#include "sendvideo.h"

#include <QtCore>

sendVideo::sendVideo(QObject *parent) :
    QObject(parent)
{
    hostAddr = new QHostAddress("127.0.0.1");
    hostPort = 45000;
}

sendVideo::~sendVideo()
{
    delete hostAddr;
}

void sendVideo::sendDatagram(QString data, int length)
{
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);
    out << length << data;
    udpSocket.writeDatagram(datagram, *hostAddr, hostPort);
}

void sendVideo::setHostAddr(char *hostIp)
{
    hostAddr->setAddress(hostIp);
}

void sendVideo::setPort(int port)
{
    hostPort = port;
}
