#include "StlSimpleThread.h"
#include "ILogWrite.h"
//////////////////////////////////////////////////////////////////////////
static const char *ThisFile = "StlSimpleThread.cpp";
static CLogManage gLog;
//////////////////////////////////////////////////////////////////////////

void CStlOneThread::StartThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = false;
    if (!m_thRun.joinable())
        m_thRun = std::thread(&CStlOneThread::Run, this);
    return;
}

void CStlOneThread::StopThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = true;
    if (m_thRun.joinable())
        m_thRun.join();
    return;
}
//////////////////////////////////////////////////////////////////////////
void CStlTwoThread::StartThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = false;
    if (!m_thRunCmd.joinable())
        m_thRunCmd = std::thread(&CStlTwoThread::Run_Cmd, this);
    if (!m_thRunResult.joinable())
        m_thRunResult = std::thread(&CStlTwoThread::Run_Result, this);
    return;
}

void CStlTwoThread::StopThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = true;
    if (m_thRunCmd.joinable())
        m_thRunCmd.join();
    if (m_thRunResult.joinable())
        m_thRunResult.join();
    return;
}
//////////////////////////////////////////////////////////////////////////
void CStlThreadThread::StartThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = false;
    if (!m_thRunCmd.joinable())
        m_thRunCmd = std::thread(&CStlThreadThread::Run_Cmd, this);
    if (!m_thRunRead.joinable())
        m_thRunRead = std::thread(&CStlThreadThread::Run_Read, this);
    if (!m_thRunResult.joinable())
        m_thRunResult = std::thread(&CStlThreadThread::Run_Result, this);
    return;
}

void CStlThreadThread::StopThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = true;
    if (m_thRunCmd.joinable())
        m_thRunCmd.join();
    if (m_thRunRead.joinable())
        m_thRunRead.join();
    if (m_thRunResult.joinable())
        m_thRunResult.join();
    return;
}
//////////////////////////////////////////////////////////////////////////
void CStlFiveThread::StartThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = false;
    if (!m_thRunCmd.joinable())
        m_thRunCmd = std::thread(&CStlFiveThread::Run_Cmd, this);
    if (!m_thRunGet.joinable())
        m_thRunGet = std::thread(&CStlFiveThread::Run_GetInfo, this);
    if (!m_thRunExe.joinable())
        m_thRunExe = std::thread(&CStlFiveThread::Run_Execute, this);
    if (!m_thRunResult.joinable())
        m_thRunResult = std::thread(&CStlFiveThread::Run_Result, this);
    if (!m_thRunStatus.joinable())
        m_thRunStatus = std::thread(&CStlFiveThread::Run_Status, this);
    return;
}

void CStlFiveThread::StopThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = true;
    if (m_thRunCmd.joinable())
        m_thRunCmd.join();
    if (m_thRunGet.joinable())
        m_thRunGet.join();
    if (m_thRunExe.joinable())
        m_thRunExe.join();
    if (m_thRunResult.joinable())
        m_thRunResult.join();
    if (m_thRunStatus.joinable())
        m_thRunStatus.join();
    return;
}

//////////////////////////////////////////////////////////////////////////
void CStlSixThread::StartThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = false;
    if (!m_thRunCmd.joinable())
        m_thRunCmd = std::thread(&CStlSixThread::Run_Cmd, this);
    if (!m_thRunGet.joinable())
        m_thRunGet = std::thread(&CStlSixThread::Run_GetInfo, this);
    if (!m_thRunExe.joinable())
        m_thRunExe = std::thread(&CStlSixThread::Run_Execute, this);
    if (!m_thRunResult.joinable())
        m_thRunResult = std::thread(&CStlSixThread::Run_Result, this);
    if (!m_thRunStatus.joinable())
        m_thRunStatus = std::thread(&CStlSixThread::Run_Status, this);
    if (!m_thRunRead.joinable())
        m_thRunRead = std::thread(&CStlSixThread::Run_Read, this);
    return;
}

void CStlSixThread::StopThread()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEndEx();

    m_bQuitRun = true;
    // 因为SP中，此线程有等待信号，所以要第一个退出
    if (m_thRunRead.joinable())
        m_thRunRead.join();
    if (m_thRunCmd.joinable())
        m_thRunCmd.join();
    if (m_thRunGet.joinable())
        m_thRunGet.join();
    if (m_thRunExe.joinable())
        m_thRunExe.join();
    if (m_thRunResult.joinable())
        m_thRunResult.join();
    if (m_thRunStatus.joinable())
        m_thRunStatus.join();

    return;
}
