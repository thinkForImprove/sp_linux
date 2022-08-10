#pragma once

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>
#include <QDateTime>

#include <thread>
#include "SimpleMutex.h"
#include "IOMCUsbdef.h"
#include "UsbDLLDef.h"
#include "USBDrive.h"

#include "INIFileReader.h"
#include "ILogWrite.h"
#include "QtShareMemoryRW.h"

//////////////////////////////////////////////////////////////////////////
#define ETC_WINPATH                         "C:/CFES/ETC/"
#define ETC_LINUXPATH                       "/usr/local/CFES/ETC/"
//////////////////////////////////////////////////////////////////////////
#define DEF_TIMEOUT                         (3*1000)
#define RES_COMMON_LEN                      90                  // response common part length
//////////////////////////////////////////////////////////////////////////
#define NULL_TIME                           0x00
#define START_SCAN_TIME                     0x01
#define ALIVE_SCAN_TIME                     0x02
#define SHUTDOWN_SCAN_TIME                  0x04
#define AUTO_POWERON_TIME                   0x08
#define FORCE_POWEROFF_TIME                 0x10

//////////////////////////////////////////////////////////////////////////
#define DATETIME_NOW                        0x0001      //現在年月日
#define POWER_CUTDOWN                       0x0002      //電力遮断
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
//struct STR_IOMC
//{
//    WORD        wLEN;                      // LEN       (2)
//    WORD        wCNTL_ID;                  // CNTL ID   (2)
//    WORD        wCNTL_LNG;                 // LNG       (2)
//    WORD        wCNTL_CMD;                 // CMD       (2)
//    WORD        wDATA_ID;                  // DATA ID   (2)
//    WORD        wDATA_LNG;                 // LNG       (2)
//    BYTE        byDATA[128];               // 数据
//};

//typedef struct strIOMCRespInfT
//{
//    WORD        wLEN;                      // LEN       (2)
//    WORD        wRESP_ID;                  // RESP ID   (2)
//    WORD        wRESP_LNG;                 // LNG       (2)
//    WORD        wRESP_RES;                 // RES       (2)
//    BYTE        byDATA[RES_COMMON_LEN];    // 共通部
//    BYTE        byRSPD[256];
//} IOMCRespInfT, *LPIOMCRespInfT;

typedef struct tag_iomc_power_param
{
    BOOL bAutoPowerOn;
    BOOL bStartScanSupport;
    BOOL bAliveScanSupport;
    BOOL bShutdownScanSupport;
    BOOL bPowerOnAllDevTpye;
    WORD wPowerOffSleepTime;
    WORD wPowerONSleepTime;
    WORD wStartScanTime;
    WORD wAliveScan1Time;
    WORD wAliveScan2Time;
    WORD wShutdownScanTime;
    WORD wForcePowerOffTime;
    char szPortParam[MAX_PATH];                 //通信端口参数

    tag_iomc_power_param() { Clear(); }
    void Clear() { memset(this, 0x00, sizeof(tag_iomc_power_param)); }
} IOMCPOWERPARAM;

//struct VERtag
//{
//    uchar    NAME[8];
//    uchar    byV[2];
//    uchar    byR[2];
//    uchar    byT[2];
//    uchar    byN[2];
//};

//struct IOMC_VERtag
//{
//    ushort    wType ;
//    VERtag    VerDt[2] ;
//};

//struct DRV_VERtag
//{
//    ushort    wType ;
//    VERtag    VerDt[4] ;
//};

//struct SVERSION
//{
//    ushort    wID;
//    ushort    wLNG;
//    char      cKatashiki[16] ;
//    char      cVER[8] ;
//    uchar     byDate[6];
//    uint      dwSum;
//};

//struct IOMCVrtInfT                          // IOMC
//{
//    ushort        wLEN ;                   // LEN      (2Byte)
//    ushort        wRESP_ID ;               // RESP ID  (2Byte)
//    ushort        wRESP_LNG ;              //      LNG (2Byte)
//    ushort        wRESP_RES ;              //      RES (2Byte)
//    uchar         byDATA[RES_COMMON_LEN] ; // commom part STD40-00-00-00
//    SVERSION      VRT[2] ;                 //
//} ;
//////////////////////////////////////////////////////////////////////////
#pragma pack()
////////////////////////////////////////////////////////////////////
class CStlOneThreadI
{
public:
    CStlOneThreadI() { m_bQuitRun = false; }
    virtual ~CStlOneThreadI() { m_bQuitRun = true; }

public:
    virtual void Run() = 0;
public:
    void ThreadStart()
    {
        m_thRun = std::thread(&CStlOneThreadI::Run, this);
        m_thRun.detach();
    }
    void ThreadStop() { m_bQuitRun = true; }

protected:
    bool m_bQuitRun;
private:
    std::thread m_thRun;
};
////////////////////////////////////////////////////////////////////
namespace Ui
{
class CAtmStart;
}

class CAtmStartPower :
    public QWidget, public CStlOneThreadI, public CLogManage
{
    Q_OBJECT

public:
    explicit CAtmStartPower(QWidget *parent = nullptr);
    ~CAtmStartPower();
private slots:
    void OnExit();
public:
    // 打开电源并检测线程
    virtual void Run();
private:
    // 读取配置
    long ReadConfig();
    // 打开连接
    long Open();
    // 关闭连接
    long Close();
    // 复位
    long Reset();
    // 向IOMC写数据
    long DataWrite(WORD wParam);
    // 向IOMC读数据
    long DataRead();
    // 打开背景灯
    long BackLightOn();
    // 关闭监控
    long StopPowerScan();
    // 上下电
    long PowerOnOff(bool bPowerOn);
    // 电源设定监控间隔时间
    long SetPowerScanTime();
    // 电源自动上电控制
    long PowerOnAutoControl();
    // 电源刷新监控信息，防止自动关机
    long PowerOnScanRefresh();
    // 电源检测监控状态
    long CheckScanTimeoutFlag();
    // 清除断电标志
    long ClearPowOffFlag();
    // 获取并记录设备固件版本
    long GetFWVer();
private:
    // 读取USB驱动版本
    long GetDrvVer(DRV_VERtag &stDrvVer);
    // 读取板子版本
    long GetIOMCVer(IOMC_VERtag &stIOMCVer);
    // 延时
    void QtSleep(ulong ulTimeOut);
    // 命令打包和发送
    long SendAndReadCmd(const STR_IOMC &strIomc, IOMCRespInfT &stRespInf, const char *ThisModule);
    // 处理其他进程IOMC命令
    void HdlOtherProcIOMCCmd();
private:
    Ui::CAtmStart *ui;
private:
    QSystemTrayIcon                 m_sysTray;
    CUSBDrive                       m_cDev;
    CUSBDrive                       m_cDevPW;
    CINIFileReader                  m_cINI;
    IOMCPOWERPARAM                  m_stPW;
    CQtShareMemoryRW                m_cIOMCShareMemoryS;
    CQtShareMemoryRW                m_cIOMCShareMemoryR;
};
