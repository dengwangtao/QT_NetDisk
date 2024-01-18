#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"
#include "tcpclient.h"
#include <QDateTime>

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = 0);
    explicit PrivateChat(const QString& chatName);
    ~PrivateChat();

    void setChatName(const QString& chatName); // 设置聊天对象的名称

    void addMsg(const QString& sender, const QString& msg); // 收到消息

    void offline(const QString& chatName); // 聊天对象未收到消息

private slots:
    void on_sendMsg_pb_clicked();

private:
    Ui::PrivateChat *ui;

    QString m_chatName; // 聊天对象的名称
    QString m_userName; // 本地用户名

//    QStringList history; // 聊天记录
};

#endif // PRIVATECHAT_H
