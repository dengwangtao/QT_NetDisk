#include "file.h"
#include "operatewidget.h"
#include "sharefile.h"

File::File(QWidget *parent) : QWidget(parent)
{
    m_pCurrenDirLB      = new QLabel;       // 显示当前的目录
    m_pFileListW        = new QListWidget;  // 文件列表
    m_pBackPB           = new QPushButton("返回上一级");     // 返回
    m_pCreateDirPB      = new QPushButton("创建文件夹");     // 创建文件夹
    m_pDeleteDirPB      = new QPushButton("删除文件夹");     // 删除文件夹
    m_pRenameDirPB      = new QPushButton("重命名文件夹");  // 重命名文件夹
    m_pFlushDirPB       = new QPushButton("刷新文件夹");    // 刷新文件夹

    m_pUploadFilePB     = new QPushButton("上传文件");      // 上传文件
    m_pDeleteFilePB     = new QPushButton("删除文件");      // 删除文件
    m_pDownloadFilePB   = new QPushButton("下载文件");      // 下载文件
    m_pShareFilePB      = new QPushButton("分享文件");      // 分享文件


//    progressDialog      = new QProgressDialog(this);
//    progressDialog->hide();
//    progressDialog->setRange(0, 100);
//    progressDialog->setValue(0);

//    timer               = new QTimer;
//    timer->setInterval(1);


    QVBoxLayout* VBL0 = new QVBoxLayout();
    VBL0->addWidget(m_pCurrenDirLB);
    VBL0->addWidget(m_pFileListW);

    QVBoxLayout* VBL1 = new QVBoxLayout();
    VBL1->addStretch();
    VBL1->addWidget(m_pBackPB);
    VBL1->addWidget(m_pCreateDirPB);
    VBL1->addWidget(m_pDeleteDirPB);
    VBL1->addWidget(m_pRenameDirPB);
    VBL1->addWidget(m_pFlushDirPB);
    VBL1->addStretch();

    QVBoxLayout* VBL2 = new QVBoxLayout();
    VBL2->addStretch();
    VBL2->addWidget(m_pUploadFilePB);
    VBL2->addWidget(m_pDeleteFilePB);
    VBL2->addWidget(m_pDownloadFilePB);
    VBL2->addWidget(m_pShareFilePB);
    VBL2->addStretch();

    QHBoxLayout *allLayout = new QHBoxLayout();
    allLayout->addLayout(VBL0);
    allLayout->addLayout(VBL1);
    allLayout->addLayout(VBL2);

    this->setLayout(allLayout);


    // 创建文件夹
    connect(m_pCreateDirPB, SIGNAL(clicked()),
            this, SLOT(createDir())
            );

    // 刷新文件夹
    connect(m_pFlushDirPB, SIGNAL(clicked()),
            this, SLOT(flushDir())
            );

    // 删除文件夹
    connect(m_pDeleteDirPB, SIGNAL(clicked()),
            this, SLOT(deleteDir())
                );

    // 重命名文件夹
    connect(m_pRenameDirPB, SIGNAL(clicked()),
            this, SLOT(renameDir())
            );

    // 进入文件夹
    connect(m_pFileListW, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(gotoDir(QModelIndex))
            );

    // 返回上级目录
    connect(m_pBackPB, SIGNAL(clicked()),
            this, SLOT(goBackDir())
            );

    // 上传文件按钮
    connect(m_pUploadFilePB, SIGNAL(clicked()),
            this, SLOT(uploadFile())
                );

    // 删除文件
    connect(m_pDeleteFilePB, SIGNAL(clicked()),
            this, SLOT(deleteFile())
                );
    // 下载文件
    connect(m_pDownloadFilePB, SIGNAL(clicked()),
            this, SLOT(downloadFile())
                );

    // 分享文件
    connect(m_pShareFilePB, SIGNAL(clicked()),
            this, SLOT(shareFile())
                );

    this->flushDir();
}

void File::updateFiles(PDU *pdu)
{
//    FileInfoUnit
    m_pFileListW->clear();
    uint len = pdu->uiMsgLen / sizeof(FileInfoUnit);
    for(uint i = 0; i < len; ++ i) {
        FileInfoUnit* finfo = reinterpret_cast<FileInfoUnit*>(pdu->caMsg) + i;

        QListWidgetItem *item = new QListWidgetItem;
        item->setText(QString(finfo->fileName));

        if(finfo->fileType & ENUM_FILE_TYPE_FILE) {
            item->setIcon(QIcon(":/images/fileTypeIcon/file.png"));
        }
        if(finfo->fileType & ENUM_FILE_TYPE_EXEC) {
            item->setIcon(QIcon(":/images/fileTypeIcon/exec.png"));
        }
        if(finfo->fileType & ENUM_FILE_TYPE_DIR) {
            item->setIcon(QIcon(":/images/fileTypeIcon/dir.png"));
        }

        m_pFileListW->addItem(item);
    }
}

void File::uploadFileContent(PDU *pdu)
{
    QString serverPath(reinterpret_cast<char*>(pdu->caMsg));
    QString clientPath(reinterpret_cast<char*>(pdu->caMsg) + serverPath.size() + 1);

    QThread *th = new QThread();
    UploadFileWorker* worker = new UploadFileWorker(clientPath, serverPath);

    worker->moveToThread(th);

    // 更新进度条
//    connect(worker, SIGNAL(updateProgress(int)),
//            progressDialog, SLOT(setValue(int))
//            );

    connect(worker, SIGNAL(finished()), th, SLOT(terminate()) );       // 运行结束后, 退出线程
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()) ); // 运行结束后, delete worker
    connect(worker, SIGNAL(finished()), th, SLOT(deleteLater())); // 运行结束后, delete线程

    connect(worker, SIGNAL(tcpWrite(char*,int)),
            &TcpClient::getInstance(), SLOT(tcpWrite(char*,int))
                );

    connect(th, SIGNAL(started()), worker, SLOT(process()));
//    progressDialog->show();
//    progressDialog->setValue(0);

//    connect(th, SIGNAL(started()), timer, SLOT(start())); // 开启计时器

//    connect(worker, SIGNAL(finished()), progressDialog, SLOT(hide()) );
//    connect(worker, SIGNAL(finished()), timer, SLOT(stop()) ); // 关闭计时器

//    connect(timer, &QTimer::timeout, [this, worker] {
//        progressDialog->setValue(worker->getProgress());
//    });

    th->start();


}

void File::createDir()
{
    QString newDirName = QInputDialog::getText(this, "输入文件夹名称", "请输入文件夹名称");
    if(newDirName.isEmpty()) {
        return;
    }
    if(newDirName.length() > 30) {
        QMessageBox::about(nullptr, "提示", "文件夹名称长度不能超过30");
        return;
    }
    QRegularExpression re("^[A-Za-z0-9_]+$");
    if(re.match(newDirName).hasMatch() == false) {
        QMessageBox::critical(nullptr, "错误", "文件夹名称只能包含大小写字母数字和下划线");
        return;
    }
    /*
     * PDU
     * caData:
     *      前32, 用户名
     *      后32, 新目录名
     * caMsg: 当前所在目录
     * */
    const QString& currentPath = TcpClient::getInstance().getCurrentDir();
    const QString& userName = TcpClient::getInstance().getUserName();

    PDU* pdu = mkPDU(currentPath.length() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DIR_CREATE_REQUEST;
    memcpy(pdu->caData, userName.toStdString().c_str(), userName.length());
    memcpy(pdu->caData + 32, newDirName.toStdString().c_str(), newDirName.length());

    memcpy(pdu->caMsg, currentPath.toStdString().c_str(), currentPath.length());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::flushDir()
{
    const QString& currentPath = TcpClient::getInstance().getCurrentDir();
    const QString& userName = TcpClient::getInstance().getUserName();

    this->m_pCurrenDirLB->setText(currentPath); // 设置label文本

    PDU* pdu = mkPDU(currentPath.length() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DIR_FLUSH_REQUEST;
    memcpy(pdu->caData, userName.toStdString().c_str(), userName.length());

    memcpy(pdu->caMsg, currentPath.toStdString().c_str(), currentPath.length());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);

}

void File::deleteDir()
{
    if(nullptr == m_pFileListW->currentItem()) {
        return;
    }

    const QString& currentPath = TcpClient::getInstance().getCurrentDir();  // 当前所在路径
    const QString& userName = TcpClient::getInstance().getUserName();       // 用户名
    const QString  delDirName = m_pFileListW->currentItem()->text();        // 要删除的文件夹

    if(QMessageBox::Yes != QMessageBox::question(this, "确认", QString("确认删除文件夹 %1 吗?").arg(delDirName), QMessageBox::Yes, QMessageBox::No)) {
        return;
    }

    PDU* pdu = mkPDU(currentPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DIR_DELETE_REQUEST;
    memcpy(pdu->caData, userName.toStdString().c_str(), userName.size());
    memcpy(pdu->caData + 32, delDirName.toStdString().c_str(), delDirName.size());

    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::renameDir()
{
    if(nullptr == m_pFileListW->currentItem()) {
        return;
    }

    // 文件夹 新名称
    QString newDirName = QInputDialog::getText(this, "输入文件夹名称", "请输入文件夹名称");
    if(newDirName.isEmpty()) {
        return;
    }
    if(newDirName.length() > 30) {
        QMessageBox::about(nullptr, "提示", "文件夹名称长度不能超过30");
        return;
    }
    QRegularExpression re("^[A-Za-z0-9_-]+$");
    if(re.match(newDirName).hasMatch() == false) {
        QMessageBox::critical(nullptr, "错误", "文件夹名称只能包含大小写字母,数字,下划线,-");
        return;
    }

    const QString& currentPath = TcpClient::getInstance().getCurrentDir();  // 当前所在路径
    const QString  oldDirName = m_pFileListW->currentItem()->text();        // 要重命名的文件夹

    PDU* pdu = mkPDU(currentPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DIR_RENAME_REQUEST;
    memcpy(pdu->caData, oldDirName.toStdString().c_str(), oldDirName.size());
    memcpy(pdu->caData + 32, newDirName.toStdString().c_str(), newDirName.size());

    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::gotoDir(const QModelIndex &idx)
{
    QString dirName = idx.data().toString(); // 要进入的路径
    if(dirName.isEmpty()) return;
    const QString& currentPath = TcpClient::getInstance().getCurrentDir();  // 当前所在路径

    PDU* pdu = mkPDU(currentPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DIR_GO_REQUEST;
    memcpy(pdu->caData, dirName.toStdString().c_str(), dirName.size());

    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::goBackDir()
{
    const QString& currentPath = TcpClient::getInstance().getCurrentDir();  // 当前所在路径

    PDU* pdu = mkPDU(currentPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DIR_GOBACK_REQUEST;

    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::uploadFile()
{
    const QString& currentPath = TcpClient::getInstance().getCurrentDir();  // 当前所在路径
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString filePath = QFileDialog::getOpenFileName(this, "上传文件", documentsPath); // 获取文件路径
    if(filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName(); // 获取文件名
    qint64 fileSize = fileInfo.size();      // 获取文件大小
    QString newFileName; // 新文件名

    QRegularExpression re("^[A-Za-z0-9_.-]{1,30}$");
    if(re.match(fileName).hasMatch() == false) {
        QMessageBox::about(nullptr, "提示", "文件名不合法, 文件名只能包含大小写字母、数字、下划线、-, 请输入新文件名");
        while(true) {
            bool ok = false;
            newFileName = QInputDialog::getText(this, "输入文件名", "请输入新文件名称", QLineEdit::Normal, QString(), &ok);
            if(!ok) { // 用户取消了输入, 直接停止上传
                return;
            }
            if(re.match(newFileName).hasMatch() == false) {
                QMessageBox::critical(nullptr, "错误", "文件名只能包含大小写字母、数字、下划线、-, 请重新输入");
            } else {
                break;
            }
        }
    } else { // 原文件名合法, 直接赋值
        newFileName = fileName;
    }

    PDU* pdu = mkPDU(currentPath.size() + 1 + filePath.toUtf8().size() + 1);
    // 当前用户路径\0文件实际路径

    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_UPLOAD_REQUEST;
    memcpy(pdu->caData, newFileName.toStdString().c_str(), newFileName.size()); // 前32字节记录文件名
    *reinterpret_cast<qint64*>(pdu->caData + 32) = fileSize; // 后32字节记录文件大小

    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());
    memcpy(reinterpret_cast<char*>(pdu->caMsg) + currentPath.size() + 1, filePath.toUtf8().data(), filePath.toUtf8().size());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::deleteFile()
{
    if(nullptr == m_pFileListW->currentItem()) {
        return;
    }

    const QString& currentPath = TcpClient::getInstance().getCurrentDir();  // 当前所在路径
    const QString& userName = TcpClient::getInstance().getUserName();       // 用户名
    const QString  delFileName = m_pFileListW->currentItem()->text();        // 要删除的文件

    if(QMessageBox::Yes != QMessageBox::question(this, "确认", QString("确认删除文件 %1 吗?").arg(delFileName), QMessageBox::Yes, QMessageBox::No)) {
        return;
    }

    PDU* pdu = mkPDU(currentPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_DELETE_REQUEST;
    memcpy(pdu->caData, userName.toStdString().c_str(), userName.size());
    memcpy(pdu->caData + 32, delFileName.toStdString().c_str(), delFileName.size());

    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::downloadFile()
{
    if(nullptr == m_pFileListW->currentItem()) {
        return;
    }

    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    // 获取保存路径
    QString downloadPath = QFileDialog::getSaveFileName(this, "保存到", documentsPath);
    if(downloadPath.isEmpty()) {
        return;
    }

    const QString& currentPath = TcpClient::getInstance().getCurrentDir();  // 当前所在路径
    const QString  delFileName = m_pFileListW->currentItem()->text();       // 要下载的文件

    PDU* pdu = mkPDU(currentPath.size() + 1 + downloadPath.toUtf8().size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_DOWNLOAD_REQUEST;
    memcpy(pdu->caData, delFileName.toStdString().c_str(), delFileName.size());

    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());
    memcpy(reinterpret_cast<char*>(pdu->caMsg) + currentPath.size() + 1,
           downloadPath.toUtf8().data(), downloadPath.toUtf8().size());

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);
    free(pdu);
}

void File::shareFile()
{
    if(nullptr == m_pFileListW->currentItem()) {
        return;
    }
    const QString  checkedFileName = m_pFileListW->currentItem()->text();        // 要分享的文件

    QStringList nameList;
    QListWidget& friendList = OperateWidget::getInstance().getFriendWidget()->getFriendListW();
    for(int i = 0; i < friendList.count(); ++ i) {
//        qDebug() << friendList.item(i)->text();
        nameList.append(friendList.item(i)->text());
    }
//    nameList.append("test1");
//    nameList.append("test2");
//    nameList.append("test3");
//    nameList.append("test4");
//    nameList.append("test5");
    ShareFile::getInstance().updateFriendList(checkedFileName, nameList);

}

//void File::updateProgress(QThread *who)
//{
//    if(m_uploadProgressDialogs.count(who) == 0) { // 当前还没有这个进度
//        m_uploadProgressDialogs[who] = new QProgressDialog;
//        m_uploadProgressDialogs[who]->setRange(0, 100);
//        m_uploadProgressDialogs[who]->show();
//        qDebug() << "new一个进度条";
//    }
//    m_uploadProgressDialogs[who]->setValue(progress);
//}

//void File::deleteProgress(QThread *who)
//{
//    if(m_uploadProgressDialogs.count(who)) {
//        m_uploadProgressDialogs[who]->close();
//        m_uploadProgressDialogs[who]->deleteLater();
//        m_uploadProgressDialogs.erase(who);
//    }
//}
