#ifndef PRIVATECHATGROUP_H
#define PRIVATECHATGROUP_H

#include <QWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <unordered_map>
#include "privatechat.h"

class PrivateChatGroup : public QWidget
{
    Q_OBJECT
    explicit PrivateChatGroup(QWidget *parent = 0);
public:
    static PrivateChatGroup& getInstance();

    void addChat(const QString& fname); // 添加聊天对象

    void recvPrivateChat(PDU* pdu); // 收到聊天消息

    void sendOkPrivateChat(PDU* pdu); // 发送聊天消息成功

    void sendFailPrivateChat(PDU* pdu); // 发送聊天消息失败
private:
    void setCurrentIndex();
signals:

public slots:

private:
    QListWidget* m_pListW;
    std::unordered_map<std::string, PrivateChat*> str2chat; // 用户名和聊天界面的对应

    QStackedWidget* m_pSW;
};

#endif // PRIVATECHATGROUP_H
