#include "online.h"
#include "ui_online.h"

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showAllOnlineUsr(PDU *pdu)
{
    if(nullptr == pdu) {
        return;
    }
    ui->online_lw->clear();
    uint len = pdu->uiMsgLen / 32;
    char temp[32];
    for(uint i = 0; i < len; ++ i) {
        memcpy(temp, reinterpret_cast<char*>(pdu->caMsg) + i * 32, 32);
        ui->online_lw->addItem(temp);
    }
}

void Online::on_addfriend_pb_clicked()
{
    QListWidgetItem *items = ui->online_lw->currentItem();
    if(nullptr == items) {
        return;
    }
    // 要添加好友的用户名
    QString addUserName = items->text();
    // 获取当前登录用户名
    QString userName = TcpClient::getInstance().getUserName();

    qDebug() << "添加好友" << addUserName;

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    // 前32个字节为请求发送者
    memcpy(pdu->caData, userName.toStdString().c_str(), std::min(32, userName.size()));
    // 后32字节为 好友用户名
    memcpy(pdu->caData + 32, addUserName.toStdString().c_str(), std::min(32, addUserName.size()));

    TcpClient::getInstance()
            .getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);

    free(pdu);
}
