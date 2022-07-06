// LogWriteThread.h: interface for the CLogWriteThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <QSemaphore>
#include <string.h>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <fstream>

#include "INIFileReader.h"
#include "data_convertor.h"
#include "file_access.h"
#include "ILogWrite.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
#define LOGFILE                     "Event.log"
#define AutoMutexThread(X)          std::lock_guard<std::recursive_mutex>   _auto_lock_##X(X)
#define INI_XFSLOG_PATH             "/usr/local/CFES/ETC/XFSLogManager.ini" // Log相关配置文件
#define LOG_ROOT_PATH               "/usr/local/LOG"                        // 缺省Log目录
#define LOG_BACK_PATH               "/usr/local/LOG.bak"                    // 缺省Log备份目录

#define MAX_PATH                    260
#define MAX_BUFF_1024               1024

//////////////////////////////////////////////////////////////////////////
typedef struct tag_log_data_info
{
    char szFile[256];       // 日志文件
    char szData[8192];      // 日志内容
    tag_log_data_info()
    {
        clear();
    }
    void clear()
    {
        memset(this, 0x00, sizeof(tag_log_data_info));
    }
} LOGDATA;
typedef queue<LOGDATA>  queueLOGDATA;
//////////////////////////////////////////////////////////////////////////
class CStlOneThread
{
public:
    CStlOneThread()
    {
        m_bQuitRun = false;
    }
    virtual ~CStlOneThread()
    {
        m_bQuitRun = true;
    }
public:
    virtual void Run() = 0;
public:
    void ThreadStart()
    {
        m_thRun = std::thread(&CStlOneThread::Run, this);
        m_thRun.detach();
    }
    void ThreadStop()
    {
        m_bQuitRun = true;
    }
protected:
    bool m_bQuitRun;
private:
    std::thread m_thRun;
};
//////////////////////////////////////////////////////////////////////////
class CLogWriteThread : public CStlOneThread
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
    std::recursive_mutex    m_cMutex;
};

//////////////////////////////////////////////////////////////////////////
/// \brief CLogBackThread 调用
/// 功能: 用于存储Log备份列表及处理
typedef struct st_Log_Back_List
{
    CHAR    szList[64][MAX_PATH];   // 列表存储
    WORD    wListCnt;               // 列表有效数目
    st_Log_Back_List()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Log_Back_List));
        wListCnt = 0;
    }
    INT AddList(LPSTR lpStr)
    {
        if (lpStr == nullptr || strlen(lpStr) < 1)
        {
            return 0;
        }
        for (INT i = 0; i < wListCnt; i ++)
        {
            if (memcmp(szList[i], lpStr, strlen(lpStr)) == 0 &&
                memcmp(szList[i], lpStr, strlen(szList[i])) == 0)
            {
                return 0;
            }
        }
        if (wListCnt < 64)
        {
            memcpy(szList[wListCnt ++], lpStr, strlen(lpStr));
        }

        return 0;
    }
    INT IsHave(LPSTR lpStr)    // 返回值:0不存在/非0存在
    {
        if (lpStr == nullptr || strlen(lpStr) < 1)
        {
            return 0;
        }
        std::string stdLister;
        stdLister.append(lpStr);
        for (INT i = 0; i < wListCnt; i ++)
        {
            std::string::size_type pos = stdLister.find(szList[i]);

            if (stdLister.find(szList[i]) != std::string::npos)
            {
                return 1;   // 已存在,返回1
            }
        }
        return 0;
    }
} STLOGBACKLIST, *LPSTLOGBACKLIST;

//////////////////////////////////////////////////////////////////////////
/// \brief The CLogBackThread class
/// 功能: 用于log备份
#define LOG_BACK_NAME "LogBackThread.log"
class CLogBackThread : public CStlOneThread, public CLogManage
{
public:
    CLogBackThread();
    virtual ~CLogBackThread();
protected:
    // 日志备份线程
    virtual void Run();
public:
    // 通知线程退出
    void Exit();
private:
    // 备份处理记录
    bool BackupLog(LPCSTR lpLogPath, LPCSTR lpBackPath, LPCSTR lpDate); // 备份处理主接口
    bool DeleteLog(LPCSTR lpLogPath, LPCSTR lpDate, bool bBackDel = true); // 删除处理主接口

private:
    INT CopySource2Dest(LPSTR lpSource, LPSTR lpDest, bool bIsCopyDir = true);// 拷贝目录或文件
    INT DelDirFile(LPCSTR lpDirFile);                       // 删除目录或文件
    int GetIniConfig();                                     // 读配置文件
    int AutoGetIniConfig();                                 // 自动读配置文件
    char* QString2Char(QString qsStr);                      // QString转为char*
    bool ChkIsDate(LPSTR lpStr);                            // 检查字符串是否为YYYYMMDD格式
    bool GetFileCreateDate(LPSTR lpName, LPSTR lpDate);     // 获取文件/目录创建时间
    ULONG GetFileSize(LPSTR lpName);                        // 获取文件大小

private:
    std::recursive_mutex    m_cMutex;

private:    // INI相关
    CINIFileReader          m_clRWIni;
    WORD                    m_wLogBackDelSup;               // 备份删除功能支持
    char                    m_szLogRootDir[MAX_PATH];       // 日志根目录
    char                    m_szLogBackDir[MAX_PATH];       // 日志备份目录
    char                    m_szLogBackDate[8+1];           // 上一次备份日期
    WORD                    m_wLogBackMode;                 // 备份方式
    WORD                    m_wLogDelSource;                // 备份完成后是否删除源目录文件
    WORD                    m_wLogBackSaveDays;             // 备份LOG保存天数
    STLOGBACKLIST           m_stOtherBackList;              // 其他备份目录/文件列表
    STLOGBACKLIST           m_stNotBackList;                // 不需要备份目录/文件列表
};
