#include "sharefile.h"
#include "tcpclient.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget(parent),
      buttonsLayout(nullptr),
      m_buttonGroup(nullptr),
      m_friendW(nullptr)
{
    m_selectAllPB = new QPushButton("全选"); // 全选按钮
    m_cancelAllPB = new QPushButton("取消全选"); // 取消全选按钮

    m_Area = new QScrollArea; // 区域

    m_confirmPB = new QPushButton("确认"); // 确认按钮
    m_cancelPB = new QPushButton("取消");  // 取消按钮

    QHBoxLayout* top = new QHBoxLayout;
    top->addWidget(m_selectAllPB);
    top->addWidget(m_cancelAllPB);


    QHBoxLayout* down = new QHBoxLayout;
    down->addWidget(m_confirmPB);
    down->addWidget(m_cancelPB);

    QVBoxLayout* allLayout = new QVBoxLayout;
    allLayout->addLayout(top);
    allLayout->addWidget(m_Area);
    allLayout->addLayout(down);

    this->setLayout(allLayout);

    // 取消按钮
    connect(m_cancelPB, SIGNAL(clicked()),
            this, SLOT(hide())
                );

    // 确认分享
    connect(m_confirmPB, SIGNAL(clicked()),
            this, SLOT(confirmShare())
                );
    // 全选
    connect(m_selectAllPB, SIGNAL(clicked()),
            this, SLOT(selectAll())
                );
    // 取消全选
    connect(m_cancelAllPB, SIGNAL(clicked()),
            this, SLOT(cancelAll())
                );
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriendList(const QString& fileName, const QStringList& nameList)
{
    m_fileName = fileName; // 保存一份

    QWidget* tmp = m_friendW;
    if(tmp) {
        tmp->deleteLater();
    }

    m_friendW = new QWidget;
    buttonsLayout = new QVBoxLayout(m_friendW);
    m_buttonGroup = new QButtonGroup(m_friendW);
    m_buttonGroup->setExclusive(false); // 可以多选

    for(const QString& name : nameList) {
        QCheckBox* chk = new QCheckBox(name, m_friendW);
        buttonsLayout->addWidget(chk);
        m_buttonGroup->addButton(chk);
    }

    m_friendW->setLayout(buttonsLayout);
    m_Area->setWidget(m_friendW);

    if(this->isHidden()) {
        this->show();
    }
}

void ShareFile::confirmShare()
{
    const QString& userName = TcpClient::getInstance().getUserName(); // 用户名
    const QString& currentPath = TcpClient::getInstance().getCurrentDir(); // 当前路径
    const QString& fileName = m_fileName; // 文件名

    QStringList reciver;
    int total_len = 0;
    for(QAbstractButton*& btn : this->m_buttonGroup->buttons()) {
        if(btn->isChecked()) {
            total_len += btn->text().length() + 1; // '\0'的长度
            reciver.append(btn->text());
        }
    }

    PDU* pdu = mkPDU(currentPath.size() + 1 + total_len);
    // caData: 用户名 文件名
    // caMsg: 路径\0 接受者1 \0 接受者2 \0 ...
    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_SHARE_REQUEST;

    memcpy(pdu->caData, userName.toStdString().c_str(), userName.size()); // 拷贝用户名
    memcpy(pdu->caData + 32, fileName.toStdString().c_str(), fileName.size()); // 拷贝文件名

    int offset = 0;
    memcpy(reinterpret_cast<char*>(pdu->caMsg), currentPath.toStdString().c_str(), currentPath.size());
    offset += currentPath.size() + 1;

    for(const QString& name : reciver) {
        memcpy(reinterpret_cast<char*>(pdu->caMsg) + offset,
               name.toStdString().c_str(), name.size());
        offset += name.size() + 1;
    }

    TcpClient::getInstance().getTcpSocket()
            .write(reinterpret_cast<char*>(pdu), pdu->uiPDULen);

    free(pdu);

    this->setVisible(false);
}

void ShareFile::selectAll()
{
    for(QAbstractButton*& btn : this->m_buttonGroup->buttons()) {
        if(!btn->isChecked()) {
            btn->setChecked(true);
        }
    }
}

void ShareFile::cancelAll()
{
    for(QAbstractButton*& btn : this->m_buttonGroup->buttons()) {
        if(btn->isChecked()) {
            btn->setChecked(false);
        }
    }
}
