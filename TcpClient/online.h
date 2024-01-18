#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>
#include <QDebug>
#include "protocol.h"
#include "tcpclient.h"

namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = 0);
    ~Online();

    void showAllOnlineUsr(PDU* pdu);

private slots:
    void on_addfriend_pb_clicked();

private:
    Ui::Online *ui;
};

#endif // ONLINE_H
