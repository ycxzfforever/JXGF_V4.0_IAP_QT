#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "include/common.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //固定尺寸
    //setFixedSize(this->width(), this->height());
    ui->setupUi(this);

    ui->download_pushButton->setDisabled(true);    
    FileStateLabel = new QLabel;
    PackStateLabel = new QLabel;
    UseTimeStateLabel = new QLabel;
    AutherInfo = new QLabel;
    AutherInfo->setAlignment(Qt::AlignRight);
    statusBar()->addPermanentWidget(AutherInfo);
    AutherInfo->setText("Design by ycx");
    statusBar()->addWidget(FileStateLabel);
    statusBar()->addWidget(PackStateLabel);
    statusBar()->addWidget(UseTimeStateLabel);
    statusBar()->addWidget(AutherInfo);
    //statusBar()->setStyleSheet(QString("QStatusBar::item{border: 0px}"));//设置状态栏无边框    
    ui->IPserver_radioButton->setChecked(true);
    selectdownload();
    connect(ui->IPserver_radioButton,SIGNAL(toggled(bool)),this,SLOT(selectdownload()));
    ui->progressBar->setValue(0);                //进度条清零
}

MainWindow::~MainWindow()
{
    Com.quit();
    Com.deleteLater();
    delete ui;
}

//打开文件
void MainWindow::on_openfile_pushButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                ".",
                                                tr("bin Files(*.bin)"));
    if(!path.isEmpty())
    {
        ui->filepath_lineEdit->setText(path);
        filepath=path;
        QFile file(filepath);
        if(file.open(QIODevice::ReadOnly))  //打开文件
        {
            filedata = file.readAll();                  //读文件
            if(ui->IPserver_radioButton->isChecked())
            {
                if(tcpClient==NULL) return;
                tcpClient->filedata=filedata;
                tcpClient->PackNumber = filedata.size() / tcpClient->PackSize;    //计算包数
                if(filedata.size() % tcpClient->PackSize)              //多出的按一包算
                {
                    tcpClient->PackNumber += 1;
                }
                ui->progressBar->setRange(0, tcpClient->PackNumber);//设置进度条范围
                if(tcpClient->isConnected==true)//连接成功
                {
                    ui->download_pushButton->setDisabled(false);
                }
                else
                {
                    ui->download_pushButton->setDisabled(true);
                }
            }
            else
            {
                Com.TxData=filedata;
                Com.PackNumber = filedata.size() / Com.PackSize;    //计算包数
                if(filedata.size() % Com.PackSize)              //多出的按一包算
                {
                    Com.PackNumber += 1;
                }
                ui->progressBar->setRange(0, Com.PackNumber);//设置进度条范围
                ui->download_pushButton->setDisabled(false);
            }
            file.close();
            ui->textBrowser->append("文件打开成功。");
        }        
        StateUpdate();        
    }
    else
    {
        QMessageBox::warning(this, tr("Path"),
                             tr("You did not select any file."));
    }   
}

//连接串口服务器
void MainWindow::on_connectserver_pushButton_clicked()
{
    QString path;
    path=ui->filepath_lineEdit->text();
    QString ServerIP=ui->ipaddr_lineEdit->text();
    qint32 ServerPort=ui->port_lineEdit->text().toLong();
    QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    if(!regExpIP.exactMatch(ServerIP) )
    {
          QMessageBox::information(this, tr("错误"), tr("ip地址错误"));
          return;
    }
    if(ServerPort <= 0)
    {
          QMessageBox::information(this, tr("错误"), tr("端口错误"));
          return;
    }
    if(ui->connectserver_pushButton->text()=="断开服务器")
    {
        tcpClient->disconnectedSlot();
        tcpClient->deleteLater();
        tcpClient=NULL;
        ui->PCcom_radioButton->setEnabled(true);
        ui->connectserver_pushButton->setText("连接服务器");
        ui->download_pushButton->setText("开始下载");
        ui->download_pushButton->setDisabled(true);
        return;
    }
    tcpClient = new TcpClient(ServerIP,ServerPort);
    if(path.length()>0)
    {
        filepath=path;
        QFile file(filepath);
        if(file.open(QIODevice::ReadOnly))  //打开文件
        {
            filedata = file.readAll();                  //读文件
            tcpClient->filedata=filedata;
            tcpClient->PackNumber = filedata.size() / tcpClient->PackSize;    //计算包数
            if(filedata.size() % tcpClient->PackSize)              //多出的按一包算
            {
                tcpClient->PackNumber += 1;
            }
            file.close();
            ui->textBrowser->append("文件打开成功。");
        }
        ui->progressBar->setRange(0, tcpClient->PackNumber);//设置进度条范围
    }
    StateUpdate();
    ui->PCcom_radioButton->setEnabled(false);
    connect(tcpClient, SIGNAL(sendupdata()), this, SLOT(StateUpdate()));
    connect(tcpClient,SIGNAL(isconnectserver()),this,SLOT(IsConnectServer()));
    connect(tcpClient,SIGNAL(updatedisplay(QByteArray)),this,SLOT(UpdateDisplay(QByteArray)));
    connect(tcpClient,SIGNAL(senddata(QByteArray,qint32)),this,SLOT(SendData(QByteArray,qint32)));
    connect(tcpClient,SIGNAL(downloadfinish()),this,SLOT(DownloadFinish()));
}

//判断是否连接到服务器
void MainWindow::IsConnectServer()
{
    if(tcpClient->isConnected==true)//连接成功
    {
        ui->connectServer_label->setText("服务器连接成功");
        ui->connectserver_pushButton->setText("断开服务器");
        if(ui->filepath_lineEdit->text().length()>0)
            ui->download_pushButton->setDisabled(false);
        else
            ui->download_pushButton->setDisabled(true);
    }
    else
    {
        ui->connectServer_label->setText("服务器未连接");
        ui->connectserver_pushButton->setText("连接服务器");

        if(ui->filepath_lineEdit->text().length()>0)
            ui->download_pushButton->setDisabled(false);
        else
            ui->download_pushButton->setDisabled(true);
    }
}


//开始下载程序
void MainWindow::on_download_pushButton_clicked()
{
    if(ui->IPserver_radioButton->isChecked())
    {
        if(ui->download_pushButton->text()=="开始下载")
        {
            tcpClient->bThreadStop=false;
            tcpClient->start();
            ui->download_pushButton->setText("取消下载");
        }
        else
        {
            tcpClient->stop();
            ui->download_pushButton->setText("开始下载");
        }
    }
    else if(Com.bIsOpen)
    {
        if(ui->download_pushButton->text()=="开始下载")
        {
            qDebug("On PushButton Send File");
            if(false == Com.bSendFile)  //没有打开文件
            {
                if(filepath.isEmpty())  //文件大小为0
                {
                    QMessageBox::about(NULL, tr("发送错误"), tr("请先打开二进制文件再下载"));
                    return;
                }
                QFile file(filepath);                               //发送前自动更新文件
                if(file.open(QIODevice::ReadOnly))//打开文件
                {
                    filedata = file.readAll();                      //读文件
                    Com.PackNumber = filedata.size() / Com.PackSize;//计算包数
                    if(filedata.size() % Com.PackSize)              //多出的按一包算
                     Com.PackNumber += 1;
                    file.close();
                }
                else
                {
                    QMessageBox::about(NULL, tr("错误"), tr("无法打开二进制文件"));
                    return;
                }

               Com.TxData = filedata;                    //拷贝文件数据
               Com.bIsSend = false;                      //已发送完成标志清零
               Com.PackCnt = 0;                          //已发送包数清零
               Com.UseTimes = 0;
               Com.bSendFile = true;                     //开始发送标志

               ui->progressBar->setRange(0, Com.PackNumber);//设置进度条范围
               ui->progressBar->setValue(0);                //进度条清零
            }
            StateUpdate();
            ui->download_pushButton->setText("取消下载");
            Com.bThreadStop = false;
            Com.start();
        }
        else
        {
            Com.Stop();
            ui->download_pushButton->setText("开始下载");
        }
    }

}

//发送命令
void MainWindow::on_oder_pushButton_clicked()
{
    QString oder=ui->oder_lineEdit->text();
    if(ui->IPserver_radioButton->isChecked())
    {
        if(tcpClient==NULL) return;
        ui->textBrowser->append(oder);
        tcpClient->TcpClientSendData(oder.toLatin1(),oder.toLatin1().length());
    }
    else if(Com.bIsOpen)
    {
        ui->textBrowser->append(oder);
        Com.Port->write(oder.toLatin1());
    }
}

//更新显示
void MainWindow::UpdateDisplay(QByteArray str)
{
    qint8 gunnum=ui->gunnum_lineEdit->text().toInt();
    if((str[4]==(char)0x51)&&(str[3]==(char)gunnum))//轮询测试返回
    {
        quint16 crc,len;
        len=str.length();
        QByteArray data=str.mid(2,len-6);
        crc=modbus_crc16(data);
        if(((quint8)(crc>>8)==(quint8)str[len-4])&&((quint8)(crc&0xFF)==(quint8)str[len-3]))
        {
            ui->textBrowser->append(QObject::tr("\n%1号枪数据返回成功!!!").arg(gunnum));
        }
    }
    else if((str[4]==(char)0xBB)&&(str[3]==(char)gunnum))//轮询测试返回
    {
        quint16 crc,len;
        len=str.length();
        QByteArray data=str.mid(2,len-6);
        crc=modbus_crc16(data);
        if(((quint8)(crc>>8)==(quint8)str[len-4])&&((quint8)(crc&0xFF)==(quint8)str[len-3]))
        {
            ui->textBrowser->append(QObject::tr("\n%1号枪版本号为:").arg(gunnum)+str.mid(7,16));
        }
    }
    else if(((str[4]==(char)0x52)||(str[4]==(char)0x53)||(str[4]==(char)0x54)||
             (str[4]==(char)0x55)||(str[4]==(char)0x56)||(str[4]==(char)0x5A)||
             (str[4]==(char)0x5B)||(str[4]==(char)0x5C)||(str[4]==(char)0x5E)
             )&&(str[3]==(char)gunnum))//其他数据返回
    {
        quint16 crc,len;
        len=str.length();
        QByteArray data=str.mid(2,len-6);
        crc=modbus_crc16(data);
        if(((quint8)(crc>>8)==(quint8)str[len-4])&&((quint8)(crc&0xFF)==(quint8)str[len-3]))
        {
            ui->textBrowser->append(QObject::tr("\n%1号枪返回其他数据:").arg(gunnum)+ByteArrayToString(str.mid(4,1),' '));
        }
    }
    else
    {
        QString tmpstr = QString::fromLocal8Bit(str.constData());//用于显示汉字
        if((str.length()==1)&&(str[0]!=(char)0x43))
        {            
            QString tmp=ui->textBrowser->toPlainText();
            qint16 len=tmp.size();
            tmp.resize(len-1);
            tmp.append(tmpstr);
            ui->textBrowser->clear();
            ui->textBrowser->insertPlainText(tmp);
        }
        else
        {
            ui->textBrowser->insertPlainText(tmpstr);
        }
        ui->textBrowser->moveCursor(QTextCursor::End);
    }
}

//发送数据
void MainWindow::SendData(QByteArray data, qint32 len)
{
    if(ui->IPserver_radioButton->isChecked())
    {
        if(tcpClient==NULL) return;
        tcpClient->TcpClientSendData(data,len);
    }
    else if(Com.bIsOpen)
    {
        Com.Port->write(data,len);
    }
}

//下载完成
void MainWindow::DownloadFinish()
{
    if(ui->IPserver_radioButton->isChecked())
    {
        tcpClient->stop();
        tcpClient->quit();
    }
    else
    {
        Com.Stop();
        Com.quit();
    }
    ui->download_pushButton->setText("下载完成");
    ui->download_pushButton->setEnabled(false);
}

//选择下载方式
void MainWindow::selectdownload()
{
    if(ui->IPserver_radioButton->isChecked())
    {
        ui->baudrate_comboBox->setEnabled(false);
        ui->comnum_comboBox->setEnabled(false);
        ui->opencom_pushButton->setEnabled(false);
        ui->ipaddr_lineEdit->setEnabled(true);
        ui->port_lineEdit->setEnabled(true);
        ui->connectserver_pushButton->setEnabled(true);
        //串口服务器相关
        QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
        QRegExp regExpNetPort("((6553[0-5])|[655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{3}|[1-9][0-9]{2}|[1-9][0-9]|[0-9])");
        ui->ipaddr_lineEdit->setValidator(new QRegExpValidator(regExpIP,this));
        ui->port_lineEdit->setValidator(new QRegExpValidator(regExpNetPort,this));

    }
    else
    {
        ui->ipaddr_lineEdit->setEnabled(false);
        ui->port_lineEdit->setEnabled(false);
        ui->connectserver_pushButton->setEnabled(false);
        ui->baudrate_comboBox->setEnabled(true);
        ui->comnum_comboBox->setEnabled(true);
        ui->opencom_pushButton->setEnabled(true);
        //PC串口相关
        //搜索串口并添加到下拉列表里
        ui->comnum_comboBox->clear();
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            qDebug() << "Name        : " << info.portName();
            qDebug() << "Description : " << info.description();
            qDebug() << "Manufacturer: " << info.manufacturer();

            QSerialPort serial;
            serial.setPort(info);
            ui->comnum_comboBox->addItem(info.portName());
        }
        //设置波特率下拉列表
        ui->baudrate_comboBox->clear();
        BaudRete[0] = QSerialPort::Baud1200;
        BaudRete[1] = QSerialPort::Baud2400,
        BaudRete[2] = QSerialPort::Baud4800,
        BaudRete[3] = QSerialPort::Baud9600,
        BaudRete[4] = QSerialPort::Baud19200,
        BaudRete[5] = QSerialPort::Baud38400,
        BaudRete[6] = QSerialPort::Baud57600,
        BaudRete[7] = QSerialPort::Baud115200;
        QString str;
        for(qint8 i = 0; i < 8; i++)
        {
            str.setNum(BaudRete[i], 10);
            ui->baudrate_comboBox->addItem(str);
        }
        ui->baudrate_comboBox->setCurrentIndex(5);  //默认38400
    }
}

//清屏
void MainWindow::on_clearscreen_pushButton_clicked()
{
    ui->textBrowser->clear();
}

//更新状态栏及进度条
void MainWindow::StateUpdate()
{
    qint32 size;
    QString str;
    float tmp;

    //更新打开文件大小
    str = tr("File size:");
    size = filedata.size();
    QString strTmp;
    if(size >= 1024){
        tmp = size;
        tmp /= 1024.00;
        strTmp.setNum(tmp, 'g', 4);
        str += strTmp;
        str += tr(" K ");
    }
    else{
        strTmp.setNum(size, 10);
        str += strTmp;
        str += tr(" Byte ");
    }
    FileStateLabel->setText(str);

    //更新总包数/发送包数
    str = tr("Send pack:");
    if(tcpClient!=NULL)
        strTmp.setNum(tcpClient->PackCnt);
    else
        strTmp.setNum(Com.PackCnt);
    str += strTmp;
    str += tr("/");
    if(tcpClient!=NULL)
        strTmp.setNum(tcpClient->PackNumber);
    else
        strTmp.setNum(Com.PackNumber);
    str += strTmp;
    str += tr(" ");
    PackStateLabel->setText(str);

    //更新用时
    str = tr("Times:");
    if(tcpClient!=NULL)
        tmp =  (tcpClient->UseTimes * 5) / 1000.00;
    else
        tmp =  (Com.UseTimes * 5) / 1000.00;
    strTmp.setNum(tmp, 'g', 4);
    str += strTmp;
    str += tr(" S ");
    UseTimeStateLabel->setText(str);
    //设置进度条
    if(tcpClient!=NULL)
       ui->progressBar->setValue(tcpClient->PackCnt);
    else
       ui->progressBar->setValue(Com.PackCnt);
}

//发送IAP指令，会擦除flash
void MainWindow::on_sendIAP_pushButton_clicked()
{    
    QString str="IAP";
    QMessageBox::StandardButton button;
    button = QMessageBox::question(this, tr("警告"),
                                   QString(tr("发送IAP将会擦除主板原程序\n<!!!不可逆，请谨慎操作!!!>")),
                                   QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::Yes)
    {
        ui->textBrowser->append(str);
        if(ui->IPserver_radioButton->isChecked())
        {
            if(tcpClient==NULL) return;
            tcpClient->TcpClientSendData(str.toLatin1(),str.length());
        }
        else if(Com.bIsOpen)
        {
            Com.Port->write(str.toLatin1());
        }
    }
}

//发送指令给加气机让其重启进入BootLoader
void MainWindow::on_boot_pushButton_clicked()
{   
    if(ui->gunnum_lineEdit->text().isEmpty())
    {
        QMessageBox::information(this, tr("错误"), tr("请输入枪号"));
        return;
    }
    QByteArray str;
    quint16 crc;
    str.append(ui->gunnum_lineEdit->text().toInt());//枪号
    str.append(QByteArray::fromHex("00AA0000"));
    str.append(QByteArray::fromHex(("20161212161718")));
    str[4]=str.size()-5;//data length
    crc=modbus_crc16(str);
    str.append((quint8)(crc >> 8));         //先发高位
    str.append((quint8)(crc & 0xff));       //后发低位
    str.prepend(QByteArray::fromHex("AAFF"));//帧头
    str.append(QByteArray::fromHex("CCFF")); //帧尾
    if(ui->IPserver_radioButton->isChecked())
    {
        if(tcpClient==NULL) return;
        tcpClient->TcpClientSendData(str,str.length());
    }
    else if(Com.bIsOpen)
    {
        Com.Port->write(str);
    }
}

//退出BootLoader
void MainWindow::on_exitboot_pushButton_clicked()
{    
    QString str="q";
    ui->textBrowser->append(str);
    if(ui->IPserver_radioButton->isChecked())
    {
        if(tcpClient==NULL) return;
        tcpClient->TcpClientSendData(str.toLatin1(),str.length());
    }
    else if(Com.bIsOpen)
    {
        Com.Port->write(str.toLatin1());
    }
}

//发送51命令测试线路是否连通
void MainWindow::on_rollcall_pushButton_clicked()
{    
    if(ui->gunnum_lineEdit->text().isEmpty())
    {
        QMessageBox::information(this, tr("错误"), tr("请输入枪号"));
        return;
    }
    QByteArray str,tmp;
    quint16 crc;
    str.append(ui->gunnum_lineEdit->text().toInt());//枪号
    str.append(QByteArray::fromHex("00510000"));
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyyMMddhhmmss");
    tmp=hexStringtoByteArray(current_date);
    str.append(tmp);
    str.append(QByteArray::fromHex("000000000000"));
    str[4]=str.size()-5;//data length
    crc=modbus_crc16(str);
    str.append((quint8)(crc >> 8));         //先发高位
    str.append((quint8)(crc & 0xff));       //后发低位
    str.prepend(QByteArray::fromHex("AAFF"));//帧头
    str.append(QByteArray::fromHex("CCFF")); //帧尾
    if(ui->IPserver_radioButton->isChecked())
    {
        if(tcpClient==NULL) return;
        tcpClient->TcpClientSendData(str,str.length());
    }
    else if(Com.bIsOpen)
    {
        Com.Port->write(str);
    }
}

//获取程序版本号
void MainWindow::on_getVersion_pushButton_clicked()
{    
    if(ui->gunnum_lineEdit->text().isEmpty())
    {
        QMessageBox::information(this, tr("错误"), tr("请输入枪号"));
        return;
    }
    QByteArray str,tmp;
    quint16 crc;
    str.append(ui->gunnum_lineEdit->text().toInt());//枪号
    str.append(QByteArray::fromHex("00BB0000"));
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyyMMddhhmmss");
    tmp=hexStringtoByteArray(current_date);
    str.append(tmp);
    str[4]=str.size()-5;//data length
    crc=modbus_crc16(str);
    str.append((quint8)(crc >> 8));         //先发高位
    str.append((quint8)(crc & 0xff));       //后发低位
    str.prepend(QByteArray::fromHex("AAFF"));//帧头
    str.append(QByteArray::fromHex("CCFF")); //帧尾
    if(ui->IPserver_radioButton->isChecked())
    {
        if(tcpClient==NULL) return;
        tcpClient->TcpClientSendData(str,str.length());
    }
    else if(Com.bIsOpen)
    {
        Com.Port->write(str);
    }
}

//打开串口
void MainWindow::on_opencom_pushButton_clicked()
{
    qint8 Index = ui->baudrate_comboBox->currentIndex();
    qDebug("on pushButton OpenCom clicked");
    if(false == Com.bIsOpen)//打开串口
    {
       Com.Open(ui->comnum_comboBox->currentText(), BaudRete[Index]);
       if(true ==  Com.bIsOpen)
       {
           ui->IPserver_radioButton->setEnabled(false);
           ui->opencom_pushButton->setText("关闭串口");
           ui->baudrate_comboBox->setEnabled(false);
           ui->comnum_comboBox->setEnabled(false);
           ui->download_pushButton->setEnabled(true);
           connect(&Com,SIGNAL(updatedisplay(QByteArray)),this,SLOT(UpdateDisplay(QByteArray)));
           connect(&Com,SIGNAL(downloadfinish()),this,SLOT(DownloadFinish()));
           connect(&Com,SIGNAL(SendPack()),this,SLOT(StateUpdate()));
       }
       else
           QMessageBox::about(NULL, tr("串口打开错误"), tr("无法打开串口或是串口被占用！"));
    }
    else//关闭串口
    {
        Com.Close();
        ui->IPserver_radioButton->setEnabled(true);
        ui->download_pushButton->setText("开始下载");
        ui->download_pushButton->setDisabled(true);
        disconnect(&Com,SIGNAL(updatedisplay(QByteArray)),this,SLOT(UpdateDisplay(QByteArray)));
        disconnect(&Com,SIGNAL(downloadfinish()),this,SLOT(DownloadFinish()));
        disconnect(&Com,SIGNAL(SendPack()),this,SLOT(StateUpdate()));
        ui->opencom_pushButton->setText("打开串口");
        ui->baudrate_comboBox->setEnabled(true);
        ui->comnum_comboBox->setEnabled(true);
    }
    StateUpdate();
}
