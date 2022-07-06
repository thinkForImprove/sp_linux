#pragma once
//////////////////////////////////////////////////////////////////////////
#include <QDateTime>
#include <QDir>
#include <string>
#include <fstream>
using namespace std;
//////////////////////////////////////////////////////////////////////////
class CAutoBeginEndTimeLog
{
public:
    CAutoBeginEndTimeLog(std::string strName) : m_strName(strName)
    {
        char szTime[256] = { 0 };
        GetDateTimeStr(szTime, m_qStartTime);
        sprintf(szTime + strlen(szTime), "   %s->Begin\n", m_strName.c_str());
        LogToFile(GetLogFile(), szTime);
    }
    ~CAutoBeginEndTimeLog()
    {
        char szTime[256] = { 0 };
        GetDateTimeStr(szTime, m_qStopTime);
        int nElapsed = m_qStartTime.msecsTo(m_qStopTime);
        sprintf(szTime + strlen(szTime), "   %s->End, [ Elapsed= %d ms ] \n", m_strName.c_str(), nElapsed);
        LogToFile(GetLogFile(), szTime);
    }
    void GetDateTimeStr(char *str, QTime &qTime)
    {
        if (str == nullptr)
            return;

        QDateTime qDT = QDateTime::currentDateTime();
        QDate qDate = qDT.date();
        qTime = qDT.time();
        sprintf(str, "%04d%02d%02d%02d%02d%02d.%03d",
                qDate.year(),
                qDate.month(),
                qDate.day(),
                qTime.hour(),
                qTime.minute(),
                qTime.second(),
                qTime.msec());
        return;
    }
    std::string &GetLogFile()
    {
        char szFile[256] = { 0 };
        QDate qDate = QDate::currentDate();
        sprintf(szFile, "D:/LOG/Event/%s_%04d%02d%02d.log", "AutoBeginEndTimeLog",
                qDate.year(), qDate.month(), qDate.day());
        m_strFile = szFile;
        return m_strFile;
    }
    bool LogToFile(string strFile, string strLog)
    {
        // 创建目录
        NewDir(strFile);

        // 打开文件
        fstream cFile(strFile.c_str(), ios::out | ios::app);
        if (!cFile.is_open())
            return false;

        // 写文件
        cFile.write(strLog.c_str(), strLog.length());
        cFile.close();
        return true;
    }

    bool NewDir(string strPath)
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
private:
    QTime       m_qStartTime;
    QTime       m_qStopTime;
    std::string m_strTime;
    std::string m_strName;
    std::string m_strFile;
};
