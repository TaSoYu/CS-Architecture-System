#include "maindialog.h"
#include "ui_maindialog.h"

#pragma execution_character_set("utf-8")

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);


    // 获取本地IP地址，将其显示到组合框控件（下拉列表和下拦菜单功能）
    QString localIP=GetLocalIPAddressFunc();//本机IP
    ui->comboIP->addItem(localIP);


    // 将获取IP地址显示至Label控件
    QString strip=("本机IP："+localIP);
    ui->label_stateip->setText(strip);


    // 初始化变量为0
    m_FileTotalBytes = 0;
    m_FileBytesReceived = 0;
    m_FileNameSizes = 0;


    P_TCPServerObject = new QTcpServer(this);
    connect(P_TCPServerObject, SIGNAL(newConnection()), this, SLOT(OnNewConnectionFunc()));


    // 新建处理聊天消息传输套接字和文件传输套接字
    P_TCPTextSocketObject = new QTcpSocket(this);
    P_TCPFileSocketObject = new QTcpSocket(this);


    // 启动监听和停止监听命令按钮切换
    ui->pushButton_StartListening->setEnabled(true);
    ui->pushButton_StopListening->setEnabled(false);


    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(width(),height()); // 固定窗口宽度和高度
    // 设置多行文本编辑器控件里面的文本为只读属性
    ui->plainTextEdit->setReadOnly(true);


    // 设置字体颜色
    QPalette platestr1=ui->label_server->palette();
    platestr1.setColor(QPalette::WindowText, Qt::yellow);// 设置字体颜色为黄色
    ui->label_server->setPalette(platestr1);
    // 设置字体加粗
    QFont font1=ui->label_server->font();
    font1.setBold(true); // 设置字体加粗
    ui->label_server->setFont(font1);


    // 设置字体颜色
    QPalette platestr2=ui->label_port->palette();
    platestr2.setColor(QPalette::WindowText, Qt::yellow); // 设置字体颜色为黄色
    ui->label_port->setPalette(platestr2);
    // 设置字体加粗
    QFont font2=ui->label_port->font();
    font2.setBold(true); // 设置字体加粗
    ui->label_port->setFont(font2);


    // 设置字体颜色
    QPalette platestr3=ui->groupBox->palette();
    platestr3.setColor(QPalette::WindowText, Qt::yellow); // 设置字体颜色为黄色
    ui->groupBox->setPalette(platestr3);
    // 设置字体加粗
    QFont font3=ui->groupBox->font();
    font3.setBold(true); // 设置字体加粗
    ui->groupBox->setFont(font3);


    // 设置字体颜色
    QPalette platestr4=ui->groupBox_2->palette();
    platestr4.setColor(QPalette::WindowText, Qt::yellow); // 设置字体颜色为黄色
    ui->groupBox_2->setPalette(platestr4);
    // 设置字体加粗
    QFont font4=ui->groupBox_2->font();
    font4.setBold(true); // 设置字体加粗
    ui->groupBox_2->setFont(font4);


    // 设置字体颜色
    QPalette platestr5=ui->label_3->palette();
    platestr5.setColor(QPalette::WindowText, Qt::yellow); // 设置字体颜色为黄色
    ui->label_3->setPalette(platestr5);
    // 设置字体加粗
    QFont font5=ui->label_3->font();
    font5.setBold(true); // 设置字体加粗
    ui->label_3->setFont(font5);


    // 设置字体颜色
    QPalette platestr6=ui->progressBar->palette();
    platestr6.setColor(QPalette::WindowText, Qt::yellow); // 设置字体颜色为黄色
    ui->progressBar->setPalette(platestr6);
    // 设置字体加粗
    QFont font6=ui->progressBar->font();
    font6.setBold(true); // 设置字体加粗
    ui->progressBar->setFont(font6);

}

MainDialog::~MainDialog()
{
    delete ui;
}

// 获取本机IP地址信息
QString MainDialog::GetLocalIPAddressFunc()
{
    QString hostName = QHostInfo::localHostName();      // 获取本地主机名称
    QHostInfo hostInfo = QHostInfo::fromName(hostName); // 通过主机名获取IP地址
    QString StrLocalIP = "";                               // 定义StrLocalIP存储IP地址

    QList<QHostAddress> addList = hostInfo.addresses();
    if (!addList.isEmpty())
    {
        for (int i=0;i<addList.count();i++)
        {
            QHostAddress aHost = addList.at(i);

            // 判断本机IP协议是否为IPv4
            if (QAbstractSocket::IPv4Protocol == aHost.protocol())
            {
                StrLocalIP = aHost.toString();
                break;
            }
        }
    }

    return StrLocalIP;
}

// 关闭窗口时，则服务器端停止监听
void MainDialog::closeEvent(QCloseEvent *event)
{
    //关闭窗口时停止监听
    if (P_TCPServerObject->isListening())
        P_TCPServerObject->close();
    event->accept();
}

// QTcpServer中专用处理newConnection()信号
void MainDialog::OnNewConnectionFunc()
{
    // 处理聊天数据消息
    if (P_TCPTextSocketObject->state() == QAbstractSocket::UnconnectedState)
    {
        P_TCPTextSocketObject = P_TCPServerObject->nextPendingConnection();

        // 客户端接入时，则输出数据消息
        connect(P_TCPTextSocketObject, SIGNAL(connected()), this, SLOT(OnClientConnectedFunc()));

        // 客户端断开连接时，则输出提示数据信息
        connect(P_TCPTextSocketObject, SIGNAL(disconnected()),this, SLOT(OnClientDisconnectedFunc()));

        // 读取缓冲区行文本数据信息
        connect(P_TCPTextSocketObject,SIGNAL(readyRead()),this,SLOT(OnSocketReadyReadFunc()));
        return;
    }

    // 处理文件传输操作
    if (P_TCPFileSocketObject->state() == QAbstractSocket::UnconnectedState)
    {
        P_TCPFileSocketObject = P_TCPServerObject->nextPendingConnection();

        // 客户端连接成功，则输出提示数据信息
        connect(P_TCPFileSocketObject, SIGNAL(connected()),this, SLOT(OnFileClientConnectedFunc()));

        // 客户端断开连接，则输出提示数据信息
        connect(P_TCPFileSocketObject, SIGNAL(disconnected()),this, SLOT(OnFileClientDisconnectedFunc()));

        // 更新进度条，接收文件数据信息
        connect(P_TCPFileSocketObject, SIGNAL(readyRead()), this, SLOT(UpdateServerProgressFunc()));

        // 错误提示信息
        connect(P_TCPFileSocketObject, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(DisplayErrorInfoFunc(QAbstractSocket::SocketError)));
    }
    ui->plainTextEdit->appendPlainText("\n[**********有客户端连接**********]");
}



void MainDialog::OnClientConnectedFunc()
{
    // 客户端接入时，则输出提示数据信息

}

// 处理断开连接，client数据文本信息Socket disconnected
void MainDialog::OnClientDisconnectedFunc()
{
    // 客户端断开连接时，则输出提示数据信息
    ui->plainTextEdit->appendPlainText("[**********客户端已断开和服务器端连接**********]");
    P_TCPTextSocketObject->close();
}

// 读取客户端socket文本数据信息
void MainDialog::OnSocketReadyReadFunc()
{
    // 处理日期时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString year = QString::number(currentDateTime.date().year());
    QString month = QString::number(currentDateTime.date().month());
    QString day = QString::number(currentDateTime.date().day());
    QString hour = QString::number(currentDateTime.time().hour());
    QString minute = QString::number(currentDateTime.time().minute());
    QString second = QString::number(currentDateTime.time().second());
    QString strtemp=year+"/"+month+"/"+day+" "+hour+":"+minute+":"+second;

    while(P_TCPTextSocketObject->canReadLine())
        ui->plainTextEdit->appendPlainText("[客户端消息 "+strtemp+"]："+P_TCPTextSocketObject->readLine());

}

// 更新进度条接收数据动态变化
void MainDialog::UpdateServerProgressFunc()
{
    // 处理日期时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString year = QString::number(currentDateTime.date().year());
    QString month = QString::number(currentDateTime.date().month());
    QString day = QString::number(currentDateTime.date().day());
    QString hour = QString::number(currentDateTime.time().hour());
    QString minute = QString::number(currentDateTime.time().minute());
    QString second = QString::number(currentDateTime.time().second());
    QString strtemp=year+"/"+month+"/"+day+" "+hour+":"+minute+":"+second;

    QDataStream in(P_TCPFileSocketObject);
    if (m_FileBytesReceived <= sizeof(qint64)*2)
    {
        if((P_TCPFileSocketObject->bytesAvailable() >= sizeof(qint64)*2) && (m_FileNameSizes == 0))
        {
            //接收数据总大小信息和文件名大小信息
            in >> m_FileTotalBytes >> m_FileNameSizes;
            m_FileBytesReceived += sizeof(qint64) * 2;
        }

        // bytesAvailable()用于获取当前可读取的字节数。
        if((P_TCPFileSocketObject->bytesAvailable() >= m_FileNameSizes) && (m_FileNameSizes != 0))
        {
            //接收文件名，并建立文件
            in >> m_FileNames;
            ui->plainTextEdit->appendPlainText(tr("[%1 ♥♥♥♥♥♥接收文件 %2 ...♥♥♥♥♥♥]").arg(strtemp).arg(m_FileNames));
            m_FileBytesReceived += m_FileNameSizes;
            m_LocalFiles= new QFile(QApplication::applicationDirPath() + "\\" + m_FileNames);
            if(!m_LocalFiles->open(QFile::WriteOnly))
            {
                qDebug() << "打开文件错误，请重新检查？";
                return;
            }
        }
        else
            return;
    }

    // 已经收到文件数据大小总数据<文件数据大小,则继续写入文件
    if(m_FileBytesReceived < m_FileTotalBytes)
    {
        m_FileBytesReceived += P_TCPFileSocketObject->bytesAvailable();
        m_InBlocks= P_TCPFileSocketObject->readAll(); // 使用 readAll() 函数将文件中的全部数据读取到 data 变量中
        m_LocalFiles->write(m_InBlocks); // 将整块数据写入文件
        m_InBlocks.resize(0); // 然后清零
    }

    //更新进度条
    ui->progressBar->setMaximum(m_FileTotalBytes);
    ui->progressBar->setValue(m_FileBytesReceived);

    if(m_FileBytesReceived == m_FileTotalBytes)
    {
        //接收数据完成时
        m_FileTotalBytes = 0;
        m_FileBytesReceived = 0;
        m_FileNameSizes = 0;
        m_LocalFiles->close();
        ui->plainTextEdit->appendPlainText(QString("[%1 ♥♥♥♥♥♥接收文件 %2 成功♥♥♥♥♥♥]").arg(strtemp).arg(m_FileNames));
    }
}

// 处理显示错误信息告诉用户
void MainDialog::DisplayErrorInfoFunc(QAbstractSocket::SocketError)
{
    P_TCPFileSocketObject->close();
    ui->progressBar->reset();
}

void MainDialog::on_pushButton_StartListening_clicked()
{
    //开始监听
    QString IP = ui->comboIP->currentText();
    quint16 port = ui->spinPort->value();
    QHostAddress addr(IP);

    P_TCPServerObject->listen(addr, port);
    ui->plainTextEdit->appendPlainText("[★★★★★服务器已开始监听★★★★★]");
    ui->plainTextEdit->appendPlainText("[IP Address："+P_TCPServerObject->serverAddress().toString()+"]"
                                       +"[Port："+QString::number(P_TCPServerObject->serverPort())+"]");

    ui->pushButton_StartListening->setEnabled(false);
    ui->pushButton_StopListening->setEnabled(true);
}

void MainDialog::on_pushButton_StopListening_clicked()
{
    //停止监听
    if (P_TCPServerObject->isListening())
    {
        P_TCPServerObject->close();
        ui->pushButton_StartListening->setEnabled(true);
        ui->pushButton_StopListening->setEnabled(false);
        ui->plainTextEdit->appendPlainText("[**********服务器已停止监听**********]\n");
    }
}


void MainDialog::on_pushButton_SendData_clicked()
{
    // 处理日期时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString year = QString::number(currentDateTime.date().year());
    QString month = QString::number(currentDateTime.date().month());
    QString day = QString::number(currentDateTime.date().day());
    QString hour = QString::number(currentDateTime.time().hour());
    QString minute = QString::number(currentDateTime.time().minute());
    QString second = QString::number(currentDateTime.time().second());
    QString strtemp=year+"/"+month+"/"+day+" "+hour+":"+minute+":"+second;

    //发送一行字符串，以换行符结束
    QString  msg=ui->editMsg->toPlainText();

    if (msg.isEmpty())
    {
        QMessageBox::critical(this, "错误", "发送消息不能为空，请重新检查？");
        ui->editMsg->setFocus();
        return;
    }

    ui->plainTextEdit->appendPlainText("[服务器消息 "+strtemp+"]："+msg);
    ui->editMsg->clear();


    QByteArray  str=msg.toUtf8();
    str.append('\n');   // 添加一个换行符
    P_TCPTextSocketObject->write(str);
}

void MainDialog::on_pushButton_ClearMsg_clicked()
{
    ui->plainTextEdit->clear();
}

void MainDialog::on_pushButton_ExitSystem_clicked()
{
    this->close();
}

// 处理文件接收socket操作信息提示
void MainDialog::OnFileClientConnectedFunc()
{
    ui->plainTextEdit->appendPlainText("[♥♥♥♥♥♥♥♥♥客户端套接字已连接成功♥♥♥♥♥♥♥♥♥]");
}

// 断开文件接收socket操作
void MainDialog::OnFileClientDisconnectedFunc()
{
    P_TCPFileSocketObject->close();
}

// 更改窗口背景图片
void MainDialog::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    painter.drawPixmap(rect(),QPixmap(":/new/prefix1/images/bgpng4.png"),QRect());
}


