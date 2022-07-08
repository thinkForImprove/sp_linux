#include "QtAppRunning.h"
#include <string>
#include <thread>
#include <QDateTime>
#include <QList>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDirIterator>

///////////////////////////////////////////////////////////////////
CQtAppRunning::CQtAppRunning()
{

}

CQtAppRunning::~CQtAppRunning()
{
    Release();
}

void CQtAppRunning::Release()
{
    if (m_cMemSingle.isAttached())
        m_cMemSingle.detach();
    if (m_cMemRunFlag.isAttached())
        m_cMemRunFlag.detach();
    if (m_cMemRunning.isAttached())
        m_cMemRunning.detach();
}

bool CQtAppRunning::IsSingleAppRunning(LPCSTR lpAppName)
{
    AutoMutex(m_cMutex);
    /*
     * 每个App打开的时候，获取一次共享内存。
     * 如果获取失败，说明是第一个启动的APP，直接创建共享内存就好了。
     * 如果获取成功，说明不是第一个，直接退出就好了。
     * 保证App在系统里只能打开一个。
     * 信号量的意义，把操作共享内存的代码锁住。因为有可能同时点击2次APP, 防止并发。
     */
    QByteArray strSemSingleKey = lpAppName + QByteArray("_SemKey");
    QSystemSemaphore cSemSingle(strSemSingleKey, 1, QSystemSemaphore::Open);
    cSemSingle.acquire();

    bool bRunning = true;
    m_strMemSingleKey = lpAppName;
    m_strMemSingleKey.push_back(lpAppName);   // 防止和进程名冲突

#ifdef QT_LINUX
    /* Windows平台上不存在应用程序崩溃后，共享内存段还存在的情况
     * LINUX应用程序崩溃后,共享内存段不会自动销毁,则该程序再次运行会出问题
     * 所以程序启动时先去检查是否有程序崩溃后还存留的共享内存段，如果有，先销毁,再创建
     */
    {
        QSharedMemory cTmpMem(m_strMemSingleKey);
        if (cTmpMem.attach())
            cTmpMem.detach();
    }
#endif
    m_cMemSingle.setKey(m_strMemSingleKey);
    if (!m_cMemSingle.attach())
    {
        bRunning = false;
        m_cMemSingle.create(16);
    }
    else
    {
        m_cMemSingle.detach();
    }

    cSemSingle.release();
    return bRunning;
}

bool CQtAppRunning::CreateRunnningFlag(LPCSTR lpName, LPCSTR lpPID)
{
    AutoMutex(m_cMutex);
    m_strName = lpName;
    m_strPID  = lpPID;
    m_strMemRunFlagKey = lpName;
    if (m_strPID.isNull() || m_strPID.isEmpty())
        m_strMemRunFlagKey.push_back(lpName);   // 防止和进程名冲突
    else
        m_strMemRunFlagKey.push_back(lpPID);
#ifdef QT_LINUX
    {
        //LINUX应用程序崩溃后,共享内存段不会自动销毁
        QSharedMemory cTmpMem(m_strMemRunFlagKey);
        if (cTmpMem.attach())
            cTmpMem.detach();
    }
#endif
    m_cMemRunFlag.setKey(m_strMemRunFlagKey);
    if (m_cMemRunFlag.attach())
        return true;
    if (!m_cMemRunFlag.create(16))
        return false;
    return true;
}

bool CQtAppRunning::CreateRunnningFlag(LPCSTR lpName, DWORD dwPID)
{
    AutoMutex(m_cMutex);
    m_strName = lpName;
    m_strPID  = dwPID > 0 ? std::to_string(dwPID).c_str() : "";
    return CreateRunnningFlag(m_strName.data(), m_strPID.data());
}

bool CQtAppRunning::IsAppRunning(LPCSTR lpName, LPCSTR lpPID)
{
    AutoMutex(m_cMutex);
    m_strName = lpName;
    m_strPID  = lpPID;

    m_strMemKey = lpName;
    if (m_strPID.isNull() || m_strPID.isEmpty())
        m_strMemKey.push_back(lpName);   // 防止和进程名冲突
    else
        m_strMemKey.push_back(lpPID);
#ifdef QT_LINUX
    {
        //LINUX应用程序崩溃后,共享内存段不会自动销毁
        QSharedMemory cTmpMem(m_strMemKey);
        if (cTmpMem.attach())
            cTmpMem.detach();
    }
#endif
    m_cMemRunning.setKey(m_strMemKey);
    if (!m_cMemRunning.attach())
        return false;
    m_cMemRunning.detach();
    return true;
}

bool CQtAppRunning::IsAppRunning(LPCSTR lpName, DWORD dwPID)
{
    AutoMutex(m_cMutex);
    m_strName = lpName;
    m_strPID  = dwPID > 0 ? std::to_string(dwPID).c_str() : "";
    return IsAppRunning(m_strName.data(), m_strPID.data());
}

bool CQtAppRunning::IsAppRunning(LPCSTR lpName)
{
    AutoMutex(m_cMutex);
    QString strName(lpName);
    QProcess tkList;

#ifdef QT_WIN32
    tkList.start("tasklist", QStringList() << "/FI" << "imagename eq " + strName);
#else
    if (strName.lastIndexOf('.') != -1)
        strName = strName.left(strName.lastIndexOf('.'));
    tkList.start("sh", QStringList() << "-c" << "ps | grep " + strName);
#endif

    tkList.waitForFinished();
    QString strOutput = QString::fromLocal8Bit(tkList.readAllStandardOutput());
    return strOutput.contains(strName);
}

bool CQtAppRunning::RunApp(LPCSTR lpName, LPCSTR lpPath, LPCSTR lpCmdLine/* = nullptr*/, long lTimeOut/* = 3000*/)
{
    AutoMutex(m_cMutex);
    QString strFile;
    QString strName(lpName);
    QString strCmdLine("command");
    if (lpCmdLine != nullptr)
        strCmdLine = QString::fromLocal8Bit(lpCmdLine);

#ifdef QT_WIN32
    if (lpPath == nullptr || strlen(lpPath) == 0)
        strFile = DEFPATH_WIN + strName;
    else
        strFile = lpPath + strName;
#else
    if (strName.lastIndexOf('.') != -1)
        strName = strName.left(strName.lastIndexOf('.'));
    if (lpPath == nullptr || strlen(lpPath) == 0)
        strFile = DEFPATH_LINUX + strName;
    else
        strFile = lpPath + strName;
#endif

    QProcess::startDetached(strFile, QStringList() << strCmdLine);

    QTime qTime;
    qTime.start();
    while (true)
    {
        StlSleep(10);
        if (IsAppRunning(strName.toLocal8Bit(), ""))
        {
            StlSleep(200);   // 延时一下，让进程完全启动
            return true;
        }
        if (qTime.elapsed() >= lTimeOut)   // 最长等待时间：默认3秒
            break;
    }
    return false;
}

bool CQtAppRunning::KillApp(LPCSTR lpName)
{
    AutoMutex(m_cMutex);
    QString strName(lpName);
#ifdef QT_WIN32
    QString strKill = "taskkill /im " + strName + " /f";
#else
    if (strName.lastIndexOf('.') != -1)
        strName = strName.left(strName.lastIndexOf('.'));
    QString strKill = "pkill -９ " + strName;
#endif

    QProcess tkList;
    tkList.execute(strKill);
    tkList.close();
    return true;
}

void CQtAppRunning::StlSleep(DWORD dwMilliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(dwMilliseconds));
}

///////////////////////////////////////////////////////////////////
QByteArray CQtAppRunning::FindPID(QByteArray strName)
{
    QByteArray strPID;
#ifdef QT_WIN32
#else
    QString strLine;
    QString strFile;
    QFile cFile;
    //    QDir cDir("/proc");
    //    QFileInfoList vtFile = cDir.entryInfoList();// 因使用此遍历目录，会导致日立机芯驱动异常，暂时不知道原因
    //    for (auto it = vtFile.begin(); it != vtFile.end(); it++)
    //    {
    //        if (!it->isDir())
    //            continue;
    //        if (!IsDigit(it->fileName().toLocal8Bit()))
    //            continue;

    //        strFile.sprintf("/proc/%s/status", it->fileName().toLocal8Bit().data());
    //        cFile.setFileName(strFile);
    //        if (!cFile.exists())
    //            continue;
    //        cFile.open(QFile::ReadOnly);
    //        if (!cFile.isOpen())
    //            continue;
    //        strLine = cFile.readLine().data();
    //        if (strLine.indexOf(strName) != -1)
    //        {
    //            strPID = it->fileName().toLocal8Bit();
    //            cFile.close();
    //            break;
    //        }
    //        cFile.close();
    //    }
    // 设置过滤参数，QDir::NoDotAndDotDot表示不会去遍历上层目录
    QFileInfo cFlInfo;
    QDirIterator cDirIt("/proc", QDir::Dirs | QDir::NoDotAndDotDot);
    do
    {
        cDirIt.next();
        cFlInfo = cDirIt.fileInfo();
        if (!cFlInfo.isDir())
            continue;
        if (!IsDigit(cFlInfo.fileName().toLocal8Bit()))
            continue;

        strFile.sprintf("/proc/%s/status", cFlInfo.fileName().toLocal8Bit().data());
        cFile.setFileName(strFile);
        if (!cFile.exists())
            continue;
        cFile.open(QFile::ReadOnly);
        if (!cFile.isOpen())
            continue;
        strLine = cFile.readLine().data();
        if (strLine.indexOf(strName) != -1)
        {
            strPID = cFlInfo.fileName().toLocal8Bit();
            cFile.close();
            break;
        }
        cFile.close();
    } while (cDirIt.hasNext());
#endif
    return strPID;
}

bool CQtAppRunning::IsSameNamePID(QByteArray strName, QByteArray strPID)
{
#ifdef QT_WIN32
#else
    QString strLine;
    QString strFile;
    QFile cFile;
    strFile.sprintf("/proc/%s/status", strPID.data());
    cFile.setFileName(strFile);
    if (!cFile.exists())
        return false;
    cFile.open(QFile::ReadOnly);
    if (!cFile.isOpen())
        return false;
    strLine = cFile.readLine().data();
    cFile.close();
    if (strLine.indexOf(strName) == -1)
        return false;
#endif
    return true;
}

bool CQtAppRunning::IsDigit(const QByteArray &strBuff)
{
    for (auto &it : strBuff)
    {
        if (!(it >= '0' && it <= '9'))
            return false;
    }
    return true;
}
