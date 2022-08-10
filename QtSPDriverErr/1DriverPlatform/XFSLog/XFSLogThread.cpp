#include "XFSLogThread.h"
#include "INIFileReader.h"

CXFSLogThread::CXFSLogThread() : CLogThread("CFES_XFS_LOG_MUTEX")
{
    CINIFileReader configfile(INIPATH);
    CINIReader     cINI = configfile.GetReaderSection("default");
    strcpy(m_szLogRootDir, cINI.GetValue("log_spi_path", SPILOGPATH));
    CreateDirIfNotExisting(m_szLogRootDir);
    int nMaxCount = ReadMaxBackupCount();
    m_LogBackupManager.SetBackupInfo(m_szLogRootDir, "", nMaxCount);
}

CXFSLogThread::~CXFSLogThread()
{
    NotifyExitAndWait();
}

//从配置文件中读最大备份个数
int CXFSLogThread::ReadMaxBackupCount()
{
    CINIFileReader configfile(INIPATH);
    CINIReader     cINI            = configfile.GetReaderSection("default");
    int            nMaxBackupCount = (WORD)cINI.GetValue("log_dir_max_count", DEF_BACKUP_COUNT);
    return nMaxBackupCount;
}

void CXFSLogThread::Log(const char *pStr)
{
    char szPath[MAX_PATH];
    GenerateCurLogPath(szPath);
    CLogThread::AddLog(szPath, pStr);
}

bool CXFSLogThread::OnBeforeLog(const char *pszFilePath, const char *pStr)
{
    m_LogBackupManager.DeleteBackups();

    return CLogThread::OnBeforeLog(pszFilePath, pStr);
}

void CXFSLogThread::GenerateCurLogDirName(char szPath[MAX_PATH])
{
    time_t t     = time(NULL);
    tm    *pTime = localtime(&t);

    strcpy(szPath, m_szLogRootDir);
    int nLen = strlen(m_szLogRootDir);
    //strftime(szPath + nLen, MAX_PATH - nLen, "/%Y-%m-%d", pTime);
    strftime(szPath + nLen, MAX_PATH - nLen, "/%04Y%02m%02d/SPI", pTime);
}

void CXFSLogThread::GenerateCurLogPath(char szPath[MAX_PATH])
{
    GenerateCurLogDirName(szPath);

    time_t t     = time(NULL);
    tm    *pTime = localtime(&t);
    int    nLen  = strlen(szPath);
    strftime(szPath + nLen, MAX_PATH - nLen, "/SPI_%Y%m%d-%H.txt", pTime);
}
