#pragma once

#include "UsbDLLDef.h"
#include "ILogWrite.h"
#include "LogWriteThread.h"
#include "IURDevice.h"
#include "DevPortLogFile.h"
#include <QLibrary>

//USB驱动函数定义
typedef unsigned int(*HT_FnATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);   // 收发USB数据
typedef unsigned int(*HT_InfATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr); // 打开关闭USB驱动连接，查询Sensor状态
//////////////////////////////////////////////////////////////////////////


typedef struct _connect_para
{
    CONNECT_TYPE    econnect;
    unsigned int    uUSBHandle;
    _connect_para()
    {
        econnect = CONNECT_TYPE_UNKNOWN;
        uUSBHandle = 0;
    }
} STCONNECTPARA, *LPSTCONNECTPARA;


typedef list<STCONNECTPARA> LDEFCONNECT;

class CVHUSBDrive : public CLogManage
{
public:
    CVHUSBDrive(const char *lpDevName);
    ~CVHUSBDrive();

public:

    // 打开连接
    bool OpenVHUSBConnect(CONNECT_TYPE eConnectType);
    // 关闭连接
    bool CloseVHUSBConnect(CONNECT_TYPE eConnectType);
    // 收发数据
    long SendAndRecvData(
    CONNECT_TYPE    eConnect,
    const char *pszSendData,
    DWORD dwSendLen,
    DWORD dwSendTimeout,
    char *pszRecvData,
    DWORD &dwRecvedLenInOut,
    DWORD dwRecvTimeout,
    const char *pActionName,
    BOOL bPDL = FALSE);

    long RecvData(DWORD uUSBHandle, char *pszRecvData, DWORD &dwRecvedLenInOut, DWORD dwRecvTimeout,
                  const char *pActionName, bool bPDL = false, bool bWriteLog = true);
protected:
    // 错误描述
    const char *GetErrDesc(unsigned long ulErrCode);
    long USBDrvError2DriverError(ULONG ulError);

    STCONNECTPARA FindConnection(CONNECT_TYPE eConnectType);

    long CloseConnection(STCONNECTPARA &stConnPara);
    void CloseAllConnection();

    // 加载VHUSB底层通讯库
    bool VHUSBDllLoad();

    bool IsVHUSBLoaded() {return m_hdll.isLoaded();}
    // 卸载VHUSB底层驱动库
    void VHUSBDllRelease();

    long VHUSBDrvCall(ULONG wCmd, PSTR_DRV pParam);
private:
    char            m_szDesc[256];
    char            m_szDevName[128];

    HT_FnATMUSB     m_pFnATMUSB;
    HT_InfATMUSB    m_pInfATMUSB;

    QLibrary        m_hdll;
    LDEFCONNECT     m_lConnect;
    std::recursive_mutex m_MutexAction;
    CDevPortLogFile      m_Log;
};

