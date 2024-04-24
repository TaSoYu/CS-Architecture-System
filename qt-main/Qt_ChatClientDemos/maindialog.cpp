#include "maindialog.h"
#include "ui_maindialog.h"

#pragma execution_character_set("utf-8")

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    // 固定窗口宽度和高度
    setFixedSize(width(),height()); // 固定尺寸是以用户设计为准

    m_TotalBytes=0;
    m_BytesWrites=0;
    m_BytesToWrites=0;
    m_LoadSizes=4*1024; // 每次发送的文件数据大小

    m_TcpMsgClient=new QTcpSocket(this);     // 处理聊天消息socket
    m_TcpFileClient=new QTcpSocket(this);    // 处理文件传输消息socket

    // 获取本机的IP地址
    QString strLocalIPs=GetLocalIPAddress();
    QString strIP=("本机IP地址："+strLocalIPs);
    ui->label_LocalIP->setText(strIP);

    // 将返回的IP地址显示到Combo box组合框控件里面来
    ui->comboBox_ServerIP->addItem(strLocalIPs);

    // 举例命令按钮切换实现
     ui->pushButton_DisconnectServer->setEnabled(false);
     ui->pushButton_SendFile->setEnabled(false);

    // 聊天信息：信号与槽函数关联
    connect(m_TcpMsgClient,SIGNAL(connected()),this,SLOT(OnConnectedFunc()));
    connect(m_TcpMsgClient,SIGNAL(disconnected()),this,SLOT(OnDisconnectedFunc()));
    connect(m_TcpMsgClient,SIGNAL(readyRead()),this,SLOT(OnSocketReadyReadFunc()));


    // 禁止消息列表控件，修改里面的数据信息，设置为只读。
    ui->plainTextEdit_MsgList->setReadOnly(true);

    // 当有数据发送成功时，更新进度条状态
    connect(m_TcpFileClient,SIGNAL(bytesWritten(qint64)),this,SLOT(UpdateClientProgressFunc(qint64)));

    // 调用系统托盘函数
    MySystemTrayFunc();

    // 文件传输错误处理
    connect(m_TcpFileClient,SIGNAL(error(QAbstractSocket::SocketError)),this,
            SLOT(DisplayErrorFunc(QAbstractSocket::SocketError)));

}

MainDialog::~MainDialog()
{
    delete ui;
}

// 实现更改窗口背景
void MainDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(rect(),QPixmap(":/new/prefix1/images/bgpng.png"),QRect());

}

// 获取本机的IP地址信息
QString MainDialog::GetLocalIPAddress()
{
    // 获取本机IPv4地址
    QString strHostName=QHostInfo::localHostName(); // 获取本地主机名称
    QHostInfo HostInfos=QHostInfo::fromName(strHostName); // 通过主机名称获取IP地址

    QString StrLocalIP="";

    // QList<QHostAddress>：返回与strHostName主机关联的IP地址列表
    QList<QHostAddress> addressList=HostInfos.addresses();
    if(!addressList.isEmpty())
    {
        for(int i=0;i<addressList.count();i++)
        {
            QHostAddress sHost=addressList.at(i);

            // 判断本机IP协议是否为IPv4
            if(QAbstractSocket::IPv4Protocol==sHost.protocol())
            {
                StrLocalIP=sHost.toString();
                break;
            }
        }
    }

    return StrLocalIP;
}

// 关闭事件，同时处理：消息socket和文件socket
void MainDialog::closeEvent(QCloseEvent *event)
{
    if(m_TcpMsgClient->state()==QAbstractSocket::ConnectedState)
        m_TcpMsgClient->disconnectFromHost();

    if(m_TcpFileClient->state()==QAbstractSocket::ConnectedState)
        m_TcpFileClient->disconnectFromHost();

    event->accept();
}

// 客户端连接服务器
void MainDialog::OnConnectedFunc()
{
    // connected()信号槽函数
    ui->plainTextEdit_MsgList->appendPlainText("[★★★★★客户端与服务器连接成功★★★★]");
    ui->plainTextEdit_MsgList->appendPlainText("[IP Address："+m_TcpMsgClient->peerAddress().toString()+"]"
                                               +"[Port："+QString::number(m_TcpMsgClient->peerPort())+"]");
    ui->pushButton_ConnectServer->setEnabled(false);  // 连接服务器命令按钮不能使用
    ui->pushButton_DisconnectServer->setEnabled(true); // 断开服务器命令按钮可以使用
}

// 客户端与服务器断开
void MainDialog::OnDisconnectedFunc()
{
    // disConnected()信号槽函数
    ui->plainTextEdit_MsgList->appendPlainText("[客户端断开与服务器连接]\n");

    ui->pushButton_ConnectServer->setEnabled(true); // 连接服务器命令按钮可以使用
    ui->pushButton_DisconnectServer->setEnabled(false); // 断开服务器命令按钮不能使用
}

// 读取服务器socket传输的数据信息
void MainDialog::OnSocketReadyReadFunc()
{
    // 日期时间处理操作
    QDateTime CurrentDataTime=QDateTime::currentDateTime();
    QString strYear=QString::number(CurrentDataTime.date().year());
    QString strMonth=QString::number(CurrentDataTime.date().month());
    QString strDay=QString::number(CurrentDataTime.date().day());
    QString strHour=QString::number(CurrentDataTime.time().hour());
    QString strMinute=QString::number(CurrentDataTime.time().minute());
    QString strSecond=QString::number(CurrentDataTime.time().second());

    // 将年月日时分秒6个字符串连接
    QString strDateTimes=strYear+"/"+strMonth+"/"+strDay+" "+strHour+":"+strMinute+":"+strSecond;
    // QMessageBox::information(this,"提示",strDateTimes,QMessageBox::Yes);

    while(m_TcpMsgClient->canReadLine())
        ui->plainTextEdit_MsgList->appendPlainText("[服务器消息"+strDateTimes+"]："+m_TcpMsgClient->readLine());
}

void MainDialog::MySystemTrayFunc() // 系统托盘函数
{
    QIcon qIcons(":/new/prefix1/images/logo.ico");
    MySystemTrays=new QSystemTrayIcon(this);
    MySystemTrays->setIcon(qIcons);


    qMiniAction=new QAction("客户端最小化",this);
    connect(qMiniAction,SIGNAL(triggered()),this,SLOT(hide())); // 隐藏

    qRestoreAction=new QAction("恢复客户端",this);
    connect(qRestoreAction,SIGNAL(triggered()),this,SLOT(showNormal())); // 应用程序回归正常状态

    qExitAction=new QAction("退出系统",this);
    connect(qExitAction,SIGNAL(triggered()),this,SLOT(close())); // 退出应用程序（请同学们quit改为close）。

    pContextMenu=new QMenu(this); // new一个菜单对象
    // 将QActionc对象添加到菜单
    pContextMenu->addAction(qMiniAction);
    pContextMenu->addAction(qRestoreAction);
    pContextMenu->addAction(qExitAction);

    // 设置指定菜单为系统托盘的上下文菜单
    MySystemTrays->setContextMenu(pContextMenu);

    // 显示系统托盘
    MySystemTrays->show();


}

// 文件传输过程中，进度条变化状态显示
void MainDialog::UpdateClientProgressFunc(qint64 numBytes) // 文件传输及显示进度条更新
{
    // 日期时间处理操作
    QDateTime CurrentDataTime=QDateTime::currentDateTime();
    QString strYear=QString::number(CurrentDataTime.date().year());
    QString strMonth=QString::number(CurrentDataTime.date().month());
    QString strDay=QString::number(CurrentDataTime.date().day());
    QString strHour=QString::number(CurrentDataTime.time().hour());
    QString strMinute=QString::number(CurrentDataTime.time().minute());
    QString strSecond=QString::number(CurrentDataTime.time().second());

    // 将年月日时分秒6个字符串连接
    QString strDateTimes=strYear+"/"+strMonth+"/"+strDay+" "+strHour+":"+strMinute+":"+strSecond;



    // 确认已发送数据信息的大小
    m_BytesWrites=m_BytesWrites+(int)numBytes;

    if(m_BytesToWrites>0) // 剩下数据大小
    {
        // 从文件当中取出数据到发送缓冲区，每次发送文件大小的数据，设置为4KB
        // 如果剩下的数据不满足4KB，就发送剩下数据大小
        m_OutDataBlock=m_LocalFiles->read(qMin(m_BytesToWrites,m_LoadSizes));

        // 从发送缓冲区发送数据，计算发送完一次数据后还剩下数据的大小
        m_BytesToWrites=m_BytesToWrites-(int)m_TcpFileClient->write(m_OutDataBlock);

        // 清空发送缓冲区数据信息
        m_OutDataBlock.resize(0);
    }
    else
    {
        m_LocalFiles->close(); // 没有数据发送啦，则直接关闭文件
    }

    // 进度条更新状态
    ui->progressBar->setMaximum(m_TotalBytes);
    ui->progressBar->setValue(m_BytesWrites);

    // 发送数据大小 等于 数据总大小
    if(m_BytesWrites==m_TotalBytes)
    {
        ui->plainTextEdit_MsgList->appendPlainText(QString("[%1 ♥♥♥♥♥♥文件已经成功传输到服务器：%2 成功♥♥♥♥♥♥]").arg(strDateTimes).arg(m_FileNames));

        m_LocalFiles->close();
    }
}

void MainDialog::DisplayErrorFunc(QAbstractSocket::SocketError) // 异常处理信息
{
    // 文件传输错误提示
    m_TcpFileClient->close();
    ui->progressBar->reset();
    // 可以将错误信息显示消息列表控件里面（大家可以自己实现）
}




// 连接服务器命令按钮：代码实现
void MainDialog::on_pushButton_ConnectServer_clicked()
{
    // 首先获取IP地址和端口
    QString ipaddress=ui->comboBox_ServerIP->currentText();
    quint16 port=ui->spinBox_ServerPort->value();

    // 连接服务器处理操作
    m_TcpMsgClient->connectToHost(ipaddress,port);

    m_BytesWrites = 0; // 初始化已发送字节为0
    m_TcpFileClient->connectToHost(ipaddress, port);
}


// 断开服务器命令按钮：代码实现
void MainDialog::on_pushButton_DisconnectServer_clicked()
{
    // 断开与服务器连接，断开聊天消息套接字
    if(m_TcpMsgClient->state()==QAbstractSocket::ConnectedState)
        m_TcpMsgClient->disconnectFromHost();

    // 断开与服务器的连接，断开文件传输套接字
    if (m_TcpFileClient->state()==QAbstractSocket::ConnectedState)
        m_TcpFileClient->disconnectFromHost();
}


void MainDialog::on_pushButton_Exit_clicked()
{
    // 退出系统则关闭套接字
    m_TcpMsgClient->close();
    m_TcpFileClient->close();
    this->close();
}

// 清除消息命令按钮：代码实现
void MainDialog::on_pushButton_ClearMsg_clicked()
{
    ui->plainTextEdit_MsgList->clear();

}

// 【发送消息】命令按钮：代码实现
void MainDialog::on_pushButton_SendMsg_clicked()
{
    // 日期时间处理操作
    QDateTime CurrentDataTime=QDateTime::currentDateTime();
    QString strYear=QString::number(CurrentDataTime.date().year());
    QString strMonth=QString::number(CurrentDataTime.date().month());
    QString strDay=QString::number(CurrentDataTime.date().day());
    QString strHour=QString::number(CurrentDataTime.time().hour());
    QString strMinute=QString::number(CurrentDataTime.time().minute());
    QString strSecond=QString::number(CurrentDataTime.time().second());

    // 将年月日时分秒6个字符串连接
    QString strDateTimes=strYear+"/"+strMonth+"/"+strDay+" "+strHour+":"+strMinute+":"+strSecond;
    // QMessageBox::information(this,"提示",strDateTimes,QMessageBox::Yes);

    // 获取多行编辑器控件，用户所输入的消息
    QString strMsg=ui->plainTextEdit_SendMsg->toPlainText();

    //  判断发送出去的消息是否为空
    if(strMsg.isEmpty())
    {
        QMessageBox::critical(this,"错误","发送到服务器的数据信息不能为空，请重新检查？",QMessageBox::Yes);
        ui->plainTextEdit_SendMsg->setFocus(); // 将光标焦点定位到用户输入控件
        return;
    }

    // 将用户发送数据信息，显示到消息列表控件（plainTextEdit_MsgList对象控件）
    ui->plainTextEdit_MsgList->appendPlainText("[客户端消息"+strDateTimes+"]："+strMsg);
    ui->plainTextEdit_SendMsg->clear();
    ui->plainTextEdit_SendMsg->setFocus();

    // QByteArray用于处理字节数组的类，可以存储和访问二进制数据、图像、音频等。
    QByteArray bytearray=strMsg.toUtf8();
    bytearray.append('\n');

    // 将数据信息发送至服务器
    m_TcpMsgClient->write(bytearray);

}

// 【选择文件】命令按钮：代码实现
void MainDialog::on_pushButton_SelectFile_clicked()
{
    // 日期时间处理操作
    QDateTime CurrentDataTime=QDateTime::currentDateTime();
    QString strYear=QString::number(CurrentDataTime.date().year());
    QString strMonth=QString::number(CurrentDataTime.date().month());
    QString strDay=QString::number(CurrentDataTime.date().day());
    QString strHour=QString::number(CurrentDataTime.time().hour());
    QString strMinute=QString::number(CurrentDataTime.time().minute());
    QString strSecond=QString::number(CurrentDataTime.time().second());

    // 将年月日时分秒6个字符串连接
    QString strDateTimes=strYear+"/"+strMonth+"/"+strDay+" "+strHour+":"+strMinute+":"+strSecond;



    m_FileNames=QFileDialog::getOpenFileName(this,"请选择要传输的文件");
    if(!m_FileNames.isEmpty())
    {
        ui->plainTextEdit_MsgList->appendPlainText(QString("[%1 客户端打开文件为：%2 成功]").arg(strDateTimes).arg(m_FileNames));
        ui->pushButton_SendFile->setEnabled(true); // 启用【发送文件】命令按钮
    }


}

// 【发送文件】命令按钮：代码实现
void MainDialog::on_pushButton_SendFile_clicked()
{
    m_LocalFiles=new QFile(m_FileNames);
    if(!m_LocalFiles->open(QFile::ReadOnly))
    {
        qDebug()<<"打开文件错误，请重新检查？"<<endl; // 大家可以使用QMessageBox
        return ;
    }

    m_TotalBytes=m_LocalFiles->size(); // 求出要发送文件的大小

    QDataStream sendDataOut(&m_OutDataBlock,QIODevice::WriteOnly);
    QString strCurrentFileName=m_FileNames.right(m_FileNames.size()-m_FileNames.lastIndexOf('/')-1);

    // 依次写入文件总大小信息空间、文件名大小信息空间、文件名
    sendDataOut<<qint64(0)<<qint64(0)<<strCurrentFileName;

    // 总大小是文件名大小等信息和实际文件大小的总和
    m_TotalBytes=m_TotalBytes+m_OutDataBlock.size();

    sendDataOut.device()->seek(0);

    // 返回sendDataOut的开始，用实际的大小信息代替两个qint64(0)空间
    sendDataOut<<m_TotalBytes<<qint64((m_OutDataBlock.size()-sizeof(qint64)*2));

    // 发送完前面数据之后，剩下数据的大小
    m_BytesToWrites=m_TotalBytes-m_TcpFileClient->write(m_OutDataBlock);

    m_OutDataBlock.resize(0);
}
