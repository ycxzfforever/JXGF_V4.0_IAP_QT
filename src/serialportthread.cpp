#include "../include/common.h"


//多项是为0x1021的CRC算法
qint16 SerialPortThread::GetCrc(QByteArray Data)
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

void SerialPortThread::readyReadSlot()
{
    unsigned short len=0;
    RxData.append(Port->readAll());
    if(RxData[0]==(char)0xBB)
    {//主板运行起来后，命令返回判断
        if(RxData.size()>6)
        {
            len=(unsigned char)RxData[6]+11;//消息体长度+帧头帧尾和crc
        }
        if(len<11) return;
        if(RxData.size()<len)
        {
            return;
        }
        else
        {
            emit updatedisplay(RxData);
            qDebug()<<QObject::tr("Uart Recv<<<<")+ByteArrayToString(RxData,'.');
            RxData.clear();
        }
    }
    else if((RxData.size()==1)&&
            (((RxData[0]==(char)0x06)&&(bSendFile==true))||((RxData[0]==(char)0x43)&&(bSendFile==false))))
    {//开始下载程序
        emit updatedisplay(RxData);
        qDebug()<<QObject::tr("Uart Recv<<<<")+ByteArrayToString(RxData,'.');
        if(bSendFile==false) RxData.clear();//未下载程序时，需要清除
    }
    else
    {//BootLoader显示        
        msleep(10);
        RxData.append(Port->readAll());        
        //if(RxData.size()<10) return;
        emit updatedisplay(RxData);
        qDebug()<<QObject::tr("Uart Recv<<<<")+ByteArrayToString(RxData,'.');
        RxData.clear();
    }
}

//串口线程体构造
SerialPortThread::SerialPortThread()
{    
    bThreadStop = false;
    bIsOpen = false;
    Port = new QSerialPort;
    bSendFile = false;

    //成员变量初始化
    PackNumber = 0;
    PackSize = 1024;
    UseTimes = 0;
    PackCnt = 0;
    bIsSend = false;
    connect(Port,SIGNAL(readyRead()),this,SLOT(readyReadSlot()));
    connect(this,SIGNAL(senddata(QByteArray,qint32)),this,SLOT(SendData(QByteArray,qint32)));
}

//串口线程体结束
void SerialPortThread::Stop()
{
    qDebug("Thread Stop");
    bThreadStop = true;
    bSendFile=false;
    Port->write(QByteArray::fromHex("04"), 1);
    this->quit();
}

//串口线程体
void SerialPortThread::run()
{
    qDebug("Thread Run");
    qint32 ValidLen = 0;
    qint32 SendLen = 0;
    qint8 Step = 1;
    QByteArray Pack, Data;
    quint16 cnt = 1;                               //xModem 包号从1开始
    quint16 Crc;
    quint32 Timeout = 0;
    RxData.clear();
    while(false == bThreadStop)
    {
        if(true == bIsOpen && true == bSendFile)
        {
            switch(Step)
            {
            case 0:                                  //等待收方的请求
                if('C' == RxData[0])
                    Step = 1;
                RxData.clear();
                break;

            case 1:                                 //发送数据包
                Pack[0] = 0x02;                     //目前只用1024byte类型
                Pack[1] = cnt;                      //包号
                Pack[2] = ~cnt;                     //包号取反
                ValidLen = TxData.size();           //有效数据长度
                if(ValidLen >= PackSize)            //大于包长
                {
                    Data = TxData.left(PackSize);   //从TxData的左侧取出PackSize长
                    SendLen = PackSize;             //已发长度
                }
                else
                {
                    QByteArray zero(PackSize - ValidLen, 0x1a); //不足部分填充0x1a
                    Data = TxData.left(ValidLen);   //不足一包
                    Data += zero;                   //填充其它数据
                    SendLen = ValidLen;             //已发长度
                }
                Crc = GetCrc(Data);                 //CRC
                Pack += Data;                       //填入数据
                Pack += (quint8)(Crc >> 8);         //先发高位
                Pack += (quint8)(Crc & 0xff);       //后发低位
                emit senddata(Pack,PackSize+5);//不能直接使用write函数，否则会有警告
                //Port->write(Pack, PackSize + 5);    //发送数据
                //Port->waitForBytesWritten(50);      //超时检测
                Pack.clear();                       //清buf
                Data.clear();                       //清buf
                while(0 == RxData.size() && Timeout < 50000)
                {
                    this->msleep(5);
                    Timeout++;
                    UseTimes++;
                }
                if(RxData.size() > 0 && RxData.at(0) == 0x06)
                {
                    TxData.remove(0, SendLen);     //回复正确删除已发的长度
                    PackCnt++;                     //进度条的包号增加
                    cnt++;                         //发送包好增加
                    emit(this->SendPack());        //给主线程发信号更新状态
                }
                RxData.clear();                    //清buf
                if(PackCnt >= PackNumber)
                { //发送完成
                    Step = 2;
                    cnt = 0;
                }
                break;

            case 2:                                 //发送停止包
                Pack.clear();                      //清除buf
                Pack += 0x04;                      //发送结束符
                emit senddata(Pack,1);//不能直接使用write函数，否则会有警告
                //Port->write(Pack, 1);              //写
                //Port->waitForBytesWritten(50);
                Pack.clear();
                while(0 == RxData.size() && Timeout < 50000)
                {
                    msleep(5);
                    Timeout++;
                    UseTimes++;
                }
                if(RxData.size() > 0)
                    cnt++;
                if(cnt == 2){                     //发送两次结束符
                    bSendFile = false;
                    bIsSend = true;
                    emit downloadfinish();
                    emit(this->SendPack());       //给主线程发信号更新状态
                    Step = 0;
                    cnt = 1;
                }
                break;
            }//switch
        }
        else
        {
            ValidLen = 0;
            SendLen = 0;
            Step = 0;
            cnt = 1;
            Timeout = 0;
        }
    }//while
}

//关闭串口
void SerialPortThread::Close()
{
    qDebug("Uart Close");
    Port->close();    
    bThreadStop = true;
    bIsOpen = false;
    bSendFile = false;
    //成员变量初始化
    PackNumber = 0;
    PackSize = 1024;
    UseTimes = 0;
    PackCnt = 0;
    bIsSend = false;
}

//打开串口
void SerialPortThread::Open(QString COM, qint32 Rate)
{
    qDebug("Uart Open");
    Port->setPortName(COM);                 //设置串口号
    //串口为读写方式
    bIsOpen = Port->open(QIODevice::ReadWrite);
    if(bIsOpen)
    {
        Port->setBaudRate(Rate);                //设置波特率
        Port->setDataBits(QSerialPort::Data8);  //数据长度为8bit
        Port->setParity(QSerialPort::NoParity); //N
        Port->setStopBits(QSerialPort::OneStop);//1
        Port->setFlowControl(QSerialPort::NoFlowControl);//无流控制
        bIsOpen = true;
    }
}

//串口发送数据
void SerialPortThread::SendData(QByteArray data, qint32 len)
{
    Port->write(data,len);
}
