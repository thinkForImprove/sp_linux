#include "LogWrite.h"

//////////////////////////////////////////////////////////////////////////
#ifdef QT_WIN32
static const char *Enter = "\n";
#else
static const char *Enter = "\r\n";
#endif

//////////////////////////////////////////////////////////////////////////
static CAutoRunApp gRunApp;
//////////////////////////////////////////////////////////////////////////
// 导出函数
extern "C" LOGWRITESHARED_EXPORT long CreateILogWrite(ILogWrite *&p)
{
    p = new CLogWrite;
    return 0;
}
//////////////////////////////////////////////////////////////////////////
CLogWrite::CLogWrite(): m_cMemW(SHAREMEMORY_MUTEX)
{
    memset(m_szLogPath, 0x00, sizeof(m_szLogPath));
    memset(m_szLogFile, 0x00, sizeof(m_szLogFile));
    memset(m_szErrFile, 0x00, sizeof(m_szErrFile));
    memset(m_szLogType, 0x00, sizeof(m_szLogType));
    memset(m_szPath, 0x00, sizeof(m_szPath));
    memset(m_szLogBuff, 0x00, sizeof(m_szLogBuff));
    memset(m_szOldLogBuff, 0x00, sizeof(m_szOldLogBuff));

    gRunApp.Start();// 启动检测线程
}

CLogWrite::~CLogWrite()
{
}

void CLogWrite::Release()
{
    //delete this;
}

void CLogWrite::SetLogFile(const char *pFileName, const char *pErrFile, const char *pType /*= nullptr*/)
{
    AutoMutex(m_cMutex);
    strcpy(m_szLogFile, pFileName);
    strcpy(m_szErrFile, pErrFile);
    if (pType != nullptr)
    {
        memset(m_szLogType, 0x00, sizeof(m_szLogType));
        sprintf(m_szLogType, "[ %s ]", pType);
    }

    if (!m_cAppRunning.IsAppRunning(LOGMANAGERNAME, ""))
    {
        m_cAppRunning.RunApp(LOGMANAGERNAME, "");
    }

    // 创建连接
    if (!m_cMemW.IsOpened())
    {
        m_cMemW.Open(SHAREMEMORY_NAME);
    }

    //gRunApp.Start();// 启动检测线程
    return;
}

void CLogWrite::Log(const char *pErrModule, int nErrCode, const char *pFormat, ...)
{
    AutoMutex(m_cMutex);
    // R6002-floating point not loaded 的问题解决方法
    // 在DLL里写一行没实际用途的浮点运算代码: float a = 0.00f，编译器就会检测到需要浮点运算
    float a = 0.00f;
    memset(m_szBuff, 0x00, sizeof(m_szBuff));
    memset(m_szLogBuff, 0x00, sizeof(m_szLogBuff));

    STR_LOG_RECORD stLog;
    stLog.nErrCode = (nErrCode > 1) ? (-nErrCode) : nErrCode;     // 特殊处理，此为记录行号
    strcpy(stLog.szErrFile, m_szErrFile);
    strcpy(stLog.szErrModule, pErrModule);
    stLog.szErrFile[sizeof(stLog.szErrFile) - 1] = 0x00;
    stLog.szErrModule[sizeof(stLog.szErrModule) - 1] = 0x00;
    FomatLog(m_szLogBuff, &stLog);

    //  格式化日志
    va_list vl;
    va_start(vl, pFormat);
    vsprintf(m_szBuff, pFormat, vl);
    va_end(vl);

    if (strlen(m_szLogType) != 0)
        strcat(m_szLogBuff, m_szLogType);
    strcat(m_szLogBuff, m_szBuff);
    strcat(m_szLogBuff, Enter);

    SetLogEndFlag(m_szLogBuff, sizeof(m_szLogBuff));
    SendLog(GetLogFile(m_szLogFile), m_szLogBuff);
    return;
}

void CLogWrite::MLog(const char *pErrModule, int nErrCode, const char *pFormat, ...)
{
    AutoMutex(m_cMutex);
    memset(m_szBuff, 0x00, sizeof(m_szBuff));
    memset(m_szLogBuff, 0x00, sizeof(m_szLogBuff));

    STR_LOG_RECORD stLog;
    stLog.nErrCode = (nErrCode > 1) ? (-nErrCode) : nErrCode;     // 特殊处理，此为记录行号
    strcpy(stLog.szErrFile, m_szErrFile);
    strcpy(stLog.szErrModule, pErrModule);
    stLog.szErrFile[sizeof(stLog.szErrFile) - 1]     = 0x00;
    stLog.szErrModule[sizeof(stLog.szErrModule) - 1] = 0x00;
    FomatLog(m_szLogBuff, &stLog);

    //  格式化日志
    va_list vl;
    va_start(vl, pFormat);
    vsprintf(m_szBuff, pFormat, vl);
    va_end(vl);

    // 判断日志内容是否相同，如果是，则不写文件
    if (IsSameLog(m_szOldLogBuff, m_szBuff, sizeof(m_szBuff)))
        return ;
    strcpy(m_szOldLogBuff, m_szBuff);

    if (strlen(m_szLogType) != 0)
        strcat(m_szLogBuff, m_szLogType);
    strcat(m_szLogBuff, m_szBuff);
    strcat(m_szLogBuff, Enter);

    // 判断是否为相同日志
    SetLogEndFlag(m_szLogBuff, sizeof(m_szLogBuff));
    SendLog(GetLogFile(m_szLogFile), m_szLogBuff);
    return;
}

void CLogWrite::LogHex(const char *pFileName, const char *lpBuff, int nBuffLen)
{
    AutoMutex(m_cMutex);
    if (pFileName == nullptr || lpBuff == nullptr || nBuffLen == 0)
        return;

    int i = 0, g = 0, k = 0;
    unsigned char *abuf = nullptr, *hbuf = nullptr;
    char szTime[128] = { 0 };
    char szBuff[BUFFSIZE] = { 0 };

    GetTimeStr(szTime);
    hbuf = (unsigned char *)lpBuff;
    abuf = (unsigned char *)lpBuff;
    sprintf(szBuff, "%s *** Record HEX Begin ... ***%s", szTime, Enter);
    for (i = 0, g = nBuffLen / 16; i < g; i++)
    {
        sprintf(szBuff + strlen(szBuff), "M(%6.6d)=< ", i * 16);
        for (k = 0; k < 16; k++)
            sprintf(szBuff + strlen(szBuff), "%02x ", *hbuf++);

        sprintf(szBuff + strlen(szBuff), "> ");
        for (k = 0; k < 16; k++, abuf++)
            sprintf(szBuff + strlen(szBuff), "%c", (*abuf > 32) ? ((*abuf < 128) ? *abuf : '*') : '.');

        sprintf(szBuff + strlen(szBuff), "%s", Enter);
    }
    if ((i = nBuffLen % 16) > 0)
    {
        sprintf(szBuff + strlen(szBuff), "M(%6.6d)=< ", nBuffLen - nBuffLen % 16);
        for (k = 0; k < i; k++)
            sprintf(szBuff + strlen(szBuff), "%02x ", *hbuf++);
        for (k = i; k < 16; k++)
            sprintf(szBuff + strlen(szBuff), "   ");

        sprintf(szBuff + strlen(szBuff), "> ");

        for (k = 0; k < i; k++, abuf++)
            sprintf(szBuff + strlen(szBuff), "%c", (*abuf > 32) ? ((*abuf < 128) ? *abuf : '*') : '.');

        sprintf(szBuff + strlen(szBuff), "%s", Enter);
    }
    sprintf(szBuff + strlen(szBuff), "%s *** Record HEX End ... ***%s", szTime, Enter);

    SetLogEndFlag(szBuff, sizeof(szBuff));
    SendLog(GetLogFile(pFileName), szBuff);
    return;
}

void CLogWrite::LogTrace(const char *pFileName, const char *pFormat, ...)
{
    AutoMutex(m_cMutex);
    memset(m_szLogBuff, 0x00, sizeof(m_szLogBuff));

    //  格式化日志
    char szBuff[BUFFSIZE] = { 0 };
    va_list vl;
    va_start(vl, pFormat);
    vsprintf(szBuff, pFormat, vl);
    va_end(vl);

    char szTime[64] = { 0 };
    GetTimeStr(szTime);
    strcpy(m_szLogBuff, szTime);
    strcat(m_szLogBuff, ": ");
    strcat(m_szLogBuff, szBuff);
    strcat(m_szLogBuff, Enter);

    SetLogEndFlag(m_szLogBuff, sizeof(m_szLogBuff));
    SendLog(GetLogFile(pFileName), m_szLogBuff);
    return;
}

void CLogWrite::SendLog(const char *lpFile, const char *lpData)
{
    DWORD dwTimes = 0;
    LOGDATA stLogData;
    strcpy(stLogData.szFile, lpFile);
    strcpy(stLogData.szData, lpData);

    do
    {
        if (m_cMemW.Write(&stLogData, sizeof(LOGDATA)))
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (++dwTimes < 3);
    return;
}

bool CLogWrite::IsSameLog(const char *pOld, const char *pNew, int nLen)
{
    if (memcmp(pOld, pNew, nLen) == 0)
        return true;
    return false;
}

const char *CLogWrite::GetLogFile(const char *pFileName)
{
    QByteArray qstrFile;
    QDate qDate = QDate::currentDate();
    QTime qTime = QTime::currentTime();     // 取当前时间
    QStringList cSplit = QString(pFileName).split(".");
    QByteArray qName(cSplit[0].toLocal8Bit());
    QByteArray qExe(cSplit[1].toLocal8Bit());

    // 使用固定目录
    if (strlen(m_szPath) == 0)
    {
#ifdef QT_WIN32
        strcpy(m_szPath, "D:/LOG");
#else
        strcpy(m_szPath, "/usr/local/LOG");
#endif
    }

    memset(m_szLogPath, 0x00, sizeof(m_szLogPath));
    /*sprintf(m_szLogPath, "%s/%s/%s%4d%02d%02d.%s", m_szPath, qName.constData(),
            qName.constData(), qDate.year(), qDate.month(), qDate.day(), qExe.constData());*/
    // 修改Log文件路径为 LOG根目录/YYYYMMDD/, LOG名后缀为YYYYMMDD.hh
    sprintf(m_szLogPath, "%s/%4d%02d%02d/%s/%s%4d%02d%02d.%02d.%s",
            m_szPath, qDate.year(), qDate.month(), qDate.day(), qName.constData(), qName.constData(),
            qDate.year(), qDate.month(), qDate.day(), qTime.hour(), qExe.constData());
    return m_szLogPath;
}

void CLogWrite::SetLogEndFlag(char *pBuff, int nBuffLen)
{
    if (pBuff[nBuffLen - 1] != 0x00)
    {
#ifdef Q_OS_UNIX
        pBuff[nBuffLen - 3] = '\r';
#endif
        pBuff[nBuffLen - 2] = '\n';
        pBuff[nBuffLen - 1] = 0x00;// 防止数据异常时越界
    }
    return;
}

//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
//
//  记录日志结构（内部使用）
//
///////////////////////////////////////////////////////////////

int CLogWrite::FomatLog(char *pBuf, STR_LOG_RECORD *rec_log)
{
    GetDateTimeStr(rec_log->szTime);
    rec_log->szTime[sizeof(rec_log->szTime) - 1] = 0;
    return sprintf(pBuf, "%s%-10d|%21.21s|%63.63s|%63.63s|",
                   LOG_WRITE_START_STRING,                          /* 记录开始标记 */
                   rec_log->nErrCode,                              /* 错误代码*/
                   rec_log->szTime,                             /* 日期时间*/
                   rec_log->szErrFile,                             /* 错误发生的文件*/
                   rec_log->szErrModule                         /* 错误发生的模块*/
                  );
}
////////////////////////////////////////////////////////////////
//
//      得到日期时间字串（内部使用）
//
////////////////////////////////////////////////////////////////
void CLogWrite::GetDateTimeStr(char *str)
{
    if (str == nullptr)
        return;

    QDateTime qDT = QDateTime::currentDateTime();
    QTime qTime = qDT.time();
    QDate qDate = qDT.date();
    sprintf(str, "%04d%02d%02d %02d:%02d:%02d.%03d",
            qDate.year(),
            qDate.month(),
            qDate.day(),
            qTime.hour(),
            qTime.minute(),
            qTime.second(),
            qTime.msec());
    str[21] = 0x00;
    return;
}
/////////////////////////////////////////////////////
//
//  得到yyyy-mm-dd hh:mm:ss:mm 字串（内部使用）
//
//////////////////////////////////////////////////////
int CLogWrite::GetTimeStr(char *date)
{
    if (date == nullptr)
        return -1;

    QDateTime qDT = QDateTime::currentDateTime();
    QTime qTime = qDT.time();
    QDate qDate = qDT.date();
    sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d.%03d", qDate.year(), qDate.month(), qDate.day(),
            qTime.hour(), qTime.minute(), qTime.second(), qTime.msec());
    return 0;
}


//////////////////////////////////////////////////////////////////////////
void CAutoRunApp::Start()
{
    m_thread = std::thread(&CAutoRunApp::Run, this);
    m_thread.detach();
}

void CAutoRunApp::Run()
{
    while (!m_bQuitRun)
    {
        // 延时，每5秒检测一次
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        // 检测并运行写日志管理程序
        if (!m_cAppRun.IsAppRunning(LOGMANAGERNAME, ""))
        {
            m_cAppRun.RunApp(LOGMANAGERNAME, "");
        }
    }
    return;
}
