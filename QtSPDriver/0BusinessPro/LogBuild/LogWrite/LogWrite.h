#pragma once

#include "ILogWrite.h"
#include <QtCore/qglobal.h>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include "QtShareMemoryRW.h"
#include "QtAppRunning.h"
#include "SimpleMutex.h"

#include <mutex>
#include <thread>
#include <string>
#include <fstream>
#include <future>
using namespace std;

//////////////////////////////////////////////////////////////////////////
#define LOG_WRITE_START_STRING  "[LOGSTART]"
#define SHAREMEMORY_NAME        "ShareMemory_LogWrite"
#define SHAREMEMORY_MUTEX       "Mutex_LogWrite"
#define LOGMANAGERNAME          "LogManager.exe"
//////////////////////////////////////////////////////////////////////////
typedef struct tag_log_data_info
{
    char szFile[256];           // 日志文件
    char szData[BUFFSIZE];      // 日志内容
    tag_log_data_info()
    {
        clear();
    }
    void clear()
    {
        memset(this, 0x00, sizeof(tag_log_data_info));
    }
} LOGDATA;

// 日志文件结构
typedef struct tag_log_record
{
    char  szTime[22];
    char  szErrFile[64];
    char  szErrModule[64];
    int   nErrCode;

    tag_log_record()
    {
        clear();
    }
    void clear()
    {
        memset(this, 0x00, sizeof(tag_log_record));
    }
} STR_LOG_RECORD;

//////////////////////////////////////////////////////////////////////////
class CLogWrite: public ILogWrite
{
public:
    CLogWrite();
    virtual ~CLogWrite();
public:
    // 释放对象
    virtual void Release();
    // 设置：日志文件名和错误发生的文件
    virtual void SetLogFile(const char *pFileName, const char *pErrFile, const char *pType = nullptr);
    // 写日志函数
    virtual void Log(const char *pErrModule, int nErrCode, const char *pFormat, ...);
    // 写日志函数，连续相同内容日志，不重复记录
    virtual void MLog(const char *pErrModule, int nErrCode, const char *pFormat, ...);
    // 写日志函数，写一行日志，自动转换为HEX字符
    virtual void LogHex(const char *pFileName, const char *pBuff, int nBuffLen);
    // 写日志函数，写一行日志
    virtual void LogTrace(const char *pFileName, const char *pFormat, ...);
private:
    // 发送日志
    void SendLog(const char *lFile, const char *lpData);
    // 判断日志是否相同
    bool IsSameLog(const char *pOld, const char *pNew, int nLen);
    // 获取全路径日志文件
    const char *GetLogFile(const char *filename);
    // 设置日志数据结束符
    void SetLogEndFlag(char *pBuff, int nBuffLen);
    // 格式化日志文件
    int  FomatLog(char *pBuf, STR_LOG_RECORD *rec_log);
    // 时间格式
    void GetDateTimeStr(char *str);
    int  GetTimeStr(char *date);

private:
    char                            m_szLogFile[256];        // 日志文件名
    char                            m_szLogPath[256];        // 日志文件名(全路径)
    char                            m_szErrFile[256];        // 错误发生的文件
    char                            m_szLogType[256];        // 日志类型或前缀
    char                            m_szPath[256];           // 日志文件路径
    char                            m_szBuff[BUFFSIZE];      // 日志内容
    char                            m_szLogBuff[BUFFSIZE];   // 日志内容
    char                            m_szOldLogBuff[BUFFSIZE];// 旧日志内容

    CSimpleMutex                    m_cMutex;
    CQtShareMemoryRW                m_cMemW;
    CQtAppRunning                   m_cAppRunning;
};

//////////////////////////////////////////////////////////////////////////
class CAutoRunApp
{
public:
    CAutoRunApp(): m_bQuitRun(false) {}
    ~CAutoRunApp()
    {
        m_bQuitRun = true;
    }
public:
    // 启动线程
    void Start();
    // 执行线程
    void Run();
private:
    bool m_bQuitRun;
    std::thread m_thread;
    CQtAppRunning m_cAppRun;
};

