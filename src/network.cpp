#include "../include/common.h"

/********************************************************************
 * 以下为TCP Client相关函数
*********************************************************************/
TcpClient::TcpClient(QString serverip,qint32 serverport)
{
    ServerIP=serverip;
    ServerPort=serverport;
    PackCnt=0;
    PackNumber=0;
    PackSize=1024;
    UseTimes=0;
	tcpClientSocket = new QTcpSocket;
	connectServer(ServerIP,ServerPort);
	connect(tcpClientSocket, SIGNAL(connected()), this, SLOT(connectedSlot()));	
    connect(tcpClientSocket, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
    connect(tcpClientSocket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
	connect(tcpClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(errorSlot(QAbstractSocket::SocketError)));
}

TcpClient::~TcpClient()
{
	tcpClientSocket->close();
	delete tcpClientSocket;
}


void TcpClient::connectServer(QString serverip,qint32 port)
{
	tcpClientSocket->connectToHost(serverip,port);
}

void TcpClient::connectedSlot()
{
	isConnected= true;
    emit isconnectserver();
}

void TcpClient::disconnectedSlot()
{
	isConnected= false;
    tcpClientSocket->close();
    emit isconnectserver();
}

void TcpClient::TcpClientSendData(QByteArray senddata,int len)
{
	if (isConnected)
	{
        tcpClientSocket->write(senddata,len);
		qDebug()<<QObject::tr("Tcp Client Send>>>>")+ByteArrayToString(senddata,'.');
	}
	else
	{
		qCritical() << QObject::tr("The client is not connected to the server!");
		connectServer(ServerIP,ServerPort);
	}
}

void TcpClient::readyReadSlot()
{
    readmessage = tcpClientSocket->readAll();
    emit updatedisplay(readmessage);
    qDebug()<<QObject::tr("Tcp Client Recv<<<<")+ByteArrayToString(readmessage,'.');
}

void TcpClient::errorSlot(QAbstractSocket::SocketError)
{
	qCritical() << tcpClientSocket->errorString();
	disconnectedSlot();
}

//多项是为0x1021的CRC算法
qint16 TcpClient::GetCrc(QByteArray Data)
{
   quint16 mCrc = 0;
   quint16 sLen = Data.size();
   for(quint32 j = 0; j < sLen; j++){
       mCrc = mCrc^(quint16)(Data.at(j)) << 8;
       for (quint32 i=8; i!=0; i--){
           if (mCrc & 0x8000)
               mCrc = mCrc << 1 ^ 0x1021;
           else
               mCrc = mCrc << 1;
       }
   }
   return mCrc;
}

void TcpClient::run()
{
    int ValidLen = 0;
    int SendLen = 0;
    qint8 Step = 1;
    QByteArray Pack, Data;
    quint8 cnt = 1;                               //xModem 包号从1开始
    quint16 Crc;
    quint32 Timeout = 0;
    readmessage.clear();
    while(bThreadStop==false)
    {
        switch(Step)
        {
        case 0:                                  //等待收方的请求
            if('C' == readmessage[0])
                Step = 1;
            readmessage.clear();
            break;

        case 1:                                 //发送数据包
            Pack[0] = 0x02;                     //发送类型01:128byte,02:1024byte
            Pack[1] = cnt;                      //包号
            Pack[2] = ~cnt;                     //包号取反
            ValidLen = filedata.size();         //有效数据长度
            if(ValidLen >= PackSize){           //大于包长
                Data = filedata.left(PackSize);   //从TxData的左侧取出PackSize长
                SendLen = PackSize;             //已发长度
            }else{
                QByteArray zero(PackSize - ValidLen, 0x1a); //不足部分填充0x1a
                Data = filedata.left(ValidLen);   //不足一包
                Data += zero;                   //填充其它数据
                SendLen = ValidLen;             //已发长度
            }
            Crc = GetCrc(Data);                 //CRC
            Pack += Data;                       //填入数据
            Pack += (quint8)(Crc >> 8);         //先发高位
            Pack += (quint8)(Crc & 0xff);       //后发低位            
            emit senddata(Pack,PackSize + 5);
            Pack.clear();                       //清buf
            Data.clear();                       //清buf
            Timeout=0;
            while(0 == readmessage.size() && Timeout < 50000)
            {
                msleep(5);
                Timeout++;
                UseTimes++;
            }

            if(readmessage.size() > 0 && readmessage.at(0) == 0x06)
            {
                readmessage.clear();
                filedata.remove(0, SendLen);   //回复正确删除已发的长度
                PackCnt++;                     //进度条的包号增加
                cnt++;                         //发送包好增加
                emit sendupdata();
            }
            readmessage.clear();               //清buf
            if(PackCnt >= PackNumber){         //发送完成
                Step = 2;
                cnt = 0;
            }
            break;

        case 2:                                //发送停止包
            Pack.clear();                      //清除buf
            Pack += 0x04;                      //发送结束符            
            emit senddata(Pack,1);
            Timeout=0;
            while(0 == readmessage.size() && Timeout < 50000)
            {
                msleep(5);
                Timeout++;
                UseTimes++;
            }
            if(readmessage.size() > 0)
                cnt++;
            if(cnt == 2){                       //发送两次结束符
                emit sendupdata();
                Step = 3;
                cnt = 1;
                break;
            }
            readmessage.clear();
            break;
        }//switch
        if(Step==3) emit downloadfinish();
    }//while
}

void TcpClient::stop()
{
    bThreadStop=true;
}
