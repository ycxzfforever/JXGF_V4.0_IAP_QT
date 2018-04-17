#-------------------------------------------------
#
# Project created by QtCreator 2017-01-12T17:02:44
#
#-------------------------------------------------

QT       += core gui network serialport
RC_FILE  += version.rc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JXGF40_IAP
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    src/bio.cpp \
    src/log.cpp \
    src/network.cpp \
    src/crc.cpp \
    src/serialportthread.cpp

HEADERS  += mainwindow.h \
    include/bio.h \
    include/log.h \
    include/network.h \
    include/common.h \
    include/xmodem.h \
    include/crc.h \
    include/serialportthread.h

FORMS    += mainwindow.ui
