#pragma once
//#include "LogWrite.h"
#include "SimpleMutex.h"
//#include "DevDLLLoader.h"
//#include "AutoHelpClass.h"
#include "ISPBaseCRS.h"
#include "IBRMAdapter.h"
#include "XfsRegValue.h"
#include "XfsSPIHelper.h"
#include "CDMResult.h"
#include "CIMResult.h"
#include "BRMCONFIGPARAM.h"
#include "CashUnitDefine.h"
#include "ICashUnitManager.h"
#include "BRMCASHUNITINFOR.h"
#include "BRMPRESENTINFOR.h"
#include "SPUtil.h"
#include "AutoSaveDataAndFireEvent.h"
#include "AutoUpdateCassData.h"
#include "AutoUpdateCassStatusAndFireCassStatus.h"
#include "AutoDeleteArray.h"
#include "ExtraInforManager.h"
#include "IMixManager.h"
#include "StatusConvertor.h"
#include "AdapterDLL.h"
#include "brmcashacceptor.h"    //test#5
#include "IAllDevPort.h"        //test#11
#include "QtTypeInclude.h"      //test#11

//等待用户取钞的标志
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待用户取钞
    WTF_CDM = 1,    //CDM等待用户取PRESENT钞票
    WTF_CIM = 2,    //CIM等待用户取INPUTREFUSE、Rollback钞票
};

enum FIND_CASS_ADP_INDEX_FLAG
{
    FCAIF_RETRACT = 0,  //回收箱
    FCAIF_REJECT,       //拒钞箱
    FCAIF_RECYCLE_CASHIN//循环箱或进钞箱
};

//////////////////////////////////////////////////////////////////////////
class XFS_CRSImp : public ICmdFunc, public CLogManage

{
public:
    XFS_CRSImp();
    virtual ~XFS_CRSImp();
public:
    // 开始运行SP
    long StartRun();

protected:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    //
    virtual void SetExeFlag(DWORD dwCmd, BOOL bInExe);
    virtual int  GetTooManeyFlag();  //test#7
    // CDM类型接口
    // 查询命令
    virtual HRESULT CDMGetStatus(LPWFSCDMSTATUS &lpStatus);
    virtual HRESULT CDMGetCapabilities(LPWFSCDMCAPS &lpCaps);
    virtual HRESULT CDMGetCashUnitInfo(LPWFSCDMCUINFO &lpCUInfor);
    virtual HRESULT CDMGetMixType(LPWFSCDMMIXTYPE *&lppMixType);
    virtual HRESULT CDMGetMixTable(USHORT usMixNumber, LPWFSCDMMIXTABLE &lpMixTable);
    virtual HRESULT CDMGetPresentStatus(WORD wPos, LPWFSCDMPRESENTSTATUS &lpPresentStatus);
    virtual HRESULT CDMGetCurrencyEXP(LPWFSCDMCURRENCYEXP *&lppCurrencyExp);
    // 执行命令
    virtual HRESULT CDMDenominate(const LPWFSCDMDENOMINATE lpDenoData, LPWFSCDMDENOMINATION &pDenoInOut);
    virtual HRESULT CDMDispense(const LPWFSCDMDISPENSE lpDisData, LPWFSCDMDENOMINATION &lpDenoOut);
    virtual HRESULT CDMPresent(WORD wPos);
    virtual HRESULT CDMReject();
    virtual HRESULT CDMRetract(const WFSCDMRETRACT &stData);
    virtual HRESULT CDMOpenShutter(WORD wPos);
    virtual HRESULT CDMCloseShutter(WORD wPos);
    virtual HRESULT CDMStartEXChange(const LPWFSCDMSTARTEX lpData, LPWFSCDMCUINFO &lpCUInfor);
    virtual HRESULT CDMEndEXChange(const LPWFSCDMCUINFO lpCUInfo);
    virtual HRESULT CDMReset(const LPWFSCDMITEMPOSITION lpResetIn);
    virtual HRESULT CDMSetCashUnitInfo(const LPWFSCDMCUINFO lpCUInfo);

    // CIM类型接口
    // 查询命令
    virtual HRESULT CIMGetStatus(LPWFSCIMSTATUS &lpStatus);
    virtual HRESULT CIMGetCapabilities(LPWFSCIMCAPS &lpCaps);
    virtual HRESULT CIMGetCashUnitInfo(LPWFSCIMCASHINFO &lpCUInfo);
    virtual HRESULT CIMGetBankNoteType(LPWFSCIMNOTETYPELIST &lpNoteList);
    virtual HRESULT CIMGetCashInStatus(LPWFSCIMCASHINSTATUS &lpCashInStatus);
    virtual HRESULT CIMGetCurrencyEXP(LPWFSCIMCURRENCYEXP *&lppCurrencyExp);
    // 执行命令
    virtual HRESULT CIMCashInStart(const LPWFSCIMCASHINSTART lpData);
    virtual HRESULT CIMCashIn(LPWFSCIMNOTENUMBERLIST &lpResult);
    virtual HRESULT CIMCashInEnd(LPWFSCIMCASHINFO &lpCUInfo);
    virtual HRESULT CIMCashInRollBack(LPWFSCIMCASHINFO &lpCUInfo);
    virtual HRESULT CIMRetract(const LPWFSCIMRETRACT lpData);
    virtual HRESULT CIMOpenShutter(WORD wPos);
    virtual HRESULT CIMCloseShutter(WORD wPos);
    virtual HRESULT CIMStartEXChange(const LPWFSCIMSTARTEX lpStartEx, LPWFSCIMCASHINFO &lpCUInfor);
    virtual HRESULT CIMEndEXChange(const LPWFSCIMCASHINFO lpCUInfo);
    virtual HRESULT CIMReset(const LPWFSCIMITEMPOSITION lpResetIn);
    virtual HRESULT CIMConfigureCashInUnits(const LPWFSCIMCASHINTYPE *lppCashInType);
    virtual HRESULT CIMConfigureNoteTypes(const LPUSHORT lpusNoteIDs);
    virtual HRESULT CIMSetCashUnitInfo(const LPWFSCIMCASHINFO lpCUInfo);
    virtual HRESULT CIMCashInLimit(const LPWFSCIMCASHINLIMIT lpCUInfo);
private:
    //定义加额外信息回调函数，用于模版函数AddExtraStOrCap中
    typedef long(*PAddExtraFunc)(LPVOID p, LPCSTR lpszKey, LPCSTR lpszValue);
    //内部函数
private:
    // 将SP管理的钞票配置信息保存到配置文件
    //long SaveBanknoteTypes();

    //从当前各变量状态计算整机状态
    WORD ComputeDeviceState(const ADPSTATUSINFOR &ADPStatus, BOOL bCDM) ;

    //从当前设备状态和钞箱状态计算fwDispense状态
    //pbStopByHardwareError:由硬件引起的暂停，默认为NULL。
    WORD ComputeDispenseState(const ADPSTATUSINFOR &ADPStatus, BOOL *pbStopByHardwareError = NULL, BOOL bLog = FALSE, BOOL bCheck = TRUE);

    //从当前设备状态和钞箱状态计算fwAcceptor状态
    WORD ComputeAcceptorState(const ADPSTATUSINFOR &ADPStatus, BOOL bLog = FALSE);

    //校验DENO是否可出，不校验张数为0的钞箱
    //ulCountAlloced，返回已分配的张数
    //返回值：>=0，计算得到的金额；<0，XFS错误码
    long DenominateIsDispensable(ICDMDenomination *pDenoIn, ULONG &ulCountAlloced);

    //校验配钞币种
    long VerifyDenominateCurrency(ICDMDenomination *pDenoIn) const;

    //校验STACKER、通道、门口不为空或SHUTTER打开能够继续操作
    //返回值：0，成功；否则失败，XFS返回码
    long VerifyCanOperatoByStackerTransportShutter(const ADPSTATUSINFOR &ADPStatus, const char *ThisModule, BOOL bCDM);

    //使用算法配钞，如果成功，结果设置到pDenoInOut中，记录结果
    //ulMaxAllocCount: 最大分配张数
    long MixByAlgorithmAndLog(ULONG ulLeftAmount, USHORT usMixNumber, ULONG ulMaxAllocCount, ICDMDenomination *pDenoInOut, const char *ThisModule);

    //检验分配方案，
    //返回：< 0，失败；==0，分配完成；>0，检验成功，但没有分配完成。如果小于等于0，调用者应返回本函数返回码
    //iAmountCalced：返回计算得到的已分配金额
    //ulCountAlloced：返回计算得到的已分配张数
    long VerifyDenominate(USHORT usMixNumber, ICDMDenomination *pDenoInOut, unsigned long &iAmountCalced, ULONG &ulCountAlloced, const char *ThisModule);

    //STACKER、通道、门口或SHUTTER状态是否引起状态STOP
    BOOL AcceptorDispenseStopByStackerTransportShutter(const ADPSTATUSINFOR &ADPStatus, BOOL bCDM) const;

    //在结束交换时，当ulCount=0时清除CDM的ulCount类数据
    void ClearCountWhenEndExchange(LPWFSCDMCUINFO pCDMCUInfo);

    //在结束交换时，当ulCount=0时清除CDM的ulCount类数据
    void ClearCountWhenEndExchange(LPWFSCIMCASHINFO pCIMCUInfo);

    //增加额外信息
    template <class INTERFACE_TYPE>
    long AddExtraStOrCap(INTERFACE_TYPE *pInterface, PAddExtraFunc pFunc)
    {
        if (m_pAdapter != NULL)
        {
            CExtraInforManager eim;
            m_pAdapter->GetExtraInfor(&eim);
            pair<string, string> s2s;
            int nRet = eim.GetFirstRecord(s2s);
            while (nRet == 0)
            {
                long iRet = pFunc(pInterface, s2s.first.c_str(), s2s.second.c_str());
                if (iRet < 0)
                    return iRet;
                nRet = eim.GetNextRecord(s2s);
            }
        }
        return 0;
    }

    //校验TestCashUnit中的lpPosition的合法性
    //long CheckItemPositionlegality(LPWFSCDMITEMPOSITION lpPosition, const char *ThisModule);

    //FIRE钞箱错误并且设置钞箱错误次数，如果wCUError都为正常，不FIRE
    void FireCUErrorAndSetErrorNum(ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE], BOOL bCDM);

    //在Denominate或Dispense中FIREE钞箱错误
    long FireDenoIsDispCUError(ICashUnit *pCU);

    //打开SHUTTER，由接口函数调用
    HRESULT OpenShutter(const char *ThisModule, BOOL bInShutter, BOOL bCDM);

    //打开入金口SHUTTER，由接口函数调用
    HRESULT OpenCashInShutter(const char *ThisModule, BOOL bInShutter, BOOL bCIM);   //test#11

    //关闭SHUTTER，由接口函数调用
    HRESULT CloseShutter(const char *ThisModule, BOOL bInShutter, BOOL bCDM);

    HRESULT OpenAllShutter(const char *ThisModule, BOOL bCDM);
    HRESULT CloseAllShutter(const char *ThisModule, BOOL bCDM);
    //查找回收箱、拒钞箱和循环箱的适配器索引
    //usIndex：输入输出，从1开始，输入时为XFS的索引，输出时适配器索引
    //返回值：>0成功；<0失败
    long FindCassetteADPIndex(FIND_CASS_ADP_INDEX_FLAG eType, BOOL bCDM, USHORT &usIndex) const;

    //得到回收位置及索引
    long GetRetractAreaAndIndex(USHORT usRetractArea, ADP_RETRACTEARE &eRetractArea, USHORT &usIndex, BOOL bCDM, BOOL bReset = FALSE);

    //获取Reset时默认回收区域和索引
    void GetResetDefaultAreaAndIndex(ADP_RETRACTEARE eRetractArea, USHORT &usRetractArea, USHORT &usIndex, BOOL bCDM) const;

    //转换适配层回收位置到XFS规定的位置
    void GetXFSRetractAreaFromADP(ADP_RETRACTEARE eRetractArea, USHORT &usRetractArea, BOOL bCDM) const;

    //回收钞币，由接口函数调用
    HRESULT Retract(const char *ThisModule, WORD fwOutputPosition, USHORT usRetractArea, USHORT usIndex, BOOL bCDM);

    //开始交换，由接口函数调用
    HRESULT StartExchange(const char *ThisModule, USHORT usCount, LPUSHORT lpusCUNumList, ICUInterface *pCUInterface);

    //结束交换，由接口函数调用
    HRESULT EndExchange(const char *ThisModule, LPVOID lpCUInfo, ICUInterface *pCUInterface);

    //设置钞箱信息，由接口函数调用
    HRESULT SetCashUnitInfo(const char *ThisModule, LPVOID lpCUInfo, ICUInterface *pCUInterface);

    //复位，由接口函数调用
    //bInnerCall：指示是否为内部调用，内部不FIRE检视到钞票事件
    HRESULT Reset(const char *ThisModule, LPWFSCDMITEMPOSITION lpResetIn, ICUInterface *pCUInterface, BOOL bInnerCall = FALSE);

    //更新状态到本地变量m_ADPStatus
    //如bUpdateFromHardware，先调用适配器的UpdateStatus，否则只调用GetStatus
    long UpdateStatus(const char *ThisModule, ADPSTATUSINFOR &ADPStatus, BOOL bUpdateFromHardware = FALSE);

    //从钞箱管理器更新钞箱信息到本地变量，并设置到适层
    long UpdateADPCassInfoFromCUManager();

    //读写PRESENT_STATUS
    //BOOL ReadWritePresentFile(ISPPersistData *pPersist);

    //读写CASH_IN_STATUS
    //BOOL ReadWriteCashInFile(ISPPersistData *pPersist);

    //更新回收数到钞箱管理器
    void UpdateRetractCountToCUManager(BOOL bCDM, USHORT usPCUIndex);

    //分析进钞结果字串，保存到NoteIDCountArray
    long AnalyseCountResult(const char arycResult[256], NOTEID_COUNT_ARRAY &NoteIDCountArray, BOOL bAddUnknownIDToArray, BOOL bVerifyNoteID);

    //保存进钞结果到输出变量中
    long SaveCashInEndResult(const NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE], ICIMSetUpdatesOfCU *pUpdates);

    //关门并保存钞币取走
    long CloseShutterAndEnsureItemTaken();

    //禁用不相关的、状态不正常、钞数为0的钞箱
    long DisableUnusedCassettes(BOOL bCDM);

    //从钞箱管理中得到钞箱启用标志
    void GetEnableFromCUManager(BOOL bCDM, BOOL bProhibitRecycleBox, BOOL bEnable[ADP_MAX_CU_SIZE]) const;

    //记录Dispense及Reject数到日志
    void LogDispenseRejectCount(const char *ThisModule, const char *lpszActionName, const ULONG aryulDispenseCount[ADP_MAX_CU_SIZE], const ULONG aryulRejects[ADP_MAX_CU_SIZE]);

    // 根据适配层使用的钞箱数组下标得到对应的CDM逻辑钞箱number, 返回0xffff表示失败
    USHORT MapIndex2NumberCIM(USHORT usIndex);

    //根据适配层索引得到钞箱对象
    BOOL FindCassByADPIndex(USHORT usIndex, ICashUnit *&pLCU);

    //设置NoteID到钞箱数据结构中
    void SetNoteIDToADPCashInfo(ICashUnit *pLCU, BRMCASHUNITINFOR &stCUInfor);

    //得到钞箱币种ID数组（不可在多线程中调用）
    //返回以0结束的币种ID数组
    static LPUSHORT FormatCUNoteIDs(ICashUnit *pCU, const LPWFSCIMNOTETYPELIST pNoteTypeList);

    //得到实际可存的币种列表
    LPWFSCIMNOTETYPELIST GetNoteTypeListDipositable() { return &m_NoteTypeListDepositable; }
    //更新实际可存的币种列表，该函数由CAutoUpdateCassStatusAndFireCassStatus类调用
    void UpdateNoteTypeListDepositable();

    //从适配层得到状态并根据内部状态更新状态
    void GetUpdatedStatus(ADPSTATUSINFOR &ADPStatus);

    //校验出钞数与要求出钞数一致，目前限制多币种时ulDispenseCounts必须与ulCount相同
    BOOL VerifyDispenseCount(const ULONG ulCount[ADP_MAX_CU_SIZE], const ULONG ulDispenseCounts[ADP_MAX_CU_SIZE], const char *pCurrencyID);

    HRESULT OnDenominate(USHORT usTellerID, USHORT usMixNumber, ICDMDenomination *pDenoInOut, BOOL bCheck = TRUE);
    void SetFlagOfCSItemFromTS(BOOL bCSItemsFromTS);
    BOOL  IsOnWaitTaken();

    //初始化适配 层 设置
    long InitAdapterSetting();
    //////////////////////////////////////////////////////////////////////////
    bool LoadAdpDll(LPCSTR ThisModule);
    HRESULT CheckAndLog(BOOL bErrCondition, const char *ThisModule, HRESULT hRes, const char *pLogDesc);
private:
    //FIRE EVENT
    bool FireStatusChanged(WORD wStatus);
    void FireDeviceHardWare();

    //CDM
    void CDMFireSafeDoorOpen();     //安全门打开
    void CDMFireSafeDoorClose();    //安全门关闭
    void CDMFireCashUnitThreshold(const LPWFSCDMCASHUNIT lpCU);   //逻辑钱箱到达最小或最大限制
    void CDMFireCUInfoChanged(const LPWFSCDMCASHUNIT lpCU);       //逻辑或物理钱箱信息发生改变
    void CDMFireCUError(WORD wFail, const LPWFSCDMCASHUNIT lpCU); //钱箱发生故障
    void CDMFireItemTaken(WORD wPos);        //用户从出口取走现金
    void CDMFireCountsChanged(LPUSHORT lpusCUNumList, USHORT usCount);  //仅对Recycle钱箱有效
    void CDMFireNoteError(USHORT usReason);  //有出钞或交换钱箱过程中发现钞票故障
    void CDMFireItemPresented();             //仅在Count方法中调用
    void CDMFireMediaDetected(const LPWFSCDMITEMPOSITION lpItemPosition); //仅在Reset中调用

    //Fire
    void CIMFireSafeDoorOpen();
    void CIMFireSafeDoorClose();
    void CIMFireCashUnitThreshold(const LPWFSCIMCASHIN lpCashIn);
    void CIMFireCUInfoChanged(const LPWFSCIMCASHIN lpCashIn);
    void CIMFireTellerInfoChanged(USHORT usTellerID);//Teller信息发生改变
    void CIMFireCUError(WORD wFail, const LPWFSCIMCASHIN lpCashIn);
    void CIMFireItemTaken();
    void CIMFireCountsChanged(LPUSHORT lpusCUNumList, USHORT usCount);
    void CIMFireInputRefuse(USHORT usReason);
    void CIMFireNoteError(USHORT usReason);
    void CIMFireItemPresented();
    void CIMFireItemsInserted();
    void CIMFireSubCashin(const LPWFSCIMNOTENUMBERLIST lpNoteNumberList);
    void CIMFireMediaDetected(const LPWFSCIMITEMPOSITION lpItemPosition);
    /*
    void CIMFireInfoAvailable(LPWFSCIMITEMINFOSUMMARY *lppItemInfoSummary);
    void CIMFireInsertItems();
    */
private:
    friend class CAutoSaveDataAndFireEvent;
    friend class CAutoUpdateCassData;
    friend class CAutoFireDeviceStatus;
    friend class CAutoUpdateCassStatusAndFireCassStatus;
    friend class CAutoSaveDataToFile;
    friend class CAutoSetTimer;

protected:
    // 更新状态
    bool UpdateStatus();
    // 更新钞口状态
    bool UpdatePostionStatus();

private:
    char                m_szLogType[MAX_PATH];
    CXfsRegValue        m_cXfsReg;
    string              m_strLogicalName;
    string              m_strSPName;
    std::mutex          m_cStatusMutex;
    bool                m_bIsWaitTaken;
    BOOL                m_bAdapterInited;

    string              m_strConfigFile;        //配置文件名,
    string              m_strCashAcceptorConfigFile;        //入金限额配置文件名,//test#5
    CNoteTypeList       m_NoteTypeListDepositable;  //可存的币种列表
    CStatusConvertor    m_StatusConvertor;      //状态转换对象

    BOOL                m_bCDMExchangeActive;   //CDM是否处于交换钱箱事务中
    BOOL                m_bCIMExchangeActive;   //CIM是否处于交换钱箱事务中
    BOOL                m_bCashInActive;        //是否处于存款事务中
    BOOL                m_bCashOutActive;       //是否处于取款事务中，Dispense、Present置为TRUE、Reset、Retract、Reject、Exchange、取钞后或其他进钞动作置为FALSE
    BOOL                m_bInCmdExecute;        //是否处于命令执行中，用于设置BUSY状态
    DWORD               m_dwLastRecoverTime;    //最后修复设备的时间
    DWORD               m_dwAutoRecoverNumber;  //自动修复次数，<=m_Param.GetAutoRecoverNumber()
    BOOL                m_bProhibitRecycleBox;  //存款交易是否禁用循环箱作为CACHE目的
    BOOL                m_bCSItemsFromTS;      //CS中的Items是否来自TS
    DWORD               m_dwCurrentCommand;     //仅当m_bInCmdExecute为TRUE时表示当前正在处理的xfs命令码，否则无意义

    DWORD               m_dwCloseShutterCount;  //已重试关门次数
    BOOL                m_bFirstStartup;
    BRMPRESENTINFOR     m_PresentInfor;         //PRESENT STATUS
    BRMCASHININFOR      m_CashInInfor;          //CASH IN STATUS
    WAIT_TAKEN_FLAG     m_eWaitTaken;//等待钞币取走然后关门的标志
    ULONG               m_ulLastDispenseCount[ADP_MAX_CU_SIZE]; //保存各钞箱的最后出钞数
    USHORT              m_usTellerID;                           //保存TellerId的值   //test#11

private:
    CCDMStatus         m_CDMStatus;        //出钞机状态数据
    CCDMCaps           m_CDMCaps;           //出钞机能力数据

    ADPSTATUSINFOR     m_LastStatus;       //适配层上一次获取的状态
    ADPDEVINFOR        m_adapterinfo;

    //CCDMCurrencyExp      m_CurrencyExp;   //所有出钞机支持的币种信息
    CCDMPresentStatus  m_PresentStatus;  //最近一次送钞给用户的数据
    CCDMDenomination   m_Denomination;   //在Dispense、Denominate中使用的配钞数据
    CCDMMixTypeList    m_MixTypes;      //所有Mix算法和Mix表

    CCIMStatus         m_CIMStatus;        //出钞机状态数据
    CCIMCaps           m_CIMCaps;           //出钞机能力数据
    CCIMBanknoteTypes  m_BanknoteTypes;
    CCIMCashInStatus   m_CashInStatus;
    CCIMCashInResult   m_CashInResult;
    CCIMSetUpdatesOfCU m_UpdateOfCU;

    //IStatusChangeListener *m_pListener;
    CINIFileReader m_ReaderConfigFile;              //test#11
    NOTEID_COUNT_ARRAY NoteIDCountTemp;             //test#13
    WORD wCashInCnt;                            //test#13
    BOOL m_bCanceled;

protected:
    CQtDLLLoader<IBRMAdapter>         m_pAdapter;
    CQtDLLLoader<ISPBaseCRS>          m_pBase;
    //CQtDLLLoader<IMixManager>       m_pMixManager;
    //CQtDLLLoader<ICashUnitManager>  m_pCUManager;
    BRMCONFIGPARAM                    m_Param;                  //SP配置数据
    ICashUnitManager                  *m_pCUManager;            //钞箱管理对象指针
    //CQtDLLLoader<ICashUnitManager>    m_pCUManager;
    IMixManager                       *m_pMixManager;           //配钞算法对象指针
    //CAdapterDLL                     m_pAdapter;               //适配器DLL，在配置文件[BRMInfo]节“AdapterDLLName”配置
    BRMCASHACCEPTORPARAM              m_CashAcceptorParam;      //SP入金限额//test#5
 
public:
    bool m_bCdmCmd;
};

