#include "privatechat.h"
#include "ui_privatechat.h"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);

    // 设置当前的用户名
    this->m_userName = TcpClient::getInstance().getUserName();
}

PrivateChat::PrivateChat(const QString &chatName):
    QWidget(nullptr),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);

    // 设置当前的用户名
    this->m_userName = TcpClient::getInstance().getUserName();
    // 设置聊天对象
    this->m_chatName = chatName;
}

PrivateChat::~PrivateChat()
{
    delete ui;
}


void PrivateChat::setChatName(const QString &chatName)
{
    this->m_chatName = chatName;
}

void PrivateChat::addMsg(const QString& sender, const QString &msg)
{
    QDateTime datetime(QDateTime::currentDateTime());

    ui->showMsg_lw->addItem(QString("[%1] %2: %3")
                            .arg(datetime.toString("MM-dd hh:mm:ss"))
                            .arg(sender)
                            .arg(msg));
}

void PrivateChat::offline(const QString &chatName)
{
    QDateTime datetime(QDateTime::currentDateTime());

    QListWidgetItem* item = new QListWidgetItem(QString("[%1] <提示>: %2 未收到消息")
                                                .arg(datetime.toString("MM-dd hh:mm:ss"))
                                                .arg(chatName));
    item->setForeground(QBrush(Qt::gray));
    ui->showMsg_lw->addItem(item);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString msg = ui->inputMsg_le->text();
    if(msg.isEmpty()) { // 消息框为空
        return;
    }
    PDU* pdu = mkPDU(msg.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_CHAT_REQUEST; // 发送消息

    memcpy(pdu->caData, m_userName.toStdString().c_str(), m_userName.size());
    memcpy(pdu->caData + 32, m_chatName.toStdString().c_str(), m_chatName.size());

    memcpy(pdu->caMsg, msg.toStdString().c_str(), msg.size()); // 拷贝消息

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu),  pdu->uiPDULen);
    free(pdu);
}
