#include "mydir.h"

#define SAVE_PATH_ROOT "D:\\development\\qt\\NetDisk_SavePath\\"


MyDir::MyDir():QDir(SAVE_PATH_ROOT) {
    m_rootPath = this->absolutePath();
    if(!this->exists()) {
        throw QString("存储路径不存在");
    }
}

MyDir::MyDir(const QString &path):QDir(QDir::cleanPath(QString(SAVE_PATH_ROOT) + QDir::separator() + path)) {
    m_rootPath = QDir::cleanPath(QString(SAVE_PATH_ROOT));
    if(!this->exists()) {
        throw QString("存储路径不存在");
    }
}

bool MyDir::cdOrMkdir(const QString &path)
{
    // 不存在则返回false
    bool res = false;
    if(!this->exists(this->absolutePath() + this->separator() + path)) {
        res = false;
        this->mkdir(path);
    }
    this->cd(path);
    return res;
}

bool MyDir::cdBack()
{
    if(this->cdUp()) {

        if(this->getRelativePath().size() == 0) { // 相对路径为 "", 表示返回到绝对"/"了
            return false;
        }
        return true;

    } else {
        return false; // 返回上一级失败
    }
}

bool MyDir::cdRoot()
{
    return this->cd(this->m_rootPath);
}

bool MyDir::checkDir(const QString &dirName)
{
    QString path = this->cleanPath(this->absolutePath() + this->separator() + dirName);
    return this->exists(path) && QFileInfo(path).isDir();
}

bool MyDir::checkFile(const QString &fileName)
{
    QString path = this->cleanPath(this->absolutePath() + this->separator() + fileName);
    return this->exists(path) && QFileInfo(path).isFile();
}

QString MyDir::getRelativePath() const
{
    QString absPath = this->absolutePath();
    return absPath.mid(this->m_rootPath.length());
}

QFileInfoList MyDir::listDir()
{
    return this->entryInfoList(NoFilter, DirsFirst | Name);
}

bool MyDir::deleteDir(const QString &dirName)
{
    if(!checkDir(dirName)) { // 不存在这个目录
        return false;
    }
    this->cdOrMkdir(dirName); // 切换到要删除的目录
    return this->removeRecursively(); // 递归删除当前
}

bool MyDir::deleteFile(const QString &fileName)
{
    if(!checkFile(fileName)) { // 不存在这个文件
        return false;
    }
    return this->remove(fileName);
}

bool MyDir::renameDir(const QString &oldDirName, const QString &newDirName)
{
    if(!checkDir(oldDirName)) { // 不存在这个目录
        return false;
    }
    return this->rename(oldDirName, newDirName);
}

QString MyDir::getNewFilePath(const QString &fileName)
{
    return this->absoluteFilePath(fileName);
}

QString MyDir::getRelativeNewFilePath(const QString &fileName)
{
    return this->cleanPath(this->getRelativePath() + this->separator() + fileName);
}
