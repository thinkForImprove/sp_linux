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
    // 写数据，不可重复写，一直等待缓存为空，注意配套使用
    bool Write(const void *pBuff, int nBuffSize);
    // 读数据，读取数据后，清空缓存
    bool Read(void *pBuff, int nBuffSize);
    // 写数据，可重复写，直接覆盖
    bool Write(const QByteArray &vtData);
    // 读数据，不会清缓存，可重复读
    bool Read(QByteArray &vtData);
private:
    QByteArray          m_vtFlagW;
    QString             m_strKey;
    QSharedMemory       m_cMem;
    QSystemSemaphore    m_cMutex;
};
