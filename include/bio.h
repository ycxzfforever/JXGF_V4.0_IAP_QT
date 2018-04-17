#ifndef BIO
#define BIO
#include <iostream>
#include <QString>
void formatString(QString &org, int n, const QChar &ch);
qint64 GetFileSize(QString FileName);
qint64 GetFolderSize(const QString &path);
void PressFolder(QString FolderName);
QString ByteArrayToString(QByteArray ByteArray,QChar ch);
QByteArray hexStringtoByteArray(QString hex);
#endif // BIO

