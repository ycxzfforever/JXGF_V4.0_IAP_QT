#include "../include/common.h"

/********************************************************************
* 函数名:DirFileListXml
* 说明:
* 功能:对目录下的文件排序
* 输入:	xmlName：文件类型（后缀名）
*			Path：路径
* 返回值:QStringList
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
QStringList DirFileListXml(QString xmlName,QString Path)
{
	QStringList fileList;
	QDir dir(Path);
	if (!dir.exists()) return fileList;
	dir.setFilter(QDir::Dirs|QDir::Files);
	dir.setSorting(QDir::DirsFirst|QDir::Time |QDir::Reversed);//排序方式 修改时间从小到大,文件夹在前
	QFileInfoList list = dir.entryInfoList();
	int i=0,filecont=0;
	do{
		QFileInfo fileInfo = list.at(i);
		if(fileInfo.fileName() == "." || fileInfo.fileName()== "..")
		{
			qDebug()<<"filedir="<<fileInfo.fileName();
			i++;
			continue;
		}
		bool bisDir=fileInfo.isDir();
		if(!bisDir)
		{
			QString currentFileName=fileInfo.fileName();
			bool Reght=currentFileName.endsWith(xmlName, Qt::CaseInsensitive);
			if(Reght)
			{
				fileList<<currentFileName;
				qDebug()<<"filelist sort="<<currentFileName;
				filecont++;
			}
		}
		i++;
	}while(i<list.size());
	return fileList;
}

/********************************************************************
* 函数名:DirFileListXml
* 说明:
* 功能:删除目录下面最旧的文件
* 输入:	NameSuffix：文件类型（后缀名）
*			Path：路径
* 返回值:NULL
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
void DeleteOldFile(QString NameSuffix,QString Path)
{
	QDir dir(Path);
	if (!dir.exists()) 	return;
	dir.setFilter(QDir::Dirs|QDir::Files);
	dir.setSorting(QDir::DirsFirst|QDir::Time |QDir::Reversed);//排序方式 修改时间从小到大,文件夹在前
	QFileInfoList list = dir.entryInfoList();
	int i=0;
	do{
		QFileInfo fileInfo = list.at(i);
		bool bisDir=fileInfo.isDir();
		if((fileInfo.fileName() != ".") &&(fileInfo.fileName()!= "..") &&(!bisDir))
		{
			QString currentFileName=fileInfo.fileName();
			bool Reght=currentFileName.endsWith(NameSuffix, Qt::CaseInsensitive);
			if(Reght)
			{
				dir.remove(currentFileName);
				break;
			}
		}
		i++;
	}while(i<list.size());
}

/********************************************************************
* 函数名:MessageOutput
* 说明:
* 功能:重定向打印信息到日志文件中
* 输入:
* 返回值:NULL
* 创建人:Yang Chao Xu
* 创建时间:2015-10-14
*********************************************************************/
void MessageOutput(QtMsgType type , const QMessageLogContext &context , const QString &msg)
{
	static QMutex mutex;
	mutex.lock();
	QString log,logname;
	switch(type)
	{
		case QtDebugMsg:
			log = QString("Debug: ");
			logname="DebugLog";
		break;
		case QtWarningMsg:
			log = QString("Warning: ");
			logname="WarningLog";
		break;
		case QtCriticalMsg:
			log = QString("Critical Error: ");
			logname="CErrorLog";
		break;
		case QtFatalMsg:
			log = QString("Fatal Error: ");
			logname="FErrorLog";
			abort();
		break;
		default:
			log = QString("Unknow Msg Type : ");
			logname="UnknowLog";
		break;
	};

	//文件夹不存在则创建//
	QDir *tmp= new QDir;
	if(!(tmp->exists(LOGFILE_FOLDERNAME)))
		tmp->mkdir(LOGFILE_FOLDERNAME);
	delete(tmp);

	//判断log文件夹的大小是否大于最大值
	if(GetFolderSize(LOGFILE_FOLDERNAME)>LOGFILE_MAXSIZE)
	{
		if(GetFileSize(LOGFILE_PRESSFOLDERNAME)>LOGFILE_PRESSMAXSIZE)
		{
			QProcess pro(0);
			QString t1="del";
			QStringList t2;
			t2.append(LOGFILE_PRESSFOLDERNAME);
			pro.execute(t1,t2);
		}
		PressFolder(LOGFILE_FOLDERNAME);
		//DeleteOldFile(LOGFILE_SUFFIXNAME,LOGFILE_FOLDERNAME);
	}
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss.zzz ddd ");
	log +=current_date ;
#ifdef	Debug
	log +=QString("File:")+ QString(context.file) ;
	log += QString(" Line:") + QString::number(context.line) ;
#endif
	log +=QString(" %1\n").arg(msg);
	logname+=current_date_time.toString("yyyyMMdd")+"."+LOGFILE_SUFFIXNAME;
	logname.prepend("/");
	logname.prepend(LOGFILE_FOLDERNAME);
	QFile logFile(logname);
	logFile.open(QIODevice::WriteOnly | QIODevice::Append);
	logFile.write(log.toStdString().c_str());
	logFile.flush();
	mutex.unlock();
}
