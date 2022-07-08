
#pragma once
#include "IDevIDC.h"
#include "IDevCRM.h"    // 退卡模块
#include "IDevCRD.h"    // 发卡模块
#include "ISPBaseIDC.h"
#include "QtTypeInclude.h"
#include "IDCForm.h"
#include "XFS_CRD_DEC.h"

#include <unistd.h>
#include <queue>

//////////////////////////////////////////////////////////////////////////

// 防盗嘴
#define  TANMPER_SENSOR_NOT_AVAILABLE       0
#define  TANMPER_SENSOR_OFF                 1
#define  TANMPER_SENSOR_ON

typedef enum
{
    MCM_INC_ONE = 1, // 吞卡数增1
    MCM_DEC_ONE = 2, // 吞卡数减1
    MCM_CLEAR = 3,   // 吞卡数清零
    MCM_NOACTION = 4 // 无操作

} MODIFY_CARDS_MODE;
//////////////////////////////////////////////////////////////////////////
struct STIDCConfig
{
    CardAction           CardInitAction;
    bool                 bAcceptWhenCardFull;
    bool                 bCanWriteTrack;
    bool                 bFireRmoveWhenRetain;
    bool                 bAutoUpdateToOK;
    bool                 bSuppDeRetractCount;
    bool                 bFluxInActive;
    bool                 bCaptureOnResetFailed;
    bool                 bTamperSensorSupport;
    DWORD                dwTotalTimeOut;
    WORD                 usResetRetryTimes;
    WORD                 usNeedWobble;
    WORD                 usReturnOpenVal;           //30-00-00-00(FT#0019)
    USHORT               usInterProtocal;           // 读卡器通信协议

    STIDCConfig() { Clear(); }
    void Clear() { memset(this, 0x00, sizeof(tag_iomc_idc_param)); }
};

//等待用户取卡标志
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取卡
};
//////////////////////////////////////////////////////////////////////////
// 退卡模块(CRM)相关声明
// INI配置变量结构体
typedef
struct st_crm_ini_config
{
    CHAR        szCRMDeviceDllName[256];    // 退卡模块动态库名
    BOOL        bCRMDeviceSup;              // 是否支持启动退卡模块,缺省F
    WORD        wCRMDeviceType;             // 退卡模块设备类型,缺省0(CRT-730B)
    CHAR        szCRMDeviceConList[24];     // 退卡模块连接字符串
    WORD        wDeviceInitAction;          // 设备初始化动作,缺省3
    WORD        wEjectCardPos;              // 退卡后卡位置，缺省0
    WORD        wEnterCardRetainSup;        // 卡在入口是否支持CMRetain,缺省0

    st_crm_ini_config()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(st_crm_ini_config));
        bCRMDeviceSup = FALSE;
        wDeviceInitAction = 3;
    }
}STCRMINICONFIG, LPSTCRMINICONFIG;

#define CRMDEVTYPE_CHG(a)  (a == CRM_DEV_CRT730B ? ICRM_TYPE_CRT730B : "")

//////////////////////////////////////////////////////////////////////////
class CXFS_IDC : public ICmdFunc, public CLogManage, public CIDCForm
{
public:
    CXFS_IDC();
    virtual ~CXFS_IDC();
public:
    // 开始运行SP
    long StartRun();

public:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // IDC类型接口
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps);
    virtual HRESULT GetFormList(LPSTR &lpFormList);
    virtual HRESULT GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm);
    // 执行命令
    virtual HRESULT ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData);
    virtual HRESULT WriteTrack(const LPWFSIDCWRITETRACK lpWriteData);
    virtual HRESULT EjectCard(DWORD dwTimeOut);
    virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData);
    virtual HRESULT ResetCount();
    virtual HRESULT SetKey(const LPWFSIDCSETKEY lpKeyData);
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut);
    virtual HRESULT WriteRawData(const LPWFSIDCCARDDATA *lppCardData);
    virtual HRESULT ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut);
    virtual HRESULT Reset(LPWORD lpResetIn);
    virtual HRESULT ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData);
    virtual HRESULT ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData);

#ifdef CARD_REJECT_GD_MODE
    //读卡器新增扩展部分
    virtual HRESULT ReduceCount();
    virtual HRESULT SetCount(LPWORD lpwCount);
    virtual HRESULT IntakeCardBack();
    //退卡部分
    virtual HRESULT CMEjectCard(LPCSTR lpszCardNo);                             // 退卡模块(CRM)-指定卡号退卡
    virtual HRESULT CMSetCardData(LPCSTR lpszCardNo);                           // 退卡模块(CRM)-执行CMRetainCard前设置收卡卡号
    virtual HRESULT CMRetainCard();                                             // 退卡模块(CRM)-执行收卡/暂存
    virtual HRESULT CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118]);        // 退卡模块(CRM)-获取状态
    virtual HRESULT CMReduceCount();                                            // 退卡模块(CRM)-设置读卡器回收盒最大计数
    virtual HRESULT CMSetCount(LPWORD lpwCount);                                // 退卡模块(CRM)-读卡器回收盒计数减1
    virtual HRESULT CMEmptyCard(LPCSTR lpszCardBox);                            // 退卡模块(CRM)-吞卡到读卡器回收盒
    virtual HRESULT CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024]);   // 退卡模块(CRM)-获取吞卡时间
    virtual HRESULT CMReset();                                                  // 退卡模块(CRM)-设备复位
#endif

    // 增加 CRD系列命令(Card Dispenser) START
    /* CRD Info Commands */
    virtual HRESULT CRD_GetStatus(LPWFSCRDSTATUS &lpStatus);
    virtual HRESULT CRD_GetCapabilities(LPWFSCRDCAPS &lpCaps);
    virtual HRESULT CRD_GetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit);
    /* CRD Execute Commands */
    virtual HRESULT CRD_DispenseCard(const LPWFSCRDDISPENSE lpDispense);                // 发卡
    virtual HRESULT CRD_EjecdCard();                                                    // 退卡
    virtual HRESULT CRD_RetainCard(const LPWFSCRDRETAINCARD lpRetainCard);              // 回收卡
    virtual HRESULT CRD_Reset(const LPWFSCRDRESET lpResset);                            // 复位
    virtual HRESULT CRD_SetGuidanceLight(const LPWFSCRDSETGUIDLIGHT lpSetGuidLight);    // 设置灯状态
    virtual HRESULT CRD_SetCardUnitInfo(const LPWFSCRDCUINFO lpCuInfo);                 // 设置卡箱信息
    virtual HRESULT CRD_PowerSaveControl(const LPWFSCRDPOWERSAVECONTROL lpPowerCtrl);   // 激活/停用节能功能

protected:

    WORD   GetMediaPresentState(IDC_IDCSTAUTS CardStatus);
    const char *ProcessReturnCode(int nCode);
    bool   LoadDevDll(LPCSTR ThisModule);

    void   UpdateRetainCards(MODIFY_CARDS_MODE mode, bool fire_threshold = true);
    long   AcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut);
    long   WaitItemTaken();
    long   UpdateCardStatus();
    void   UpdateDevStatus(int iRet);
    int    Init();
    void   InitStatus();
    int    InitConfig();
    void   InitCaps();
    int    GetFWVersion();
    long   ReadTrackData(DWORD dwReadOption);
    long   WriteTrackData(DWORD dwWriteOption);
    long   ReadChip();
    long   SPParseData(SP_IDC_FORM *pForm);
    long   OnWriteTrack(WFSIDCWRITETRACK *pWrite);
    void   SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);
    bool   GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho);
    long   Convert2XFSErrCode(long lIDCErrCode);
    long   RetainCard();
    int    SetRecycleCount(LPCSTR pszCount);
    // CRT 退卡，仅返卡模块退卡时使用
    int   MoveCardToMMPosition();

    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireCardInserted();
    void FireMediaRemoved();
    void FireMediaRetained();
    void FireMediaDetected(WORD ResetOut);
    void FireRetainBinThreshold(WORD wReBin);
    void FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);

private:
    BOOL                              m_bNeedRepair;
    DWORD                             m_dwTamperSensorStatus;
    DWORD                             m_nResetFailedTimes;
    STIDCConfig                       m_stConfig;
    IDC_IDCSTAUTS                     m_Cardstatus;
    WAIT_TAKEN_FLAG                   m_WaitTaken;
    BOOL                              m_bJamm;
    BOOL                              m_bICCActived;
    //
    CQtDLLLoader<IDevIDC>             m_pDev;
    CQtDLLLoader<ISPBaseIDC>          m_pBase;
    //返回数据
    CWfsIDCStatus                     m_Status;
    WFSIDCCAPS                        m_Caps;
    CWFSIDCCardDataPtrArray           m_CardDatas;
    WFSIDCRETAINCARD                  m_CardRetain;
    CWFSChipIO                        m_ChipIO;
    CMultiMultiString                 m_TrackData;              //最后的磁道数据
    CWFSChipPower                     m_ChipPowerRes;       // chip power返回的ATR数据
    CExtraInforHelper                 m_cExtra;
    CSimpleMutex                     *m_pMutexGetStatus;
    string                            m_strDevName;
    string                            m_strPort;                //30-00-00-00(FT#0019)

private:    // 退卡模块(CRM)相关变量
    STCRMINICONFIG                    m_stCRMINIConfig;
    CQtDLLLoader<IDevCRM>             m_pCRMDev;
    BOOL                              m_bIsSetCardData;         // RetainCard前需要执行SetCardData命令
    CHAR                              m_szStorageCardNo[128];   // 收卡卡号
    INT                               m_CardReaderEjectFlag;    // 读卡器退卡执行标记
    BOOL                              m_bThreadEjectExit;
    std::thread                       m_thRunEjectWait;

protected:  // 退卡模块(CRM)相关函数
    bool   LoadCRMDevDll(LPCSTR ThisModule);
    long   CRMErr2XFSErrCode(long lCRMErrCode);     // CRM ErrCode 转换为 XFS ErrCode
    void   ThreadEject_Wait();
    BOOL   IsRetainCardFull();                      // 读卡器回收盒是否FULL

private:    // 发卡模块(CRD)相关变量
    STCRDINICONFIG                    m_stCRDINIConfig;     // CRD模块INI
    CQtDLLLoader<IDevCRD>             m_pCRDDev;            // CRD模块连接
    CWfsCRDStatus                     m_CRDStatus;          // CRD模块当前状态(XFS)
    CWfsCRDStatus                     m_CRDStatusOld;       // CRD模块上一次状态(XFS)
    CWfsCRDCaps                       m_CRDCaps;            // CRD模块能力值
    STDISPBOXLIST                     m_stCRDBoxList;       // CRD模块卡箱信息列表
    STDISPBOXLIST                     m_stCRDBoxListOld;    // CRD模块卡箱信息列表上一次记录
    CExtraInforHelper                 m_cCRDExtra;          // CRD模块扩展信息类变量
    WAIT_TAKEN_FLAG                   m_CRDWaitTaken;       // CRD模块Take标记

protected:  // 发卡模块(CRD)相关函数
    void    InitCRDStatus();
    void    InitCRDCaps();
    int     InitCRDConfig();                            // 发卡模块INI配置参数获取
    void    UpdateDevCRDStatus(LPSTCRDDEVSTATUS lpStat = nullptr);// 取CRD状态
    bool    LoadCRDDevDll(LPCSTR ThisModule);           // 加载CRD模块处理动态库
    long    CRDErr2XFSErrCode(INT nCRDErrCode);         // CRD ErrCode 转换为 XFS ErrCode
    long    IDCErr2CRDXFSErrCode(long lCode);           // IDC ErrCode 转换为 CRD XFS ErrCode
    LONG    ConvertCRDDeviceStatus(WORD wDevStat);      // 设备状态转换为WFS格式
    LONG    ConvertCRDDispensrStatus(WORD wStat);       // 总单元状态转换为WFS格式
    LONG    ConvertCRDTransportStatus(WORD wStat);      // 传送模块状态转换为WFS格式
    LONG    ConvertCRDMediaStatus(WORD wStat);          // 介质状态转换为WFS格式
    LONG    ConvertCRDShutterStatus(WORD wStat);        // 门状态转换为WFS格式
    LONG    ConvertCRDDevicePosStatus(WORD wStat);      // 指定设备位置转换为WFS格式
    LONG    ConvertCRDAntiFraudStatus(WORD wStat);      // 反欺诈模块状态转换为WFS格式
    LONG    ConvertCRDUnitInfoStatus(WORD wStat);       // 单个单元状态转换为WFS格式
    int     SetDispBoxCfg(USHORT usBoxNo, ULONG ulData, USHORT usType);// 设置INI中卡箱单元信息
    int     SetDispBoxCfg(USHORT usBoxNo, LPSTR lpData, USHORT usType);// 设置INI中卡箱单元信息
    int     UpdateRetain2BoxDisp();                     // 更新回收计数和状态到卡单元结构体 // 40-00-00-00(FT#0018)
    void    CardUnitInfoPack(LPWFSCRDCARDUNIT lpUnit, USHORT usBoxNo);// 根据卡单元索引号赋值卡信息结构体
    void    CRD_FireUnitThresHold_Pack();               // EVENT封装: WFS_USRE_CRD_CARDUNITTHRESHOLD
    void    CRD_FireUnitInfoChanged_Pack();             // EVENT封装: WFS_SRVE_CRD_CARDUNITINFOCHANGED
    void    CRD_FireUnitError_Pack(USHORT usBoxNo, LONG lCode);// EVENT封装: WFS_EXEE_CRD_CARDUNITERROR

protected:  // 发卡模块(CRD)事件上报相关函数
    void    CRD_FireMediaRemoved();                                     // 上报 EVENT: WFS_SRVE_CRD_MEDIAREMOVED
    void    CRD_FireMediaDetected(LPWFSCRDMEDIADETECTED lpMediaDet);    // 上报 EVENT: WFS_SRVE_CRD_MEDIADETECTED
    void    CRD_FireUnitTheshold(LPWFSCRDCARDUNIT lpCardUnit);          // 上报 EVENT: WFS_USRE_CRD_CARDUNITTHRESHOLD
    void    CRD_FireUnitInfoChanged(LPWFSCRDCARDUNIT lpCardUnit);       // 上报 EVENT: WFS_SRVE_CRD_CARDUNITINFOCHANGED
    void    CRD_FireUnitError(LPWFSCRDCUERROR lpCardUnitError);         // 上报 EVENT: WFS_EXEE_CRD_CARDUNITERROR
    void    CRD_FireDEvicePosition(LPWFSCRDDEVICEPOSITION lpDevicePosition);// 上报 EVENT: WFS_SRVE_CRD_DEVICEPOSITION
};


//////////////////////////////////////////////////////////////////////////
