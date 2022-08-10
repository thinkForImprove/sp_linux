/***************************************************************************
* 文件名称：XFS_IDC.h
* 文件描述：读卡器模块命令处理接口头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
***************************************************************************/
#pragma once

#include "IDevIDC.h"        // 读卡器
#include "IDevCRM.h"        // 退卡模块
#include "IDevSIU.h"        // 用于异物检知
#include "ISPBaseIDC.h"
#include "QtTypeInclude.h"
#include "IDCFORM/IDCForm.h"
#include "ErrorDetail.h"
#include "def.h"

#include <unistd.h>
#include <queue>
#include <qthread.h>


/*************************************************************************
// 宏定义
*************************************************************************/
// INI中指定[int型]设备类型 转换为 STR型
#define DEVTYPE2STR(n) \
    (n == XFS_CRT350N ? IDEV_CRT350N_STR : "")

// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_IDC_DEVONLINE && DSTAT != WFS_IDC_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device Status != ONLINE | BUSY, Return: %d.", WFS_ERR_HARDWARE_ERROR); \
        switch(DSTAT) \
        { \
            case WFS_IDC_DEVOFFLINE: SetErrorDetail(1, (LPSTR)EC_ERR_DevOffLine); break; \
            case WFS_IDC_DEVNODEVICE: SetErrorDetail(1, (LPSTR)EC_ERR_DevNotFound); break; \
            case WFS_IDC_DEVHWERROR: SetErrorDetail(1, (LPSTR)EC_ERR_DevHWErr); break; \
            case WFS_IDC_DEVPOWEROFF: SetErrorDetail(1, (LPSTR)EC_ERR_DevPowerOff); break; \
            case WFS_IDC_DEVUSERERROR: SetErrorDetail(1, (LPSTR)EC_ERR_DevUserErr); break; \
            case WFS_IDC_DEVFRAUDATTEMPT: SetErrorDetail(1, (LPSTR)EC_ERR_DevFraud); break; \
        } \
        return WFS_ERR_HARDWARE_ERROR; \
    }

// 无卡状态检查,有返回
#define CHK_MEDIA_NOHAVE_RET(m) \
    if (m == WFS_IDC_MEDIANOTPRESENT) \
    { \
        Log(ThisModule, __LINE__, "Media Status = %d|MEDIANOTPRESENT, 没有检测到卡, Return: %d.", \
            m, WFS_ERR_IDC_NOMEDIA); \
        SetErrorDetail(1, (LPSTR)EC_ERR_MedNotFound); \
        return WFS_ERR_IDC_NOMEDIA; \
    }

// 无卡状态检查,不返回
#define CHK_MEDIA_NOHAVE(m) \
    if (m == WFS_IDC_MEDIANOTPRESENT) \
    { \
        Log(ThisModule, __LINE__, "Media Status = %d|MEDIANOTPRESENT, 没有检测到卡, Return: %d", \
            m, WFS_ERR_IDC_NOMEDIA); \
    }

// 有卡状态检查,不返回
#define CHK_MEDIA_ISHAVE(m) \
    (m == WFS_IDC_MEDIAPRESENT || \
     m == WFS_IDC_MEDIAJAMMED || \
     m == WFS_IDC_MEDIAENTERING || \
     m == WFS_IDC_MEDIALATCHED)

// 卡JAM状态检查
#define CHK_MEDIA_ISJAM(m) \
    if (m == WFS_IDC_MEDIAJAMMED) \
    { \
        Log(ThisModule, __LINE__, "Media Status = %d|MEDIAJAMMED, JAM, Return: %d", \
            m, WFS_ERR_IDC_MEDIAJAM); \
        SetErrorDetail(1, (LPSTR)EC_ERR_MedJammed); \
        return WFS_ERR_IDC_MEDIAJAM; \
    }

// 回收箱满状态
#define CHK_RETAIN_ISFULL(m) \
    if (m == WFS_IDC_RETAINBINFULL) \
    { \
        Log(ThisModule, __LINE__, "RetainBin = %d|RETAINBINFULL, 回收箱满, Return: %d", \
            m, WFS_ERR_IDC_RETAINBINFULL); \
        FireRetainBinThreshold(WFS_IDC_RETAINBINFULL); \
        SetErrorDetail(1, (LPSTR)EC_ERR_BoxFull); \
        return WFS_ERR_IDC_RETAINBINFULL; \
    }

// 等待用户取卡标志 定义
enum WAIT_TAKEN_FLAG
{
    WTF_NONE            = 0,    // 不等待
    WTF_TAKEN           = 1,    // 等待用户取卡
};

// 回收计数模式 定义
enum EN_RETAIN_CNT
{
    RETCNT_NOACTION     = 0,    // 无操作
    RETCNT_ADD_ONE      = 1,    // 回收计数增1
    RETCNT_RED_ONE      = 2,    // 回收计数减1
    RETCNT_CLEAR        = 3,    // 回收计数清零
    RETCNT_SETSUM       = 4,    // 设置回收上限
};


/***************************************************************************
* 主处理类                                                                  *
***************************************************************************/
class CXFS_IDC : public ICmdFunc, public CLogManage, public CIDCForm,
                 public ConvertVarIDC, public ConvertVarCRM
{
public:
    CXFS_IDC();
    virtual ~CXFS_IDC();
public:
    // 开始运行SP
    long StartRun();

public: // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

public: // IDC类型接口(读卡器)
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps);
    virtual HRESULT GetFormList(LPSTR &lpFormList);
    virtual HRESULT GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm);
    // 执行命令
    virtual HRESULT ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData);
    //virtual HRESULT WriteTrack(const LPWFSIDCWRITETRACK lpWriteData);
    virtual HRESULT EjectCard(DWORD dwTimeOut);
    virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData);
    virtual HRESULT ResetCount();
    //virtual HRESULT SetKey(const LPWFSIDCSETKEY lpKeyData);
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut);
    //virtual HRESULT WriteRawData(const LPWFSIDCCARDDATA *lppCardData);
    virtual HRESULT ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut);
    virtual HRESULT Reset(LPWORD lpResetIn);
    virtual HRESULT ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData);
    virtual HRESULT ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData);
    //virtual HRESULT ReduceCount();
    //virtual HRESULT SetCount(LPWORD lpwCount);
    //virtual HRESULT IntakeCardBack();

protected:  // XFS_IDC_DEC.cpp 子处理相关接口
    HRESULT InnerOpen(BOOL bReConn = FALSE);                        // Open设备及初始化相关子处理
    BOOL LoadDevIDCDll(LPCSTR ThisModule);                          // 加载DevXXX动态库
    INT InitConfig();                                               // 加载INI设置    
    INT PrintIniIDC();                                              // INI配置输出
    void InitStatus();                                              // 状态结构体实例初始化
    void InitCaps();                                                // 能力值结构体实例初始化
    void UpdateExtra();                                             // 更新扩展数据
    WORD UpdateDeviceStatus();                                      // 设备状态实时更新
    HRESULT DeviceIDCInit(BOOL bReConn = FALSE);                    // 读卡器设备初始化
    HRESULT InnerOpenReset();                                       // Open后复位
    HRESULT InnerAcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut);  // 读卡子处理
    HRESULT InnerAcceptAndWriteTrack(DWORD dwReadOption, DWORD dwTimeOut); // 写卡子处理
    HRESULT InnerEject();                                           // 退卡子处理
    HRESULT InnerRetainCard();                                      // 吞卡子处理
    HRESULT InnerReset(WORD wAction, BOOL bIsCheck = TRUE);         // 复位子处理

private:    // XFS_IDC_DEC.cpp 数据处理相关接口
    INT UpdateRetainCards(EN_RETAIN_CNT enMode, INT nCnt = 0);      // 更新吞卡计数相关
    void SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);  // 读卡应答数据处理
    bool GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho);// 读卡应答数据处理
    HRESULT SPParseData(SP_IDC_FORM *pForm);                        // 用指定FORM分析磁道数据
    INT SetErrorDetail(WORD wDevType = 0, LPSTR lpCode = nullptr);  // 设置ErrorDetail错误码

private:    // XFS_IDC_FIRE.cpp 事件消息子处理相关接口
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireCardInserted();
    void FireMediaRemoved();
    void FireMediaRetained();
    void FireMediaDetected(WORD ResetOut);
    void FireRetainBinThreshold(WORD wReBin);
    void FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);

private:    // 读卡器相关变量
    STINICONFIG                     m_stConfig;                     // INI结构体
    CQtDLLLoader<IDevIDC>           m_pIDCDev;                      // DevXXX调用实例
    CQtDLLLoader<IDevSIU>           m_pSIUDev;                      // DevXXX调用实例
    CQtDLLLoader<ISPBaseIDC>        m_pBase;
    CSimpleMutex                    *m_pMutexGetStatus;
    CXfsRegValue                    m_cXfsReg;
    INT                             m_nRetErrOLD[8];                // 处理错误值保存(0:IDC断线重连)
    WAIT_TAKEN_FLAG                 m_enWaitTaken;                  // Taken标记
    CExtraInforHelper               m_cStatExtra;                   // 状态扩展数据
    CExtraInforHelper               m_cCapsExtra;                   // 能力值扩展数据
    CWfsIDCStatus                   m_stStatus;                     // 全局状态
    CWfsIDCStatus                   m_stStatusOLD;                  // 上一次备份全局状态
    BOOL                            m_bChipPowerOff;                // 是否上电标记
    BOOL                            m_bAfteIsHaveCard;              // 后出口是否有卡
    CErrorDetail                    m_clErrorDet;                   // ErrorDetail处理类实例

private:    // 读卡器命令应答数据相关
    CMultiMultiString               m_TrackData;                    // 磁道数据
    CWFSIDCCardDataPtrArray         m_CardDatas;                    // 封装的LPLFSIDCCARDDATA数组
    WFSIDCRETAINCARD                m_stWFSIdcRetainCard;           // 吞卡命令应答
    CWFSChipIO                      m_ChipIO;                       // ChipIO命令应答数据
    CWFSChipPower                   m_ChipPower;                    // ChipPoqwe返回的ATR数据
    WFSIDCCAPS                      m_stCaps;                       // 能力值结构体
    WFSIDCRETAINCARD                m_CardRetain;

//--------------------以下为 退卡模块(CRM) 相关定义--------------------
public: // XFS_CRM.cpp CRM类型接口(退卡模块)
    virtual HRESULT CMEjectCard(LPCSTR lpszCardNo);                 // 退卡模块(CRM)-指定卡号退卡
    virtual HRESULT CMSetCardData(LPCSTR lpszCardNo);               // 退卡模块(CRM)-执行CMRetainCard前设置收卡卡号
    virtual HRESULT CMRetainCard();                                 // 退卡模块(CRM)-执行收卡/暂存
    virtual HRESULT CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118]);// 退卡模块(CRM)-获取状态
    virtual HRESULT CMReduceCount();                                // 退卡模块(CRM)-设置读卡器回收盒最大计数
    virtual HRESULT CMSetCount(LPWORD lpwCount);                    // 退卡模块(CRM)-读卡器回收盒计数减1
    virtual HRESULT CMEmptyCard(LPCSTR lpszCardBox);                // 退卡模块(CRM)-吞卡到读卡器回收盒
    virtual HRESULT CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024]);// 退卡模块(CRM)-获取吞卡时间
    virtual HRESULT CMReset();                                      // 退卡模块(CRM)-设备复位
    virtual HRESULT CMEmpytAllCard();                               // 退卡模块(CRM)-所有卡吞到读卡器回收盒
    virtual HRESULT CMClearSlot(LPCSTR lpszSlotNo);                 // 退卡模块(CRM)-清除指定卡槽信息

private:    // 退卡模块(CRM)相关变量
    STCRMINICONFIG                  m_stCRMConfig;                  // INI结构体(CRM)
    CQtDLLLoader<IDevCRM>           m_pCRMDev;                      // DevCRM实例
    BOOL                            m_bCRMIsOnLine;                 // 是否连线
    BOOL                            m_bIsSetCardData;               // RetainCard前需要执行SetCardData命令
    CHAR                            m_szStorageCardNo[128];         // 吞卡卡卡号
    INT                             m_CardReaderEjectFlag;          // 读卡器退卡执行标记
    BOOL                            m_bThreadEjectExit;
    std::thread                     m_thRunEjectWait;
    CExtraInforHelper               m_cCRMStatExtra;                // 状态扩展数据
    CExtraInforHelper               m_cCRMCapsExtra;                // 能力值扩展数据

protected:  // 退卡模块(CRM)相关接口
    HRESULT StartOpenCRM();                                         // OpenCRM命令入口    
    HRESULT EndCloseCRM();                                          // CloseCRM命令入口
    HRESULT InnerOpenCRM(BOOL bReConn = FALSE);                     // OpenCRM设备及初始化相关子处理
    BOOL    LoadCRMDevDll(LPCSTR ThisModule);                       // 加载CRM动态库
    INT     InitConfigCRM();                                        // 加载CRM_INI设置
    INT     InitCRM();                                              // 变量初始化
    INT     PrintIniCRM();                                          // INI配置输出
    HRESULT InnerCMEmptyCard(INT nSlotNo);                          // 指定卡槽号退卡回收
    void    ThreadEject_Wait();

//--------------------以下为 SIU模块(异物检知) 相关定义--------------------
public: // XFS_SIU.cpp
    HRESULT StartOpenSIU();                                         // OpenSIU命令入口
    HRESULT EndCloseSIU();                                          // CloseSIU命令入口
    HRESULT InnerOpenSIU(BOOL bReConn = FALSE);                     // Open设备及初始化相关子处理
    BOOL    LoadSIUDevDll(LPCSTR ThisModule);                       // 加载DevSIU动态库
    INT     InitSIU();                                              // 变量初始化
    void    ThreadSkimming();                                       // 异物检知处理进程
    INT     SendSkimmingNoticeToDev(BOOL bIsHave);                  // 发送异物检知通知到DevXXX

private:
    BOOL                            m_bSIUIsOnLine;                 // SIU模块是否连线
    std::thread                     m_thRunSkimming;                // 异物检知进程句柄
    BOOL                            m_bThreadSkiExit;               // 通知异物检知进程退出
    BOOL                            m_bIsHaveSkimming;              // 异物检知是否有异物发现
};

// -------------------------------------- END --------------------------------------
