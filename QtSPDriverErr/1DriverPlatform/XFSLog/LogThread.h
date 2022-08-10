#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include "SimpleMutex.h"
#include <unistd.h>
#include <list>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <semaphore.h>
#include <QDir>
#include <QMutexLocker>
using namespace std;

#define MAX_PATH          260

//日志线程类
class CLogThread
{
public:
    //pMutexName: 写文件互斥变量名
    CLogThread(const char *pMutexName);
    virtual ~CLogThread();

    //通知线程退出并等待退出（须在系统退出前调用）
    void NotifyExitAndWait();

    //增加一条日志
    //pszFilePath: 写追加到的文件的文件路径名
    //pStr: 要追加的日志数据，以\0结束
    void AddLog(const char *pszFilePath, const char *pStr);

    //得到写文件互斥量
    CQtSimpleMutexEx &GetWriteFileMutex() { return m_LogToFileMutex; }

    //可重载的保护成员
protected:
    //在写日志数据到文件前调用，如返回FALSE，该数据丢弃不记录，默认创建日志目录
    //pszFilePath: 要记录的文件路径名
    //pStr: 要记录的内容，以\0结束
    virtual bool OnBeforeLog(const char *pszFilePath, const char *pStr);

    //在写日志数据到文件后调用
    //bLogSuccess: 是否记录到文件成功
    virtual void OnAfterLog(const char *pszFilePath, const char *pStr, bool bLogSuccess) { }

    //检查目录如果不存在则创建
    //返回：FALSE，创建失败
    static bool CreateDirIfNotExisting(const char *pDirName);
    //私有成员
private:
    typedef struct _tag_log_data //内部日志数据
    {
        char szFilePath[MAX_PATH];
        char szData[1];
    } LOG_DATA;

    //读取一条日志记录，返回的数据用完必须delete
    LOG_DATA *PeekALog();

    //写日志数据到指定文件，返回TRUE，成功
    bool LogToFile(const char *pszFilePath, const char *pStr);

    //读数据并写到文件
    void ReadAndLog();

    //线程相关函数
    void InnerThreadProc();
    static void *ThreadProc(void *pParam)
    {
        ((CLogThread *)pParam)->InnerThreadProc();
        sem_post(&(((CLogThread *)pParam)->m_semFinish));
        pthread_exit(NULL);
        return 0;
    }


    //私有数据
private:
    sem_t                   m_semExit;      //指示线程是否退出
    sem_t                   m_semHasData;   //指示是否有日志数据
    sem_t                   m_semFinish;    //指示日志线程是否完成
    QMutex                  m_LogQueueMutex;//访问队列及事件临界区自动锁
    CQtSimpleMutexEx            m_LogToFileMutex;   //写到文件的事件互斥，使用用命名互斥量，多进程互斥
    list<LOG_DATA *>         m_slContent;       //要记录的日志列表
    pthread_t               m_threadId;         //子线程id
};

#endif // LOGTHREAD_H
