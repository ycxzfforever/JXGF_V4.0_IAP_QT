#ifndef NETWORK
#define NETWORK

#include<QtNetwork/QtNetwork>

class TcpClient : public QThread
{
		Q_OBJECT

	public:
        TcpClient(QString ServerIP,qint32 ServerPort);
		~TcpClient();
		void ReadServerInfoConf();
		void connectServer(QString serverip,qint32 port);
        bool isConnected;
        QTcpSocket  *tcpClientSocket;//本客户端连接外部服务器产生的socket
        QByteArray  recvdata;
        QString     ServerIP;
        qint32      ServerPort;
        bool        bThreadStop;
        int         PackSize;       //包大小
        int         PackNumber;     //计算包数
        int         PackCnt;        //已发送包数
        int         UseTimes;       //发送时间
        QByteArray  filedata;       //文件内容
        QByteArray  readmessage;    //收到的信息
        void run();
        void stop();
        qint16 GetCrc(QByteArray Data);
    signals:
        void sendupdata();//更新进度条和状态栏
        void isconnectserver();//用于主界面判断是否连接到服务器
        void updatedisplay(QByteArray);//更新下位机上传的数据显示
        void senddata(QByteArray,qint32);//发送数据
        void downloadfinish();//下载完成
	private slots:
		void connectedSlot();		
		void readyReadSlot();
		void errorSlot(QAbstractSocket::SocketError);
    public slots:
        void disconnectedSlot();
        void TcpClientSendData(QByteArray senddata, int len);
};


#endif // NETWORK

