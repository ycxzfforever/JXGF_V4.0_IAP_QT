#ifndef LOG
#define LOG
#include <QtDebug>
#include <QFile>
#include <QTextStream>

#define		LOGFILE_SUFFIXNAME					"log"								//日志文件后缀名
#define		LOGFILE_MAXSIZE							1024*1024*20					//40M
#define		LOGFILE_PRESSMAXSIZE				1024*1024*10					//压缩后的日志文件最大10M
#define		LOGFILE_FOLDERNAME					"log"								//日志文件文件夹名称
#define		LOGFILE_PRESSFOLDERNAME		"log.rar"
#define		WINRAR_PATH										"C:/Program Files (x86)/WinRAR/WinRaR.exe"			//压缩软件winrar安装路径

void MessageOutput(QtMsgType type , const QMessageLogContext &context , const QString &msg);
QStringList DirFileListXml(QString xmlName,QString Path);
void DeleteOldFile(QString NameSuffix,QString Path);
#endif // LOG

