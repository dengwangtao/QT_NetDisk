#include "privatechatgroup.h"

PrivateChatGroup::PrivateChatGroup(QWidget *parent) : QWidget(parent)
{
    m_pListW = new QListWidget(this);
    m_pSW = new QStackedWidget;

    QHBoxLayout* allLayout = new QHBoxLayout;
    allLayout->addWidget(m_pListW);
    allLayout->addWidget(m_pSW);

    this->setLayout(allLayout);

    connect(m_pListW,
            static_cast<void (QListWidget::*)(const QString&)>(&QListWidget::currentTextChanged),
            [this](const QString& item) {
                m_pSW->setCurrentWidget(str2chat[item.toStdString()]);
    });

    QString name = TcpClient::getInstance().getUserName(); // 用户名
    this->setWindowTitle(QString("%1 聊天").arg(name));
}

PrivateChatGroup &PrivateChatGroup::getInstance()
{
    static PrivateChatGroup instance;
    return instance;
}

void PrivateChatGroup::addChat(const QString &fname)
{
    if(str2chat.count(fname.toStdString())) {
        return;
    }
    str2chat[fname.toStdString()] = new PrivateChat(fname);

    m_pListW->addItem(fname); // 添加这一项
    m_pSW->addWidget(str2chat[fname.toStdString()]);

    // 选中
    for(QListWidgetItem* item : m_pListW->findItems(fname, Qt::MatchExactly)) {
        m_pListW->setCurrentItem(item);
        break;
    }
}

void PrivateChatGroup::recvPrivateChat(PDU *pdu)
{
    char sender[32] = {0};  // 发送方
//    char reciver[32] = {0}; // 接收方
    memcpy(sender, pdu->caData, 32);
//    memcpy(reciver, pdu->caData + 32, 32);


    std::string sender_str(sender);
    QString sender_qstr(sender);

    if(str2chat.count(sender_str) == 0) {   // 不存在这个聊天对象
        this->addChat(sender_qstr);         // 创建这个聊天对象
    }

    // 选中
    for(QListWidgetItem* item : m_pListW->findItems(sender_qstr, Qt::MatchExactly)) {
        m_pListW->setCurrentItem(item);
        break;
    }

    str2chat[sender_str]->addMsg(sender_qstr, QString(reinterpret_cast<char*>(pdu->caMsg)));

    if(this->isHidden()) {
        this->show();
    }
}

void PrivateChatGroup::sendOkPrivateChat(PDU *pdu)
{
    char sender[32] = {0};  // 发送方
    char reciver[32] = {0}; // 信息接收方
    memcpy(sender, pdu->caData, 32);
    memcpy(reciver, pdu->caData + 32, 32);


    std::string reciver_str(reciver);
    QString reciver_qstr(reciver);

    QString sender_qstr(sender);

    // 选中
    for(QListWidgetItem* item : m_pListW->findItems(reciver_qstr, Qt::MatchExactly)) {
        m_pListW->setCurrentItem(item);
        break;
    }

    str2chat[reciver_str]->addMsg(sender_qstr, QString(reinterpret_cast<char*>(pdu->caMsg)));

    if(this->isHidden()) {
        this->show();
    }
}

void PrivateChatGroup::sendFailPrivateChat(PDU *pdu)
{
    char name[32] = {0};
    memcpy(name, pdu->caData, 32);

    str2chat[std::string(name)]->offline(QString(name));
}
