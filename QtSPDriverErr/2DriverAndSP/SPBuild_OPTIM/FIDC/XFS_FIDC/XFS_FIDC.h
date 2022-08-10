/***************************************************************
* 文件名称：XFS_FIDC.h
* 文件描述：非接读卡器模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#pragma once
#include "IDevIDC.h"
#include "ISPBaseIDC.h"
#include "QtTypeInclude.h"
#include "IDCFORM/IDCForm.h"
#include "def.h"

/*************************************************************************
// 宏定义
*************************************************************************/
// INI中指定[int型]设备类型 转换为 STR型
#define DEVTYPE2STR(n) \
    (n == XFS_MT50 ? IDEV_MT50 : \
     (n == XFS_CJ201 ? IDEV_CJ201 : \
      (n == XFS_TMZ ? IDEV_TMZ : \
       (n == XFS_CRT603CZ7 ? IDEV_CRT603CZ7: ""))))

// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_IDC_DEVONLINE && DSTAT != WFS_IDC_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device Status[%d] != ONLINE | BUSY, Return: %d", DSTAT, WFS_ERR_HARDWARE_ERROR); \
        return WFS_ERR_HARDWARE_ERROR; \
    }

// 无卡状态检查
#define CHK_MEDIA_ISHAVE(m) \
    if (m == WFS_IDC_MEDIANOTPRESENT) \
    { \
        Log(ThisModule, __LINE__, "Media Status = %d|MEDIANOTPRESENT, 没有检测到卡, Return: %d", \
            m, WFS_ERR_IDC_NOMEDIA); \
        return WFS_ERR_IDC_NOMEDIA; \
    }

// 卡JAM状态检查
#define CHK_MEDIA_ISJAM(m) \
    if (m == WFS_IDC_MEDIAJAMMED) \
    { \
        Log(ThisModule, __LINE__, "Media Status = %d|EDIAJAMMED, JAM, Return: %d", \
            m, WFS_ERR_IDC_MEDIAJAM); \
        return WFS_ERR_IDC_MEDIAJAM; \
    }

// 等待用户取卡标志 定义
enum WAIT_TAKEN_FLAG
{
    WTF_NONE    = 0,                                // 不等待
    WTF_TAKEN   = 1,                                // 等待用户取卡
};

/*************************************************************************
// 结构体 声明
*************************************************************************/
typedef struct st_idc_ini_config
{
    CHAR                szDevDllName[256];                  // DevXXX动态库名
    WORD                wDeviceType;                        // 设备类型
    WORD                wBeepControl;                       //beep on/off
    STDEVICEOPENMODE    stDevOpenMode;                      // 设备打开模式
    CHAR                szSDKPath[256];                     // 设备SDK库路径
    WORD                wOpenFailRet;                       // Open失败时返回值
    BOOL                bPostRemovedAftEjectFixed;          // 退卡时无卡是否报MediaRemoved事件
    INT                 nTakeCardTimeout;                   // 排卡后未取走卡,灭灯超时时间
    STCONFIGBEEP        stConfig_Beep;                      // 设备鸣响设置
    STCONFIGLIGHT       stConfig_Light;                     // 指示灯设置
    WORD                wCanWriteTrack;                     // 是否支持写磁道
    STIMAGEPAR          stImageParam;                       // 图像参数

    st_idc_ini_config()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_idc_ini_config));
    }

} STINICONFIG, *LPSTINICONFIG;


/*************************************************************************
// 主处理类                                                               *
*************************************************************************/
class CXFS_FIDC : public ICmdFunc, public CLogManage, public CIDCForm, public ConvertVarIDC
{
public:
    CXFS_FIDC();
    virtual ~CXFS_FIDC();
public:
    // 开始运行SP
    long StartRun();

public:
    //----------------------------基本接口----------------------------
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    //----------------------------IDC类型接口----------------------------
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps);
    virtual HRESULT GetFormList(LPSTR &lpFormList);
    virtual HRESULT GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm);
    // 执行命令
    // virtual HRESULT ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData);// 不支持
    // virtual HRESULT WriteTrack(const LPWFSIDCWRITETRACK lpWriteData);// 不支持
    virtual HRESULT EjectCard(DWORD dwTimeOut);
    // virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData);// 不支持
    // virtual HRESULT ResetCount();                                // 不支持
    // virtual HRESULT SetKey(const LPWFSIDCSETKEY lpKeyData);      // 不支持
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut);
    // virtual HRESULT WriteRawData(const LPWFSIDCCARDDATA *lppCardData);// 不支持
    virtual HRESULT ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut);
    virtual HRESULT Reset(LPWORD lpResetIn);
    virtual HRESULT ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData);
    // virtual HRESULT ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData);// 不支持

protected:  // XFS_FIDC_DEC.cpp 子处理相关接口
    HRESULT InnerOpen(BOOL bReConn = FALSE);                        // Open设备及初始化相关子处理
    bool LoadDevDll(LPCSTR ThisModule);                             // 加载DevXXX动态库
    void InitConfig();                                              // 加载INI设置
    void InitStatus();                                              // 状态结构体实例初始化
    void InitCaps();                                                // 能力值结构体实例初始化
    WORD UpdateDeviceStatus();                                      // 设备状态实时更新
    HRESULT InnerReadRawData(DWORD dwReadOption, DWORD dwTimeOut);  // ReadRawData子处理
    void SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);  // ReadRawData应答数据处理
    bool GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho);// ReadRawData应答数据处理
    void ControlLight(WORD wAction, WORD wType = GLIGHTS_TYPE_BLUE);// 指示灯控制
    void ControlBeep();                                             // 鸣响控制
    void UpdateExtra();                                             // 更新扩展数据

protected:  // XFS_FIDC_FIRE.cpp 事件处理相关接口
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireCardInserted();
    void FireCardInvalidMedia();
    void FireMediaRemoved();
    void FireMediaRetained();
    void FireMediaDetected(WORD ResetOut);
    void FireRetainBinThreshold(WORD wReBin);
    void FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);

private:
    STINICONFIG                     m_stConfig;                         // INI结构体
    CQtDLLLoader<IDevIDC>           m_pDev;                             // DevXXX调用实例
    CQtDLLLoader<ISPBaseIDC>        m_pBase;
    CSimpleMutex                    *m_pMutexGetStatus;
    CXfsRegValue                    m_cXfsReg;

private:
    CExtraInforHelper               m_cStaExtra;                        // 状态扩展数据
    CExtraInforHelper               m_cCapExtra;                        // 能力值扩展数据
    CWfsIDCStatus                   m_stStatus;                         // 全局状态
    CWfsIDCStatus                   m_stStatusOLD;                      // 上一次备份全局状态
    BOOL                            m_bMultiCard;                       // 是否出现多张卡
    BOOL                            m_bChipPowerOff;                    // 是否上电标记
    WAIT_TAKEN_FLAG                 m_WaitTaken;                        // Taken事件标记
    ULONG                           m_ulTakeMonitorStartTime;           // 拿卡监视开始时间
    INT                             m_nReConErr;                        // 断线重连错误码记录

private:    // 应答数据变量
    CWFSIDCCardDataPtrArray         m_cCardData;                        // ReadRawData应答
    CWFSChipIO                      m_ChipIO;                           // ChipIO命令应答数据
    CWFSChipPower                   m_ChipPower;                        // ChipPoqwe返回的ATR数据
    WFSIDCCAPS                      m_Caps;

};

