#include "dbhelper.h"
#include <QMessageBox>
#include <QDebug>

DBHelper::~DBHelper()
{
    m_conn.close();
}

// 单例模式
DBHelper &DBHelper::getInstance()
{
    static DBHelper helper;
    return helper;
}

DBHelper::DBHelper(QObject *parent) : QObject(parent)
{
    m_conn = QSqlDatabase::addDatabase("QSQLITE");
}

void DBHelper::init()
{
    m_conn.setHostName("localhost");
    m_conn.setDatabaseName("D:\\development\\qt\\TcpServer\\cloud.db");
    if(m_conn.open()) {
        /*
        QSqlQuery query;
        query.exec("select id, name FROM userInfo;");
        while(query.next()) {
            QString row = QString("%1, %2")
                            .arg(query.value(0).toString())
                            .arg(query.value(1).toString());
            qDebug() << row;
        }
        */
        qDebug() << "连接数据库成功";
    } else {
        QMessageBox::critical(nullptr, "Warning", "connect database failed");
    }
}

bool DBHelper::handleRegist(const char *name, const char *pwd)
{
    if(name == nullptr || pwd == nullptr) {
        return false;
    }
    if(strlen(name) < 3 || strlen(pwd) < 3) {
        return false;
    }
    QString sql = QString("INSERT INTO userInfo(name, pwd) VALUES('%1', '%2');")
                    .arg(name).arg(pwd);

    qDebug() << sql;

    QSqlQuery query;
    return query.exec(sql);
}

bool DBHelper::handleLogin(const char *name, const char *pwd)
{
    if(name == nullptr || pwd == nullptr) {
        return false;
    }
    if(strlen(name) < 3 || strlen(pwd) < 3) {
        return false;
    }
    QString sql = QString("SELECT id FROM userInfo WHERE name = '%1' AND pwd = '%2' AND online = 0;")
                    .arg(name).arg(pwd);

    qDebug() << sql;

    QSqlQuery query;
    if(query.exec(sql) && query.next()){
        return query.exec(QString("UPDATE userInfo SET online = 1 WHERE name = '%1';")
                    .arg(name));
    }
    return false;
}

bool DBHelper::handleLogout(const QString &name)
{
    if(name.size() < 3) return false;
    return DBHelper::handleLogout(name.toStdString().c_str());
}

bool DBHelper::handleLogout(const char *name)
{
    if(name == nullptr) {
        return false;
    }
    if(strlen(name) < 3) {
        return false;
    }
    QString sql = QString("UPDATE userInfo SET online = 0 WHERE name = '%1';")
                    .arg(name);

    qDebug() << sql;

    QSqlQuery query;
    return query.exec(sql);
}

QStringList DBHelper::handleAllOnline()
{
    QString sql = QString("SELECT name FROM userInfo WHERE online = 1;");

    qDebug() << sql;

    QSqlQuery query;
    query.exec(sql);

    QStringList nameList;
    while(query.next()) {
        nameList.append(query.value(0).toString());
    }
    return nameList;
}

int DBHelper::handleSearchUser(const char *name)
{
    if(nullptr == name) {
        return -1;
    }
    QString sql = QString("SELECT online FROM userInfo WHERE name = '%1';").arg(name);
    QSqlQuery query;
    query.exec(sql);
    if(query.next()) {
        // 0不在线, 1 不在线
        int online = query.value(0).toInt();
        return online;
    } else {
        // 用户不存在
        return -1;
    }
}

/*
 * return -1: 未知错误
 * return 0 : 不能发送好友申请, 用户不存在或者不在线
 * return 1 : 可以发送好友申请
 * return 2 : 不能发送好友申请, 已经是好友了
 * */
int DBHelper::handleAddFriend(const char *name1, const char *name2)
{
    if(nullptr == name1 || nullptr == name2) {
        return -1;
    }
    // 重新判断这两个用户是否存在或者在线
    QString sql0 = QString("SELECT COUNT(1) FROM userInfo WHERE name IN ('%1', '%2') AND online = 1;")
            .arg(name1)
            .arg(name2);
    qDebug() << sql0;
    QSqlQuery checkOnlineQuery;
    checkOnlineQuery.exec(sql0);
    if(checkOnlineQuery.next()) { // 查询到了数据
        if(checkOnlineQuery.value(0).toInt() < 2) { // 其中一方 不存在或者不在线
            return 0; // 不能发送好友申请
        }
    } else {
        return -1;
    }

    QString sql = QString("SELECT id, friendId FROM friend WHERE id = (SELECT id FROM userInfo WHERE name = '%1') AND friendId = (SELECT id FROM userInfo WHERE name = '%2');")
            .arg(name1).arg(name2);
    qDebug() << sql;

    QSqlQuery query;
    query.exec(sql);
    if(query.next()) {
        return 2; // 已经是好友了, 不能发送好友申请
    } else {
        return 1; // 可以发送申请
    }
}

bool DBHelper::handleEstablishFriend(const char *name1, const char *name2)
{
    if(nullptr == name1 || nullptr == name2) {
        return false;
    }
    // 建立双方好友关系
    {
        QString sql = QString("SELECT id, friendId FROM friend WHERE id = (SELECT id FROM userInfo WHERE name = '%1') AND friendId = (SELECT id FROM userInfo WHERE name = '%2');")
                .arg(name1).arg(name2);
        QSqlQuery query;
        query.exec(sql);
        if(query.next() == false) { // 数据不存在
            QSqlQuery q;
            q.exec(QString("INSERT OR IGNORE INTO friend(id, friendId) VALUES((SELECT id FROM userInfo WHERE name = '%1'), (SELECT id FROM userInfo WHERE name = '%2'));")
                   .arg(name1).arg(name2));
        }
    }


    {
        QString sql = QString("SELECT id, friendId FROM friend WHERE id = (SELECT id FROM userInfo WHERE name = '%1') AND friendId = (SELECT id FROM userInfo WHERE name = '%2');")
                .arg(name2).arg(name1);
        QSqlQuery query;
        query.exec(sql);
        if(query.next() == false) { // 数据不存在
            QSqlQuery q;
            q.exec(QString("INSERT OR IGNORE INTO friend(id, friendId) VALUES((SELECT id FROM userInfo WHERE name = '%1'), (SELECT id FROM userInfo WHERE name = '%2'));")
                   .arg(name2).arg(name1));
        }
    }

    return true;

}

QStringList DBHelper::handleGetFriends(const char *name)
{
    if(nullptr == name) {
        return QStringList();
    }
    QStringList friendList;
    QString sql = QString("SELECT name FROM userInfo WHERE online = 1 AND id in (SELECT friendId FROM friend WHERE id = (SELECT id FROM userInfo WHERE name = '%1'));")
            .arg(name);
    qDebug() << sql;
    QSqlQuery query;
    query.exec(sql);
    while(query.next()) {
//        qDebug() << query.value(0).toString();
        friendList.append(query.value(0).toString());
    }
    return friendList;
}

bool DBHelper::handleDeleteFriend(const char *name1, const char *name2)
{
    if(nullptr == name1 || nullptr == name2) {
        return false;
    }
    QString sql = QString("DELETE FROM friend WHERE (id = (SELECT id FROM userInfo WHERE name = '%1') AND friendId = (SELECT id FROM userInfo WHERE name = '%2')) OR (id = (SELECT id FROM userInfo WHERE name = '%3') AND friendId = (SELECT id FROM userInfo WHERE name = '%4'));")
            .arg(name1).arg(name2).arg(name2).arg(name1);
    QSqlQuery query;
    return query.exec(sql);
}
