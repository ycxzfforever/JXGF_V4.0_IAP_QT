#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QDebug>

class SerialPortThread : public QThread
{
    Q_OBJECT
public:
    SerialPortThread();     //串口线程构造
    void Stop();            //线结束
    QSerialPort *Port;      //串口对象指针
    volatile bool bIsOpen;  //串口打开状态
    void Close();           //串口关闭
    void Open(QString COM, qint32 Rate);//串口打开
    QByteArray RxData;      //接收buf
    QByteArray TxData;      //发送buf
    bool bSendFile;         //可以发送标志
    qint32 PackNumber;      //发送文件的包数
    qint32 PackCnt;         //已发送的包数
    qint32 PackSize;        //1包字节数
    quint32  UseTimes;      //用时
    bool   bIsSend;         //发送完成
    volatile bool bThreadStop;


protected:
    void run();
    qint16 GetCrc(QByteArray Data);

signals:
    void SendPack();       //每发送完成一包产生一个信号
    void updatedisplay(QByteArray);//更新下位机上传的数据显示
    void downloadfinish();//下载完成
    void senddata(QByteArray data, qint32 len);

public slots:
    void readyReadSlot();
    void SendData(QByteArray data,qint32 len);
private:


};

#endif // SERIALPORTTHREAD_H
