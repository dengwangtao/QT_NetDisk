#include "uploadfilethread.h"

UploadFileThread::UploadFileThread(const QString &clientPath, const QString &serverPath,
                                   const QString &IP, const quint16 port)
    :m_clientPath(QString::fromUtf8(clientPath.toStdString().c_str())),
      m_serverPath(serverPath),
      m_IP(IP),
      m_port(port)
{

}

void UploadFileThread::run()
{
    socket = new QTcpSocket; // 创建socket

    // 绑定连接成功信号
    connect(socket, &QTcpSocket::connected,
            this, SLOT(sendData()));
    // 断开连接后, 自动释放
    connect(socket, SIGNAL(disconnected()),
            socket, SLOT(deleteLater())
            );

    // 连接服务器
    socket->connectToHost(QHostAddress(m_IP), m_port);

}

void UploadFileThread::sendData()
{
    qDebug() << "线程" << UploadFileThread::currentThreadId() << "上传文件: "
             << m_clientPath << " ===> " << m_serverPath;
    char buffer[4096] = {0}; // 缓冲
    QFile file(m_clientPath);
    qint64 fileSize = file.size(); // 文件大小
    qint64 current = 0; // 当前已处理的数据
    file.open(QIODevice::ReadOnly); // 这里假设 一定可以打开
    int len = 0;
    while((len = file.read(buffer, 4096)) > 0) { // 循环读数据
        int send_len = m_serverPath.size() + 1 + len; // Msg中, 保存路径 + \0 + 二进制文件数据
        PDU* pdu = mkPDU(send_len);
        pdu->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_CONTINUE; //上传文件内容
        *reinterpret_cast<int*>(pdu->caData) = len; // data域存放 数据的 大小
        memcpy(reinterpret_cast<char*>(pdu->caMsg), m_serverPath.toStdString().c_str(), m_serverPath.size()); // 拷贝路径
        memcpy(reinterpret_cast<char*>(pdu->caMsg) + m_serverPath.size() + 1,
               buffer, len);

        socket->write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);

        free(pdu);

        current += len;
        emit updateProgress(currentThread(), static_cast<int>(current * 100.0 / fileSize));

        //this->msleep(100);
    }
    socket->close();
}


const QString &UploadFileThread::getClientPath()
{
    return this->m_clientPath;
}

const QString &UploadFileThread::getServerPath()
{
    return this->m_serverPath;
}

UploadFileThread::~UploadFileThread()
{
}
