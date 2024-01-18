#ifndef OPERATEWIDGET_H
#define OPERATEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "friend.h"
#include "file.h"
#include <QStackedWidget>
#include <QHBoxLayout>

class OperateWidget : public QWidget
{
    Q_OBJECT
public:
    static OperateWidget& getInstance();
    Friend* getFriendWidget();
    File* getFileWidget();

private:
    explicit OperateWidget(QWidget *parent = 0);

signals:

public slots:

private:
    QListWidget* m_pListw;

    Friend* m_pFriendW;
    File* m_pFileW;

    QStackedWidget* m_pSW;
};

#endif // OPERATEWIDGET_H
