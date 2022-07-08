#pragma once
//////////////////////////////////////////////////////////////////////////
#include <QString>
#include <QMutex>
#include <QSharedMemory>
#include <QSystemSemaphore>
//////////////////////////////////////////////////////////////////////////
#include <mutex>
//////////////////////////////////////////////////////////////////////////
//同步对象结构
struct ISimpleSyncObj
{
    //申请锁定
    virtual void Lock() = 0;
    //解锁
    virtual void Unlock() = 0;
};

//////////////////////////////////////////////////////////////////////////
//STL简单互斥量
class CSimpleMutex : public ISimpleSyncObj
{
public:
    CSimpleMutex();
    virtual ~CSimpleMutex();
    virtual void Lock();
    virtual void Unlock();
private:
    std::recursive_mutex m_cMutex;
};
//QT简单互斥量
class CQtSimpleMutex : public ISimpleSyncObj
{
public:
    CQtSimpleMutex();
    virtual ~CQtSimpleMutex();
    virtual void Lock();
    virtual void Unlock();
private:
    QMutex m_cMutex;
};

//QT简单互斥量，跨进程
class CQtSimpleMutexEx : public ISimpleSyncObj
{
public:
    CQtSimpleMutexEx(QString strName);
    virtual ~CQtSimpleMutexEx();
    virtual void Lock();
    virtual void Unlock();
private:
    QSystemSemaphore    m_cSemMutex;
};

// 单实例控制类，判断是否已在运行
class CQtIsRunning
{
public:
    CQtIsRunning(QString strKey);
    ~CQtIsRunning();
public:
    bool IsRunning();
private:
    QString             m_strKey;
    QSharedMemory       m_cMem;
    QSystemSemaphore    m_cSemSingle;
};
//////////////////////////////////////////////////////////////////////////
//自动解锁辅助类
class CAutoMutex
{
public:
    CAutoMutex(ISimpleSyncObj *SyncObj);
    ~CAutoMutex();
private:
    ISimpleSyncObj *m_pSyncObj;
};

//////////////////////////////////////////////////////////////////////////
//自动解锁辅助宏
#define AutoMutex(X)            CAutoMutex  _auto_mutex_obj(&X)
#define AutoMutexStl(X)         std::lock_guard<std::recursive_mutex>   _auto_lock_guard_obj(X)
//////////////////////////////////////////////////////////////////////////
