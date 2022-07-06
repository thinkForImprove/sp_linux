#include "DevPortLogFile.h"
//////////////////////////////////////////////////////////////////////////
CDevPortLogFile::CDevPortLogFile(LPCSTR lpDevName)
{
    m_cLog.ThreadStart();
    memset(m_szLogFile, 0x00, sizeof(m_szLogFile));
    memset(m_szDeviceName, 0x00, sizeof(m_szDeviceName));
    memset(m_szActionName, 0x00, sizeof(m_szActionName));
    memset(&m_stLastWrite, 0x00, sizeof(m_stLastWrite));

    strcpy(m_szDeviceName, lpDevName);
    FmtLogFileName();
}

CDevPortLogFile::~CDevPortLogFile()
{
    EndLine();
}

void CDevPortLogFile::LogAction(LPCSTR lpActionName)
{
    EndLine();
    FmtLogFileName();
    if (lpActionName == nullptr)
        return;

    strcpy(m_szActionName, lpActionName);

    char szTime[64] = { 0 };
    GetLogTimeStr(szTime);
    m_cLogData.clear();
    strcpy(m_cLogData.szFile, m_szLogFile);
    sprintf(m_cLogData.szData, "\n\n%s  %s:", szTime, m_szActionName);
    return;
}

void CDevPortLogFile::FlushLog()
{
    EndLine();
    memset(m_szActionName, 0x00, sizeof(m_szActionName));
}

CDevPortLogFile &CDevPortLogFile::NewLine()
{
    if (m_szDeviceName[0] == '\0' || m_szLogFile[0] == '\0' || m_szActionName[0] == '\0')
        return *this;

    strcpy(m_cLogData.szFile, m_szLogFile);

    char szTime[64] = { 0 };
    GetShortLogTimeStr(szTime);
    RawLog("\n");
    RawLog(szTime);
    RawLog("  ");
    return *this;
}

CDevPortLogFile &CDevPortLogFile::EndLine()
{
    if (m_szDeviceName[0] == '\0' || m_szLogFile[0] == '\0' || m_szActionName[0] == '\0')
        return *this;

    if (strlen(m_cLogData.szFile) > 0 &&
        strlen(m_cLogData.szData) > 0)
    {
        // 补全为8倍数
        int nDataLen = (int)strlen(m_cLogData.szData);
        int nAddLen = nDataLen % 8;
        if (nAddLen != 0)
        {
            memcpy(m_cLogData.szData + nDataLen, "        ", 8 - nAddLen);
            nDataLen += (8 - nAddLen);
            nAddLen = nDataLen % 8;
        }
        m_cLog.AddLog(m_cLogData);
    }

    m_cLogData.clear();
    return *this;
}

CDevPortLogFile &CDevPortLogFile::Log(LPCSTR lpData)
{
    if (lpData == nullptr)
        return *this;
    if (m_szDeviceName[0] == '\0' || m_szLogFile[0] == '\0' || m_szActionName[0] == '\0')
        return *this;

    RawLog(lpData);
    return *this;
}

CDevPortLogFile &CDevPortLogFile::LogHex(LPCSTR lpData, DWORD dwDataLen)
{
    if (lpData == nullptr)
        return *this;
    if (m_szDeviceName[0] == '\0' || m_szLogFile[0] == '\0' || m_szActionName[0] == '\0')
        return *this;

    char szBuf[32] = {0};
    for (DWORD i = 0; i < dwDataLen; i++)
    {
        memset(szBuf, 0x00, sizeof(szBuf));
        sprintf(szBuf, "%02.02X ", (BYTE)(lpData[i]));
        RawLog(szBuf);
    }

    return *this;
}

void CDevPortLogFile::CancelLog()
{
    memset(m_szActionName, 0x00, sizeof(m_szActionName));
    m_cLogData.clear();
}

void CDevPortLogFile::RawLog(LPCSTR lpLog)
{
    if (lpLog == nullptr)
        return;
    int nLen = strlen(m_cLogData.szData) ;
    if (nLen + strlen(lpLog) >= 8192)
        return;

    sprintf(m_cLogData.szData + nLen, "%s", lpLog);
    return;
}

void CDevPortLogFile::FmtLogFileName()
{
    if (m_szDeviceName[0] == 0x00)              //设备名为空，不记日志
        return;

    SYSTEMTIME tCur;
    CQtTime::GetLocalTime(tCur);
    if (tCur.wYear == m_stLastWrite.wYear &&
        tCur.wMonth == m_stLastWrite.wMonth &&
        tCur.wDay == m_stLastWrite.wDay)        //日期未变，用老的文件名
    {
        return;
    }
    memcpy(&m_stLastWrite, &tCur, sizeof(tCur));

#ifdef Q_OS_WIN
    const char *pLogPath = "D:/LOG";
#else
    string strLogRootDir;

    string strIniFilePath = SPETCPATH;
    strIniFilePath += "/XFSLogManager.ini";
    CINIFileReader cINIFileReader;
    cINIFileReader.LoadINIFile(strIniFilePath);
    CINIReader cIR = cINIFileReader.GetReaderSection("LOG_BACK_CFG");
    strLogRootDir = (LPCSTR)cIR.GetValue("LogRootDir", LOGPATH);
    if(strLogRootDir.empty()){
        strLogRootDir = LOGPATH;
    }
#endif

    /*sprintf(m_szLogFile, "%s/DeviceLog/%s/%s_%04d%02d%02d.log",
            pLogPath,
            m_szDeviceName, m_szDeviceName, tCur.wYear, tCur.wMonth, tCur.wDay);*/
    sprintf(m_szLogFile, "%s/%04d%02d%02d/DeviceLog/%s/%s_%04d%02d%02d.%02d.log",
            strLogRootDir.c_str(), tCur.wYear, tCur.wMonth, tCur.wDay,
            m_szDeviceName, m_szDeviceName, tCur.wYear, tCur.wMonth, tCur.wDay, tCur.wHour);
    return;
}

void CDevPortLogFile::GetLogTimeStr(char *pTime)
{
    SYSTEMTIME t;
    CQtTime::GetLocalTime(t);
    sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
            t.wYear, t.wMonth, t.wDay,
            t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
}

void CDevPortLogFile::GetShortLogTimeStr(char *pTime)
{
    SYSTEMTIME t;
    CQtTime::GetLocalTime(t);
    sprintf(pTime, "%02d:%02d:%02d:%03d", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
}
