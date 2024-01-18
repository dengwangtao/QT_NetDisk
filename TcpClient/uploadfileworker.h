#ifndef UPLOADFILEWORKER_H
#define UPLOADFILEWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>
#include <QFile>
#include "protocol.h"

class UploadFileWorker: public QObject
{
    Q_OBJECT
private:
    QString m_clientPath;
    QString m_serverPath;
    int progress;
public:
    UploadFileWorker(const QString& clientPath, const QString& serverPath);
    int getProgress();

public slots:
    void process();

signals:
    void updateProgress(int progress);
    void tcpWrite(char* data, int len);
    void finished();
};

#endif // UPLOADFILEWORKER_H
