
#pragma once
#include "IDevIDC.h"
#include "ISPBaseIDC.h"
#include "QtTypeInclude.h"
#include "IDCForm.h"
//////////////////////////////////////////////////////////////////////////

// 防盗嘴
#define  TANMPER_SENSOR_NOT_AVAILABLE       0
#define  TANMPER_SENSOR_OFF                 1
#define  TANMPER_SENSOR_ON

typedef enum
{
    MCM_INC_ONE = 1, // 吞卡数增1
    MCM_DEC_ONE = 2, // 吞卡数减1
    MCM_CLEAR = 3    // 吞卡数清零
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
    bool                 bLedOper;
    string               strComType;
    bool                 bPostRemovedAftEjectFixed;
};

//等待用户取卡标志
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取卡
};



//////////////////////////////////////////////////////////////////////////
class CXFS_FIDC : public ICmdFunc, public CLogManage, public CIDCForm
{
public:
    CXFS_FIDC();
    virtual ~CXFS_FIDC();
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

public:
    void SetDevBusyStatus();                                    //40-00-00-00(FT#0003)
    void UnsetDevBusyStatus();                                  //40-00-00-00(FT#0003)
protected:

    WORD   GetMediaPresentState(IDC_IDCSTAUTS CardStatus);
    const char *ProcessReturnCode(int nCode);
    bool   LoadDevDll(LPCSTR ThisModule);

    long   AcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut);
    long   WaitItemTaken();
    long   UpdateCardStatus();
    void   UpdateDevStatus(int iRet);
    int    Init();
    void   InitStatus();
    void   InitConfig();
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
    void   ControlLED(BYTE byLedCtrl);      //0:关闭 1:打开 2:闪烁

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
    char                              m_szLogType[MAX_PATH];
    BOOL                              m_bNeedRepair;
    DWORD                             m_nResetFailedTimes;
    STIDCConfig                       m_stConfig;
    IDC_IDCSTAUTS                     m_Cardstatus;
    WAIT_TAKEN_FLAG                   m_WaitTaken;
    BOOL                              m_bJamm;
    CXfsRegValue                      m_cXfsReg;
    BOOL                              m_bChipPowerOff;
    BOOL                              m_bMultiCard;
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

    CExtraInforHelper                 m_cStaExtra;
    CExtraInforHelper                 m_cCapExtra;
    friend class CAutoControlLed;

    bool                              m_bCmdExecute;       //设备命令执行中,为BUSY状态  //40-00-00-00(FT#0003)
    bool                              m_bStartCheckCard;   //是否开始检测卡
    bool                              m_bExecChipPoweron;  //是否芯片上过电
};

class CAutoControlLed
{
public:
    CAutoControlLed(CXFS_FIDC *pSP)
    {
        m_pSP = pSP;
        if (m_pSP != nullptr)
        {
            m_pSP->ControlLED(true);
        }
    }
    ~CAutoControlLed()
    {
        if (m_pSP != nullptr)
        {
            m_pSP->ControlLED(false);
        }
    }
private:
    CXFS_FIDC *m_pSP;
};

//40-00-00-00(FT#0003)
class CAutoSetDevBusyStatus
{
public:
    CAutoSetDevBusyStatus(CXFS_FIDC *pSP) : m_pSP(pSP)
    {
        if(m_pSP){
            m_pSP->SetDevBusyStatus();
        }
    }
    ~CAutoSetDevBusyStatus()
    {
        if(m_pSP){
            m_pSP->UnsetDevBusyStatus();
        }
    }
private:
    CXFS_FIDC *m_pSP;
};

//////////////////////////////////////////////////////////////////////////
