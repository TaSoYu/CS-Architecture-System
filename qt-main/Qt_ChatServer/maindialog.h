#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>

// 在Qt中，QT += network是用来添加网络模块的指令。

/* C++中用于引入QT网络模块中的QTcpServer类的头文件。 */
#include <QTcpServer>

/* QFile是Qt框架提供的类，用于读取和写入文件。 */
#include <QFile>

/* #include <QtNetwork> 是用于包含 Qt 中网络相关功能的头文件。*/
 #include <QtNetwork>

/* #include <QPainter> 是C++中使用Qt库时的一个常见的头文件引用。*/
#include <QPainter>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

#pragma execution_character_set("utf-8")

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private:
    Ui::MainDialog *ui;


private:
    QTcpServer *P_TCPServerObject;                          // TCP服务器指针
    QTcpSocket * P_TCPTextSocketObject;                     // 聊天数据信息Socket指针
    QTcpSocket *P_TCPFileSocketObject;                      // 传输文件数据信息socket指针

    qint64 m_FileTotalBytes;                                // 存放文件总大小数据
    qint64 m_FileBytesReceived;                             // 接收已经收到文件数据的大小
    qint64 m_FileNameSizes;                                 // 文件名称大小信息
    QString m_FileNames;                                    // 存放文件名称信息
    QFile *m_LocalFiles;                                    // 存放本地文件指针
    QByteArray m_InBlocks;                                  // 存放数据块信息

    QString GetLocalIPAddressFunc();                        // 获取本机IP地址信息
    void paintEvent(QPaintEvent *ev);                       // 更改窗口背景图片

protected:
    void closeEvent(QCloseEvent *event);                    // 关闭窗口时，则服务器端停止监听


private slots:
    void OnNewConnectionFunc();                             // QTcpServer中专用处理newConnection()信号
    void OnClientConnectedFunc();                           // 该函数处理数据文本信息socket的connected
    void OnClientDisconnectedFunc();                        // 处理断开连接，client数据文本信息Socket disconnected
    void OnSocketReadyReadFunc();                           // 读取客户端socket文本数据信息

    void OnFileClientConnectedFunc();                       // 处理文件接收socket操作信息提示
    void OnFileClientDisconnectedFunc();                    // 断开文件接收socket操作

    void UpdateServerProgressFunc();                        // 更新进度条接收数据动态变化
    void DisplayErrorInfoFunc(QAbstractSocket::SocketError socketError); // 处理显示错误信息告诉用户



    void on_pushButton_StartListening_clicked();
    void on_pushButton_StopListening_clicked();
    void on_pushButton_SendData_clicked();
    void on_pushButton_ClearMsg_clicked();
    void on_pushButton_ExitSystem_clicked();

};
#endif // MAINDIALOG_H
