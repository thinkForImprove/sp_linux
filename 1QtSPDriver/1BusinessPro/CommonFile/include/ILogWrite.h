#pragma once

#include <QtCore/qglobal.h>
#include <QLibrary>
#include <QTime>

#if defined(LOGWRITE_LIBRARY)
#  define LOGWRITESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LOGWRITESHARED_EXPORT Q_DECL_IMPORT
#endif

//////////////////////////////////////////////////////////////////////////
#define LOGFILE             "Event.log"
#define THISMODULE(X)       const char* ThisModule = X
#define BUFFSIZE            (8192)                      // 8K大小
//////////////////////////////////////////////////////////////////////////
typedef const char         *LPCSTR;
//////////////////////////////////////////////////////////////////////////
struct ILogWrite
{
    // 释放对象
    virtual void Release() = 0;
    // 设置：日志文件名和错误发生的文件
    virtual void SetLogFile(const char *pFileName, const char *pErrFile, const char *pType = nullptr) = 0;
    // 写日志函数
    virtual void Log(const char *pErrModule, int nErrCode, const char *pFormat, ...) = 0;
    // 写日志函数，连续相同内容日志，不重复记录
    virtual void MLog(const char *pErrModule, int nErrCode, const char *pFormat, ...) = 0;
    // 写日志函数，写一行日志，自动转换为HEX字符
    virtual void LogHex(const char *pFileName, const char *pBuff, int nBuffLen) = 0;
    // 写日志函数，写一行日志
    virtual void LogTrace(const char *pFileName, const char *pFormat, ...) = 0;
};

extern "C" LOGWRITESHARED_EXPORT long CreateILogWrite(ILogWrite *&p);
//////////////////////////////////////////////////////////////////////////
// 日志管理类
class CLogManage
{
public:
    typedef long(*LPCREATELOGWRITE)(ILogWrite *&ppLog);
public:
    CLogManage(): m_pLog(nullptr) {}
    ~CLogManage()
    {
        if (m_cDll.isLoaded())
        {
            if (m_pLog != nullptr)
            {
                //m_pLog->Release();
                m_pLog = nullptr;
            }
            m_cDll.unload();
        }

    }
public:
    // 设置：日志文件名和错误发生的文件
    void SetLogFile(const char *pFileName, const char *pErrFile, const char *pType = nullptr)
    {
        LoadDll();
        if (m_pLog == nullptr)
            return;
        m_pLog->SetLogFile(pFileName, pErrFile, pType);
    }
    // 写日志函数
    void Log(const char *pErrModule, int nErrCode, const char *pFormat, ...)
    {
        if (m_pLog == nullptr)
            return;

        char szBuff[BUFFSIZE] = { 0 };
        va_list vl;
        va_start(vl, pFormat);
        vsprintf(szBuff, pFormat, vl);
        va_end(vl);

        m_pLog->Log(pErrModule, nErrCode, szBuff);
        return;
    }
    // 写日志函数，只有在Debug模式下，才记录日志
    void DLog(const char *pErrModule, int nErrCode, const char *pFormat, ...)
    {
#ifdef QT_DEBUG
        if (m_pLog == nullptr)
            return;

        char szBuff[BUFFSIZE] = { 0 };
        va_list vl;
        va_start(vl, pFormat);
        vsprintf(szBuff, pFormat, vl);
        va_end(vl);

        m_pLog->Log(pErrModule, nErrCode, szBuff);
        return;
#endif
    }
    // 写日志函数，连续相同内容日志，不重复记录
    void MLog(const char *pErrModule, int nErrCode, const char *pFormat, ...)
    {
        if (m_pLog == nullptr)
            return;

        char szBuff[BUFFSIZE] = { 0 };
        va_list vl;
        va_start(vl, pFormat);
        vsprintf(szBuff, pFormat, vl);
        va_end(vl);

        m_pLog->MLog(pErrModule, nErrCode, szBuff);
        return;
    }
    // 写日志函数，写一行日志，自动转换为HEX字符
    void LogHex(const char *pFileName, const char *pBuff, int nBuffLen)
    {
        if (m_pLog == nullptr)
            return;

        m_pLog->LogHex(pFileName, pBuff, nBuffLen);
        return;
    }
    // 写日志函数，写一行日志
    void LogTrace(const char *pFileName, const char *pFormat, ...)
    {
        if (m_pLog == nullptr)
            return;

        char szBuff[BUFFSIZE] = { 0 };
        va_list vl;
        va_start(vl, pFormat);
        vsprintf(szBuff, pFormat, vl);
        va_end(vl);

        m_pLog->LogTrace(pFileName, szBuff);
        return;
    }
private:
    void LoadDll()
    {
        if (!m_cDll.isLoaded())
        {
#ifdef QT_WIN32
            m_cDll.setFileName("LogWrite");
#else
            m_cDll.setFileName("/usr/local/CFES/BIN/LogWrite");
#endif
            if (m_cDll.load())
            {
                LPCREATELOGWRITE pCreate = (LPCREATELOGWRITE)m_cDll.resolve("CreateILogWrite");
                if (pCreate != nullptr)
                {
                    pCreate(m_pLog);
                }
            }
        }
    }
private:
    ILogWrite *m_pLog;
    QLibrary m_cDll;
};
//////////////////////////////////////////////////////////////////////////
// 进出函数及时间统计类
// 因QT频烦的加载和释放库会有问题：占用的CPU资源无法释放导致进程异常
class CAutoLogFuncBeginEndEx
{
public:
    CAutoLogFuncBeginEndEx(CLogManage *pLog, LPCSTR ThisFile, LPCSTR ThisModule, int nFlag = 0, LPCSTR lpLogFile = LOGFILE, LPCSTR lpPrefixType = nullptr)
    {
        m_nFlag = nFlag;
        m_qTime = QTime::currentTime();
        m_pLog  = pLog;
        strcpy(m_szModule, ThisModule);
        m_pLog->SetLogFile(lpLogFile, ThisFile, lpPrefixType);
        m_pLog->Log(m_szModule, m_nFlag, "%s->开始调用", m_szModule);
    }
    ~CAutoLogFuncBeginEndEx()
    {
        m_pLog->Log(m_szModule, m_nFlag, "%s->结束调用[耗时 %d 毫秒]", m_szModule, m_qTime.elapsed());
    }
private:
    char        m_szModule[256];
    int         m_nFlag;
    QTime       m_qTime;
    CLogManage *m_pLog;
};

class CAutoLogFuncBeginEnd
{
public:
    CAutoLogFuncBeginEnd(CLogManage *pLog, LPCSTR ThisModule): m_pLog(pLog)
    {
        strcpy(m_szModule, ThisModule);
        m_qTime = QTime::currentTime();
        m_pLog->Log(m_szModule, 0, "%s->开始调用", m_szModule);
    }
    ~CAutoLogFuncBeginEnd()
    {
        m_pLog->Log(m_szModule, 0, "%s->结束调用[耗时 %d 毫秒]", m_szModule, m_qTime.elapsed());
    }
private:
    char  m_szModule[256];
    CLogManage *m_pLog;
    QTime m_qTime;
};


#define AutoLogFuncBeginEnd()           CAutoLogFuncBeginEnd _auto_log_begin_end(this, ThisModule)
#define AutoLogFuncBeginEndEx()         CAutoLogFuncBeginEndEx _auto_log_begin_end_ex(&gLog, ThisFile, ThisModule)
#define AutoLogFuncBeginEndExFile(X)    CAutoLogFuncBeginEndEx _auto_log_begin_end_ex(&gLog, ThisFile, ThisModule, 0, X)
//////////////////////////////////////////////////////////////////////////
