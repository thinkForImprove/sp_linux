#pragma once
#include "IDevPort.h"
#include "SimpleMutex.h"
#include "StlSimpleThread.h"
#include "AutoQtHelpClass.h"
#include "ILogWrite.h"
#include "DevPortLogFile.h"
#include <stdio.h>
#include "hidapi.h"

#include <unistd.h>
#include <libusb-1.0/libusb.h>
//////////////////////////////////////////////////////////////////////////
#define ENDPOINT    0x00
#define CTRL_IN     ( 0x83 | LIBUSB_ENDPOINT_IN )
#define CTRL_OUT    ( ENDPOINT | LIBUSB_ENDPOINT_OUT )
//////////////////////////////////////////////////////////////////////////
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
class CLinuxHIDPort : public IDevPort, public CLogManage
{
public:
    CLinuxHIDPort(LPCSTR lpDevName, LPCSTR lpDevType);
    virtual ~CLinuxHIDPort();
public:
    // 释放接口
    virtual void Release();
    // 开关串口，格式lpMode = VID,PID
    virtual long Open(LPCSTR lpMode, bool bExclusiveUse = true);
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
    string                  m_strOpenMode;
    CSimpleMutex            m_cMutex;
    CDevPortLogFile         m_cLog;

    STUSBPARAM              m_USBparam;
    //libusb_device_handle*   m_hComm;  //a USB device handle
    //libusb_context*         m_ctx;    //a libusb session
    hid_device             *m_pdev;
};
