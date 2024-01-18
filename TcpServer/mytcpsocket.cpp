#include "mytcpsocket.h"
#include "mytcpserver.h"

MyTcpSocket::MyTcpSocket()
    : readStart(0),
      readEnd(0),
      readBuffer(QByteArray(4096, '\0')) // 初始化缓冲区4096
{
    // 自己的信号, 自己处理
    connect(
                this, SIGNAL(readyRead()),
                this, SLOT(recvMsg_Shot())
                );

    // 断开连接信号
    connect(
                this, SIGNAL(disconnected()),
                this, SLOT(clientOffline())
                );
}

MyTcpSocket::~MyTcpSocket()
{
    qDebug() << this->m_strName << "析构了";
}

const QString &MyTcpSocket::getName() const
{
    return this->m_strName;
}

void MyTcpSocket::clientOffline()
{
    DBHelper::getInstance().handleLogout(this->m_strName);
    emit offline(this); // 触发下线信号
}

void MyTcpSocket::recvMsg_Shot()
{
    QByteArray data = this->readAll();
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

qint64 MyTcpSocket::bytesAvailable()
{
    return readEnd - readStart;
}

void MyTcpSocket::recvMsg()
{
    //    uint pdu_len = 0;
    //    this->read(reinterpret_cast<char*>(&pdu_len), sizeof(uint)); // 接受到的PDU的总长度

    //    uint msg_len = pdu_len - sizeof(PDU); // 消息的长度

    //    PDU* pdu = mkPDU(msg_len); // 声明一个消息域大小为msg_len的PDU
    //    pdu->uiPDULen = pdu_len;
    //    // 读取剩余的信息
    //    this->read(reinterpret_cast<char*>(pdu) + sizeof(uint), pdu_len - sizeof(uint));


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

        switch(pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_REQUEST: {
            PDU* res_pdu = mkPDU(0); // 响应数据包
            char name[32] = {0};
            char pwd[32]  = {0};
            strncpy(name, pdu->caData, 32);
            strncpy(pwd, pdu->caData + 32, 32);
            qDebug() << "收到注册请求";
            bool ret = DBHelper::getInstance().handleRegist(name, pwd);
            res_pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPONSE;
            if(ret) {
                qDebug() << "注册成功";
                strcpy(res_pdu->caData, REGIST_OK);
            } else {
                qDebug() << "注册失败";
                strcpy(res_pdu->caData, REGIST_FAIL);
            }
            // 响应数据包
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST: {
            PDU* res_pdu = mkPDU(0); // 响应数据包
            char name[32] = {0};
            char pwd[32]  = {0};
            strncpy(name, pdu->caData, 32);
            strncpy(pwd, pdu->caData + 32, 32);
            qDebug() << "收到登录请求";
            bool ret = DBHelper::getInstance().handleLogin(name, pwd);
            res_pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPONSE;
            if(ret) {
                qDebug() << "登录成功";
                strcpy(res_pdu->caData, LOGIN_OK);

                this->m_strName = name;

                MyDir dir;
                dir.cdOrMkdir(QString(name)); // 如果不存在, 创建用户目录

            } else {
                qDebug() << "登录失败";
                strcpy(res_pdu->caData, LOGIN_FAIL);
            }

            // 响应数据包
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST: {

            QStringList ret(std::move(DBHelper::getInstance().handleAllOnline())); // 移动构造
            uint msg_len = 32 * ret.size();

            PDU* res_pdu = mkPDU(msg_len);

            for(int i = 0; i < ret.size(); ++ i) {
                memcpy(
                            reinterpret_cast<char*>(res_pdu->caMsg) + i * 32,
                            ret.at(i).toStdString().c_str(),
                            ret.at(i).size());
            }

            res_pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE;
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);

            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USER_REQUEST: {
            int stat = DBHelper::getInstance().handleSearchUser(pdu->caData);
            PDU* res_pdu = mkPDU(0);
            res_pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPONSE;
            *reinterpret_cast<int*>(res_pdu->caData) = stat;
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
            char name1[32] = {0};
            char name2[32] = {0};
            memcpy(name1, pdu->caData, 32);
            memcpy(name2, pdu->caData + 32, 32);

            int ret = DBHelper::getInstance().handleAddFriend(name1, name2);
            /*
             * return -1: 未知错误
             * return 0 : 不能发送好友申请, 用户不存在或者不在线
             * return 1 : 可以发送好友申请
             * return 2 : 不能发送好友申请, 已经是好友了
             * */
            if(ret == -1) {
                PDU* res_pdu = mkPDU(0);
                res_pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
                strcpy(res_pdu->caData, UNKNOWN_ERROR);
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            } else if(ret == 0) {
                PDU* res_pdu = mkPDU(0);
                res_pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
                strcpy(res_pdu->caData, CAN_NOT_ADD_FRIEND);
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            } else if(ret == 2) {
                PDU* res_pdu = mkPDU(0);
                res_pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
                strcpy(res_pdu->caData, ALREADY_FRIEND);
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            } else if(ret == 1) { // 可以添加好友, 转发申请 到目标client

                bool forword_ret = MyTcpServer::getInstance().forwardRequest(name2, pdu);

                if(forword_ret == false) { // 请求转发给失败
                    PDU* res_pdu = mkPDU(0);
                    res_pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
                    strcpy(res_pdu->caData, UNKNOWN_ERROR);
                    this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                    free(res_pdu);
                }
            }

            break;
        }
        case ENUM_MSG_TYPE_AGREE_FRIEND_RESPONSE: {
            char name1[32] = {0};
            char name2[32] = {0};
            memcpy(name1, pdu->caData, 32);
            memcpy(name2, pdu->caData + 32, 32);

            // 建立双方好友关系
            DBHelper::getInstance().handleEstablishFriend(name1, name2);

            MyTcpServer::getInstance().forwardRequest(name2, pdu);
            break;
        }
        case ENUM_MSG_TYPE_REFUSE_FRIEND_RESPONSE: {
            // 转发
            //        char name1[32] = {0};
            char name2[32] = {0};
            //        memcpy(name1, pdu->caData, 32);
            memcpy(name2, pdu->caData + 32, 32);
            MyTcpServer::getInstance().forwardRequest(name2, pdu);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST: {
            char name[32] = {0};
            memcpy(name, pdu->caData, 32);
            QStringList friends(std::move(DBHelper::getInstance().handleGetFriends(name)));
            PDU* res_pdu = mkPDU(32 * friends.size());
            res_pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE;
            for(int i = 0; i < friends.size(); ++ i) {
                memcpy(
                            reinterpret_cast<char*>(res_pdu->caMsg) + i * 32,
                            friends.at(i).toStdString().c_str(),
                            friends.at(i).size());
            }
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: { // 删除好友请求
            char name1[32] = {0};
            char name2[32] = {0};
            memcpy(name1, pdu->caData, 32);// 删除的发起方
            memcpy(name2, pdu->caData + 32, 32); // 被删除的一方

            PDU* res_pdu = mkPDU(0);
            res_pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPONSE;
            if(DBHelper::getInstance().handleDeleteFriend(name1, name2)) {
                // 发送给请求发送方, 删除成功
                strcpy(res_pdu->caData, DELETE_FRIEND_SUCCESS);
                // 原请求转发到目标方
                MyTcpServer::getInstance().forwardRequest(name2, pdu);
            }
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_CHAT_REQUEST: {
            char name1[32] = {0};
            char name2[32] = {0};
            memcpy(name1, pdu->caData, 32);// 聊天发送方
            memcpy(name2, pdu->caData + 32, 32); // 聊天接收方
            // TODO 转发
            // 原请求转发到目标方
            if(false == MyTcpServer::getInstance().forwardRequest(name2, pdu)) {
                // 转发失败
                PDU* res_pdu = mkPDU(0);
                res_pdu->uiMsgType = ENUM_MSG_TYPE_CHAT_FAIL_RESPONSE; // 回复一下, 发送失败
                memcpy(res_pdu->caData, name2, 32); // 聊天接收方的名字传回去
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            } else {
                // 转发成功
                PDU* res_pdu = mkPDU(pdu->uiMsgLen);
                res_pdu->uiMsgType = ENUM_MSG_TYPE_CHAT_OK_RESPONSE;// 回复一下, 发送成功
                memcpy(res_pdu->caData, pdu->caData, 64);           // 原封不动
                memcpy(res_pdu->caMsg, pdu->caMsg, pdu->uiMsgLen);  // 消息回复回去
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_BROADCAST_FRIEND_MESSAGE: { // 好友广播消息
            char sender[32] = {0};
            memcpy(sender, pdu->caData, 32);// 聊天发送方

            // 获取朋友列表
            QStringList flist(std::move(DBHelper::getInstance().handleGetFriends(sender)));

            // 广播转发
            MyTcpServer::getInstance().forwardRequest(flist, pdu);

            // 再回传给自己
            MyTcpServer::getInstance().forwardRequest(sender, pdu);
            break;
        }
        case ENUM_MSG_TYPE_DIR_CREATE_REQUEST: { // 创建文件夹
            char sender[32] = {0};
            char newDirName[32] = {0};

            memcpy(sender, pdu->caData, 32);
            memcpy(newDirName, pdu->caData + 32, 32);

            MyDir dir(QString(reinterpret_cast<char*>(pdu->caMsg))); // 直接定位到当前目录

            PDU* res_pdu = mkPDU(0);
            res_pdu->uiMsgType = ENUM_MSG_TYPE_DIR_CREATE_RESPONSE;

            if(dir.checkDir(QString(newDirName))) { // 已经存在, 不能创建
                *reinterpret_cast<int*>(res_pdu->caData) = 0; // 创建失败
            } else {
                dir.cdOrMkdir(QString(newDirName)); // 创建
                *reinterpret_cast<int*>(res_pdu->caData) = 1; // 创建成功
            }

            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);

            break;
        }
        case ENUM_MSG_TYPE_DIR_FLUSH_REQUEST: { // 刷新文件夹
            char sender[32] = {0};
            memcpy(sender, pdu->caData, 32);

            MyDir dir(QString(reinterpret_cast<char*>(pdu->caMsg))); // 直接定位到当前目录
            QFileInfoList list(std::move(dir.listDir())); // 列出目录

            PDU* res_pdu = mkPDU(sizeof(FileInfoUnit) * (list.size() - 2));
            res_pdu->uiMsgType = ENUM_MSG_TYPE_DIR_FLUSH_RESPONSE;
            uint idx = 0;
            for(uint i = 0; i < list.size(); ++ i) {
                // 强转为FileInfoUnit类型
                FileInfoUnit* finfo = reinterpret_cast<FileInfoUnit*>(res_pdu->caMsg) + idx;
                if("." == list.at(i).fileName() || ".." == list.at(i).fileName()) {
                    continue;
                }
                ++ idx;
                memcpy(finfo->fileName,
                       list.at(i).fileName().toStdString().c_str(),
                       std::max(list.at(i).fileName().size(), 30)
                       );
                finfo->fileType = 0;
                if(list.at(i).isFile()) {
                    finfo->fileType |= ENUM_FILE_TYPE_FILE;
                }
                if(list.at(i).isExecutable()) {
                    finfo->fileType |= ENUM_FILE_TYPE_EXEC;
                }
                if(list.at(i).isDir()) {
                    finfo->fileType |= ENUM_FILE_TYPE_DIR;
                }

            }
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
            break;
        }
        case ENUM_MSG_TYPE_DIR_DELETE_REQUEST: {
            char sender[32] = {0};
            char delDir[32] = {0};
            memcpy(sender, pdu->caData, 32);
            memcpy(delDir, pdu->caData + 32, 32);

            MyDir dir(QString(reinterpret_cast<char*>(pdu->caMsg))); // 直接定位到当前目录

            PDU* res_pdu = mkPDU(0);
            res_pdu ->uiMsgType = ENUM_MSG_TYPE_DIR_DELETE_RESPONSE;

            bool ret = dir.deleteDir(delDir); // 是否删除成功, (不存在或者删除失败)
            *reinterpret_cast<int*>(res_pdu->caData) = static_cast<int>(ret);

            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);

            break;
        }
        case ENUM_MSG_TYPE_DIR_RENAME_REQUEST: { // 文件夹重命名

            char oldDirName[32] = {0};
            char newDirName[32] = {0};
            memcpy(oldDirName, pdu->caData, 32);
            memcpy(newDirName, pdu->caData + 32, 32);

            MyDir dir(QString(reinterpret_cast<char*>(pdu->caMsg))); // 直接定位到当前目录

            PDU* res_pdu = mkPDU(0);
            res_pdu ->uiMsgType = ENUM_MSG_TYPE_DIR_RENAME_RESPONSE;

            bool ret = dir.rename(oldDirName, newDirName); // 是否重命名成功
            *reinterpret_cast<int*>(res_pdu->caData) = static_cast<int>(ret);

            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);

            break;
        }
        case ENUM_MSG_TYPE_DIR_GO_REQUEST: {
            char dirName[32] = {0};
            memcpy(dirName, pdu->caData, 32);

            MyDir dir(QString(reinterpret_cast<char*>(pdu->caMsg))); // 直接定位到当前目录

            if(dir.checkDir(dirName)) { // 是文件夹, 才能进入

                dir.cdOrMkdir(dirName); // 进入
                QString path = dir.getRelativePath(); // 获取路径

                PDU* res_pdu = mkPDU(path.size() + 1);
                res_pdu ->uiMsgType = ENUM_MSG_TYPE_DIR_GO_RESPONSE;
                *reinterpret_cast<int*>(res_pdu->caData) = 1;

                memcpy(res_pdu->caMsg, path.toStdString().c_str(), path.size()); // 返回相对路径

                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            } else {
                PDU* res_pdu = mkPDU(0);
                res_pdu ->uiMsgType = ENUM_MSG_TYPE_DIR_GO_RESPONSE;
                *reinterpret_cast<int*>(res_pdu->caData) = 0; // 进入失败

                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            }

            break;
        }
        case ENUM_MSG_TYPE_DIR_GOBACK_REQUEST: {
            MyDir dir(QString(reinterpret_cast<char*>(pdu->caMsg))); // 直接定位到当前目录
            if(dir.cdBack()) {
                QString path = dir.getRelativePath(); // 获取路径

                PDU* res_pdu = mkPDU(path.size() + 1);
                res_pdu ->uiMsgType = ENUM_MSG_TYPE_DIR_GOBACK_RESPONSE;
                *reinterpret_cast<int*>(res_pdu->caData) = 1;
                memcpy(res_pdu->caMsg, path.toStdString().c_str(), path.size()); // 返回相对路径
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            } else {
                PDU* res_pdu = mkPDU(0);
                res_pdu ->uiMsgType = ENUM_MSG_TYPE_DIR_GOBACK_RESPONSE;
                *reinterpret_cast<int*>(res_pdu->caData) = 0; // 不能再返回了

                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_FILE_UPLOAD_REQUEST: { // 上传文件请求
            char newFileName[32] = {0};
            memcpy(newFileName, pdu->caData, 32);
            qint64 fileSize = *reinterpret_cast<qint64*>(pdu->caData + 32);
            QString path(reinterpret_cast<char*>(pdu->caMsg)); // 当前所在路径
            QString clientFilePath = QString::fromUtf8(reinterpret_cast<char*>(pdu->caMsg) + path.size() + 1); // 客户端实际文件路径

            MyDir dir(path);
            if(dir.checkFile(newFileName)) { // 同名文件已存在, 不能上传
                PDU* res_pdu = mkPDU(0);
                res_pdu ->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_RESPONSE;
                *reinterpret_cast<int*>(res_pdu->caData) = 0; // 不能上传
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);

            } else {
                QFile uploadFile(dir.getNewFilePath(newFileName)); // 根据绝对文件路径打开文件
                if(uploadFile.open(QIODevice::WriteOnly)) { // 打开成功, 可以上传
                    uploadFile.close();

                    QString relativeNewFilePath = dir.getRelativeNewFilePath(newFileName);
                    qDebug() << clientFilePath << " ====> " << relativeNewFilePath;

                    PDU* res_pdu = mkPDU(relativeNewFilePath.size() + 1 + clientFilePath.toUtf8().size() + 1);
                    res_pdu ->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_RESPONSE;
                    *reinterpret_cast<int*>(res_pdu->caData) = 1;

                    memcpy(reinterpret_cast<char*>(res_pdu->caMsg), relativeNewFilePath.toStdString().c_str(), relativeNewFilePath.size());
                    memcpy(reinterpret_cast<char*>(res_pdu->caMsg) + relativeNewFilePath.size() + 1, clientFilePath.toUtf8().data(), clientFilePath.toUtf8().size());

                    this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                    free(res_pdu);

                } else { // 创建文件失败, 不能上传
                    PDU* res_pdu = mkPDU(0);
                    res_pdu ->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_RESPONSE;
                    *reinterpret_cast<int*>(res_pdu->caData) = 0; // 不能上传
                    this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                    free(res_pdu);
                }

            }
            break;
        }
        case ENUM_MSG_TYPE_FILE_UPLOAD_CONTINUE: { // 接受到上传文件内容
            int len = *reinterpret_cast<int*>(pdu->caData);
            QString path(reinterpret_cast<char*>(pdu->caMsg));
            QFileInfo finfo(path);
            MyDir dir(finfo.path());

//            qDebug() << dir.getNewFilePath(finfo.fileName()) << " " << len;
            QFile file(dir.getNewFilePath(finfo.fileName()));// 打开文件

            file.open(QIODevice::Append); // 假设总是可以写入

            file.write(reinterpret_cast<char*>(pdu->caMsg) + path.size() + 1, len);// 写入文件

            file.close();

            break;
        }
        case ENUM_MSG_TYPE_FILE_UPLOAD_END: { // 上传文件完成
            PDU* res_pdu = mkPDU(0);
            res_pdu ->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_END;
            *reinterpret_cast<int*>(res_pdu->caData) = 1;
            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);
        }
        case ENUM_MSG_TYPE_FILE_DELETE_REQUEST: {
            char sender[32] = {0};
            char delFile[32] = {0};
            memcpy(sender, pdu->caData, 32);
            memcpy(delFile, pdu->caData + 32, 32);

            MyDir dir(QString(reinterpret_cast<char*>(pdu->caMsg))); // 直接定位到当前目录

            PDU* res_pdu = mkPDU(0);
            res_pdu ->uiMsgType = ENUM_MSG_TYPE_FILE_DELETE_RESPONSE;

            bool ret = dir.deleteFile(delFile); // 是否删除成功, (不存在或者删除失败)
            *reinterpret_cast<int*>(res_pdu->caData) = static_cast<int>(ret);

            this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
            free(res_pdu);

            break;
        }
        case ENUM_MSG_TYPE_FILE_DOWNLOAD_REQUEST: { // 下载文件请求
            char downloadFileName[32] = {0};
            memcpy(downloadFileName, pdu->caData, 32);

            QString serverPath(reinterpret_cast<char*>(pdu->caMsg));
            QString clientPath(reinterpret_cast<char*>(pdu->caMsg) + serverPath.size() + 1);

            MyDir dir(serverPath);

            qDebug() << dir.getNewFilePath(downloadFileName) << " ===> " << clientPath;

            if(dir.checkFile(downloadFileName)) {
                // 是文件
                PDU* res_pdu = mkPDU(clientPath.toUtf8().size() + 1);
                res_pdu ->uiMsgType = ENUM_MSG_TYPE_FILE_DOWNLOAD_RESPONSE;
                *reinterpret_cast<int*>(res_pdu->caData) = 1;
                memcpy(res_pdu->caMsg, clientPath.toUtf8().data(), clientPath.toUtf8().size());
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);

                // 传输内容
                downloadFile(dir.getNewFilePath(downloadFileName), clientPath);

            } else {
                //不是文件, 不能下载
                PDU* res_pdu = mkPDU(0);
                res_pdu ->uiMsgType = ENUM_MSG_TYPE_FILE_DOWNLOAD_RESPONSE;
                *reinterpret_cast<int*>(res_pdu->caData) = 0;
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            }

            break;
        }
        case ENUM_MSG_TYPE_FILE_SHARE_REQUEST: {
            // caData: 用户名 文件名
            // caMsg: 路径\0 接受者1 \0 接受者2 \0 ...
            QString sender(pdu->caData);
            QString fileName(pdu->caData + 32);
            int offset = 0;

            QString serverPath(reinterpret_cast<char*>(pdu->caMsg));
            offset += serverPath.size() + 1;

//            qDebug() << "sender: " << sender;
//            qDebug() << "fileName: " << fileName;
//            qDebug() << "serverPath: " << serverPath;

            PDU* res_pdu = mkPDU(serverPath.size() + 1);
            // caData: 用户名 文件名
            // caMsg: 路径
            res_pdu->uiMsgType = ENUM_MSG_TYPE_FILE_SHARE_REQUEST;
            memcpy(res_pdu->caData, pdu->caData, 64); // 全拷贝过去
            memcpy(reinterpret_cast<char*>(res_pdu->caMsg),
                   serverPath.toStdString().c_str(), serverPath.size()); // 拷贝路径

            while(offset < pdu->uiMsgLen) {
                QString reciver(reinterpret_cast<char*>(pdu->caMsg) + offset);
                offset += reciver.size() + 1;

                MyTcpServer::getInstance().forwardRequest(reciver.toStdString().c_str(), res_pdu); // 将这个PDU转发过去
            }

            free(res_pdu);

            break;
        }
        case ENUM_MSG_TYPE_FILE_SHARE_RESPONSE: { // 客户端确认接受分享的文件
            QString sender(pdu->caData); // 文件接收者
            QString fileName(pdu->caData + 32); // 文件名
            QString sourcePath(reinterpret_cast<char*>(pdu->caMsg));
            QString targetPath(reinterpret_cast<char*>(pdu->caMsg) + sourcePath.size() + 1);

            //qDebug() << sender << "接收文件" << fileName;
            //qDebug() << sourcePath << " ==> " << targetPath;

            MyDir dir_source(sourcePath);
            MyDir dir_target(targetPath);

            qDebug() << dir_source.getNewFilePath(fileName) << " ==> " << dir_target.getNewFilePath(fileName);
            if(dir_source.checkFile(fileName)) {
                bool ret = QFile::copy(
                            dir_source.getNewFilePath(fileName),
                            dir_target.getNewFilePath(fileName));

                PDU* res_pdu = mkPDU(0);
                res_pdu->uiMsgType = ENUM_MSG_TYPE_FILE_SHARE_END;
                *reinterpret_cast<int*>(res_pdu->caData) = static_cast<int>(ret);
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            } else { // 不是文件
                PDU* res_pdu = mkPDU(0);
                res_pdu->uiMsgType = ENUM_MSG_TYPE_FILE_SHARE_END;
                *reinterpret_cast<int*>(res_pdu->caData) = 0;
                this->write(reinterpret_cast<char*>(res_pdu), res_pdu->uiPDULen);
                free(res_pdu);
            }

            break;
        }
        default: {
            break;
        }
        }
        free(pdu);
    }
}

// 下载文件
void MyTcpSocket::downloadFile(const QString &serverPath, const QString &clientPath)
{
    const int MAX_SIZE = 409600;
    QFile file(serverPath);
    file.open(QIODevice::ReadOnly); // 这里有可能为false

    char buffer[MAX_SIZE] = {0}; // 缓冲区
    qint64 fileSize = file.size(); // 文件大小
    qint64 current = 0; // 当前已处理的数据

    int len = 0;
    while((len = file.read(buffer, MAX_SIZE)) > 0) { // 循环读数据
        int send_len = clientPath.toUtf8().size() + 1 + len; // Msg中, 保存路径 + \0 + 二进制文件数据
        PDU* pdu = mkPDU(send_len);
        pdu->uiMsgType = ENUM_MSG_TYPE_FILE_DOWNLOAD_CONTINUE; //下载的文件内容
        *reinterpret_cast<int*>(pdu->caData) = len; // data域存放 数据的 大小

        // 客户端保存的路径
        memcpy(reinterpret_cast<char*>(pdu->caMsg), clientPath.toUtf8().data(), clientPath.toUtf8().size());

        // 实际数据
        memcpy(reinterpret_cast<char*>(pdu->caMsg) + clientPath.toUtf8().size() + 1,
               buffer, len);

        this->write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);

        free(pdu); //不在此处free
        current += len;
    }

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_DOWNLOAD_END; //下载文件完成
    *reinterpret_cast<int*>(pdu->caData) = 1; // data域存放 数据的 大小
    this->write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
}
