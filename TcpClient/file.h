#ifndef FILE_H
#define FILE_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QProgressDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <unordered_map>
#include <QTimer>
#include "tcpclient.h"
#include "protocol.h"
#include "uploadfileworker.h"


class File : public QWidget
{
    Q_OBJECT
public:
    explicit File(QWidget *parent = 0);

    void updateFiles(PDU* pdu); // 更新文件夹

    void uploadFileContent(PDU* pdu); // 上传文件夹内容

    void deleteProgress(QThread* who); // 删除进度条

signals:

public slots:
    void createDir();
    void flushDir();
    void deleteDir();
    void renameDir();
    void gotoDir(const QModelIndex& idx);
    void goBackDir();
    void uploadFile();
    void deleteFile();
    void downloadFile();
    void shareFile();

//    void updateProgress(QThread* who); // 更新进度条

private:
    QLabel* m_pCurrenDirLB;         // 显示当前的目录
    QListWidget* m_pFileListW;      // 文件列表
    QPushButton* m_pBackPB;         // 返回
    QPushButton* m_pCreateDirPB;    // 创建文件夹
    QPushButton* m_pDeleteDirPB;    // 删除文件夹
    QPushButton* m_pRenameDirPB;    // 重命名文件夹
    QPushButton* m_pFlushDirPB;     // 刷新文件夹

    QPushButton* m_pUploadFilePB;   // 上传文件
    QPushButton* m_pDeleteFilePB;   // 删除文件
    QPushButton* m_pDownloadFilePB; // 下载文件
    QPushButton* m_pShareFilePB;    // 分享文件

    // 存储当前的所有上传进度条
//    std::unordered_map<QThread*, QProgressDialog*> m_uploadProgressDialogs;
    QProgressDialog* progressDialog;
    QTimer* timer;
};

#endif // FILE_H
