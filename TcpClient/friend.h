#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "online.h"
#include "protocol.h"
#include <QInputDialog>
#include "privatechatgroup.h"
#include "privatechat.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = 0);

    void showAllOnlineUsr(PDU* pdu);

    void showAllFriends(PDU* pdu);

    QString m_searchName; // 临时保存搜索用户的名称

    QListWidget& getFriendListW();

    void recvBroadcastMsg(PDU* pdu);

signals:

public slots:
    void showOnline();
    void searchUser();
    void flushFriend();
    void deleteFriend();
    void privateChat();
    void broadcastFriend();

private:
    QListWidget* m_pShowMsgLW;
    QListWidget* m_pFriendListWidget;
    QLineEdit* m_pInputMsgLE;

    QPushButton* m_pDelFriendPB;
    QPushButton* m_pFlushFriendPB;
    QPushButton* m_pShowOnlineUserPB;
    QPushButton* m_pSearchsUserPB;

    QPushButton* m_pMsgSendPB;
    QPushButton* m_pPrivateChatPB;

    Online* m_pOnline;
};

#endif // FRIEND_H
