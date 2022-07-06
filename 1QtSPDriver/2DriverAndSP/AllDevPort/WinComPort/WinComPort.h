#pragma once

#include <qt_windows.h>

#include "IDevPort.h"
#include "SimpleMutex.h"
#include "AutoQtHelpClass.h"
#include "ILogWrite.h"
#include "DevPortLogFile.h"

//////////////////////////////////////////////////////////////////////////
class CAutoEventCom
{
public:
    CAutoEventCom() {m_hEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);}
    virtual ~CAutoEventCom() {if (m_hEvent) ::CloseHandle(m_hEvent);}
    HANDLE &GetHandle() {return m_hEvent;}
private:
    HANDLE m_hEvent;
};

//////////////////////////////////////////////////////////////////////////
class CWinComPort : public IDevPort, public CLogManage
{
public:
    CWinComPort(LPCSTR lpDevName, LPCSTR lpDevType);
    virtual ~CWinComPort();
public:
    // 释放接口
    virtual void Release();
    // 开关连接，格式lpMode: 串口=COM1:9600,N,8,1; HID/USB=VID,PID
    virtual long Open(LPCSTR lpMode);
    // 关闭串口
    virtual long Close();
    // 发送成功，返回已发送数据个数
    virtual long Send(LPCSTR lpData, DWORD dwDataLen, DWORD dwTimeOut);
    // 读取成功, 返回已读取数据个数
    // dwInOutReadLen 表示要读取的和读到的数据长度，如为输入为-1，则读全部(最大数据长度为4096字节)
    virtual long Read(LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut);
    // 收发数据
    virtual long SendAndRead(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut);
    // 取消等待读取
    virtual void CancelRead();
    // 清缓存数据
    virtual void ClearBuffer();
    // 是否已打开
    virtual bool IsOpened();
    // 设置日志记录动作名称
    virtual void SetLogAction(LPCSTR lpActName);
    // 刷新日志到文件中
    virtual void FlushLog();
    // 取消当前这条日志，建议是等待输入按键这些的超时日志，才取消不记录，防止日志过大和多条超时日志
    virtual void CancelLog();
private:
    void SetTimeOut(DWORD dwTimeOut, bool bReadTime = false);
private:
    string                  m_strOpenMode;
    CDevPortLogFile         m_cLog;

    HANDLE                  m_hCom;
    CSimpleMutex            m_cFuncMutex;
    CAutoEventCom           m_cCancleEvent;
    CAutoEventCom           m_cOverlappedWriteEvent;
    CAutoEventCom           m_cOverlappedReadEvent;
};
