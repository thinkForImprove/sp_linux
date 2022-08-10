﻿#pragma once
#include "IDevSIU.h"
#include "QtTypeInclude.h"

#include "USBDrive.h"
#include "UsbDLLDef.h"
#include "IOMCUsbdef.h"

#include <bitset>
using namespace std;

//////////////////////////////////////////////////////////////////////////
#define DEF_TIMEOUT                         (3*1000)
#define RES_COMMON_LEN                      90                  // response common part length
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
struct VERtag
{
    uchar    NAME[8];
    uchar    byV[2];
    uchar    byR[2];
    uchar    byT[2];
    uchar    byN[2];
};


struct IOMC_VERtag
{
    ushort    wType ;
    VERtag    VerDt[2] ;
};

struct DRV_VERtag
{
    ushort    wType ;
    VERtag    VerDt[4] ;
};

struct SVERSION
{
    ushort    wID;
    ushort    wLNG;
    char      cKatashiki[16] ;
    char      cVER[8] ;
    uchar     byDate[6];
    uint      dwSum;
};

struct IOMCVrtInfT                          // IOMC
{
    ushort        wLEN ;                   // LEN      (2Byte)
    ushort        wRESP_ID ;               // RESP ID  (2Byte)
    ushort        wRESP_LNG ;              //      LNG (2Byte)
    ushort        wRESP_RES ;              //      RES (2Byte)
    uchar         byDATA[RES_COMMON_LEN] ; // commom part STD40-00-00-00
    SVERSION      VRT[2] ;                 //
} ;

struct STR_IOMC
{
    WORD        wLEN;                      // LEN      (2Byte)
    WORD        wCNTL_ID;                  // CNTL ID  (2Byte)
    WORD        wCNTL_LNG;                 //      LNG (2Byte)
    WORD        wCNTL_CMD;                 //      CMD (2Byte)
    WORD        wDATA_ID;                  // DATA ID  (2Byte)
    WORD        wDATA_LNG;                 //      LNG (2Byte)
    BYTE        byDATA[64];                //
};

struct IOMCASYNCRespInfT
{
    BYTE        byOutDataSz[4];
    WORD        wLEN;                    // LEN      (2Byte)
    WORD        wRESP_ID;                // RESP ID  (2Byte)
    WORD        wRESP_LNG;               //      LNG (2Byte)
    WORD        wRESP_RES;               //      RES (2Byte)
    BYTE        byDATA[RES_COMMON_LEN];  // common part STD40-00-00-00
    BYTE        byRSPD[200];             // RSPD  STD40-00-00-00
};

typedef struct strIOMCRespInfT
{
    WORD        wLEN;                      // LEN      (2Byte)
    WORD        wRESP_ID;                  // RESP ID  (2Byte)
    WORD        wRESP_LNG;                 //      LNG (2Byte)
    WORD        wRESP_RES;                 //      RES (2Byte)
    BYTE        byDATA[RES_COMMON_LEN];    //
    BYTE        byRSPD[64];
} IOMCRespInfT, *LPIOMCRespInfT;

typedef struct _IOMC_Flicker_data
{
    BYTE Flk[15];   // Flicker 1~ 15
    BYTE Reserve;   // Reserve area
} STIOMCFLICKERDATA, *LPSTIOMCFLICKERDATA;


typedef struct _IOMCCOMMONOUTPUT{
    WORD wLen;
    WORD wCNTLId;
    WORD WCNTLLen;
    WORD wCNTLCmd;
    WORD wOutput1Id;
    WORD wOutput1Len;
    BYTE byOutput1Data[32];
    WORD wOutput2Id;
    WORD wOutput2Len;
    BYTE byOutput2Data[8];
}IOMCCOMMONOUTPUT, *LPIOMCCOMMONOUTPUT;

struct STR_IOMC_SENSE_INF_OUT               // IOMC Info sense Structure
{
    WORD        wLEN;                       // LEN      (2Byte)
    WORD        wRESP_ID;                   // RESP ID  (2Byte)
    WORD        wRESP_LNG;                  //      LNG (2Byte)
    WORD        wRESP_RES;                  //      RES (2Byte)
    BYTE        byDATA[RES_COMMON_LEN];     // Common part
    // Warning infomation
    WORD        wID_WARNING;
    WORD        wLNG_WARNING;
    BYTE        byWARNING1[30];         // MechaStatus(4Byte)
    BYTE        byWARNING2[30];         // MechaStatus(4Byte)
    BYTE        byWARNING3[30];         // MechaStatus(4Byte)
    BYTE        byWARNING4[30];         // MechaStatus(4Byte)
    // Sekkayku sense
    WORD        wID_SEKKYAKU;
    WORD        wLNG_SEKKYAKU;
    BYTE        bySEKKYAKU_SENSE_INF[4];    // Basic sense status   (4byte)
    // MC hikidashi sense
    WORD        wID_MC_HIKIDASHI;
    WORD        wLNG_MC_HIKIDASHI;
    BYTE        byMC_HIKIDASHI_SENSE_INF[2];// Reserve   (2byte)
    // Fuka sense
    WORD        wID_FUKA_SENSE;
    WORD        wLNG_FUKA_SENSE;
    BYTE        byFUKA_SENSE_INF[8];       // Sub sense status(8byte)

};  // Sensor Info


typedef struct strIOMCBoardVersionRespInfT      // BoardVersion IOMC response structure
{
    WORD        wLEN;                           // LEN      (2Byte)
    WORD        wRESP_ID;                       // RESP ID  (2Byte)
    WORD        wRESP_LNG;                      //      LNG (2Byte)
    WORD        wRESP_RES;                      //      RES (2Byte)
    BYTE        byCOMMON[RES_COMMON_LEN];       // Common part
    WORD        wRSPD_ID;                       // RSPD ID  (2Byte)
    WORD        wRSPD_LNG;                      //      LNG (2Byte)
    WORD        wCONFIGURATION;                 // Configuration  data(2Byte)
    BYTE        byBOARDVERSION;                 // BoardVersion data(1Byte)
    BYTE        bySYSTEMVERSION;                // SystemVersion data(1Byte)
    WORD        wDIPRSPD_ID;                    // RSPD ID  (2Byte)
    WORD        wDIPRSPD_LNG;                   //      LNG (2Byte)
    WORD        DIPswitch;                      // DIPswitch  data(2Byte)
    BYTE        byReserve;
    WORD        wDATERSPD_ID;                   // RSPD ID  (2Byte)
    WORD        wDATERSPD_LNG;                  //      LNG (2Byte)
    BYTE        byDAte[8];                      // year month day (8Byte)
} IOMCBoardVerRespInfT, *LPIOMCBoardVerRespInfT;
//////////////////////////////////////////////////////////////////////////
#pragma pack()

typedef long (* FNICReaderOpenUsbByFD)(unsigned int uiFD);          //30-00-00-00(FS#0012)
typedef int  (* FNICReaderClose)(long icdev);                       //30-00-00-00(FS#0012)
typedef int  (* FNICReaderLEDCtrl)(long icdev, unsigned char uLEDCtrl, unsigned char uDelay);     //30-00-00-00(FS#0012)
//////////////////////////////////////////////////////////////////////////
class CDevSIU_IOMC: public IDevSIU, public CLogManage
{
public:
    CDevSIU_IOMC(LPCSTR lpDevType);
    virtual ~CDevSIU_IOMC();
public:
    // 释放接口
    virtual void Release();
    // 打开连接
    virtual long Open(LPCSTR lpMode);
    // 关闭连接
    virtual long Close();
    // 复位
    virtual long Reset();
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo);
    // 取状态
    virtual long GetStatus(DEVSIUSTATUS &stStatus);
    // 控制门
    virtual long SetDoors(WORD wDoors[DEFSIZE]);
    // 控制指示符
    virtual long SetIndicators(WORD wIndicators[DEFSIZE]);
    // 控制辅助器
    virtual long SetAuxiliaries(WORD wAuxiliaries[DEFSIZE]);
    // 控制灯
    virtual long SetGuidLights(WORD wGuidLights[DEFSIZE]);
    // 获取设备固件版本
    virtual long GetFirmWareVer(char *pFwVer);
    // 执行固件升级
    virtual long UpdateDevPDL();
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);

    virtual int SetFlickerLed(int iFlickerLedIdx, int iAction);
    virtual int SetSkimLed(int iFlickerLedIdx, int iAction);
public:
    // 线程使用接口
    long SendCmdData(WORD wCmd, PSTR_DRV pParam);
    int m_iFlashSleepTime;
protected:
    // 设置灯信息
    long SetLightsCmd(WORD wID, WORD wCmd);
    // 获取传感器状态
    long GetSensStatus();
    // 更新传感状态
    void UpdateDoorsStatus();
    void UpdateSensorsStatus();
    void UpdateAuxiliariesStatus();
    // 设置支持的
    bool SetSupportStatus();
    // 复位全部状态
    void ResetAllStatus();
    void ResetNotSupportStatus();
    // 更新状态
    void UpdateStatus(WORD wDevice, long lErrCode);
    // 获取板子，驱动和固件版本
    long GetBoardVer(IOMCBoardVerRespInfT &stBoardVer);
    long GetDrvVer(DRV_VERtag &stDrvVer);
    long GetIOMCVer(IOMC_VERtag &stIOMCVer);
    // 控制非接灯
    long SetNonContactLightCmd(WORD wCmd);
    // 退出非接灯闪烁线程
    void ExitNoncontactFlashThread();

    // 命令打包和发送
    long SendAndReadCmd(const STR_IOMC &strIomc, LPVOID lpRespInf, UINT uRespSize, const char *ThisModule);

    //打开天梦者非接
    long OpenTMZFidc();                         //30-00-00-00(FS#0012)
    long CloseTMZFidc();                        //30-00-00-00(FS#0012)

    INT  GetCardReaderSkimming();               // 获取读卡器异物检知结果

private:
//    CSimpleMutex            m_cMutex;
    string                  m_strOpenMode;
    CUSBDrive               m_cDev;
    CUSBDrive               m_cPWDev;
    CINIFileReader          m_cINI;
    bool                    m_bFrontType;
    bool                    m_bOnlyRearDoor;
    int                     m_iSlowFlashSleepTime;
    int                     m_iMediumFlashSleepTime;
    int                     m_iQuickFlashSleepTime;
    int                     m_iFIDCType;                    //30-00-00-00(FS#0012)

    DEVSIUSTATUS            m_stStatus;
    STR_IOMC_SENSE_INF_OUT  m_stSensInfo;
    BOOL                    m_bSensorsSupport[DEFSIZE];
    BOOL                    m_bDoorsSupport[DEFSIZE];
    BOOL                    m_bIndicatorsSupport[DEFSIZE];
    BOOL                    m_bAuxiliariesSupport[DEFSIZE];
    BOOL                    m_bLightSupport[DEFSIZE];
    pthread_t               m_tid;

    FNICReaderLEDCtrl       m_ICReaderLEDCtrl;            //30-00-00-00(FS#0012)
    FNICReaderOpenUsbByFD   m_ICReaderOpenUsbByFD;        //30-00-00-00(FS#0012)
    FNICReaderClose         m_ICReaderClose;              //30-00-00-00(FS#0012)
    QLibrary                m_cLibrary;                   //30-00-00-00(FS#0012)
    long                    m_lDevHdl;                    //30-00-00-00(FS#0012)
};

//////////////////////////////////////////////////////////////////////////
