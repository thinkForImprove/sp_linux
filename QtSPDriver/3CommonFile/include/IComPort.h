//////////////////////////////////////////////////////////////////////
#pragma once

#if defined(WINCOMPORT_EXPORTS)
#define COMPORT_EXPORT     __declspec(dllexport)
#else
#ifdef Q_OS_WIN
#define COMPORT_EXPORT     __declspec(dllimport)
#else
#define COMPORT_EXPORT     Q_DECL_IMPORT
#endif
#endif
//////////////////////////////////////////////////////////////////////////
#define ERR_COMM_SUCCESS            (0)   //成功
#define ERR_COMM_NOTOPEN                        (-1)  //串口没打开
#define ERR_COMM_FAIL                           (-2)  //通讯错误
#define ERR_COMM_PARAM                          (-3)  //参数错误
#define ERR_COMM_CANCELED           (-4)  //操作取消
#define ERR_COMM_READERR            (-5)  //读取错误    
#define ERR_COMM_TIMEOUT            (-9)  //操作超时
#define ERR_COMM_NODEFINED                      (-99) //未知错误
//////////////////////////////////////////////////////////////////////////
#define MAX_BUFFER_SIZE             (4096)
//////////////////////////////////////////////////////////////////////////
struct IComPort
{
    // 释放接口
    virtual void Release() = 0;
    // 开关串口，格式lpMode = COM1:9600,N,8,1
    virtual long Open(LPCSTR lpMode) = 0;
    // 关闭串口
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
};

extern "C" COMPORT_EXPORT long CreateIComPort(LPCSTR lpDeviceName, IComPort *&p);
//////////////////////////////////////////////////////////////////////////
class CIComPortAutoLog
{
public:
    CIComPortAutoLog(IComPort *pDevice, const char *pActionName) : m_pDevice(pDevice)
    {
        if (pDevice != nullptr) pDevice->SetLogAction(pActionName);
    }
    ~CIComPortAutoLog() {}
private:
    IComPort *m_pDevice;
};

//////////////////////////////////////////////////////////////////////////
#define LOGDEVACTION()  CIComPortAutoLog _auto_log_this_action(m_pCom.GetDll(), ThisModule)
//////////////////////////////////////////////////////////////////////////
