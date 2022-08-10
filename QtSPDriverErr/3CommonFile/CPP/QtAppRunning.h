#pragma once
#include "QtTypeDef.h"
#include "SimpleMutex.h"

#include <QSharedMemory>
#include <QSystemSemaphore>

//////////////////////////////////////////////////////////////////////////
#define DEFPATH_WIN             "C:/CFES/BIN/"
#define DEFPATH_LINUX           "/usr/local/CFES/BIN/"
///////////////////////////////////////////////////////////////////
/// \brief The CQtAppRunning class
/// 因使用此遍历目录，会导致日立机芯驱动异常，暂时不知道原因，因此LINUX修改和WIN一样的检测
class CQtAppRunning
{
public:
    CQtAppRunning();
    virtual ~CQtAppRunning();
public:
    // 释放接口
    void Release();
    // 判断单实例APP是否在运行
    bool IsSingleAppRunning(LPCSTR lpAppName);
    // 创建判断是否在运行标志（WIN有效）
    bool CreateRunnningFlag(LPCSTR lpName, LPCSTR lpPID);
    bool CreateRunnningFlag(LPCSTR lpName, DWORD dwPID);
    // 判断进程是否在运行，当lpPID为空或空指针时，以名称判断
    bool IsAppRunning(LPCSTR lpName, LPCSTR lpPID);
    bool IsAppRunning(LPCSTR lpName, DWORD dwPID);
    // 判断进程是否在运行，判断速度慢，约200毫秒，并且LINUX下，使用终端打开进程时，无法检测到进程
    bool IsAppRunning(LPCSTR lpName);
    // 运行进程，注意：如果lpPath=null或空，则内部增加默认路径
    bool RunApp(LPCSTR lpName, LPCSTR lpPath, LPCSTR lpCmdLine = nullptr, long lTimeOut = 3000);
    // 杀进进程
    bool KillApp(LPCSTR lpName);
    // 阻塞延时，此延时不占用CPU资源
    void StlSleep(DWORD dwMilliseconds);
private:
    // 查找PID
    QByteArray FindPID(QByteArray strName);
    // 判断是否相同
    bool IsSameNamePID(QByteArray strName, QByteArray strPID);
    // 判断是否全部是数据字符
    bool IsDigit(const QByteArray &strBuff);

private:
    CSimpleMutex                    m_cMutex;
    QByteArray                      m_strPID;
    QByteArray                      m_strName;

    QByteArray                      m_strMemSingleKey;
    QByteArray                      m_strMemRunFlagKey;
    QByteArray                      m_strMemKey;
    QSharedMemory                   m_cMemSingle;
    QSharedMemory                   m_cMemRunFlag;
    QSharedMemory                   m_cMemRunning;
};


