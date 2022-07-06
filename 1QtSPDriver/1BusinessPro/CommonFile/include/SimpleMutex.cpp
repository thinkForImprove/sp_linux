#include "SimpleMutex.h"


//////////////////////////////////////////////////////////////////////////
CSimpleMutex::CSimpleMutex()
{
}


CSimpleMutex::~CSimpleMutex()
{
}

void CSimpleMutex::Lock()
{
    m_cMutex.lock();
}
void CSimpleMutex::Unlock()
{
    m_cMutex.unlock();
}

//////////////////////////////////////////////////////////////////////////

CQtSimpleMutex::CQtSimpleMutex()
{
}

CQtSimpleMutex::~CQtSimpleMutex()
{
}

void CQtSimpleMutex::Lock()
{
    m_cMutex.lock();
}

void CQtSimpleMutex::Unlock()
{
    m_cMutex.unlock();
}
//////////////////////////////////////////////////////////////////////////

CQtIsRunning::CQtIsRunning(QString strKey):
    m_strKey(strKey), m_cSemSingle(strKey + "_SemKey", 1)
{
}

CQtIsRunning::~CQtIsRunning()
{
    m_cMem.detach();
}

bool CQtIsRunning::IsRunning()
{
    // 信号量的意义，把操作共享内存的代码锁住。因为有可能同时点击2次APP, 防止并发
    m_cSemSingle.acquire();
#ifdef Q_OS_LINUX
    /*  Windows平台上不存在应用程序崩溃后，共享内存段还存在的情况
        LINUX应用程序崩溃后,共享内存段不会自动销毁,则该程序再次运行会出问题
        所以程序启动时先去检查是否有程序崩溃后还存留的共享内存段，如果有，先销毁,再创建
    */
    QSharedMemory qMem(m_strKey);
    // 尝试将进程附加到共享内存段
    if (qMem.attach())
    {
        // 将共享内存与主进程分离, 如果此进程是附加到共享存储器段的最后一个进程，则系统释放共享存储器段，即销毁内容
        qMem.detach();
    }
#endif

    /*
        每个App打开的时候，获取一次共享内存。
        如果获取失败，说明是第一个启动的APP，直接创建共享内存就好了。
        如果获取成功，说明不是第一个，直接退出就好了。
        保证App在系统里只能打开一个。
    */
    bool bRunning = true;
    m_cMem.setKey(m_strKey);
    if (!m_cMem.attach())
    {
        m_cMem.create(1);
        bRunning = false;
    }
    m_cSemSingle.release();
    return bRunning;
}
//////////////////////////////////////////////////////////////////////////

CQtSimpleMutexEx::CQtSimpleMutexEx(QString strName):
    m_cSemMutex(strName, 1, QSystemSemaphore::Open)
{
}

CQtSimpleMutexEx::~CQtSimpleMutexEx()
{
}

void CQtSimpleMutexEx::Lock()
{
    m_cSemMutex.acquire();
}

void CQtSimpleMutexEx::Unlock()
{
    m_cSemMutex.release();
}

//////////////////////////////////////////////////////////////////////////
CAutoMutex::CAutoMutex(ISimpleSyncObj *SyncObj): m_pSyncObj(SyncObj)
{
    if (m_pSyncObj != nullptr)
        m_pSyncObj->Lock();
}
CAutoMutex::~CAutoMutex()
{
    if (m_pSyncObj != nullptr)
        m_pSyncObj->Unlock();
}
