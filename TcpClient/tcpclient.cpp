#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QTextStream>
#include <QHostAddress>
#include "protocol.h"
#include "operatewidget.h"

TcpClient::TcpClient(QWidget *parent) : QWidget(parent), ui(new Ui::TcpClient),
    readStart(0),
    readEnd(0),
    readBuffer(QByteArray(4096, '\0')) // 初始化缓冲区4096
{
    ui->setupUi(this);
    loadConfig();

    // 绑定连接成功信号
    connect(&m_tcpSocket, SIGNAL(connected()),
            this, SLOT(showConnect()));

    // 绑定收到消息信号
    connect(&m_tcpSocket, SIGNAL(readyRead()),
            this, SLOT(recvMsg_Shot()));

    // 连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
}

// TCP连接成功的槽函数
void TcpClient::showConnect()
{
    //    QMessageBox::about(this, "connect", "连接服务器成功");
}

void TcpClient::recvMsg_Shot()
{
    QByteArray data = m_tcpSocket.readAll();
    int data_len = data.size(); // 新数据的长度
    if(readBuffer.size() - readEnd < data_len) {
        // 1. 之前的数据,往前挪, 是否可以放入
        if((readStart) + (readBuffer.size() - readEnd) >= data_len) {
            // 将未读的数据前移
            memcpy(readBuffer.data(), readBuffer.data() + readStart, readEnd - readStart);
            readEnd -= readStart;
            readStart = 0;
            // 放入新数据
            memcpy(readBuffer.data() + readEnd, data.data(), data_len);
        } else { // 无论如何都放不下的话, 扩容
            readBuffer.resize(readEnd + data_len);
        }
    }
    memcpy(readBuffer.data() + readEnd, data.data(), data_len); // 放入新数据
    readEnd += data_len;

    recvMsg();
}

const QString& TcpClient::getCurrentDir() const
{
    return this->m_currentDir;
}

void TcpClient::setCurrentDir(const QString &dirName)
{
    this->m_currentDir = dirName;
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient lo;
    return lo;
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream ssData(&file);

        this->m_strIP = ssData.readLine();
        this->m_usPort = ssData.readLine().toUShort();

        qDebug() << this->m_strIP << " " << this->m_usPort;
        file.close();
    } else {
        QMessageBox::information(this, "open config file error", "open config file failed");
    }

}

const QString &TcpClient::getUserName() const
{
    return this->m_userName;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return this->m_tcpSocket;
}

QString TcpClient::getIP() const
{
    return m_strIP;
}

quint16 TcpClient::getPort() const
{
    return m_usPort;
}


#if 0
void TcpClient::on_send_btn_clicked()
{
    QString msg = ui->lineEdit->text();
    if(msg.isEmpty()) return;
    PDU* pdu = mkPDU(msg.size() + 1); // 创建协议数据单元
    pdu->uiMsgType = 8888;
    memcpy(pdu->caMsg, msg.toStdString().c_str(), msg.size());
    m_tcpSocket.write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}
#endif

// 登录按钮点击
void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(strName.isEmpty() || strPwd.isEmpty()) {
        // 用户名或者密码为空
        QMessageBox::information(this, "提示", "用户名或者密码不能为空");
        return;
    }

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
    // 前32字节放置用户名
    strncpy(pdu->caData, strName.toStdString().c_str(), std::min(32, strName.size()));
    // 后32字节放置 密码
    strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), std::min(32, strPwd.size()));

    // 发送
    m_tcpSocket.write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 注册按钮点击
void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(strName.isEmpty() || strPwd.isEmpty()) {
        // 用户名或者密码为空
        QMessageBox::information(this, "提示", "用户名或者密码不能为空");
        return;
    }

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
    // 前32字节放置用户名
    strncpy(pdu->caData, strName.toStdString().c_str(), std::min(32, strName.size()));
    // 后32字节放置 密码
    strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), std::min(32, strPwd.size()));

    // 发送
    m_tcpSocket.write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void TcpClient::on_logout_pb_clicked()
{

}


// 接受网络消息
void TcpClient::recvMsg()
{
    //    uint pdu_len = 0;
    //    m_tcpSocket.read(reinterpret_cast<char*>(&pdu_len), sizeof(uint)); // 接受到的PDU的总长度
    //    uint msg_len = pdu_len - sizeof(PDU); // 消息的长度
    //    PDU* pdu = mkPDU(msg_len); // 声明一个消息域大小为msg_len的PDU
    //    pdu->uiPDULen = pdu_len;
    //    m_tcpSocket.read(
    //                reinterpret_cast<char*>(pdu) + sizeof(uint),
    //                pdu_len - sizeof(uint)
    //                ); // 读取剩余的信息

    while(bytesAvailable() > sizeof(uint)) { // 大于一个uint数据
        uint pdu_len = 0;
        memcpy(&pdu_len, readBuffer.data() + readStart, sizeof(uint)); // 接受到的PDU的总长度
        if(bytesAvailable() < pdu_len) {
            break; //这个PDU还没完全到达
        }
        uint msg_len = pdu_len - sizeof(PDU); // 消息的长度
        PDU* pdu = mkPDU(msg_len);
        memcpy(pdu, readBuffer.data() + readStart, pdu_len);
        readStart += pdu_len; //指针移动

        // ----------------------------------------------

        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_RESPONSE: { // 注册响应
            if(0 == strcmp(REGIST_OK, pdu->caData)) {
                QMessageBox::about(this, "成功", "注册成功");
            } else {
                QMessageBox::critical(this, "失败", "注册失败, 更换用户名重试");
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPONSE: { // 登录响应
            if(0 == strcmp(LOGIN_OK, pdu->caData)) {
                QMessageBox::about(this, "成功", "登录成功");

                m_userName = ui->name_le->text();
                m_currentDir = QString("/%1").arg(m_userName); // 设置目录

                OperateWidget::getInstance().show(); // 一定要在设置完目录之后, 再getInstance()

                OperateWidget::getInstance().setWindowTitle(QString("TcpClient-%1").arg(m_userName));

                this->hide(); // 登录窗口隐藏

            } else {
                QMessageBox::critical(this, "失败", "登录失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE: { // 查看在线用户的响应
            OperateWidget::getInstance()
                    .getFriendWidget()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USER_RESPONSE: { // 查找用户的响应
            int stat = *reinterpret_cast<int*>(pdu->caData);
            if(-1 == stat) { // 用户不存在
                QMessageBox::about(this, "提示",
                                   QString("用户:%1 不存在")
                                   .arg(OperateWidget::getInstance().getFriendWidget()->m_searchName)
                                   );
            } else if(0 == stat) {
                QMessageBox::about(this, "提示",
                                   QString("用户:%1 不在线")
                                   .arg(OperateWidget::getInstance().getFriendWidget()->m_searchName)
                                   );
            } else if(1 == stat) {
                QMessageBox::about(this, "提示",
                                   QString("用户:%1 在线")
                                   .arg(OperateWidget::getInstance().getFriendWidget()->m_searchName)
                                   );
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE: {// 添加好友的响应(来自服务器)
            if(0 == strcmp(pdu->caData, CAN_NOT_ADD_FRIEND)) {
                QMessageBox::critical(this, "提示", "不能添加为好友");
            } else if(0 == strcmp(pdu->caData, ALREADY_FRIEND)) {
                QMessageBox::about(this, "提示", "已经是好友了");
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: { // 处理 添加好友的请求
            char reqName[32] = {0};
            char localName[32] = {0};
            memcpy(reqName, pdu->caData, 32); // 获得请求方的用户名
            memcpy(localName, pdu->caData + 32, 32); // 获得当前的用户名

            PDU* res_pdu = mkPDU(0); // 响应的协议数据单元
            memcpy(res_pdu->caData, localName, 32); // 发送方
            memcpy(res_pdu->caData + 32, reqName, 32); //  接受方

            if(QMessageBox::question(this, "好友请求",
                                     QString("用户: %1 请求添加您为好友, 是否同意?").arg(reqName),
                                     QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                // 同意好友请求
                res_pdu->uiMsgType = ENUM_MSG_TYPE_AGREE_FRIEND_RESPONSE;
            } else {
                // 拒绝好友请求
                res_pdu->uiMsgType = ENUM_MSG_TYPE_REFUSE_FRIEND_RESPONSE;
            }

            //write
            m_tcpSocket.write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_AGREE_FRIEND_RESPONSE: { // 添加好友同意
            char friendName[32] = {0};
            memcpy(friendName, pdu->caData, 32);
            QMessageBox::about(this, "提示", QString("添加好友: %1 成功!").arg(friendName));
            break;
        }
        case ENUM_MSG_TYPE_REFUSE_FRIEND_RESPONSE: { // 添加好友被拒绝
            char friendName[32] = {0};
            memcpy(friendName, pdu->caData, 32);
            QMessageBox::about(this, "提示", QString("添加好友: %1 被拒绝!").arg(friendName));
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE: { // 刷新好友响应
            OperateWidget::getInstance()
                    .getFriendWidget()->showAllFriends(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: { // 收到其他客户端的删除好友请求
            char remoteName[32] = {0}; // 获取删除请求方的名称
            memcpy(remoteName, pdu->caData, 32);
            QMessageBox::about(this, "提示", QString("%1已断开与您的好友关系").arg(remoteName));

            // 重新刷新好友
            OperateWidget::getInstance().getFriendWidget()->flushFriend();
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPONSE: { // 删除好友的响应
            if(0 == strcmp(DELETE_FRIEND_SUCCESS, pdu->caData)) {
                QMessageBox::about(this, "提示", "删除好友成功");
            } else {
                QMessageBox::critical(this, "提示", "删除好友失败");
            }

            // 重新刷新好友
            OperateWidget::getInstance().getFriendWidget()->flushFriend();
            break;
        }
        case ENUM_MSG_TYPE_CHAT_FAIL_RESPONSE: { // 聊天发送后, 失败
            PrivateChatGroup::getInstance().sendFailPrivateChat(pdu);
            //        QMessageBox::critical(this, "提示", QString("向 %1 私聊失败, 对方不在线或者不存在").arg(pdu->caData));
            break;
        }
        case ENUM_MSG_TYPE_CHAT_OK_RESPONSE: { // 聊天发送后, 成功
            PrivateChatGroup::getInstance().sendOkPrivateChat(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CHAT_REQUEST: { // 收到聊天请求
            PrivateChatGroup::getInstance().recvPrivateChat(pdu);
            break;
        }
        case ENUM_MSG_TYPE_BROADCAST_FRIEND_MESSAGE: { // 广播消息
            OperateWidget::getInstance().getFriendWidget()->recvBroadcastMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DIR_CREATE_RESPONSE: { // 创建文件夹
            if(*reinterpret_cast<int*>(pdu->caData) == 1) { // 创建成功
                //            qDebug() << "创建成功";
                OperateWidget::getInstance().getFileWidget()->flushDir();
            } else { // 创建失败
                //            qDebug() << "创建失败";
                QMessageBox::about(&OperateWidget::getInstance(), "提示", "创建目录失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_DIR_FLUSH_RESPONSE: { // 刷新文件夹请求
            OperateWidget::getInstance().getFileWidget()->updateFiles(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DIR_DELETE_RESPONSE: { //  删除文件夹的响应
            if(*reinterpret_cast<int*>(pdu->caData) == 1) { // 删除成功
                //            qDebug() << "删除成功";
                OperateWidget::getInstance().getFileWidget()->flushDir();
            } else { // 删除失败
                //            qDebug() << "删除失败";
                QMessageBox::about(&OperateWidget::getInstance(), "提示", "删除文件夹失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_DIR_RENAME_RESPONSE: { // 重命名回复
            if(*reinterpret_cast<int*>(pdu->caData) == 1) { // 重命名成功
                OperateWidget::getInstance().getFileWidget()->flushDir();
            } else { // 重命名失败
                QMessageBox::about(&OperateWidget::getInstance(), "提示", "重命名文件夹失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_DIR_GO_RESPONSE: { // 进入文件夹回复
            if(*reinterpret_cast<int*>(pdu->caData) == 1) {

                // 设置新路径
                TcpClient::getInstance().setCurrentDir(reinterpret_cast<char*>(pdu->caMsg));
                // 刷新
                OperateWidget::getInstance().getFileWidget()->flushDir();
            }
            break;
        }
        case ENUM_MSG_TYPE_DIR_GOBACK_RESPONSE: {
            if(*reinterpret_cast<int*>(pdu->caData) == 1) {
                // 设置新路径
                TcpClient::getInstance().setCurrentDir(reinterpret_cast<char*>(pdu->caMsg));
                // 刷新
                OperateWidget::getInstance().getFileWidget()->flushDir();
            }
            break;
        }
        case ENUM_MSG_TYPE_FILE_UPLOAD_RESPONSE: { // 上传文件的回复
            if(*reinterpret_cast<int*>(pdu->caData) == 1) { // 可以继续上传
                OperateWidget::getInstance().getFileWidget()->uploadFileContent(pdu);

            } else {
                QMessageBox::about(&OperateWidget::getInstance(), "提示", "上传文件失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_FILE_UPLOAD_END: { // 文件上传成功
            // 刷新
            OperateWidget::getInstance().getFileWidget()->flushDir();
            QMessageBox::about(&OperateWidget::getInstance(), "提示", "文件上传完成");
            break;
        }
        case ENUM_MSG_TYPE_FILE_DELETE_RESPONSE: { // 删除文件的回复
            if(*reinterpret_cast<int*>(pdu->caData) == 1) { // 可以继续上传
                OperateWidget::getInstance().getFileWidget()->flushDir();
            } else {
                QMessageBox::about(&OperateWidget::getInstance(), "提示", "删除文件失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_FILE_DOWNLOAD_RESPONSE: {
            if(*reinterpret_cast<int*>(pdu->caData) == 0) { // 不能下载
                QMessageBox::about(&OperateWidget::getInstance(), "提示", "文件下载失败");
            } else {
                QFile file(QString(reinterpret_cast<char*>(pdu->caMsg)));
                file.open(QIODevice::WriteOnly);
                file.close();
            }
            break;
        }
        case ENUM_MSG_TYPE_FILE_DOWNLOAD_CONTINUE: { // 下载内容
            int len = *reinterpret_cast<int*>(pdu->caData);
            qDebug() << "收到数据包 " << len;
            QString clientPath(reinterpret_cast<char*>(pdu->caMsg));

            QFile file(clientPath);
            file.open(QIODevice::Append);
            file.write(reinterpret_cast<char*>(pdu->caMsg) + clientPath.toUtf8().size() + 1, len);
            file.close();

            break;
        }
        case ENUM_MSG_TYPE_FILE_DOWNLOAD_END: { // 下载完成
            QMessageBox::about(&OperateWidget::getInstance(), "提示", "文件下载完成");
            break;
        }
        case ENUM_MSG_TYPE_FILE_SHARE_REQUEST: { // 有人要分享文件
            QString sender(pdu->caData);
            QString fileName(pdu->caData + 32);
            QString serverPath(reinterpret_cast<char*>(pdu->caMsg));

            if(QMessageBox::Yes == QMessageBox::question(
                        this, "分享",QString("用户%1 要分享 %2 给您, 是否接收?").arg(sender).arg(fileName),
                        QMessageBox::Yes, QMessageBox::No)) {
                // 确认接收
                QString currentPath = TcpClient::getInstance().getCurrentDir(); // 当前路径
                QString userName = TcpClient::getInstance().getUserName();

                PDU* confirm_pdu = mkPDU(serverPath.size() + 1 + currentPath.size() + 1);
                confirm_pdu->uiMsgType = ENUM_MSG_TYPE_FILE_SHARE_RESPONSE;
                memcpy(confirm_pdu->caData, userName.toStdString().c_str(), userName.size());
                memcpy(confirm_pdu->caData + 32, fileName.toStdString().c_str(), fileName.size());

                memcpy(reinterpret_cast<char*>(confirm_pdu->caMsg),
                       serverPath.toStdString().c_str(), serverPath.size());

                memcpy(reinterpret_cast<char*>(confirm_pdu->caMsg) + serverPath.size() + 1,
                       currentPath.toStdString().c_str(), currentPath.size());

                TcpClient::getInstance().getTcpSocket()
                        .write(reinterpret_cast<char*>(confirm_pdu), confirm_pdu->uiPDULen);

                free(confirm_pdu);
            }

            break;
        }
        case ENUM_MSG_TYPE_FILE_SHARE_END: { // 接收分享文件的结果
            if(*reinterpret_cast<int*>(pdu->caData) == 1) { // 接收文件成功
                OperateWidget::getInstance().getFileWidget()->flushDir();
            } else {
                QMessageBox::about(&OperateWidget::getInstance(), "提示", "接收文件失败");
            }
            break;
        }
        default:
            break;
        }
        free(pdu);
    }
}

qint64 TcpClient::bytesAvailable()
{
    return readEnd - readStart;
}

void TcpClient::tcpWrite(char *data, int len)
{
    m_tcpSocket.write(data, len);
    free(data); // 释放空间
}
