//////////////////////////////////////////////////////////////////////
#pragma once

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"

//////////////////////////////////////////////////////////////////////////
#if defined(IDEVPORT_LIBRARY)
#define DEVPORT_EXPORT     Q_DECL_EXPORT
#else
#define DEVPORT_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
#define ERR_PORT_SUCCESS                        (0)   //成功
#define ERR_PORT_NOTOPEN                        (-1)  //串口没打开
#define ERR_PORT_FAIL                           (-2)  //通讯错误
#define ERR_PORT_PARAM                          (-3)  //参数错误
#define ERR_PORT_CANCELED                       (-4)  //操作取消
#define ERR_PORT_READERR                        (-5)  //读取错误
#define ERR_PORT_WRITE                          (-6)  //发送错误
#define ERR_PORT_RTIMEOUT                       (-7)  //操作超时
#define ERR_PORT_WTIMEOUT                       (-8)  //操作超时
#define ERR_PORT_NODEFINED                      (-99) //未知错误
//////////////////////////////////////////////////////////////////////////
#define MAX_BUFFER_SIZE                         (4096)
//////////////////////////////////////////////////////////////////////////
struct IDevPort
{
    // 释放接口
    virtual void Release() = 0;
    // 开关连接，格式lpMode: 串口=COM1:9600,N,8,1; HID/USB=VID,PID
    virtual long Open(LPCSTR lpMode, bool bExclusiveUse = true) = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 发送成功，返回已发送数据个数
    virtual long Send(LPCSTR lpData, DWORD dwDataLen, DWORD dwTimeOut) = 0;
    // 读取成功, 返回已读取数据个数
    // dwInOutReadLen 表示要读取的和读到的数据长度，如为输入为-1，则读全部(最大数据长度为4096字节)
    virtual long Read(LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut) = 0;
    // 收发数据
    virtual long SendAndRead(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut) = 0;
    // 取消等待读取
    virtual void CancelRead() = 0;
    // 清缓存数据
    virtual void ClearBuffer() = 0;
    // 是否已打开
    virtual bool IsOpened() = 0;
    // 设置日志记录动作名称
    virtual void SetLogAction(LPCSTR lpActName) = 0;
    // 刷新日志到文件中
    virtual void FlushLog() = 0;
    // 取消当前这条日志，建议是等待输入按键这些的超时日志，才取消不记录，防止日志过大和多条超时日志
    virtual void CancelLog() = 0;
};

extern "C" DEVPORT_EXPORT long CreateIDevPort(LPCSTR lpDevName, LPCSTR lpDevType, IDevPort *&pDev);
//////////////////////////////////////////////////////////////////////////
class CDevPortAutoLog
{
public:
    CDevPortAutoLog(IDevPort *pDev, const char *pActionName) : m_pDev(pDev)
    {if (m_pDev != nullptr) m_pDev->SetLogAction(pActionName);}
    ~CDevPortAutoLog()
    {if (m_pDev != nullptr) m_pDev->FlushLog();}
private:
    IDevPort *m_pDev;
};

//////////////////////////////////////////////////////////////////////////
#define LOGACTION()         CDevPortAutoLog _auto_log_this_action(m_pDev, ThisModule)
#define LOGACTIONEX(X)      CDevPortAutoLog _auto_log_this_action(X, ThisModule)
//////////////////////////////////////////////////////////////////////////
