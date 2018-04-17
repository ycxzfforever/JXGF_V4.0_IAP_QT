#include "mainwindow.h"
#include <QApplication>
#include "include/common.h"
#define VERSION "V1.0.1"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(MessageOutput);
    MainWindow w;
    w.setWindowTitle(QObject::tr("金星股份4.0主板下载器%1").arg(VERSION));
    w.show();
    return a.exec();
}
