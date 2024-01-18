#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>

namespace Ui {
class TcpClient;
}

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    static TcpClient& getInstance();

    ~TcpClient();

    void loadConfig();

    const QString& getUserName() const;

    const QString& getCurrentDir() const;

    void setCurrentDir(const QString& dirName);

    QTcpSocket& getTcpSocket();

    QString getIP() const;
    quint16 getPort() const;

private:
    explicit TcpClient(QWidget *parent = 0);

    void recvMsg();

    qint64 bytesAvailable(); // 缓冲区可读的字节数

public slots:
    void showConnect();

    void recvMsg_Shot(); // 收到信息的槽函数

    void tcpWrite(char* data, int len); // Tcp写数据
//    void tcpSend(); // Tcp写数据

private slots:
//    void on_send_btn_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_logout_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;    // IP地址
    quint16 m_usPort;   //端口号

    QTcpSocket m_tcpSocket; // 和Server进行交互的socket

    QString m_userName; // 当前用户的用户名

    QString m_currentDir; // 当前所在目录


    QByteArray readBuffer; // 从网络中读取数据的缓冲区
    int readStart;      // 当前缓冲区的可读位置
    int readEnd;        // 当前数据的可写位置
};

#endif // TCPCLIENT_H
