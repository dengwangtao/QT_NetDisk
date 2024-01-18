#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QCheckBox>
#include <QStringList>
#include <QDebug>
#include "protocol.h"

class ShareFile : public QWidget
{
    Q_OBJECT

    explicit ShareFile(QWidget *parent = 0);
public:
    static ShareFile& getInstance();

    void updateFriendList(const QString& fileName, const QStringList& nameList);
signals:

public slots:
    void confirmShare();
    void selectAll();
    void cancelAll();

private:
    QPushButton* m_selectAllPB; // 全选按钮
    QPushButton* m_cancelAllPB; // 取消全选按钮

    QScrollArea* m_Area; // 区域
    QWidget* m_friendW;
    QButtonGroup* m_buttonGroup; // 按钮组
    QVBoxLayout* buttonsLayout; // 按钮组的布局

    QPushButton* m_confirmPB; // 确认按钮
    QPushButton* m_cancelPB;  // 取消按钮

    QString m_fileName; // 要发送的文件名
};

#endif // SHAREFILE_H
