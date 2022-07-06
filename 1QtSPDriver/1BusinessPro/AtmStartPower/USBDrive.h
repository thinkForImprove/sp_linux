#pragma once
#include "QtTypeDef.h"
#include "UsbDLLDef.h"
#include "ILogWrite.h"
#include <QLibrary>
#include "SimpleMutex.h"

////////////////////////////////////////////////////////////////////
#define ROM_DATA_SIZE  4096
#define RES_COMMON_LEN 90
#define SN_LEN         16
////////////////////////////////////////////////////////////////////
struct ROMREAD
{
    WORD wLEN;       // LEN      (2 BYTE)
    WORD wCNTL_ID;   // CNTL ID  (2 BYTE)
    WORD wCNTL_LNG;  //      LNG (2 BYTE)
    WORD wCNTL_CMD;  //      CMD (2 BYTE)
    WORD wCMDD_ID;   // DATA ID  (2 BYTE)
    WORD wCMDD_LNG;  //      LNG (2 BYTE)
    BYTE byCMDD_Sector;
    BYTE byCMDD_Offset;
    ROMREAD() { memset(this, 0x00, sizeof(ROMREAD)); }
};

struct ROMCLEAR
{
    WORD wLEN;       // LEN      (2 BYTE)
    WORD wCNTL_ID;   // CNTL ID  (2 BYTE)
    WORD wCNTL_LNG;  //      LNG (2 BYTE)
    WORD wCNTL_CMD;  //      CMD (2 BYTE)
    WORD wCMDD_ID;   // DATA ID  (2 BYTE)
    WORD wCMDD_LNG;  //      LNG (2 BYTE)
    BYTE byCMDD_Sector;
    BYTE byCMDD_Reserve;
    ROMCLEAR() { memset(this, 0x00, sizeof(ROMCLEAR)); }
};

struct ROMWRITE
{
    WORD wLEN;       // LEN      (2 BYTE)
    WORD wCNTL_ID;   // CNTL ID  (2 BYTE)
    WORD wCNTL_LNG;  //      LNG (2 BYTE)
    WORD wCNTL_CMD;  //      CMD (2 BYTE)
    WORD wCMDD_ID;   // DATA ID  (2 BYTE)
    WORD wCMDD_LNG;  //      LNG (2 BYTE)
    BYTE byCMDD_Sector;
    BYTE byCMDD_Offset;
    BYTE byWriteData[ROM_DATA_SIZE];
    ROMWRITE() { memset(this, 0x00, sizeof(ROMWRITE)); }
};

struct IOMCResp
{
    WORD wLEN;                    // LEN      (2 BYTE)
    WORD wRESP_ID;                // RESP ID  (2 BYTE)
    WORD wRESP_LNG;               //      LNG (2 BYTE)
    WORD wRESP_RES;               //      RES (2 BYTE)
    BYTE byDATA[RES_COMMON_LEN];  // 共通部
    IOMCResp() { memset(this, 0x00, sizeof(IOMCResp)); }
};

struct MEMORYREAD
{
    WORD wLEN;       // LEN      (2 BYTE)
    WORD wCNTL_ID;   // CNTL ID  (2 BYTE)
    WORD wCNTL_LNG;  //      LNG (2 BYTE)
    WORD wCNTL_CMD;  //      CMD (2 BYTE)
    WORD wCMDD_ID;   // DATA ID  (2 BYTE)
    WORD wCMDD_LNG;  //      LNG (2 BYTE)
    WORD wCMDD_DATA;
    MEMORYREAD() { memset(this, 0x00, sizeof(MEMORYREAD)); }
};

struct MEMORYREADResp
{
    WORD wLEN;       // LEN      (2ハ＂イト)
    WORD wRESP_ID;   // RESP ID  (2ハ＂イト)
    WORD wRESP_LNG;  //      LNG (2ハ＂イト)
    WORD wRESP_RES;  //      RES (2ハ＂イト)
    BYTE byCommon[34];
    BYTE byStatus[28];
    WORD wID;   // ID  (2 BYTE)
    WORD wLNG;  //      LNG (2 BYTE)
    BYTE byTarget;
    BYTE byReserve;
    char cMemoryData[128];
    MEMORYREADResp() { memset(this, 0x00, sizeof(MEMORYREADResp)); }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// USB驱动函数定义
typedef unsigned int (*HT_FnATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);   // 收发USB数据
typedef unsigned int (*HT_InfATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);  // 打开关闭USB驱动连接，查询Sensor状态
//////////////////////////////////////////////////////////////////////////
class CUSBDrive : public CLogManage
{
public:
    CUSBDrive(LPCSTR lpDevType);
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

protected:
    // 错误描述
    LPCSTR GetErrDesc(ULONG ulErrCode);

private:
    char m_szDesc[256];
    char m_szDevName[128];
    UINT m_ulUSBHandle;
    QLibrary m_cUsbDll;
    HT_FnATMUSB m_pFnATMUSB;
    HT_InfATMUSB m_pInfATMUSB;
    CQtSimpleMutexEx m_cSysMutex;
};
