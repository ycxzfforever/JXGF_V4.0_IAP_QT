#include "../include/common.h"

/*************************************************
Function: formatString()
Description: 将十六进制字串每字节中间加空格分隔
Calls:
Called By:hexStringtoByteArray()
Input: org->待处理的字串
	   n->间隔数默认为2
	   ch->分隔标志，在此取空格
Output: NULL
Return: void
Others: NULL
*************************************************/
void formatString(QString &org, int n=2, const QChar &ch=QChar(' '))
{
	int size= org.size();
	int space= qRound(size*1.0/n+0.5)-1;
	if(space<=0)
		return;
	for(int i=0,pos=n;i<space;++i,pos+=(n+1))
	{
		org.insert(pos,ch);
	}
}

//将一个字符串数组转换成BCD字符串,与下面函数StringToHex()一样的功能
QByteArray hexStringtoByteArray(QString hex)
{
    QByteArray ret;
    hex=hex.trimmed();
    formatString(hex,2,' ');
    QStringList sl=hex.split(" ");
    foreach(QString s,sl)
    {
        if(!s.isEmpty())
            ret.append((char)s.toInt(0,16)&0xFF);
    }
    return ret;
}

/********************************************************************
* 函数名:GetFileSize
* 说明:
* 功能:
* 输入:FileName:文件名
* 返回值:文件大小
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
qint64 GetFileSize(QString FileName)
{
	QFile file(FileName);
	return file.size();
}

/********************************************************************
* 函数名:GetFolderSize
* 说明:
* 功能:类似于linux下面du命令，查看文件夹大小
* 输入:path：文件夹路径
* 返回值:文件夹大小
* 创建人:Yang Chao Xu
* 创建时间:2015-10-14
*********************************************************************/
qint64 GetFolderSize(const QString &path)
{
	QDir dir(path);//QDir类具有存取目录结构和内容的能力
	qint64 size = 0;

	//entryInfoList(Qdir::Files)函数返回文件信息，根据这些信息计算文件的大小
	foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
		size += fileInfo.size();
	// 判断是否有子目录,如果有，则递归计算
	//entryList(QDir::Dirs | QDir::NoDotAndDotDot 返回所有子目录并过滤掉
	// '.' '..'目录
	//使用QDir::separator() 函数来返回特定平台的目录分割符
	foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
		size += GetFolderSize(path + QDir::separator() + subDir);

	//	char unit = 'B';
	//	quint64 curSize = size;
	//	if(curSize > 1024) {
	//			curSize /= 1024;
	//			unit = 'K';
	//			if(curSize > 1024) {
	//					curSize /= 1024;
	//					unit = 'M';
	//					if(curSize > 1024) {
	//							curSize /= 1024;
	//							unit = 'G';
	//					}
	//			}
	//	}
	//	qDebug() << curSize << unit << "\t" << qPrintable(path) << endl;
	return size;
}

/********************************************************************
* 函数名:PressFolder
* 说明:
* 功能:调用windows下的winrar来压缩文件夹
* 输入:FolderName:文件夹名字
* 返回值:NULL
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
void PressFolder(QString FolderName)
{
	QProcess pro(0);
	QString t1=WINRAR_PATH;
	QStringList t2;
	//t2.append("a");//追加，不删除源文件夹
	t2.append("m");//压缩完成后，删除源文件夹
	t2.append("-epl");
	t2.append("-r");
	t2.append("-ibck");
	t2.append("-o+");
	t2.append(FolderName+".rar");
	t2.append(FolderName);
	pro.execute(t1,t2);
}


QString ByteArrayToString(QByteArray ByteArray,QChar ch)
{
	QString str,t;
	char tmp[4];
	for(int i=0;i<ByteArray.size();i++){
		sprintf(tmp,"%02X",(uchar)ByteArray.at(i));
		str.append(tmp);
		if(i!=ByteArray.size()-1)
			str.append(ch);
	}
	t=QObject::tr(" Len=%1").arg(ByteArray.size());
	str.append(t);
	return str;
}
