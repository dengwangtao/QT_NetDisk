#include "tcpclient.h"
#include <QApplication>
#include <QFont>
#include "sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font("Times", 20, QFont::Bold);
    a.setFont(font);

    TcpClient::getInstance().show();

//    ShareFile w;
//    w.show();

    return a.exec();
}
