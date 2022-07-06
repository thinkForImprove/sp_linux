#pragma once

#include <thread>
#include <mutex>
#include <algorithm>
using namespace std;
//////////////////////////////////////////////////////////////////////////
class CStlOneThread
{
public:
    CStlOneThread() { m_bQuitRun = false; }
    virtual ~CStlOneThread() { m_bQuitRun = true; }
public:
    virtual void Run() = 0;
protected:
    void StartThread();
    void StopThread();
protected:
    bool m_bQuitRun;
private:
    std::thread m_thRun;
};
//////////////////////////////////////////////////////////////////////////
class CStlTwoThread
{
public:
    CStlTwoThread() { m_bQuitRun = false; }
    virtual ~CStlTwoThread() { m_bQuitRun = true; }
public:
    virtual void Run_Cmd() = 0;
    virtual void Run_Result() = 0;
protected:
    void StartThread();
    void StopThread();
protected:
    bool m_bQuitRun;
private:
    std::thread m_thRunCmd;
    std::thread m_thRunResult;
};
//////////////////////////////////////////////////////////////////////////
class CStlThreadThread
{
public:
    CStlThreadThread() { m_bQuitRun = false; }
    virtual ~CStlThreadThread() { m_bQuitRun = true; }
public:
    virtual void Run_Read() = 0;
    virtual void Run_Cmd() = 0;
    virtual void Run_Result() = 0;
protected:
    void StartThread();
    void StopThread();
protected:
    bool m_bQuitRun;
private:
    std::thread m_thRunCmd;
    std::thread m_thRunRead;
    std::thread m_thRunResult;
};
//////////////////////////////////////////////////////////////////////////
class CStlFiveThread
{
public:
    CStlFiveThread() { m_bQuitRun = false; }
    virtual ~CStlFiveThread() { m_bQuitRun = true; }
public:
    virtual void Run_Cmd() = 0;
    virtual void Run_GetInfo() = 0;
    virtual void Run_Execute() = 0;
    virtual void Run_Result() = 0;
    virtual void Run_Status() = 0;
protected:
    void StartThread();
    void StopThread();
protected:
    bool m_bQuitRun;
private:
    std::thread m_thRunCmd;
    std::thread m_thRunGet;
    std::thread m_thRunExe;
    std::thread m_thRunResult;
    std::thread m_thRunStatus;
};
//////////////////////////////////////////////////////////////////////////
class CStlSixThread
{
public:
    CStlSixThread() { m_bQuitRun = false; }
    virtual ~CStlSixThread() { m_bQuitRun = true; }
public:
    virtual void Run_Read() = 0;
    virtual void Run_Cmd() = 0;
    virtual void Run_GetInfo() = 0;
    virtual void Run_Execute() = 0;
    virtual void Run_Result() = 0;
    virtual void Run_Status() = 0;

protected:
    void StartThread();
    void StopThread();
protected:
    bool m_bQuitRun;
private:
    std::thread m_thRunCmd;
    std::thread m_thRunGet;
    std::thread m_thRunExe;
    std::thread m_thRunResult;
    std::thread m_thRunStatus;
    std::thread m_thRunRead;
};
//////////////////////////////////////////////////////////////////////////
