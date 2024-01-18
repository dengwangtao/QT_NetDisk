#ifndef MYDIR_H
#define MYDIR_H

#include <QDir>
#include <QVector>
#include <QPair>

class MyDir : public QDir
{
public:
    MyDir();
    MyDir(const QString& path);

    bool cdOrMkdir(const QString& path); // 进入目录, 不存在则创建

    bool cdBack();   // 返回上一级目录

    bool cdRoot();   // 返回根目录

    bool checkDir(const QString& dirName); // 判断当前路径下是否存在这个文件夹

    bool checkFile(const QString& fileName); // 判断当前路径下是否存在这个文件

    QString getRelativePath() const; // 返回相对路径

    QFileInfoList listDir(); // 列出当前的文件夹内容 {name, isFile}

    bool deleteDir(const QString& dirName); // 删除当前路径下这个文件夹

    bool deleteFile(const QString& fileName); // 删除当前路径下这个文件

    bool renameDir(const QString& oldDirName, const QString& newDirName); // 重命名文件夹

    QString getNewFilePath(const QString& fileName); // 拼接当前路径和文件名, 并返回

    QString getRelativeNewFilePath(const QString& fileName); // 拼接当前路径和文件名, 并返回 (相对路径)

private:
    QString m_rootPath;
};

#endif // MYDIR_H
