#pragma once
#include "QtTypeDef.h"
#include "UsbDLLDef.h"
#include "ILogWrite.h"
#include <QLibrary>
#include "SimpleMutex.h"
#include "QtTypeInclude.h"
#include "IAllDevPort.h"
#include "IOMCDef.h"
#include "IOMCUsbdef.h"

#define HID_REPORT_SIZE 64
//////////////////////////////////////////////////////////////////////////
//IOMC 类型
enum IOMCTYPE{
    IOMC_TYPE_CFES,         //自研IOMC
    IOMC_TYPE_HT            //日立IOMC
};

enum CMDFLAG{
    CMD_FLAG_PDL = 0,
    CMD_FLAG_NORMAL = 1,
    CMD_FLAG_STOP = 3,
    CMD_FLAG_NORESPONSE = 4
};
//////////////////////////////////////////////////////////////////////////
//USB驱动函数定义
typedef unsigned int(*HT_FnATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);   // 收发USB数据
typedef unsigned int(*HT_InfATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);  // 打开关闭USB驱动连接，查询Sensor状态
//////////////////////////////////////////////////////////////////////////
class CUSBDrive: public CLogManage
{
public:
    CUSBDrive(LPCSTR lpDevType, IOMCTYPE eIomcType = IOMC_TYPE_HT);
    ~CUSBDrive();
public:
    // 加载USB底层通讯库
    long USBDllLoad();
    // 卸载USB底层驱动库
    void USBDllRelease();
    // 打开连接
    long USBOpen(const char *pDevName);
    // 关闭连接
    long USBClose();
    // 是否已打开连接
    bool IsOpen();
    // USB命令调用
    long USBDrvCall(WORD wCmd, PSTR_DRV pParam);
    // 设置USB库接口
    void SetFnATMUSB(HT_FnATMUSB pFnATMUSB, HT_InfATMUSB pInfATMUSB);
    // 获取接口指针
    HT_FnATMUSB GetFnATMUSB();
    HT_InfATMUSB GetInfATMUSB();

    //设置IOMC类型(若未通过构造函数取得，需要调用其他接口前设定)
    void SetIomcType(const IOMCTYPE &eIomcType);

protected:
    // 错误描述
    LPCSTR GetErrDesc(ULONG ulErrCode);

    // 命令打包和发送
    long SendAndReadData(LPBYTE lpbySendData, UINT uiSendLen, LPBYTE lpbyRecvData, UINT &uiRecvLen, UINT uiTimeout);
    long RecvData(LPBYTE lpbyRecvData, UINT &uiRecvLen, UINT uiTimeout);
    long CheckRecvData(LPBYTE lpbyRecvData, UINT uiRecvLen);
    WORD CalcBCC(LPBYTE lpbyData, UINT uiDataLen);

    //自研IOMC
    BYTE GetCmdFlag(WORD wCmdId);
    BOOL GetLogEnable(WORD wCmdId);
    UINT GetCmdTimeout(WORD wCmdId);

private:
    IOMCTYPE        m_eIomcType;
    string          m_strDevType;
    char            m_szDesc[256];
    char            m_szDevName[128];
    UINT            m_ulUSBHandle;
    QLibrary        m_cUsbDll;
    HT_FnATMUSB     m_pFnATMUSB;
    HT_InfATMUSB    m_pInfATMUSB;
    CQtSimpleMutexEx m_cSysMutex;

    //自研IOMC
    CQtDLLLoader<IAllDevPort> m_pDevPort;
    BYTE             m_bySeqNo;
};

