#pragma once

#include "UsbDLLDef.h"
#include "ILogWrite.h"
#include "LogWriteThread.h"
#include "IURDevice.h"
#include "DevPortLogFile.h"
#include <QLibrary>
#include "FNUSB.h"                      //30-00-00-00(FS#0016)

//USB驱动函数定义
typedef unsigned int(*HT_FnATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);   // 收发USB数据
typedef unsigned int(*HT_InfATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr); // 打开关闭USB驱动连接，查询Sensor状态
//FUNC函数指针定义
typedef unsigned int (*FN_F_GetInitResult)(void);							// Function library Initialize
typedef unsigned int (*FN_F_FnATMUSB)(unsigned int, PSTR_DRV);				// Function Series API
typedef unsigned int (*FN_F_InfATMUSB)(unsigned int, PSTR_DRV);             // Informatin Series API
typedef unsigned int (*FN_F_SetFnSetting)(unsigned int, PSTR_SETPRM);		// Function Set API
typedef unsigned int (*FN_F_SetCommonSetting)(unsigned int, PSTR_SETPRM);	// Parameter Set API
typedef unsigned int (*FN_F_GetFnSetting)(unsigned int, PSTR_GETPRM);		// Function Settings Acquisitoin API
typedef unsigned int (*FN_F_GetCommonSetting)(unsigned int, PSTR_GETPRM);	// Parameter Settings Acquisition API
typedef unsigned int (*FN_F_GetStatus)(unsigned int, PSTR_STATUS);          // Status Acquisition API
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
    CVHUSBDrive(const char *lpDevName, bool bUseFuncLib = false);       //30-00-00-00(FS#0016)
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

    //配置机能参数
    DWORD SetFuncSetting(UINT uiFncNo, USHORT usParam, const char *pszSendData = nullptr,  //30-00-00-00(FS#0016)
                         UINT uiSendLen = 0);                                              //30-00-00-00(FS#0016)
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

    FN_F_GetInitResult m_pFnFGetInitResult;             //30-00-00-00(FS#0016)
    FN_F_FnATMUSB      m_pFnFFnATMUSB;                  //30-00-00-00(FS#0016)
    FN_F_InfATMUSB     m_pFnFInfATMUSB;                 //30-00-00-00(FS#0016)
    FN_F_SetFnSetting  m_pFnFSetFnSetting;              //30-00-00-00(FS#0016)

    QLibrary        m_hdll;
    LDEFCONNECT     m_lConnect;
    std::recursive_mutex m_MutexAction;
    CDevPortLogFile      m_Log;

    bool            m_bUseFuncLib;          //30-00-00-00(FS#0016)
};

