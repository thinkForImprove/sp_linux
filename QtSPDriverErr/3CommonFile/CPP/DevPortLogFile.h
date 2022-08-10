#pragma once
#include "QtTypeDef.h"
#include "LogWriteThread.h"
#include "AutoQtHelpClass.h"

//////////////////////////////////////////////////////////////////////////
class CDevPortLogFile
{
public:
    CDevPortLogFile(LPCSTR lpDevName);
    virtual ~CDevPortLogFile();
public:
    // 日志类型
    void LogAction(LPCSTR lpActionName);
    // 更新并清缓存
    void FlushLog();
    // 记录日志，注意最大缓存为8192
    CDevPortLogFile &NewLine();
    CDevPortLogFile &EndLine();
    CDevPortLogFile &operator<<(LPCSTR lpStr) { return Log(lpStr); }
    CDevPortLogFile &Log(LPCSTR lpData);
    CDevPortLogFile &LogHex(LPCSTR lpData, DWORD dwDataLen);
    // 取消当前日志
    void CancelLog();
protected:
    // 加内容
    void RawLog(LPCSTR lpLog);
    // 格式化文件名
    void FmtLogFileName();
    // 格式化时间
    void GetLogTimeStr(char *pTime);
    void GetShortLogTimeStr(char *pTime);
private:
    SYSTEMTIME          m_stLastWrite;
    char                m_szDeviceName[MAX_PATH];
    char                m_szLogFile[MAX_PATH];
    char                m_szActionName[MAX_PATH];
    LOGDATA             m_cLogData;
    CLogWriteThread     m_cLog;
};
//////////////////////////////////////////////////////////////////////////
