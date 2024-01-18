#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <unordered_set>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static MyTcpServer& getInstance();

    bool forwardRequest(const char* targetName, PDU* pdu); // 单独转发

    bool forwardRequest(QStringList names, PDU* pdu); // 广播转发

    // This virtual function is called by QTcpServer when a new connection is available.
    virtual void incomingConnection(qintptr socketDescriptor); // 重写

public slots:
    void deleteSocket(MyTcpSocket* socket);

private:
    MyTcpServer();

    //QList<MyTcpSocket*> m_tcpSocketList;
    std::unordered_set<MyTcpSocket*> m_tcpSocketSet;

};

#endif // MYTCPSERVER_H
