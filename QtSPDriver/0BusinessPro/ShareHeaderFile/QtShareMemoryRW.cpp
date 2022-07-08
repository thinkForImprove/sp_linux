#include "QtShareMemoryRW.h"
#include <thread>

//////////////////////////////////////////////////////////////////////////
class CAutoMemoryLock
{
public:
    CAutoMemoryLock(QSharedMemory *pMemory): m_pMemory(pMemory) { if (m_pMemory != nullptr) m_pMemory->lock(); }
    ~CAutoMemoryLock() { if (m_pMemory != nullptr) m_pMemory->unlock(); }
private:
    QSharedMemory *m_pMemory;
};
//class CAutoQMutexLock
//{
//public:
//    CAutoQMutexLock(QMutex *pMutex) : m_pMutex(pMutex) { if (m_pMutex != nullptr) m_pMutex->lock(); }
//    ~CAutoQMutexLock() { if (m_pMutex != nullptr) m_pMutex->unlock(); }
//private:
//    QMutex *m_pMutex;
//};
//#define AutoQtMutex(X)            CAutoQMutexLock _auto_mutex_obj(&X)

class CAutoSemMutexLock
{
public:
    CAutoSemMutexLock(QSystemSemaphore *pMutex) : m_pMutex(pMutex) { if (m_pMutex != nullptr) m_pMutex->acquire(); }
    ~CAutoSemMutexLock() { if (m_pMutex != nullptr) m_pMutex->release(); }
private:
    QSystemSemaphore *m_pMutex;
};

#define AutoQtMutex(X)          CAutoSemMutexLock   _auto_mutex_obj(&X)
//////////////////////////////////////////////////////////////////////////
CQtShareMemoryRW::CQtShareMemoryRW(QString strMutex):
    m_cMutex(strMutex, 1, QSystemSemaphore::Open)
{
    m_vtFlagW.push_back('W');
}

CQtShareMemoryRW::~CQtShareMemoryRW()
{
    if (m_cMem.isAttached())
        m_cMem.detach();
}

void CQtShareMemoryRW::Release()
{
}

bool CQtShareMemoryRW::Open(QString strKey, bool bCreate, int nSize)
{
    AutoQtMutex(m_cMutex);
#ifdef QT_LINUX
    /* Windows平台上不存在应用程序崩溃后，共享内存段还存在的情况
     * LINUX应用程序崩溃后,共享内存段不会自动销毁,则该程序再次运行会出问题
     * 所以程序启动时先去检查是否有程序崩溃后还存留的共享内存段，如果有，先销毁,再创建
     */
    QSharedMemory qTmpMem(strKey);
    if (qTmpMem.attach())   // 尝试将进程附加到共享内存段
    {
        // 将共享内存与主进程分离, 如果此进程是附加到共享存储器段的最后一个进程，则系统释放共享存储器段，即销毁内容
        qTmpMem.detach();
    }
#endif
    // 如果已有，尝试释放一次
    if (!m_strKey.isEmpty())
    {
        if (m_cMem.isAttached())
            m_cMem.detach();
    }
    m_cMem.setKey(strKey);
    if (bCreate)
    {
        // 先判断是否已存在，如果不存在，则重新创建
        if (!m_cMem.attach())
        {
            if (!m_cMem.create(nSize + m_vtFlagW.size()))
                return false;
            // 清空缓存
            CAutoMemoryLock _auto(&m_cMem);
            memset((char *)m_cMem.data(), 0x00, m_cMem.size());
        }
    }
    else
    {
        if (!m_cMem.attach())
            return false;
    }

    m_strKey = strKey;
    return true;
}

bool CQtShareMemoryRW::IsOpened()
{
    AutoQtMutex(m_cMutex);
    if (m_strKey.isEmpty())
        return false;
    if (m_cMem.isAttached())
        return true;
    if (m_cMem.attach())
        return true;
    return false;
}

int CQtShareMemoryRW::Size()
{
    AutoQtMutex(m_cMutex);
    return m_cMem.size() - m_vtFlagW.size();
}

bool CQtShareMemoryRW::Write(const void *pBuff, int nBuffSize)
{
    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        {
            AutoQtMutex(m_cMutex);
            if (!m_cMem.isAttached())
            {
                if (!m_cMem.attach())
                {
                    return false;
                }
            }

            CAutoMemoryLock _auto(&m_cMem);
            // 先比较标志，防止重复写，导致内容覆盖
            if (memcmp(m_vtFlagW.constData(), (char *)m_cMem.constData(), m_vtFlagW.size()) == 0)
            {
                continue;
            }

            memcpy((char *)m_cMem.data(), m_vtFlagW.data(), m_vtFlagW.size());
            memcpy((char *)m_cMem.data() + m_vtFlagW.size(), pBuff, qMin(m_cMem.size() - m_vtFlagW.size(), nBuffSize));
            break;
        }
    } while (true);
    return true;
}

bool CQtShareMemoryRW::Read(void *pBuff, int nBuffSize)
{
    AutoQtMutex(m_cMutex);
    if (!m_cMem.isAttached())
    {
        if (!m_cMem.attach())
        {
            return false;
        }
    }

    CAutoMemoryLock _auto(&m_cMem);
    // 先比较标志，防止空读
    if (memcmp(m_vtFlagW.constData(), (char *)m_cMem.data(), m_vtFlagW.size()) != 0)
    {
        return false;
    }

    memcpy(pBuff, (char *)m_cMem.data() + m_vtFlagW.size(), qMin(m_cMem.size() - m_vtFlagW.size(), nBuffSize));
    memset((char *)m_cMem.data(), 0x00, m_cMem.size());     // 清空缓存
    return true;
}

bool CQtShareMemoryRW::Write(const QByteArray &vtData)
{
    AutoQtMutex(m_cMutex);
    if (!m_cMem.isAttached())
    {
        if (!m_cMem.attach())
        {
            return false;
        }
    }

    CAutoMemoryLock _auto(&m_cMem);
    QByteArray vtLen(8, 0x00);
    sprintf(vtLen.data(), "%08d", vtData.size());
    memcpy((char *)m_cMem.data(), vtLen.data(), vtLen.size());
    memcpy((char *)m_cMem.data() + vtLen.size(), vtData.data(), qMin(m_cMem.size() - vtLen.size(), vtData.size()));
    return true;
}

bool CQtShareMemoryRW::Read(QByteArray &vtData)
{
    AutoQtMutex(m_cMutex);
    if (!m_cMem.isAttached())
    {
        if (!m_cMem.attach())
        {
            return false;
        }
    }

    CAutoMemoryLock _auto(&m_cMem);
    QByteArray vtLen(8, 0x00);
    memcpy(vtLen.data(), (char *)m_cMem.data(), vtLen.size());
    vtData.resize(vtLen.toInt());
    memcpy(vtData.data(), (char *)m_cMem.data() + vtLen.size(), qMin(m_cMem.size() - vtLen.size(), vtLen.toInt()));
    return true;
}
