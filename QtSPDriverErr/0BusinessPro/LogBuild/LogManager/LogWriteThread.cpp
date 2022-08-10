// LogWriteThread.cpp: implementation of the CLogWriteThread class.
//
//////////////////////////////////////////////////////////////////////
#include "LogWriteThread.h"
#include <QDir>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////
CLogWriteThread::CLogWriteThread()
{

}

CLogWriteThread::~CLogWriteThread()
{
    Exit();
}

void CLogWriteThread::Exit()
{
    ThreadStop();
}

void CLogWriteThread::Run()
{
    ulong lLogCount = 0;
    LOGDATA stLogData;
    while (true)
    {
        // 延时，防止占用过高CPU
        if (!IsHasLog() || lLogCount >= 1000)
        {
            lLogCount = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // 读取一条日志
        if (!PeekALog(stLogData))
        {
            // 判断是否退出
            if (m_bQuitRun)
                break;
            continue;
        }

        // 写一条日志
        lLogCount++;
        if (!LogToFile(stLogData))
        {
            long lCurErrTimes = 0;
            long lMaxErrTimes = 10;// 最大尝试次数
            while (++lCurErrTimes < lMaxErrTimes)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                if (LogToFile(stLogData))
                    break;
            }
        }
    }
    return;
}

void CLogWriteThread::AddLog(const LOGDATA &stLog)
{
    AutoMutexThread(m_cMutex);
    m_queueLog.push(stLog);
    return;
}

bool CLogWriteThread::IsHasLog()
{
    AutoMutexThread(m_cMutex);
    if (m_queueLog.empty())
        return false;
    return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLogWriteThread::PeekALog(LOGDATA &stLog)
{
    AutoMutexThread(m_cMutex);
    stLog.clear();
    if (m_queueLog.empty())
        return false;

    memcpy(&stLog, &m_queueLog.front(), sizeof(LOGDATA));
    m_queueLog.pop();
    if (m_queueLog.empty())
    {
        queueLOGDATA quTmp;
        m_queueLog.swap(quTmp);
    }
    return true;
}

bool CLogWriteThread::LogToFile(const LOGDATA &stLog)
{
    // 创建目录
    NewDir(stLog.szFile);

    // 打开文件
    fstream cFile(stLog.szFile, ios::out | ios::app);
    if (!cFile.is_open())
        return false;

    // 写文件
    size_t uLen  = strlen(stLog.szData);
    size_t uSize = sizeof(stLog.szData);
    cFile.write(stLog.szData, qMin(uLen, uSize));
    cFile.close();
    return true;
}

bool CLogWriteThread::NewDir(string strPath)
{
    if (strPath.empty())
        return false;
    // 删除文件名，只保留目录名
    if (string::npos != strPath.find('.'))
        strPath.erase(strPath.rfind('/') + 1, string::npos);

    // 创建目录
    QDir qDir;
    QString qstrPath = QString::fromLocal8Bit(strPath.c_str());
    if (qDir.exists(qstrPath))
        return true;
    if (!qDir.mkpath(qstrPath))
        return false;
    return true;
}
