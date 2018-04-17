#ifndef COMMON
#define COMMON

#define		Debug

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QProcess>
#include <QCloseEvent>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QVBoxLayout>
#include "network.h"
#include "log.h"
#include "bio.h"
#include "../mainwindow.h"
#include "xmodem.h"
#include "crc.h"
#include "serialportthread.h"

#endif // COMMON
