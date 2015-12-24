#-------------------------------------------------
#
# Project created by QtCreator 2014-03-06T17:24:26
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qv4l2_video
TEMPLATE = app


SOURCES += main.cpp\
        qv4l2video.cpp \
    v4l2.cpp \
    image.cpp \
    sendvideo.cpp

HEADERS  += qv4l2video.h \
    v4l2.h \
    image.h \
    sendvideo.h
