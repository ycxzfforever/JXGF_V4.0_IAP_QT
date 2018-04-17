#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "include/network.h"
#include "include/serialportthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    TcpClient *tcpClient=NULL;
    QLabel *ComStateLabel;
    QLabel *FileStateLabel;
    QLabel *PackStateLabel;
    QLabel *UseTimeStateLabel;
    QLabel *AutherInfo;
    SerialPortThread Com;       //串口线程对象
    QString     filepath;       //文件路径
    QByteArray  filedata;       //文件内容    
private slots:
    void StateUpdate(); //更新lable
    void IsConnectServer();//判断是否连接到服务器
    void UpdateDisplay(QByteArray str);  //更新下位机发上来的数据显示
    void SendData(QByteArray data,qint32 len);//发送数据
    void DownloadFinish();//下载完成
    void selectdownload();//选择下载方式
    void on_openfile_pushButton_clicked();
    void on_connectserver_pushButton_clicked();
    void on_download_pushButton_clicked();    
    void on_oder_pushButton_clicked();
    void on_clearscreen_pushButton_clicked();


    void on_sendIAP_pushButton_clicked();

    void on_boot_pushButton_clicked();

    void on_exitboot_pushButton_clicked();

    void on_rollcall_pushButton_clicked();

    void on_getVersion_pushButton_clicked();

    void on_opencom_pushButton_clicked();

private:
    qint32 BaudRete[8];
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
