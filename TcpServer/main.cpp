#include "tcpserver.h"
#include "dbhelper.h"
#include <QApplication>
#include <QMessageBox>
#include "mydir.h"
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    try {
        MyDir dir;                  // 测试存储路径是否存在
//        MyDir dir("test/blue");
//        qDebug() << dir.getRelativePath();
//        dir.cdOrMkdir("test");
//        dir.cdOrMkdir("blue");
//        dir.cdOrMkdir("deepblue");
//        qDebug() << dir.getRelativePath();
//        dir.cdRoot();
//        qDebug() << dir.getRelativePath();
    } catch(const QString& excp) {
        QMessageBox::critical(nullptr, "错误", excp);
        return 1;
    }

    // 初始化数据库
    DBHelper::getInstance().init();

    TcpServer w;
    w.show();

    return a.exec();
}
