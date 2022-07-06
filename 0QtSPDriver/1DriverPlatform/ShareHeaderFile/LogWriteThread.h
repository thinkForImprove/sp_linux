// LogWriteThread.h: interface for the CLogWriteThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "SimpleMutex.h"
#include <string.h>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <fstream>
using namespace std;

//////////////////////////////////////////////////////////////////////////
typedef struct tag_log_data_info
{
    char szFile[256];       // 日志文件
    char szData[8192];      // 日志内容
    tag_log_data_info() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_log_data_info)); }
} LOGDATA;
typedef queue<LOGDATA>  queueLOGDATA;
//////////////////////////////////////////////////////////////////////////
class CStlOneThreadEx
{
public:
    CStlOneThreadEx() { m_bQuitRun = false; }
    virtual ~CStlOneThreadEx() { m_bQuitRun = true; }
public:
    virtual void Run() = 0;
public:
    void ThreadStart()
    {
        m_thRun = std::thread(&CStlOneThreadEx::Run, this);
        m_thRun.detach();
    }
    void ThreadStop() { m_bQuitRun = true; }
protected:
    bool m_bQuitRun;
private:
    std::thread m_thRun;
};
//////////////////////////////////////////////////////////////////////////
class CLogWriteThread : public CStlOneThreadEx
{
public:
    CLogWriteThread();
    virtual ~CLogWriteThread();
protected:
    // 写日志线程
    virtual void Run();
public:
    // 通知线程退出
    void Exit();
    // 增加一条日志
    void AddLog(const LOGDATA &stLog);
    // 检测是否还是没写日志
    bool IsHasLog();
private:
    // 读取一条日志记录
    bool PeekALog(LOGDATA &stLog);
    // 写一条日志记录，返回TRUE，成功
    bool LogToFile(const LOGDATA &stData);
    // 创建目录
    bool NewDir(std::string strPath);

private:
    queueLOGDATA            m_queueLog;         //日志队列
    CSimpleMutex            m_cMutex;
};
