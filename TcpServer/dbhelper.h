#ifndef DBHELPER_H
#define DBHELPER_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QStringList>

class DBHelper : public QObject
{
    Q_OBJECT
public:
    // 禁用拷贝构造和拷贝赋值
    DBHelper(const DBHelper& helper) = delete;
    DBHelper& operator=(const DBHelper& helper) = delete;
    ~DBHelper();

    static DBHelper& getInstance();

    void init();

    bool handleRegist(const char* name, const char* pwd);

    bool handleLogin(const char* name, const char* pwd);

    bool handleLogout(const char* name);
    bool handleLogout(const QString& name);

    QStringList handleAllOnline();

    int handleSearchUser(const char* name);

    int handleAddFriend(const char* name1, const char* name2);

    bool handleEstablishFriend(const char* name1, const char* name2);

    QStringList handleGetFriends(const char* name);

    bool handleDeleteFriend(const char* name1, const char* name2);


private:
    // 构造函数
    explicit DBHelper(QObject *parent = 0);

    QSqlDatabase m_conn; //数据库连接

signals:

public slots:
};

#endif // DBHELPER_H
