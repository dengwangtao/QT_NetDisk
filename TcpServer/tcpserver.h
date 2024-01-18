#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include "mytcpserver.h"

namespace Ui {
class TcpServer;
}

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    explicit TcpServer(QWidget *parent = 0);
    ~TcpServer();

    void loadConfig();

private:
    Ui::TcpServer *ui;

    QString m_strIP; // IP地址
    quint16 m_usPort;// 端口号
};

#endif // TCPSERVER_H
