#pragma once
#include "IDevSIU.h"
#include "DevSIU.h"
#include "QtTypeInclude.h"
#include "IAllDevPort.h"
#include "USBDrive.h"
#include "IOMCUsbdef.h"
#include "IOMCDef.h"

#include "QtShareMemoryRW.h"
#include <bitset>
using namespace std;

//////////////////////////////////////////////////////////////////////////
#define DEF_CMD_TIMEOUT                           (30*1000)
//////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
//SKM接口灯闪烁参数结构体
typedef struct _SKMLAMPFLASHCFES{
    BYTE        byPortNum;                      // SKM端口号(1~8)
    BOOL        bLedOn;                         // 灯亮标志
    int         iOnTime;                        // 灯亮时间ms
    int         iOffTime;                       // 灯灭时间ms
    ULONG       ulLastOnOffTime;                // 最后一次亮灭起始时间　0:未记录时间
}SKMLAMPFLASHCFES, *LPSKMLAMPFLASHCFES;
#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////
class CDevSIU_CFES: public IDevSIU, public CLogManage
{
public:
    CDevSIU_CFES(LPCSTR lpDevType);
    virtual ~CDevSIU_CFES();
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
#if defined(SPBuild_OPTIM)
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);
#endif
    //测试Flicker,Skim灯下标接口
    virtual int SetFlickerLed(int iFlickerLedIdx, int iAction);
    virtual int SetSkimLed(int iFlickerLedIdx, int iAction);
public:
    // 线程使用接口
    long SendCmdData(WORD wCmd, PSTR_DRV pParam);
    SKMLAMPFLASHCFES m_stSkmLampFlash[SKM_LAMP_MAX];
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
    long GetVersion(VERSIONINFO &stVersion);
    // 控制非接灯
    long SetSKMLampCmd(WORD wCmd, BYTE byPortNum);
    // 退出非接灯闪烁线程
    void ExitSKMLampFlashThread();

    // 命令打包和发送
    long SendAndReadCmd(LPBYTE lpbySendData, UINT uiSendLen, LPVOID lpRespData, UINT &uRespLen);
    long AddPkgData(LPBYTE lpData, WORD wPkgId, LPBYTE lpbyPkgData, int iPkgDataLen);

    //打开天梦者非接
    long OpenTMZFidc();                         //30-00-00-00(FS#0012)
    long CloseTMZFidc();                        //30-00-00-00(FS#0012)

	 // 设置灯闪烁循环模式参数
    long SetFlashRecycleParamer();
	INT  GetCardReaderSkimming();               // 获取读卡器异物检知结果

    //通过共享内存发送和接收数据
    long SendAndReadCmd(PSTR_DRV pParam);
    BOOL IsOpen();
private:
//    CSimpleMutex            m_cMutex;
    CQtDLLLoader<IAllDevPort> m_pDevPort;
    CUSBDrive               m_cDev;
    string                  m_strOpenMode;
    string                  m_strDevType;
    CINIFileReader          m_cINI;
    bool                    m_bFrontType;
    bool                    m_bOnlyRearDoor;
    int                     m_iSlowFlashSleepTime;
    int                     m_iMediumFlashSleepTime;
    int                     m_iQuickFlashSleepTime;
    int                     m_iFIDCType;                    //30-00-00-00(FS#0012)
	BYTE                    m_byOnOffTime[6];       //recycle1~3,on and off time    //40-00-00-00(FT#0001)

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

    BYTE                    m_bySendBuff[DEF_WRITE_BUFF_SIZE];
    BYTE                    m_byRecvBuff[DEF_READ_BUFF_SIZE];
    CQtShareMemoryRW        m_cIOMCShareMemoryS;
    CQtShareMemoryRW        m_cIOMCShareMemoryR;
    CQtSimpleMutexEx        m_cIOMCProcMutex;
};

//////////////////////////////////////////////////////////////////////////
