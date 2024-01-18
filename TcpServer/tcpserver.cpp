#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    loadConfig(); // 加载配置文件

    // 开始监听
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream ssData(&file);

        this->m_strIP = ssData.readLine();
        this->m_usPort = ssData.readLine().toUShort();

        qDebug() << "监听 " << this->m_strIP << ":" << this->m_usPort;
        file.close();
    } else {
        QMessageBox::critical(this, "open config file error", "open config file failed");
    }
}
