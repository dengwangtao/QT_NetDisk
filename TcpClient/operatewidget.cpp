#include "operatewidget.h"

OperateWidget &OperateWidget::getInstance()
{
    static OperateWidget ow;
    return ow;
}

Friend *OperateWidget::getFriendWidget()
{
    return this->m_pFriendW;
}

File *OperateWidget::getFileWidget()
{
    return this->m_pFileW;
}

OperateWidget::OperateWidget(QWidget *parent) : QWidget(parent)
{
    m_pListw = new QListWidget(this);
    m_pListw->setMaximumWidth(300); // 最大宽度
    m_pListw->setMinimumWidth(200); // 最小宽度
    m_pListw->addItem("好友");
    m_pListw->addItem("文件");

    m_pFriendW = new Friend;
    m_pFileW = new File;

    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriendW);
    m_pSW->addWidget(m_pFileW);

    QHBoxLayout* pAllHBL = new QHBoxLayout;
    pAllHBL->addWidget(m_pListw);
    pAllHBL->addWidget(m_pSW);

    this->setLayout(pAllHBL);

    // 绑定stack绑定
    connect(m_pListw, SIGNAL(currentRowChanged(int)),
            m_pSW, SLOT(setCurrentIndex(int))
            );
}
