//////////////////////////////////////////////////////////////////////
#pragma once

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
//////////////////////////////////////////////////////////////////////////
#if defined(ALLDEVPORT_LIBRARY)
#define ALLDEVPORT_EXPORT     Q_DECL_EXPORT
#else
#define ALLDEVPORT_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
#define ERR_DEVPORT_SUCCESS                        (0)   //成功
#define ERR_DEVPORT_NOTOPEN                        (-1)  //没打开
#define ERR_DEVPORT_FAIL                           (-2)  //通讯错误
#define ERR_DEVPORT_PARAM                          (-3)  //参数错误
#define ERR_DEVPORT_CANCELED                       (-4)  //操作取消
#define ERR_DEVPORT_READERR                        (-5)  //读取错误
#define ERR_DEVPORT_WRITE                          (-6)  //发送错误
#define ERR_DEVPORT_RTIMEOUT                       (-7)  //操作超时
#define ERR_DEVPORT_WTIMEOUT                       (-8)  //操作超时
#define ERR_DEVPORT_LIBRARY                        (-98) //加载通讯库失败
#define ERR_DEVPORT_NODEFINED                      (-99) //未知错误


// IAllDevPort接口返回(BRM使用)--遗留问题:是否归入BRM
#define MAGNET_OPEN_TIMEOUT                         0xEA    //test#11
#define HAND_SENSOR_TIMEOUT                         0xEB    //test#11
#define HAND_SENSOR_RETRY_TIMEOUT                   0xEC    //test#11
#define SHUTTER_OPEN_TIMEOUT                        0xF1    //test#11
#define SHUTTER_CLOSE_TIMEOUT                       0xF2    //test#11
#define LOCK_OPEN_TIMEOUT                           0xF3    //test#11
#define HAND_SENSON_ON                              0xF4    //test#11
#define HAND_SENSOR_ON_TIMEOUT                      0xF5    //test#11
#define SHUTTER_OPEN_RESPONSE                       0x61    //test#11
#define SHUTTER_CLOSE_RESPONSE                      0x62    //test#11
#define SHUTTER_STATUS_RESPONSE                     0x64    //test#11
#define SHUTTER_PORT_OPEN_SUCCESS                   0x66    //test#11
#define SHUTTER_OPEN_SUCCESS                        0xD1    //test#11
#define SHUTTER_CLOSE_SUCCESS                       0xD2    //test#11
#define SHUTTER_SENSOR_OFF                          0x00    //test#11
#define SHUTTER_SENSOR_ON                           0x01

//////////////////////////////////////////////////////////////////////////
struct IAllDevPort
{
    // 释放接口
    virtual void Release() = 0;
    // 开关连接，格式lpMode: 串口=COM1:9600,N,8,1; HID:VID,PID; USB:Name/VID,PID
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

extern "C" ALLDEVPORT_EXPORT long CreateIAllDevPort(LPCSTR lpDevName, LPCSTR lpDevType, IAllDevPort *&pPort);
//////////////////////////////////////////////////////////////////////////
class CAllDevPortAutoLog
{
public:
    CAllDevPortAutoLog(IAllDevPort *pDev, const char *pActionName) : m_pDev(pDev)
    { if (m_pDev != nullptr) m_pDev->SetLogAction(pActionName); }
    ~CAllDevPortAutoLog() { if (m_pDev != nullptr) m_pDev->FlushLog(); }
private:
    IAllDevPort *m_pDev;
};

//////////////////////////////////////////////////////////////////////////
#define LOGDEVACTION()  CAllDevPortAutoLog _auto_log_this_action(m_pDev, ThisModule)
//////////////////////////////////////////////////////////////////////////
