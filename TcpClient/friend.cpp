#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"

Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pShowMsgLW = new QListWidget; // 群聊信息显示
    m_pFriendListWidget = new QListWidget; // 好友列表
    m_pFriendListWidget->setMaximumWidth(300);

    m_pInputMsgLE = new QLineEdit; // 消息输入框

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUserPB = new QPushButton("显示在线用户");
    m_pSearchsUserPB = new QPushButton("查找用户");

    m_pMsgSendPB = new QPushButton("向全部好友发送");
    m_pPrivateChatPB = new QPushButton("私聊");

    m_pOnline = new Online;

    // 按钮垂直布局
    QVBoxLayout* pRightPBVBL = new QVBoxLayout();
    pRightPBVBL->addStretch();
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUserPB);
    pRightPBVBL->addWidget(m_pSearchsUserPB);
    pRightPBVBL->addStretch();

    // 上部分的水平布局
    QHBoxLayout* pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgLW);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    // 发送消息部分
    QHBoxLayout* pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);
    pMsgHBL->addWidget(m_pPrivateChatPB);

    QVBoxLayout* pAllVBL = new QVBoxLayout;
    pAllVBL->addLayout(pTopHBL);
    pAllVBL->addLayout(pMsgHBL);
    pAllVBL->addWidget(m_pOnline);

    m_pOnline->hide(); // 当前在线用户窗口

    this->setLayout(pAllVBL);

    // 配置信号, 查看当前所有在线用户
    connect(m_pShowOnlineUserPB, SIGNAL(clicked()),
            this, SLOT(showOnline()));

    // 配置信号, 搜索用户
    connect(m_pSearchsUserPB, SIGNAL(clicked()),
            this, SLOT(searchUser())
            );

    // 配置信号, 刷新好友
    connect(m_pFlushFriendPB, SIGNAL(clicked()),
            this, SLOT(flushFriend())
                );

    // 配置信号, 删除好友
    connect(m_pDelFriendPB, SIGNAL(clicked()),
            this, SLOT(deleteFriend())
                );

    // 配置信号, 私聊好友
    connect(m_pPrivateChatPB, SIGNAL(clicked()),
            this, SLOT(privateChat())
                );

    // 配置信号, 广播消息(好友)
    connect(m_pMsgSendPB, SIGNAL(clicked()),
            this, SLOT(broadcastFriend()));
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if(nullptr == pdu) {
        return;
    }
    this->m_pOnline->showAllOnlineUsr(pdu);
}

void Friend::showAllFriends(PDU *pdu)
{
    uint flen = pdu->uiMsgLen / 32;
    char tempName[32] = {0};
    this->m_pFriendListWidget->clear();
    for(uint i = 0; i < flen; ++ i) {
        memcpy(tempName, reinterpret_cast<char*>(pdu->caMsg) + 32 * i, 32);
        this->m_pFriendListWidget->addItem(QString(tempName));
    }
}

QListWidget& Friend::getFriendListW()
{
    return *this->m_pFriendListWidget;
}

void Friend::recvBroadcastMsg(PDU *pdu)
{
    char sender[32] = {0};
    memcpy(sender, pdu->caData, 32);


    QDateTime datetime(QDateTime::currentDateTime());

    QListWidgetItem* item = new QListWidgetItem(QString("[%1]<广播> %2: %3")
                                                .arg(datetime.toString("MM-dd hh:mm:ss"))
                                                .arg(sender)
                                                .arg(reinterpret_cast<char*>(pdu->caMsg)));
    item->setForeground(QBrush(Qt::blue));
    m_pShowMsgLW->addItem(item);
}

// 显示当前在线用户 槽函数
void Friend::showOnline()
{
    if(m_pOnline->isHidden()) {
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance()
                .getTcpSocket()
                .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
        m_pOnline->show();

    } else {
        m_pOnline->hide();
    }
}

void Friend::searchUser()
{
    QString userName = QInputDialog::getText(this, "搜索用户", "用户名: ");
    if(!userName.isEmpty()) {
//        qDebug() << userName;
        this->m_searchName = userName;
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
        memcpy(pdu->caData, userName.toStdString().c_str(), std::min(userName.size(), 32));
        TcpClient::getInstance()
                .getTcpSocket()
                .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
        free(pdu);
    }
}

void Friend::flushFriend()
{
    QString name = TcpClient::getInstance().getUserName();
    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, name.toStdString().c_str(), name.size());
    TcpClient::getInstance()
           .getTcpSocket()
           .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
   free(pdu);
}

void Friend::deleteFriend()
{
    QListWidgetItem* selected = m_pFriendListWidget->currentItem();
    if(selected == nullptr) {
        return;
    }

    QString name = TcpClient::getInstance().getUserName(); // 用户名
    QString fname = selected->text(); // 好友名

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    memcpy(pdu->caData,     name.toStdString().c_str(),     std::max(32, name.size()));
    memcpy(pdu->caData + 32,fname.toStdString().c_str(),    std::max(32, fname.size()));
    TcpClient::getInstance()
           .getTcpSocket()
           .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void Friend::privateChat()
{
    QListWidgetItem* selected = m_pFriendListWidget->currentItem();
    if(selected == nullptr) {
        return;
    }

    QString fname = selected->text(); // 好友名

    PrivateChatGroup& chatGroup = PrivateChatGroup::getInstance(); // 获取单例

    chatGroup.addChat(fname);
    if(chatGroup.isHidden()) {
        chatGroup.show();
    }
}

void Friend::broadcastFriend()
{
    QString name = TcpClient::getInstance().getUserName(); // 用户名
    QString msg = m_pInputMsgLE->text();
    if(msg.isEmpty()) {
        return;
    }

    PDU* pdu = mkPDU(msg.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_BROADCAST_FRIEND_MESSAGE; // 好友广播
    memcpy(pdu->caData, name.toStdString().c_str(), std::max(32, name.size()));
    memcpy(pdu->caMsg, msg.toStdString().c_str(), msg.size());
    TcpClient::getInstance()
           .getTcpSocket()
           .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}
