#ifndef XFSLOGTHREAD_H
#define XFSLOGTHREAD_H

#include "LogThread.h"
#include "LogBackupManager.h"

#ifdef QT_WIN32
#define INIPATH "C:/CFES/ETC/XFSLogManager.ini"
#else
#define INIPATH "/usr/local/CFES/ETC/XFSLogManager.ini"
#endif
#ifdef QT_WIN32
#define SPILOGPATH "D:/LOG/SPI"
#else
//#define SPILOGPATH "/usr/local/LOG/SPI"
#define SPILOGPATH "/usr/local/LOG"
#endif

#define DEF_BACKUP_COUNT "90"

// XFS日志线程
class CXFSLogThread : protected CLogThread
{
public:
    CXFSLogThread();
    virtual ~CXFSLogThread();

    //记录日志
    void Log(const char *pStr);

    //目录操作
protected:
    //生成当前日志目录名
    // szPath：返回生成的路径名
    void GenerateCurLogDirName(char szPath[MAX_PATH]);

    //生成当前日志文件路径名
    void GenerateCurLogPath(char szPath[MAX_PATH]);

    //重载函数
protected:
    //在写日志数据到文件前调用，如返回FALSE，该数据丢弃不记录
    // pszFilePath: 要记录的文件路径名
    // pStr: 要记录的内容，以\0结束
    virtual bool OnBeforeLog(const char *pszFilePath, const char *pStr);

    //从配置文件中读最大备份个数
    int ReadMaxBackupCount();

    //私有数据
private:
    char              m_szLogRootDir[MAX_PATH]; //日志根目录
    int               m_nMaxBackupCount;        //最大日志备份数
    CLogBackupManager m_LogBackupManager;
};
#endif // XFSLOGTHREAD_H
