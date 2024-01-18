#include "uploadfileworker.h"

UploadFileWorker::UploadFileWorker(const QString &clientPath, const QString &serverPath)
    :m_clientPath(QString::fromUtf8(clientPath.toStdString().c_str())),
      m_serverPath(serverPath),
      progress(0)
{

}

int UploadFileWorker::getProgress()
{
    return progress;
}

void UploadFileWorker::process()
{
    const int MAX_SIZE = 409600;
    qDebug() << "线程" << QThread::currentThreadId() << "上传文件: "
             << m_clientPath << " ===> " << m_serverPath;
    char buffer[MAX_SIZE] = {0}; // 缓冲区
    QFile file(m_clientPath);
    qint64 fileSize = file.size(); // 文件大小
    qint64 current = 0; // 当前已处理的数据
    file.open(QIODevice::ReadOnly); // 这里假设 一定可以打开
    int len = 0;
    while((len = file.read(buffer, MAX_SIZE)) > 0) { // 循环读数据
        int send_len = m_serverPath.size() + 1 + len; // Msg中, 保存路径 + \0 + 二进制文件数据
        PDU* pdu = mkPDU(send_len);
        pdu->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_CONTINUE; //上传文件内容
        *reinterpret_cast<int*>(pdu->caData) = len; // data域存放 数据的 大小
        memcpy(reinterpret_cast<char*>(pdu->caMsg), m_serverPath.toStdString().c_str(), m_serverPath.size()); // 拷贝路径
        memcpy(reinterpret_cast<char*>(pdu->caMsg) + m_serverPath.size() + 1,
               buffer, len);

        emit tcpWrite(reinterpret_cast<char*>(pdu), pdu->uiPDULen);

//        free(pdu); //不在此处free

        current += len;
//        emit updateProgress(static_cast<int>(current * 100.0 / fileSize));
        progress = static_cast<int>(current * 100.0 / fileSize);

        QThread::usleep(50);
    }

    QThread::msleep(1);

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_END; //上传文件完成
    *reinterpret_cast<int*>(pdu->caData) = 1; // data域存放 数据的 大小
    emit tcpWrite(reinterpret_cast<char*>(pdu), pdu->uiPDULen);

    QThread::msleep(200);
    emit finished();
}
