#include "mytcpserver.h"

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

bool MyTcpServer::forwardRequest(const char *targetName, PDU *pdu)
{
    // 转发请求
    // 找到目标对应的socket
    for(MyTcpSocket* sock : this->m_tcpSocketSet) {
        if(0 == strcmp(targetName, sock->getName().toStdString().c_str())) { // 找到了对应的socket
            sock->write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
            return true;
        }
    }
    // 没有找到对应的socket, 发送失败
    return false;
}

bool MyTcpServer::forwardRequest(QStringList names, PDU *pdu)
{
    for(QString& name : names) {
        this->forwardRequest(name.toStdString().c_str(), pdu);
    }
    return true;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected" << socketDescriptor;
    MyTcpSocket* socket = new MyTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);
    //m_tcpSocketList.append(socket);
    m_tcpSocketSet.insert(socket);

    // 绑定下线的信号和槽
    connect(
                socket, SIGNAL(offline(MyTcpSocket*)),
                this, SLOT(deleteSocket(MyTcpSocket*))
            );
}

void MyTcpServer::deleteSocket(MyTcpSocket* socket)
{
    m_tcpSocketSet.erase(socket);
    qDebug() << socket->getName() << "正在下线";
    socket->deleteLater();
    // delete socket; // 释放内存
    qDebug() << socket->getName() << "已下线";
}
