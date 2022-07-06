#pragma once
#include "IDevPort.h"
#include "SimpleMutex.h"
#include "StlSimpleThread.h"
#include "AutoQtHelpClass.h"
#include "ILogWrite.h"
#include "DevPortLogFile.h"
#include "usbapi.h"
#include "IAllDevPort.h"

#include <semaphore.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <termios.h>
//////////////////////////////////////////////////////////////////////////
#define ENDPOINT    0x00
#define CTRL_IN     ( 0x83 | LIBUSB_ENDPOINT_IN )
#define CTRL_OUT    ( ENDPOINT | LIBUSB_ENDPOINT_OUT )
//////////////////////////////////////////////////////////////////////////
struct usb_device_;
typedef struct usb_device_ usb_device;
const size_t USB_READ_LENTH = 64;

typedef enum
{
    FLUSH_IN = 1,
    FLUSH_OUT,
    FLUSH_IO,
} FLUSH_FLAG;

typedef struct tag_usb_param
{
    uint16_t usVID;
    uint16_t usPID;
} STUSBPARAM, *LPSTUSBPARAM;
//////////////////////////////////////////////////////////////////////////
inline void EvaltimeAddMS(struct timeval *a, unsigned long ms)
{
    a->tv_usec += ms * 1000;
    if (a->tv_usec >= 1000000)
    {
        a->tv_sec += a->tv_usec / 1000000;
        a->tv_usec %= 1000000;
    }
}

inline timespec AddTime2Evaltime(unsigned long timeMSec)
{
    struct timespec wait_time;
    struct timeval now;
    gettimeofday(&now, nullptr);
    EvaltimeAddMS(&now, timeMSec);
    wait_time.tv_sec = now.tv_sec;
    wait_time.tv_nsec = now.tv_usec * 1000;

    return wait_time;
}

class CSimSem
{
public:
    CSimSem(/*const char*lpName*/)
    {
        //m_psem = sem_open(m_psem, O_CREAT, 0, 0);
        ResetSem();
    }
    ~CSimSem()
    {
        sem_destroy(&m_sem);
    }
    //void ResetSem(){ m_sem.acquire();*/ }//
    void ResetSem() { sem_init(&m_sem, 0, 0); }
    void SetSem() { sem_post(&m_sem);}
    bool WaitSem(unsigned long timeout)
    {
        timespec times = AddTime2Evaltime(timeout);
        int iRet = sem_timedwait(&m_sem, &times);
        if ((iRet == -1) && (errno == ETIMEDOUT))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
private:
    sem_t m_sem;
};

class CLinuxUSBPort : public IDevPort, public CLogManage
{
public:
    CLinuxUSBPort(LPCSTR lpDevName, LPCSTR lpDevType);
    virtual ~CLinuxUSBPort();
public:
    // 释放接口
    virtual void Release();
    // 开关串口，格式lpMode = VID,PID
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
    //USB ReportLen
    //nInlen传入参数:通过该参数传入得到ReportID
    //nOutLen传出:通过传入参数nInlen得到实际要发送的数据长度
    //返回值:ReportID
    DWORD GetReportLenFromLength(int nInLen, int &nOutSendLen);
    //返回值:报文长度
    DWORD GetReportLen(LPCSTR lpReportLen);
    //
    long GetUSBParam(LPCSTR lpMode, STUSBPARAM &stParam);

private:
    bool                    m_bIsOpen;
    CSimpleMutex            m_cMutex;
    CDevPortLogFile         m_cLog;

    usb_device             *m_pUsb_handle;
    CSimSem                 m_SemCancel;
};
