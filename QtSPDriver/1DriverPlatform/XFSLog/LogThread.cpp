#include "LogThread.h"

//////////////////////////////////////////////////////////////////////
//      CLogThread
//////////////////////////////////////////////////////////////////////

CLogThread::CLogThread(const char *pMutexName)
    : m_LogToFileMutex(pMutexName)
{
    m_threadId = 0;
    sem_init(&m_semExit, 0, 0);
    sem_init(&m_semHasData, 0, 0);
    sem_init(&m_semFinish, 0, 0);
    int nRet = pthread_create(&m_threadId, NULL, ThreadProc, this);
    if (nRet != 0)
    {
    }
    pthread_detach(m_threadId);
}

CLogThread::~CLogThread()
{
    sem_destroy(&m_semExit);
    sem_destroy(&m_semHasData);
    sem_destroy(&m_semFinish);
}

void CLogThread::NotifyExitAndWait()
{
    sem_post(&m_semExit);
    if (0 != m_threadId)
    {
        sem_wait(&m_semFinish);
        m_threadId = 0;
    }
}

void CLogThread::AddLog(const char *pszFilePath, const char *pStr)
{
    LOG_DATA *pData;
    try
    {
        pData = (LOG_DATA *)new char[sizeof(LOG_DATA) + strlen(pStr)];
        strcpy(pData->szFilePath, pszFilePath);
        strcpy(pData->szData, pStr);
    }
    catch (...)
    {
        return;
    }

    QMutexLocker qAutoLock(&m_LogQueueMutex);

    m_slContent.push_back(pData);
    sem_post(&m_semHasData);
}

CLogThread::LOG_DATA *CLogThread::PeekALog()
{
    //线程内互斥锁
    QMutexLocker qAutoLock(&m_LogQueueMutex);
    if (m_slContent.size() == 0)
    {
        return NULL;
    }

    LOG_DATA *pData = m_slContent.front();
    m_slContent.pop_front();
    if (m_slContent.size() == 0)
    {
        while (0 == sem_trywait(&m_semHasData));
    }

    return pData;
}

void CLogThread::ReadAndLog()
{
    LOG_DATA *pData = PeekALog();
    if (pData == NULL)
        return;

    {
        //进程间互斥锁
        CAutoMutex __auto_unlock(&m_LogToFileMutex);
        try
        {
            bool bRet = OnBeforeLog(pData->szFilePath, pData->szData);
            if (bRet)
            {
                bRet = LogToFile(pData->szFilePath, pData->szData);
                OnAfterLog(pData->szFilePath, pData->szData, bRet);
            }
        }
        catch (...)
        {
        }
    }

    delete [](char *)pData;
}

void CLogThread::InnerThreadProc()
{
    while (true)
    {
        //没有数据且退出信号被激活
        int nRet = sem_trywait(&m_semHasData);
        if (0 != nRet)
        {
            nRet = sem_trywait(&m_semExit);
            if (0 == nRet)
            {
                return;
            }
            //读数据失败
            usleep(1000 * 10);
            continue;
        }
        ReadAndLog();
    }
}

inline bool Mymkdir(QString strDirPath)
{
    if (strDirPath.isEmpty())
        return false;
    //    FILE* pOutStream = NULL;
    //    QDir dir(strDirPath);
    //    QString qstrCmd = "mkdir";
    //    qstrCmd = qstrCmd + " " + strDirPath;

    //    pOutStream = popen(qPrintable(qstrCmd), "r");
    //    pclose(pOutStream);
    //    return dir.exists();

    QDir qDir;
    QString qstrPath = QString::fromLocal8Bit(strDirPath.toStdString().c_str());
    if (qDir.exists(qstrPath))
        return true;
    if (!qDir.mkpath(qstrPath))
        return false;
    return true;

}

bool CLogThread::LogToFile(const char *pszFilePath, const char *pStr)
{
    //打开文件
    FILE *pFile = NULL;
    pFile = fopen(pszFilePath, "at");   //追加打开一个文本文件，并在文件末尾写数据
    if (NULL == pFile)
        return false;
    int nRet = fwrite(pStr, strlen(pStr), 1, pFile);
    if (0 > nRet)
    {
        fclose(pFile);
        return false;
    }
    fclose(pFile);
    return true;
}

bool CLogThread::OnBeforeLog(const char *pszFilePath, const char *pStr)
{
    char szLogDir[MAX_PATH];
    strcpy(szLogDir, pszFilePath);
    char *pLastSep = strrchr(szLogDir, '/');
    assert(pLastSep != NULL);
    *pLastSep = 0;
    return CreateDirIfNotExisting(szLogDir);
}

bool CLogThread::CreateDirIfNotExisting(const char *pDirName)
{
    QDir qDir(pDirName);
    if (!qDir.exists())
    {
        QString strDirPath = QString("%1").arg(pDirName);
        if (strDirPath.at(strDirPath.length() - 1) != '/')
        {
            strDirPath.append("/");
        }
        QStringList qDirPathList;
        QString strTemp;
        bool bSuccess = false;
        bool bFirst = true;
        int i;
        for (i = 0; i < strDirPath.length(); ++i)
        {
            if (strDirPath.at(i) == '/' && !bFirst)
            {
                qDirPathList.append(strTemp);
            }
            strTemp.append(strDirPath.at(i));
            bFirst = false;
        }
        for (i = 0; i < qDirPathList.size(); ++i)
        {
            bSuccess = Mymkdir(qDirPathList.at(i));
        }
        return bSuccess;

    }
    return true;
}
