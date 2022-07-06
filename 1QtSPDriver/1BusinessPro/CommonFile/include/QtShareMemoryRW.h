#pragma once
#include <QSharedMemory>
#include <QSystemSemaphore>

//////////////////////////////////////////////////////////////////////////
class CQtShareMemoryRW
{
public:
    CQtShareMemoryRW(QString strMutex);
    ~CQtShareMemoryRW();
public:
    // 释放读阻塞一次
    void Release();
    // 打开连接或创建
    bool Open(QString strKey, bool bCreate = false, int nSize = 10 * 1024);
    // 是否已打开
    bool IsOpened();
    // 内存空间大小
    int Size();
    // 写数据
    bool Write(const void *pBuff, int nBuffSize);
    // 读数据
    bool Read(void *pBuff, int nBuffSize);
private:
    QByteArray          m_vtFlagW;
    QString             m_strKey;
    QSharedMemory       m_cMem;
    QSystemSemaphore    m_cMutex;
};
