#pragma once
#include "QtTypeDef.h"
#include <QCoreApplication>
#include <QSemaphore>
#include <QDateTime>
#include <QSystemSemaphore>
#include <thread>
using namespace std;
//////////////////////////////////////////////////////////////////////////
namespace CQtTime
{
// 阻塞延时，此延时不占用CPU资源
inline void Sleep(DWORD dwMilliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(dwMilliseconds));
}
//非阻塞
inline void SleepAsyn(int msec)
{
    QTime t;
    t.start();
    while (t.elapsed() < msec)
    {
        QCoreApplication::processEvents();
    }
}
inline ULONG GetSysTick()
{
    QDateTime time = QDateTime::currentDateTime();
    return (ULONG)time.toMSecsSinceEpoch();
}

// 获取当前时间
inline void GetLocalTime(SYSTEMTIME &stTime)
{
    QDateTime qDT = QDateTime::currentDateTime();
    QTime qTime = qDT.time();
    QDate qDate = qDT.date();

    memset(&stTime, 0x00, sizeof(SYSTEMTIME));
    stTime.wYear = qDate.year();
    stTime.wMonth = qDate.month();
    stTime.wDay = qDate.day();
    stTime.wDayOfWeek = qDate.dayOfWeek();
    stTime.wHour = qTime.hour();
    stTime.wMinute = qTime.minute();
    stTime.wSecond = qTime.second();
    stTime.wMilliseconds = qTime.msec();
    return;
}
// 获取当前时间，字符串：%04d-%02d-%02d %02d:%02d:%02d.%03d
inline void GetLocalTime(char *pTime)
{
    if (pTime == nullptr)
        return;

    QDateTime qDT = QDateTime::currentDateTime();
    QTime qTime = qDT.time();
    QDate qDate = qDT.date();
    sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d", qDate.year(), qDate.month(), qDate.day(),
            qTime.hour(), qTime.minute(), qTime.second(), qTime.msec());
    return;
}
// 当前时间
inline SYSTEMTIME CurrentTime()
{
    SYSTEMTIME stTime;
    GetLocalTime(stTime);
    return stTime;
}
// 时间差，最大精确到天，最小精确到毫秒
inline SYSTEMTIME DiffTime(const SYSTEMTIME &st1, const SYSTEMTIME &st2)
{
    QDate qD1(st1.wYear, st1.wMonth, st1.wDay);
    QTime qT1(st1.wHour, st1.wMinute, st1.wSecond, st1.wMilliseconds);
    QDate qD2(st2.wYear, st2.wMonth, st2.wDay);
    QTime qT2(st2.wHour, st2.wMinute, st2.wSecond, st2.wMilliseconds);
    QDateTime qdt1(qD1, qT1);
    QDateTime qdt2(qD2, qT2);
    QDateTime qdt3;
    SYSTEMTIME stTime;
    memset(&stTime, 0x00, sizeof(stTime));
    // 相等
    if (qdt1 == qdt2)
        return stTime;
    // 有差值
    qint64 qDays = qAbs(qdt1.daysTo(qdt2));
    qint64 qSecs = qAbs(qdt1.secsTo(qdt2));
    qint64 qMSec = qAbs(qdt1.msecsTo(qdt2));
    stTime.wDay = (WORD)qDays;
    stTime.wHour = (WORD)(qSecs % (24 * 3600)) / 3600;
    stTime.wMinute = (WORD)(qSecs % 3600) / 60;
    stTime.wSecond = (WORD)(qSecs % 60);
    stTime.wMilliseconds = (WORD)(qMSec % 1000);
    return stTime;
}
}

//////////////////////////////////////////////////////////////////////////
// 自动删除：char, int, short, long, float, double类型的动态分配内存
template<typename _Type>
class CAutoDeleteArray
{
public:
    CAutoDeleteArray(_Type **ppPoint)
    {
        m_ppPoint = ppPoint;
    }
    ~CAutoDeleteArray()
    {
        if (m_ppPoint != NULL && *m_ppPoint != NULL)
        {
            delete[](*m_ppPoint);
            *m_ppPoint = NULL;
        }
        m_ppPoint = NULL;
    }
private:
    _Type **m_ppPoint;
};
#define AutoDeleteArray(P, Type)        CAutoDeleteArray<Type>  _cAutoDeleteArray_##P(&(P))
//////////////////////////////////////////////////////////////////////////
// 自动创建和注销事件
class CAutoEvent
{
public:
    CAutoEvent(bool bInit = true) { if (bInit) m_qSemEvent.release(); }
    ~CAutoEvent() {}
    void SetEvent() { m_qSemEvent.release(); }
    bool WaitForEvent(int nTimeOut) { return m_qSemEvent.tryAcquire(1, nTimeOut); }
    QSemaphore &GetEvent() { return m_qSemEvent; }
    operator QSemaphore &() { return m_qSemEvent; } // 重载类型：QSemaphore
private:
    QSemaphore  m_qSemEvent;
};
// 此跨进程事件
class CAutoEventEx
{
public:
    CAutoEventEx(QString strName, int n = 0) : m_qSemEvent(strName, n, QSystemSemaphore::Open) {}
    ~CAutoEventEx() {}
    bool SetEvent() { return m_qSemEvent.release(); }
    bool WaitForEvent() { return m_qSemEvent.acquire(); }
    QSystemSemaphore &GetEvent() { return m_qSemEvent; }
    operator QSystemSemaphore &() { return m_qSemEvent; } // 重载类型：QSystemSemaphore
private:
    QSystemSemaphore     m_qSemEvent;
};
//////////////////////////////////////////////////////////////////////////
// char内存自动申请和释放类
class CAutoNewDeleteBuff
{
public:
    CAutoNewDeleteBuff(unsigned long ulSize) : m_pBuff(nullptr)
    {
        m_pBuff = new char[ulSize + 1];
        if (m_pBuff != nullptr)
            memset(m_pBuff, 0x00, ulSize + 1);
    }
    ~CAutoNewDeleteBuff()
    {
        if (m_pBuff != nullptr)
            delete[]m_pBuff;
        m_pBuff = nullptr;
    }
    operator char *() { return m_pBuff; } // 重载类型：char*
    operator BYTE *() { return (BYTE *)m_pBuff; } // 重载类型：BYTE*
    operator void *() { return (void *)m_pBuff; } // 重载类型：void*
public:
    char *Data() { return m_pBuff; }
    bool IsNew() { return (m_pBuff != nullptr); }
    unsigned long NewSize() { return (m_ulSize + 1); }
    void Clear() { if (m_pBuff != nullptr) memset(m_pBuff, 0x00, m_ulSize + 1); }
private:
    char         *m_pBuff;
    unsigned long m_ulSize;
};

//////////////////////////////////////////////////////////////////////////
// 按分隔符，分离字符串
class CAutoSplitByStep
{
public:
    typedef vector<std::string> vectorString;
public:
    CAutoSplitByStep() { m_vtString.clear(); }
    CAutoSplitByStep(LPCSTR lpStr, LPCSTR lpStep) { SplitByStep(lpStr, lpStep); }
    CAutoSplitByStep(std::string Str, string strStep) { SplitByStep(Str.c_str(), strStep.c_str()); }
    ~CAutoSplitByStep() { m_vtString.clear(); }
public:
    UINT Count() { return m_vtString.size(); }
    LPCSTR At(UINT uIndex)
    {
        if (!m_vtString.empty() && uIndex < m_vtString.size())
            return m_vtString[uIndex].c_str();
        return NULL;
    }
    void GetString(vectorString &vtStr)
    {
        vtStr.clear();
        if (!m_vtString.empty())
        {
            vtStr.resize(m_vtString.size());
            copy(m_vtString.begin(), m_vtString.end(), vtStr.begin());
        }
    }
    void SplitByStep(LPCSTR lpStr, LPCSTR lpStep)
    {
        m_vtString.clear();
        QString strStep = QString::fromLocal8Bit(lpStep);
        QString strBuff = QString::fromLocal8Bit(lpStr);
        QStringList strList = strBuff.split(QRegExp("[" + strStep + "]"), QString::SkipEmptyParts);
        for (auto it : strList)
        {
            m_vtString.push_back(it.toLocal8Bit().toStdString());
        }
    }
private:
    vectorString m_vtString;
};

//////////////////////////////////////////////////////////////////////////
class CAutoHex
{
public:
    CAutoHex() {}
    CAutoHex(const QByteArray &vtBcd) { Bcd2Hex((LPBYTE)vtBcd.constData(), vtBcd.size()); }
    CAutoHex(const BYTE *pBcd, UINT uLen) { Bcd2Hex(pBcd, uLen); }
    ~CAutoHex() {}
public:
    const char *GetHex() { return m_strHex.c_str(); }
    const char *Bcd2Hex(const BYTE *pBcd, UINT uLen)
    {
        m_strHex = "";
        char szHex[8] = { 0 };
        for (UINT i = 0; i < uLen; i++)
        {
            memset(szHex, 0x00, sizeof(szHex));
            sprintf(szHex, "%02X", pBcd[i]);
            m_strHex += szHex;
        }
        return m_strHex.c_str();
    }
    static char *Bcd2Hex(const BYTE *pBcd, UINT uLen, char *pHex)
    {
        for (UINT i = 0; i < uLen; i++)
        {
            sprintf(pHex + i * 2, "%02X", pBcd[i]);
        }
        return pHex;
    }
    static char *Bcd2Hex(const BYTE *pBcd, UINT uLen, QByteArray &vtHex)
    {
        vtHex.resize(uLen * 2);
        for (UINT i = 0; i < uLen; i++)
        {
            sprintf(vtHex.data() + i * 2, "%02X", pBcd[i]);
        }
        return vtHex.data();
    }
    static void Hex2Bcd(const char *pHex, BYTE *pBcd, UINT &uLen)
    {
        UINT bBCD = 0;
        uLen = strlen(pHex) / 2;
        for (UINT i = 0; i < uLen; i++)
        {
            sscanf(pHex + i * 2, "%02X", &bBCD);
            sprintf((char *)pBcd + i, "%c", bBCD & 0xFF);
        }
    }
    static void Hex2Bcd(const char *pHex, QByteArray &vtBCD)
    {
        UINT bBCD = 0;
        UINT uLen = strlen(pHex) / 2;
        vtBCD.clear();
        for (UINT i = 0; i < uLen; i++)
        {
            sscanf(pHex + i * 2, "%02X", &bBCD);
            vtBCD.push_back(bBCD & 0xFF);
        }
    }
    static ULONG HexToLen(const char *pHex)
    {
        UINT uHex = strlen(pHex);
        UINT uVal = 0;
        ULONG ulLen = 0;
        for (UINT i = 0; i < uHex; i++)
        {
            if (pHex[i] < '0' || pHex[i] > 'F')// 不是有效数字位
            {
                ulLen = 0;
                break;
            }

            if ((pHex[i] >= 'A') && (pHex[i] <= 'F'))
            {
                uVal = pHex[i] - 'A' + 10;
            }
            else
            {
                uVal = pHex[i] - '0';
            }

            uVal = uVal / 16 * 10 + uVal % 16;
            ulLen = ulLen * 16 + uVal;
        }

        return ulLen;
    }
    static const char *HexXOR(const char *pData, const char *pIV, QByteArray &vtXOR, bool bAppend = false)
    {
        UINT uX = 0;
        UINT uR = 0;
        UINT uXORLen = strlen(pIV);
        char szXOR[16] = { 0 };
        vtXOR.clear();
        for (UINT i = 0; i < uXORLen / 2; i++)
        {
            sscanf(pData + i * 2, "%02X", &uX);
            sscanf(pIV + i * 2, "%02X", &uR);
            sprintf(szXOR, "%02X", (uX & 0xFF) ^ (uR & 0xFF));
            vtXOR += szXOR;
        }
        if (bAppend)
            vtXOR.append(pData + uXORLen);
        return vtXOR.constData();
    }
    const char *HexXOR(const char *pX, const char *pR, UINT uLen = 16)
    {
        UINT uX = 0;
        UINT uR = 0;
        char szXOR[512] = { 0 };
        for (UINT i = 0; i < uLen / 2; i++)
        {
            sscanf(pX + i * 2, "%02X", &uX);
            sscanf(pR + i * 2, "%02X", &uR);
            sprintf(szXOR + i * 2, "%02X", (uX & 0xFF) ^ (uR & 0xFF));
        }

        m_strHex = szXOR;
        return m_strHex.c_str();
    }
    static const char *HexXOR(QByteArray &vtNew, const QByteArray &vtIV, UINT uLen = 16)
    {
        if (vtIV.length() < uLen)
            return vtNew.constData();

        UINT uX = 0;
        UINT uR = 0;
        char szXOR[512] = { 0 };
        char *pX = vtNew.data();
        const char *pY = vtIV.data();
        for (UINT i = 0; i < uLen / 2; i++)
        {
            sscanf(pX + i * 2, "%02X", &uX);
            sscanf(pY + i * 2, "%02X", &uR);
            sprintf(szXOR + i * 2, "%02X", (uX & 0xFF) ^ (uR & 0xFF));
        }

        vtNew.replace(0, uLen, szXOR);
        return vtNew.constData();
    }
private:
    string m_strHex;
};
//////////////////////////////////////////////////////////////////////////
template <class DEV_TYPE>
class CAutoCopyDevStatus
{
public:
    CAutoCopyDevStatus(DEV_TYPE *pOut, DEV_TYPE *pNew) : m_pOut(pOut), m_pNew(pNew) {}
    ~CAutoCopyDevStatus()
    {
        if (m_pOut != nullptr && m_pNew != nullptr)
            memcpy(m_pOut, m_pNew, sizeof(DEV_TYPE));
    }
private:
    DEV_TYPE *m_pOut;
    DEV_TYPE *m_pNew;
};
//////////////////////////////////////////////////////////////////////////
