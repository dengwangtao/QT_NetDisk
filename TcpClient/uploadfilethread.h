#ifndef UPLOADFILETHREAD_H
#define UPLOADFILETHREAD_H

#include <QThread>
#include <QDebug>
#include <QFile>
#include <QTcpSocket>
#include <QHostAddress>
#include "protocol.h"
#include "tcpclient.h"

class UploadFileThread : public QThread
{
    Q_OBJECT
private:
    QString m_clientPath;
    QString m_serverPath;
    QString m_IP;
    quint16 m_port;
    QTcpSocket* socket;
public:
    UploadFileThread(const QString& clientPath, const QString& serverPath,
                     const QString& IP, const quint16 port);

    void run() override;

    const QString& getClientPath();
    const QString& getServerPath();
    ~UploadFileThread();

public slots:
    void sendData();

signals:
    void updateProgress(QThread* who, int progress);
    void tcpWrite(char* who, int len);
};

#endif // UPLOADFILETHREAD_H
