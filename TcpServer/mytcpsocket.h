#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QDebug>
#include "protocol.h"
#include "dbhelper.h"
#include <QStringList>
#include <QDebug>
#include "mydir.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    virtual ~MyTcpSocket();

    const QString& getName() const;

signals:
    void offline(MyTcpSocket* socket);

public slots:
    void recvMsg_Shot();

    void clientOffline();

private:
    void recvMsg();

    void downloadFile(const QString& serverPath, const QString& clientPath);

    qint64 bytesAvailable(); // 缓冲区可读的字节数

    QByteArray readBuffer; // 从网络中读取数据的缓冲区
    int readStart;      // 当前缓冲区的可读位置
    int readEnd;        // 当前数据的可写位置


    QString m_strName;
};

#endif // MYTCPSOCKET_H
