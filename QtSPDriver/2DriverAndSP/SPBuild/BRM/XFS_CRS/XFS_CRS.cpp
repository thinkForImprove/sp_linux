#include "XFS_CRS.h"
#include "INIFileReader.h"

static const char *ThisFile = "XFS_CRS";
static const char *DEVTYPE  = "CRS";

//////////////////////////////////////////////////////////////////////////
//检验经果，如果小于0，记录日志并返回
#define VERIFY_RESULT(func_name, iRet) \
    if ((iRet) < 0)\
    {\
        Log(ThisModule, (iRet), "%s failed", func_name);\
        return iRet;\
    }

//检验经果，如果为NULL，记录日志并返回
#define VERIFY_RESULT_PTR(func_name, ptr) \
    if ((ptr) == NULL)\
    {\
        Log(ThisModule, -1, "%s return NULL", func_name);\
        return WFS_ERR_INTERNAL_ERROR;\
    }

//校验错误条件
//如bErrCondition为真，记录pErrorDesc日志，并返回ErrorCode错误码
#define VERIFY_ERR_CONDITION(bErrCondition, ErrorCode, pErrorDesc) \
    {\
        HRESULT hRes = CheckAndLog((bErrCondition), ThisModule, (ErrorCode), (pErrorDesc));\
        if (hRes != 0)\
            return hRes;\
    }

//检查条件并记录日志
HRESULT XFS_CRSImp::CheckAndLog(BOOL bErrCondition, const char *ThisModule,
                                HRESULT hRes, const char *pLogDesc)
{
    if (bErrCondition)
    {
        Log(ThisModule, -1, pLogDesc);
        return hRes;
    }
    return 0;
}

//30-00-00-00(FT#0026)
HRESULT XFS_CRSImp::CheckDenominateParam(const LPWFSCDMDENOMINATE lpDenoData)
{
    int iTotalAmountByEachBox = 0;
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();

    //计算各钞箱张数列表对应出钞总金额
    if(lpDenoData->lpDenomination->usCount > 0 && lpDenoData->lpDenomination->lpulValues != nullptr){
        for(int i = 0; i < pCDM->GetCUCount() && i < lpDenoData->lpDenomination->usCount; i++){
            ICashUnit *pCashUnit = pCDM->GetCUByNumber(i + 1);
            ADP_CASSETTE_TYPE eCstType = pCashUnit->GetType();
            //可以出钞的只有出金箱和循环箱
            if(eCstType == ADP_CASSETTE_BILL || eCstType == ADP_CASSETTE_RECYCLING){
                iTotalAmountByEachBox += pCashUnit->GetValues() * lpDenoData->lpDenomination->lpulValues[i];
            } else {

            }
        }
    }

    if(lpDenoData->usMixNumber == WFS_CDM_INDIVIDUAL){
        if(lpDenoData->lpDenomination->ulAmount == 0 &&
           lpDenoData->lpDenomination->usCount > 0 &&
           iTotalAmountByEachBox == 0){
            return WFS_ERR_INVALID_DATA;
        }
    }

    if(lpDenoData->usMixNumber == WFS_CDM_MIX_MINIMUM_NUMBER_OF_BILLS){
        if(lpDenoData->lpDenomination->ulAmount != 0 &&
           lpDenoData->lpDenomination->usCount > 0 &&
           lpDenoData->lpDenomination->ulAmount != iTotalAmountByEachBox){
            return WFS_ERR_CDM_INVALIDDENOMINATION;
        }
    }

    return WFS_SUCCESS;
}

//30-00-00-00(FT#0026)
HRESULT XFS_CRSImp::CheckDispenseParam(const LPWFSCDMDISPENSE lpDisData)
{
    WFSCDMDENOMINATE stCdmDenominate = {0};
    stCdmDenominate.usTellerID = lpDisData->usTellerID;
    stCdmDenominate.usMixNumber = lpDisData->usMixNumber;
    stCdmDenominate.lpDenomination = lpDisData->lpDenomination;
    return CheckDenominateParam(&stCdmDenominate);
}

//30-00-00-00(FT#0042)
void XFS_CRSImp::RelieveBoxManipStatus()
{
    BOOL bNeedUpdateStatus = FALSE;
    ICashUnit *pCU = nullptr;
    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    USHORT usCIMCuCount = pCIM->GetCUCount();
    USHORT usCDMCuCount = pCDM->GetCUCount();
    for (int i = 0; i < usCIMCuCount; i++)
    {
        pCU = pCIM->GetCUByNumber(i + 1);
        if(pCU->GetStatus() == ADP_CASHUNIT_MANIP){
            pCU->SetExchangeState(TRUE);
            bNeedUpdateStatus = TRUE;
        }
    }
    for(int i = 0; i < usCDMCuCount; i++){
        pCU = pCDM->GetCUByNumber(i + 1);
        if(pCU->GetStatus() == ADP_CASHUNIT_MANIP){
            pCU->SetExchangeState(TRUE);
            bNeedUpdateStatus = TRUE;
        }
    }
    if(bNeedUpdateStatus){
        UpdateStatus();
        for (int i = 0; i < usCIMCuCount; i++)
        {
            pCU = pCIM->GetCUByNumber(i + 1);
            pCU->SetExchangeState(FALSE);
        }
        for(int i = 0; i < usCDMCuCount; i++){
            pCU = pCDM->GetCUByNumber(i + 1);
            pCU->SetExchangeState(FALSE);
        }
    }

    return;
}

//在币种类型列表pNoteTypeList中查找usID，存在返回索引，否则返回-1
int FindNoteIDInNoteTypeList(USHORT usID, const LPWFSCIMNOTETYPELIST pNoteTypeList)
{
    assert(pNoteTypeList != NULL);
    for (USHORT i = 0; i < pNoteTypeList->usNumOfNoteTypes; i++)
    {
        assert(pNoteTypeList->lppNoteTypes[i] != NULL);
        if (pNoteTypeList->lppNoteTypes[i]->usNoteID == usID)
            return i;
    }

    return -1;
}

//在币种ID列表lpusNoteIDs中查找usID，存在返回TRUE
inline BOOL FindNoteIDInList(USHORT usID, LPUSHORT lpusNoteIDs)
{
    LPUSHORT pID = lpusNoteIDs;
    while (*pID != 0)
    {
        if (usID == *pID)
            return TRUE;
        pID++;
    }
    return FALSE;
}

template <class CurrencyExpInterface, class XFSTYPE>
HRESULT GetCurrencyExp(const char *ThisModule, CurrencyExpInterface *pInterface, LPBRMCONFIGPARAM lpParam)
{
    assert(lpParam != NULL &&
           lpParam->GetCurrencyExp() != NULL);

    XFSTYPE **pList = (XFSTYPE **)lpParam->GetCurrencyExp();
    assert(pList != NULL);
    XFSTYPE *pType = *pList;

    while (pType != NULL)
    {
        pInterface->AddCurrencyExp(*pType);
        pList++;
        pType = *pList;
    }
    return 0;
}

//翻译适配层拒钞原因错误码为XFS错误码
inline USHORT RejectCauseADP2XFS(ADP_ERR_REJECT eRejectCause)
{
    switch (eRejectCause)
    {
    case ADP_ERR_REJECT_OK:
        return 0;

    case ADP_ERR_REJECT_INVALIDBILL:
        return WFS_CIM_INVALIDBILL;

    case ADP_ERR_REJECT_STACKERFULL:
        return WFS_CIM_STACKERFULL;

    case ADP_ERR_REJECT_NOBILL:
        return WFS_CIM_NOBILLSTODEPOSIT;

    default:
        //return WFS_CIM_DEPOSITFAILURE;
        return WFS_CIM_INVALIDBILL;
    }
}

//将TransPort的ADP转换为XFS
inline WORD TransportADPToXFS(DWORD fwTransport, BOOL bCDM)
{
    if (fwTransport == ADP_TRANSPORT_EMPTY ||
        fwTransport == ADP_TRANSPORT_NOTEMPTY)
    {
        return bCDM ? WFS_CDM_TPOK : WFS_CIM_TPOK;
    }
    else if (fwTransport == ADP_TRANSPORT_NOTSUPPORTED)
    {
        return bCDM ? WFS_CDM_TPNOTSUPPORTED : WFS_CIM_TPNOTSUPPORTED;
    }
    else
    {
        return bCDM ? WFS_CDM_TPUNKNOWN : WFS_CIM_TPUNKNOWN;
    }
}

//转换ADP钞箱错误码为XFS钞箱错误码
inline WORD ADPCUError2XFS_CDM(ADP_CUERROR eError)
{
    switch (eError)
    {
    case ADP_CUERROR_EMPTY:     return WFS_CDM_CASHUNITEMPTY;
    case ADP_CUERROR_FULL:      return WFS_CDM_CASHUNITFULL;
    case ADP_CUERROR_RETRYABLE: return WFS_CDM_CASHUNITERROR;
    case ADP_CUERROR_FATAL:     return WFS_CDM_CASHUNITERROR;
    case ADP_CUERROR_CONTINUEREJECT:                            //30-00-00-00(FS#0020)
    case ADP_CUERROR_TOOMANYREJECT:                             //30-00-00-00(FS#0020)
    case ADP_CUERROR_MISSFEED:                                  //30-00-00-00(FS#0020)
         return WFS_CDM_CASHUNITERROR;                          //30-00-00-00(FS#0020)
    case ADP_CUERROR_OK:
    case ADP_CUERROR_NOTUSED:
    default:
        assert(false);
        return -1;
    }
}

inline WORD ADPCUError2XFS_CIM(ADP_CUERROR eError)
{
    switch (eError)
    {
    case ADP_CUERROR_EMPTY: return WFS_CIM_CASHUNITEMPTY;
    case ADP_CUERROR_FULL: return WFS_CIM_CASHUNITFULL;
    case ADP_CUERROR_RETRYABLE: return WFS_CIM_CASHUNITERROR;
    case ADP_CUERROR_FATAL: return WFS_CIM_CASHUNITERROR;

    case ADP_CUERROR_OK:
    case ADP_CUERROR_NOTUSED:
    default:
        assert(false);
        return -1;
    }
}
//检查回收区域是否合法
static inline long CheckRetractArea(USHORT usRetractArea, WORD wRetractAreas, BOOL bReset = FALSE)
{
    long iRet;
    if (usRetractArea == WFS_CIM_RA_RETRACT ||
        usRetractArea == WFS_CIM_RA_TRANSPORT ||
        usRetractArea == WFS_CIM_RA_STACKER ||
        usRetractArea == WFS_CIM_RA_BILLCASSETTES)
    {
        if ((usRetractArea & wRetractAreas) == 0)
        {
            iRet = bReset ? WFS_ERR_UNSUPP_DATA : WFS_ERR_CIM_NOTRETRACTAREA;
            return iRet;
        }
        return 0;
    }
    return WFS_ERR_INVALID_DATA;
}

//检查CIM钞口是否合法
static inline long CheckCIMPos(WORD wPos, WORD fwPositions)
{
    if (wPos == WFS_CIM_POSNULL)
    {
        return 0;
    }

    if (wPos == WFS_CIM_POSINLEFT ||
        wPos == WFS_CIM_POSINRIGHT ||
        wPos == WFS_CIM_POSINCENTER ||
        wPos == WFS_CIM_POSINTOP ||
        wPos == WFS_CIM_POSINBOTTOM ||
        wPos == WFS_CIM_POSINFRONT ||
        wPos == WFS_CIM_POSINREAR ||
        wPos == WFS_CIM_POSOUTLEFT ||
        wPos == WFS_CIM_POSOUTRIGHT ||
        wPos == WFS_CIM_POSOUTCENTER ||
        wPos == WFS_CIM_POSOUTTOP ||
        wPos == WFS_CIM_POSOUTBOTTOM ||
        wPos == WFS_CIM_POSOUTFRONT ||
        wPos == WFS_CIM_POSOUTREAR)
    {
        if ((wPos & fwPositions) == 0)
        {
            return WFS_ERR_CIM_UNSUPPOSITION;
        }
        return 0;
    }
    return WFS_ERR_INVALID_DATA;
}

//检查CIM入钞口是否合法
static inline long CheckCIMInputPos(WORD wPos, WORD fwPositions)
{
    if (wPos == WFS_CIM_POSNULL)
    {
        return 0;
    }

    if (wPos == WFS_CIM_POSINLEFT ||
        wPos == WFS_CIM_POSINRIGHT ||
        wPos == WFS_CIM_POSINCENTER ||
        wPos == WFS_CIM_POSINTOP ||
        wPos == WFS_CIM_POSINBOTTOM ||
        wPos == WFS_CIM_POSINFRONT ||
        wPos == WFS_CIM_POSINREAR)
    {
        if ((wPos & fwPositions) == 0)
        {
            return WFS_ERR_CIM_UNSUPPOSITION;
        }
        return 0;
    }
    return WFS_ERR_INVALID_DATA;
}

//检查输入钞口是否合法
static inline long CheckCIMOutputPos(WORD wPos, WORD fwPositions)
{
    if (wPos == WFS_CIM_POSNULL)
    {
        return 0;
    }

    if (wPos == WFS_CIM_POSOUTLEFT ||
        wPos == WFS_CIM_POSOUTRIGHT ||
        wPos == WFS_CIM_POSOUTCENTER ||
        wPos == WFS_CIM_POSOUTTOP ||
        wPos == WFS_CIM_POSOUTBOTTOM ||
        wPos == WFS_CIM_POSOUTFRONT ||
        wPos == WFS_CIM_POSOUTREAR)
    {
        if ((wPos & fwPositions) == 0)
        {
            return WFS_ERR_CIM_UNSUPPOSITION;
        }
        return 0;
    }
    return WFS_ERR_INVALID_DATA;
}

//检查输出钞口是否合法
static inline long CheckOutputPos(WORD wPos, WORD fwPositions)
{
    if (wPos == WFS_CDM_POSNULL)
    {
        return 0;
    }

    if (wPos == WFS_CDM_POSLEFT ||
        wPos == WFS_CDM_POSRIGHT ||
        wPos == WFS_CDM_POSCENTER ||
        wPos == WFS_CDM_POSTOP ||
        wPos == WFS_CDM_POSBOTTOM ||
        wPos == WFS_CDM_POSREJECT ||
        wPos == WFS_CDM_POSFRONT ||
        wPos == WFS_CDM_POSREAR)
    {
        if ((wPos & fwPositions) == 0)
        {
            return WFS_ERR_CDM_UNSUPPOSITION;
        }
        return 0;
    }
    return WFS_ERR_INVALID_DATA;
}

//格式化日志描述的辅助函数
static inline const char *FormatLogDesc(const char *pFormat, ...)
{
    static char buf[1024];
    va_list vl;
    va_start(vl, pFormat);
    vsprintf(buf, pFormat, vl);
    return buf;
}

//区分不能取款的原因是硬件还是其它原因
#define STOPDENOMINATE_BY_HARDWARE(bHardWare)\
    {\
        if (pbStopByHardwareError != NULL)\
            *pbStopByHardwareError = bHardWare;\
    }

#define  CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(condition, iRet)\
    if (condition) \
    {\
        if (bLog)\
        {\
            Log(ThisModule, -1, "error : %s", #condition);\
        }\
        return iRet;\
    }

//------ 以下宏用于检查参数的正确性-----------------
#define CHECK_PTR_VALID(ptr)    VERIFY_ERR_CONDITION(ptr == NULL, WFS_ERR_INVALID_POINTER, #ptr "= NULL")
#define CHECK_RETRACT_AREA(usRetractArea, fwRetractAreas, bReset) \
    {\
        long iRet = CheckRetractArea((usRetractArea), (fwRetractAreas), (bReset));\
        VERIFY_ERR_CONDITION(iRet != 0, iRet, FormatLogDesc("RetractArea can't be supported(%d)", (usRetractArea)));\
    }

#define CHECK_OUTPUT_POS(wPos, fwPositions) \
    {\
        long iRet = CheckOutputPos(wPos, fwPositions);\
        VERIFY_ERR_CONDITION(iRet != 0, iRet, FormatLogDesc("Position can't be supported(%d)", wPos));\
    }

#define CHECK_CIM_POS(wPos, fwPositions) \
    {\
        long iRet = CheckCIMPos(wPos, fwPositions);\
        VERIFY_ERR_CONDITION(iRet != 0, iRet, FormatLogDesc("Position can't be supported(%d)", wPos));\
    }

#define CHECK_CIM_OUTPUT_POS(wPos, fwPositions) \
    {\
        long iRet = CheckCIMOutputPos(wPos, fwPositions);\
        VERIFY_ERR_CONDITION(iRet != 0, iRet, FormatLogDesc("Position can't be supported(%d)", wPos));\
    }

#define CHECK_CIM_INPUT_POS(wPos, fwPositions) \
    {\
        long iRet = CheckCIMInputPos(wPos, fwPositions);\
        VERIFY_ERR_CONDITION(iRet != 0, iRet, FormatLogDesc("Position can't be supported(%d)", wPos));\
    }

#define CHECK_CDM_OR_CIM_OUTPUT_POS(bCDM, wPos, fwCDMPositions, fwCIMPositions)\
    {\
        long iRet = bCDM ? CheckOutputPos(wPos, fwCDMPositions) : CheckCIMOutputPos(wPos, fwCIMPositions);\
        VERIFY_ERR_CONDITION(iRet != 0, iRet, FormatLogDesc("OutPutPosition can't be supported(%d)", wPos));\
    }

//DisableUnusedCassettes并检验其经果，如果小于0，记录日志并返回
#define VERIFY_DISABLE_UNUSED_CASSETTE(bCDM) \
    {\
        long iRet = DisableUnusedCassettes(bCDM);\
        if (iRet < 0)\
        {\
            Log(ThisModule, iRet, "DisableUnusedCassettes failed");\
            return iRet;\
        }\
    }

//清除数组
#define CLEAR_ARRAY(a)  memset(a, 0, sizeof(a))
//设置数组每个元素值为v
#define SET_ARRAY(a, v) \
    {\
        for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++)\
        {\
            a[i] = v;\
        }\
    }

//30-00-00-00(FS#0004) add start
//校验安全门状态
#define VERIFY_SAFEDOOR_OPEN(wSafedoorStatus, bCDM)      VERIFY_ERR_CONDITION(wSafedoorStatus == ADP_SAFEDOOR_OPEN, \
                                                         bCDM ? WFS_ERR_CDM_SAFEDOOROPEN : WFS_ERR_CIM_SAFEDOOROPEN, "Safedoor is open")
//30-00-00-00(FS#0004) add end
//校验不在交换状态
#define VERIFY_NOT_IN_EXCHANGE(bCDM)    VERIFY_ERR_CONDITION(m_bCDMExchangeActive || m_bCIMExchangeActive, \
                                                             bCDM ? WFS_ERR_CDM_EXCHANGEACTIVE:WFS_ERR_CIM_EXCHANGEACTIVE, "Exchange is active")

//校验处于交换状态
#define VERIFY_IN_EXCHANGE()        VERIFY_ERR_CONDITION(!m_bCDMExchangeActive && !m_bCIMExchangeActive, WFS_ERR_CDM_NOEXCHANGEACTIVE, "Exchange is NOT active")

//校验不在CASH_IN事务中
#define VERIFY_NOT_IN_CASH_IN()     VERIFY_ERR_CONDITION(m_bCashInActive, WFS_ERR_CIM_CASHINACTIVE, "It is in CashIn transaction")

//校验处于CASH_IN事件中
#define VERIFY_IN_CASH_IN()         VERIFY_ERR_CONDITION(!m_bCashInActive, WFS_ERR_CIM_NOCASHINACTIVE, "It is NOT in CashIn transaction")

//校验是否支持SHUTER操作
#define VERIFY_SUPPORT_SHUTER(cap)  VERIFY_ERR_CONDITION(!cap.bShutter, WFS_ERR_UNSUPP_COMMAND, #cap ".bShutter = FALSE")

//自动FIRE门开关事件
#define AUTO_FIRE_SHUTTER_STATUS()  CAutoFireDeviceStatus _auto_fire_device_status(this)

//SHUTTER由SP控件
#define SHUTTER_CONTROL_BY_SP(cap)  (cap.bShutter && cap.bShutterControl)

//自动更新钞箱数据
#define AUTO_UPDATE_CASS_DATA(wDispenseCounts, wRejectCounts, aryNoteIDNumAaary, bCDM, bSubstractRejectCountFromCount)  \
    CAutoUpdateCassData _auto_update_cass_data(this, wDispenseCounts, wRejectCounts, NULL, aryNoteIDNumAaary, bCDM, bSubstractRejectCountFromCount)

inline WORD CIM_INPUT_POSITION(WORD fwPositions)
{
    return (fwPositions & 0x7F);
}

inline WORD CIM_OUTPUT_POSITION(WORD fwPositions)
{
    return (fwPositions & 0x3F80);
}

//增加CDM额外状态辅助函数
long AddExtraStCDM(ICDMStatus *lpStatus, LPCSTR lpszKey, LPCSTR lpszValue)
{
    return lpStatus->AddExtraSt(lpszKey, lpszValue);
}

//增加CIM额外状态辅助函数
long AddExtraStCIM(ICIMStatus *lpStatus, LPCSTR lpszKey, LPCSTR lpszValue)
{
    return lpStatus->AddExtraSt(lpszKey, lpszValue);
}

//增加CDM能力额外信息辅助函数
long AddExtraCapCDM(ICDMCaps *lpCaps, LPCSTR lpszKey, LPCSTR lpszValue)
{
    return lpCaps->AddExtraCp(lpszKey, lpszValue);
}

//增加CIM能力额外信息辅助函数
long AddExtraCapCIM(ICIMCaps *lpCaps, LPCSTR lpszKey, LPCSTR lpszValue)
{
    return lpCaps->AddExtraCp(lpszKey, lpszValue);
}

//////////////////////////////////////////////////////////////////////////
XFS_CRSImp::XFS_CRSImp()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);

    m_strConfigFile = string(SPETCPATH) + "/BRMSPConfig.ini";
    m_strCashAcceptorConfigFile = string(WSAPCFGPATH) + "/CashAcceptor.ini";    //test#5
    m_pCUManager = nullptr;
    m_pMixManager = NULL;
    m_bCDMExchangeActive = FALSE;
    m_bCIMExchangeActive = FALSE;
    m_bCashInActive = FALSE;
    m_bCashOutActive = FALSE;
    m_bInCmdExecute = FALSE;
    m_eWaitTaken = WTF_NONE;
    m_bProhibitRecycleBox = FALSE;
    CLEAR_ARRAY(m_ulLastDispenseCount);

    m_dwAutoRecoverNumber = 0;

    m_dwCloseShutterCount = 0;

    m_bCSItemsFromTS = FALSE;
    m_bFirstStartup = TRUE;
    m_bAdapterInited = FALSE;

    m_ulMaxNumber = 0xFFFFFFFF;                  //30-00-00-00(FS#0002)
    m_ulMaxAmount = 0xFFFFFFFF;                  //30-00-00-00(FS#0002)
    m_bCashInExecuted = false;

    m_bCdmCmd = false;
}

XFS_CRSImp::~XFS_CRSImp()
{
    OnClose();
}

long XFS_CRSImp::StartRun()
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    // 加载BaseCRS
    if (0 != m_pBase.Load("SPBaseCRS.dll", "CreateISPBaseCRS", "CRS"))
    {
        string strErr = m_pBase.LastError().toStdString();
        Log(ThisModule, __LINE__, "加载SPBaseCRS失败,Err:%s", m_pBase.LastError().toStdString().c_str());
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

//bool XFS_CRSImp::LoadCashManageDll(LPCSTR ThisModule)
//{
//    if (m_pAdapter == nullptr)
//    {
//        char szDevDllName[256] = { 0 };
//        strcpy(szDevDllName, m_Param.GetAdapterDLLName().c_str());
//        if (0 != m_pAdapter.Load("", "CreateIDevIDC"))
//        {
//            Log(ThisModule, __LINE__, "加载库失败: AgentDllName=%s", szDevDllName);
//            return false;
//        }
//    }
//    return (m_pAdapter != nullptr);
//}

bool XFS_CRSImp::LoadAdpDll(LPCSTR ThisModule)
{
    if (m_pAdapter == nullptr)
    {
        char szDllName[256] = { 0 };
        strcpy(szDllName, m_Param.GetAdapterDLLName().c_str());
        if (0 != m_pAdapter.Load(szDllName, "CreateBRMAdapter"))
        {
            Log(ThisModule, __LINE__, "加载库失败: DllName=%s[%s]", szDllName, m_pAdapter.LastError().toStdString().c_str());
            return false;
        }
    }
    return (m_pAdapter != nullptr);
}

HRESULT XFS_CRSImp::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT iRet = 0;
    m_cXfsReg.SetLogicalName(lpLogicalName);
    //创建配钞算法对象
    m_pMixManager = CreateMixManager();
    VERIFY_RESULT_PTR("CreateMixManager", m_pMixManager);

    //装载入金限额配置参数
    iRet = m_CashAcceptorParam.LoadCashAcceptorParam(m_strCashAcceptorConfigFile.c_str());  //test#5
    VERIFY_RESULT("m_CashAcceptorParam.LoadCashAcceptorParam()", iRet);                     //test#5

    //装载配置参数
    iRet = m_Param.LoadParam(m_strConfigFile.c_str());
    VERIFY_RESULT("m_Param.LoadParam()", iRet);
    assert(m_Param.GetCurrencyExp() != NULL);
    assert(m_Param.GetNoteTypeList() != NULL);

    //拷贝币种列表到可存币种列表中
    m_NoteTypeListDepositable.Copy(*m_Param.GetNoteTypeList());

    //创建钞箱管理模块、初始化并确保GetCUInterface_XXX为有效值
    iRet = CreateCashUnitManager(m_pCUManager, "BRMCashUnit.xml");
    if (iRet != 0 || m_pCUManager == nullptr)
        return WFS_ERR_INTERNAL_ERROR;


    iRet = m_pCUManager->Initialize(m_Param.GetNoteTypeList());
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "m_pCUManager->Initialize() failed");
        return WFS_ERR_INTERNAL_ERROR;
    }
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    if (pCDM == NULL || pCIM == NULL)
    {
        m_pCUManager->Uninitialize();
        m_pCUManager = NULL;
        Log(ThisModule, -1, "m_pCUManager->GetCUInterface_CDM()=0x%08X, m_pCUManager->GetCUInterface_CIM()=0x%08X",
            pCDM, pCIM);
        return WFS_ERR_INTERNAL_ERROR;
    }

    iRet = LoadAdpDll(ThisModule);
    string s = string("LoadLibrary(") + m_Param.GetAdapterDLLName() + ")";
    VERIFY_RESULT(s.c_str(), iRet);

    //获取门支持信息
    m_pAdapter->GetDevConfig(m_adapterinfo);        //30-00-00-00(FT#0005)

    // 打开连接
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();
    iRet = m_pAdapter->Open();
	if (iRet != WFS_SUCCESS)
	{
		Log(ThisModule, iRet, "m_pAdapter->Open failed");
		return iRet;
    }

    if(m_pAdapter->IsNeedDownLoadFW())
    {
        return 0;
    }

    iRet = InitAdapterSetting();
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "InitAdapterSetting failed");
        m_bAdapterInited = FALSE;
        return iRet;
    }
    m_bAdapterInited = TRUE;
    // 此防止Open完成后，发送一次钞箱改变事件
    CAutoUpdateCassStatusAndFireCassStatus autoFireCassStatus(this);
    //复位机芯
    if (m_Param.GetResetOnOpen())
    {
        iRet = Reset(ThisModule, NULL, m_pCUManager->GetCUInterface_CDM(), TRUE);
        Log(ThisModule, iRet < 0 ? iRet : 1, "Reset %s", iRet == 0 ? "成功" : "失败");
    }

    // 更新一次状态
    //UpdateStatus();

    Log(ThisModule, 1, "打开设备连接成功");
    return WFS_SUCCESS;
}

long XFS_CRSImp::InitAdapterSetting()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    HRESULT iRet = 0;

    //设置适配器支持的币种类型
    assert(m_Param.GetNoteTypeList() != NULL);
    iRet = m_pAdapter->SetSupportNoteTypes(m_Param.GetNoteTypeList());
    VERIFY_RESULT("m_pAdapter->SetSupportNoteTypes()", iRet);


    //从钞箱管理模块设置适配层的钞箱信息
    iRet = UpdateADPCassInfoFromCUManager();
    VERIFY_RESULT("UpdateADPCassInfoFromCUManager", (iRet == 0 ? 0 : WFS_ERR_INTERNAL_ERROR));

    //从适配层读取CDM能力
    ADPCDMCAPS CDMCaps;
    iRet = m_pAdapter->GetCDMCapabilities(&CDMCaps);
    VERIFY_RESULT("m_pAdapter->GetCDMCapabilities()", iRet);
    if (CDMCaps.fwPositions == 0)
    {
        char szBuf[100] = {0};
        sprintf(szBuf, "CDMCaps.fwPositions(%d)错误", CDMCaps.fwPositions);
        VERIFY_ERR_CONDITION(TRUE, WFS_ERR_INTERNAL_ERROR, szBuf);
    }
    if (m_Param.GetCashUnitType() == 0)
    {
        //从适配层读取CIM能力
        ADPCIMCAPS CIMCaps;
        iRet = m_pAdapter->GetCIMCapabilities(&CIMCaps);
        VERIFY_RESULT("m_pAdapter->GetCIMCapabilities()", iRet);
        if (CIM_INPUT_POSITION(CIMCaps.fwPositions) == 0 || CIM_OUTPUT_POSITION(CIMCaps.fwPositions) == 0)
        {
            char szBuf[100];
            sprintf(szBuf, "CIMCaps.fwPositions(%d)错误", CIMCaps.fwPositions);
            VERIFY_ERR_CONDITION(TRUE, WFS_ERR_INTERNAL_ERROR, szBuf);
        }
    }

    //m_pAdapter->GetDevConfig(m_adapterinfo); todo

    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::OnClose()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    if ((IBRMAdapter *)m_pAdapter != nullptr)
    {
        m_pAdapter->Close();
        //m_pAdapter.Release();
    }

    if (m_pCUManager != nullptr)
    {
        m_pCUManager->Uninitialize();
        m_pCUManager = nullptr;
    }

    if (m_pMixManager != nullptr)
    {
        m_pMixManager->Release();
        m_pMixManager = nullptr;
    }

    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::OnStatus()
{
    THISMODULE(__FUNCTION__);

    // 空闲更新状态
    UpdateStatus();
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();
    // 不在等待状态时，直接返回
    if (m_eWaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }

    UpdatePostionStatus();
    if(m_eWaitTaken == WTF_NONE){                       //30-00-00-00(FT#0060)
        return WFS_ERR_CANCELED;                        //30-00-00-00(FT#0060)
    }                                                   //30-00-00-00(FT#0060)
    CloseShutterAndEnsureItemTaken();
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (!m_pAdapter->IsNeedDownLoadFW())
    {
        return WFS_SUCCESS;
    }

    long lRes = m_pAdapter->DownLoadFW();
    if (lRes < 0)
    {
        Log(ThisModule, __LINE__, "DownLoadFW failed");
        return WFS_ERR_HARDWARE_ERROR;
    }

    CAutoUpdateCassStatusAndFireCassStatus autoFireCassStatus(this);
    //复位机芯
    if (m_Param.GetResetOnOpen())
    {
        lRes = Reset(ThisModule, nullptr, m_pCUManager->GetCUInterface_CDM(), TRUE);
        Log(ThisModule, lRes < 0 ? __LINE__ : 1, "Reset %s", lRes == 0 ? "成功" : "失败");
    }

    return WFS_SUCCESS;
}

//30-00-00-00(FS#0002) add start
HRESULT XFS_CRSImp::CIMCashInLimit(const LPWFSCIMCASHINLIMIT lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //处于Exchange中检查
    VERIFY_NOT_IN_EXCHANGE(FALSE);

    //检查命令发送时机是否正确
    if(!m_bCashInActive || m_bCashInExecuted){
        return WFS_ERR_SEQUENCE_ERROR;
    }

    //检查是否多次设定
    if(m_ulMaxAmount != 0xFFFFFFFF && m_ulMaxAmount != 0xFFFFFFFF){
        return WFS_ERR_SEQUENCE_ERROR;
    }

    //检查参数
    m_ulMaxNumber = lpCUInfo->ulTotalItemsLimit;
    if(lpCUInfo->lpAmountLimit != nullptr){
        if(!strncmp(lpCUInfo->lpAmountLimit->cCurrencyID, "CNY", 3)){
            m_ulMaxAmount = lpCUInfo->lpAmountLimit->ulAmount;
        } else {
            return WFS_ERR_INVALID_DATA;
        }
    }

    return WFS_SUCCESS;
    //    return WFS_ERR_UNSUPP_COMMAND;
}

//30-00-00-00(FS#0022)
HRESULT XFS_CRSImp::CIMCashUnitCount(const LPWFSCIMCOUNT lpCIMCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_SUCCESS;

    bool bEnableRoom[5][2] = {{true, false}, {true, false}, {true, false}, {false, false},
                              {false, false}};
    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    //入参检查
    if(lpCIMCount != nullptr && lpCIMCount->lpusCUNumList != nullptr){
        for(int i = 0; i < 5; i++){
            bEnableRoom[i][0] = false;
        }

        for(int i = 0; i < lpCIMCount->usCount; i++){
            ICashUnit *lpCashUnit = pCIM->GetCUByNumber(lpCIMCount->lpusCUNumList[i]);
            if(lpCashUnit == nullptr){
                Log(ThisModule, __LINE__, "参数非法：精查对象逻辑钞箱[%d]不存在", lpCIMCount->lpusCUNumList[i]);
                return WFS_ERR_CIM_INVALIDCASHUNIT;
            }

            ADP_CASSETTE_TYPE eCstType = lpCashUnit->GetType();
            if(eCstType == ADP_CASSETTE_RETRACT || eCstType == ADP_CASSETTE_REJECT){
                Log(ThisModule, __LINE__, "参数非法：精查对象包含回收箱");
                return WFS_ERR_CIM_INVALIDCASHUNIT;
            }

            if(lpCashUnit->GetIndex() == 5){
                Log(ThisModule, __LINE__, "参数非法：精查对象包含RB5");
                return WFS_ERR_CIM_INVALIDCASHUNIT;
            }

            bEnableRoom[lpCashUnit->GetIndex() - 1][0] = true;
        }
    }

    //Exchange/CashIn中检查
    VERIFY_NOT_IN_CASH_IN();
    VERIFY_NOT_IN_EXCHANGE(FALSE);

    //TS/ESC非空检查
    ADPSTATUSINFOR ADPStatus;
    UpdateStatus(ThisModule, ADPStatus, TRUE);
    if(ADPStatus.stOutPutPosition != ADP_OUTPOS_EMPTY || ADPStatus.stStacker != ADP_STACKER_EMPTY)
    {
        Log(ThisModule, -1, "CS或ESC非空");
        return WFS_ERR_CIM_COUNTPOSNOTEMPTY;
    }

    //各钞箱状态检查
//    BOOL bAllSelfCountRoomErr = TRUE;
    for(int i = 0; i < pCIM->GetCUCount(); i++){
        ICashUnit *lpCashUnitTemp = pCIM->GetCUByNumber(i + 1);
        int iPhIdx = lpCashUnitTemp->GetIndex();

        ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE] = {ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK};
        BOOL bCashUnitErr = FALSE;
        if(lpCashUnitTemp->GetStatus() == ADP_CASHUNIT_MANIP){
            bCashUnitErr = TRUE;
        } else {
            //所有精查钞箱对象状态异常判断
//            if(bEnableRoom[iPhIdx - 1][0]){
//               if((lpCashUnitTemp->GetStatus() == ADP_CASHUNIT_MISSING || lpCashUnitTemp->GetStatus() == ADP_CASHUNIT_INOP ||
//                   lpCashUnitTemp->GetStatus() == ADP_CASHUNIT_UNKNOWN)){

//               } else {
//                   bAllSelfCountRoomErr = FALSE;
//               }
//            }
        }

        if(bCashUnitErr){
            wCUErrors[iPhIdx - 1] = ADP_CUERROR_FATAL;
            FireCUErrorAndSetErrorNum(wCUErrors, FALSE);
            return WFS_ERR_CIM_INVALIDCASHUNIT;
        }

        //检查回收箱状态
        if(lpCashUnitTemp->GetType() == ADP_CASSETTE_REJECT || lpCashUnitTemp->GetType() == ADP_CASSETTE_RETRACT){
            if(lpCashUnitTemp->GetStatus() == ADP_CASHUNIT_FULL){
                wCUErrors[iPhIdx - 1] = ADP_CUERROR_FULL;
                FireCUErrorAndSetErrorNum(wCUErrors, FALSE);
                return WFS_ERR_CIM_CASHUNITERROR;
            }
        }
    }

    //精查所有钞箱对象状态异常
//    if(bAllSelfCountRoomErr){
//        return WFS_ERR_HARDWARE_ERROR;
//    }

    //RB5状态检查
    ICashUnit *lpCashUnitTemp = pCIM->GetFirstCUByPHIndex(5);
    if(lpCashUnitTemp == NULL){
        //发送钞箱异常事件
        ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE] = {ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_FATAL};
        FireCUErrorAndSetErrorNum(wCUErrors, FALSE);
        Log(ThisModule, -1, "RB5钞箱非循环钞箱");
        return WFS_ERR_CIM_CASHUNITERROR;
    } else {
        CASHUNIT_STATUS eCstStatus = lpCashUnitTemp->GetStatus();
        if(eCstStatus == ADP_CASHUNIT_MISSING ||
                eCstStatus == ADP_CASHUNIT_INOP ||
                eCstStatus == ADP_CASHUNIT_MANIP ||
                eCstStatus == ADP_CASHUNIT_UNKNOWN){
            if(eCstStatus == ADP_CASHUNIT_MISSING){
                return WFS_ERR_CIM_COUNTPOSNOTEMPTY;
            } else {
                //发送钞箱异常事件
                ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE] = {ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_FATAL};
                FireCUErrorAndSetErrorNum(wCUErrors, FALSE);
                Log(ThisModule, -1, "RB5钞箱状态异常:%d", eCstStatus);
                return WFS_ERR_CIM_CASHUNITERROR;
            }
        } else {
            if(lpCashUnitTemp->GetStatus() != ADP_CASHUNIT_EMPTY){
                Log(ThisModule, -1, "RB5钞箱非空");
                return WFS_ERR_CIM_COUNTPOSNOTEMPTY;
            }
        }
    }

    m_pAdapter->SetLFSCmdId(ADP_CIM_SELFCOUNT);
    //精查命令发行
    ULONG ulNumSelfCountPerRoom[ADP_MAX_CU_SIZE] = {0, 0, 0, 0, 0, 0};
    ULONG ulNumRejectPerRoom[ADP_MAX_CU_SIZE] = {0, 0, 0, 0, 0, 0};
    bool bErrCUInfo[ADP_MAX_CU_SIZE] = {false, false, false, false, false, false};
    hRet = m_pAdapter->SelfCount(bEnableRoom, ulNumSelfCountPerRoom, ulNumRejectPerRoom, bErrCUInfo);

    //RJ钞箱满
    if(hRet == WFS_ERR_CIM_CASHUNITERROR){
        ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE] = {ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_FULL, ADP_CUERROR_OK};
        FireCUErrorAndSetErrorNum(wCUErrors, FALSE);
        Log(ThisModule, -1, "精查RJ钞箱满");
    }

    //更新各钞箱计数信息(拒钞信息)
    ULONG ulDispenseCounts[ADP_MAX_CU_SIZE] = {0, 0, 0, 0, 0, 0};
    NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE];
    if (m_Param.GetCashUnitType() == 0)
    {
        char cNoteIdCountResult[256] = {0};
        ULONG ulRejCount(0);
        long lRs = m_pAdapter->GetRetractNoteIdAndCountPairs(cNoteIdCountResult, ulRejCount);
        if (lRs != 0)
        {
            Log(ThisModule, lRs, "获取拒钞金种别计数信息失败(m_pAdapter->GetRetractNoteIdAndCountPairs()失败)");
        } else {
            USHORT usIndex = 1;
            if (FindCassetteADPIndex(FCAIF_REJECT, TRUE, usIndex) < 0)
            {
                usIndex = 1;
                if (FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usIndex) < 0)
                {
                    Log(ThisModule, -1, "没有拒收钞和回收箱");
                    return WFS_ERR_INTERNAL_ERROR;
                }
            }

            lRs = AnalyseCountResult(cNoteIdCountResult, NoteIDCountArray[usIndex - 1], TRUE, FALSE);
            if (lRs < 0)
            {
                Log(ThisModule, lRs, "分析拒钞金种别计数信息(%s)失败", cNoteIdCountResult);
            }
        }
    }

    //更新钞箱计数
    {
        AUTO_UPDATE_CASS_DATA(ulDispenseCounts, ulNumRejectPerRoom, \
                          m_Param.GetCashUnitType() == 0 ? NoteIDCountArray : NULL, FALSE, TRUE);
    }

    //设置精查钞箱逻辑计数
    pCIM = m_pCUManager->GetCUInterface_CIM();
    for(int i = 0; i < ADP_MAX_CU_SIZE - 1; i++){
        if(bEnableRoom[i][0]){
            for(int j = 0; j < pCIM->GetCUCount(); j++){
                ICashUnit *CU = pCIM->GetCUByNumber(j + 1);
                if(CU->GetIndex() == i + 1){
                    CU->SetCount(ulNumSelfCountPerRoom[i]);
                    USHORT usNoteIDs[MAX_NOTEID_SIZE];
                    memset(&usNoteIDs, 0, sizeof(usNoteIDs));
                    if(CU->GetNoteIDs(usNoteIDs) > 0){
                        //TBD:多个金种的情况
                        CU->SetNumbersByNote(usNoteIDs[0], ulNumSelfCountPerRoom[i]);
                    }
                    break;
                }
            }
        }
    }
    ////同步计数信息到CDM
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    pCDM->SyncDataByPhysicalIndex(pCIM);

    return hRet;
}
//30-00-00-00(FS#0002) add end

long XFS_CRSImp::CloseShutterAndEnsureItemTaken()
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();
    assert(m_eWaitTaken != WTF_NONE);

    //从适配层得到状态，查看SHUTTER状态
    long iRet = 0;
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);

    if (!m_adapterinfo.bHaveOutShutter)
    {
        m_bCashOutActive = FALSE;
        m_eWaitTaken = WTF_NONE;
        return 0;
    }

    if (ADPStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED)
    {
        if (ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY)
        {
            m_bCashOutActive = FALSE;
            m_eWaitTaken = WTF_NONE;
        }
    }
    else
    {
        if (ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY) //钞口无钞
        {
            iRet = m_pAdapter->CloseShutter(FALSE);
            if (iRet < 0)
            {
                Log(ThisModule, iRet, "m_pAdapter->CloseShutter(FALSE)失败");
                m_dwCloseShutterCount++;
                if (m_dwCloseShutterCount > m_Param.GetCloseShutterCountBeforeJammed())
                {
                    Log(ThisModule, iRet, "m_dwCloseShutterCount > m_Param.GetCloseShutterCountBeforeJammed()(%d)",
                        m_Param.GetCloseShutterCountBeforeJammed());
                    m_bCashOutActive = FALSE;
                    m_eWaitTaken = WTF_NONE;
                }
            }
            else
            {
                //再次检测钞口是否非空，防止关门前纸币再次放入
                if(m_eWaitTaken == WTF_CDM){
                    long lRet = WFS_SUCCESS;
                    UpdateStatus(ThisModule, ADPStatus, TRUE);
                    if(ADPStatus.stOutPutPosition != ADP_OUTPOS_EMPTY ||
                            ADPStatus.stShutterPosition != ADP_SHUTPOS_EMPTY){
                        //重新打开钞门
                        for(int i = 0; i < 10; i++){
                            lRet = m_pAdapter->OpenShutter(FALSE);
                            if(lRet == WFS_SUCCESS){
                                break;
                            }
                        }
                        if(lRet != WFS_SUCCESS){
                            Log(ThisModule, -1, "打开钞门失败达最大重试次数");
                        }

                        return 0;
                    } else {
                        //钞票成功取走，返回成功
                        //发送Taken事件
                        if(m_Param.GetPresentTakenEventAfterCloseShutterFlag()){
                            ADPCDMCAPS stADPCdmCaps;
                            m_pAdapter->GetCDMCapabilities(&stADPCdmCaps);
                            CDMFireItemTaken(stADPCdmCaps.fwPositions);
                        }
                    }
                }

                m_eWaitTaken = WTF_NONE;            //30-00-00-00(FT#0005)
                Log(ThisModule, 1, "m_pAdapter->CloseShutter()成功");
                m_dwCloseShutterCount = 0;
            }
        }
        else                                            //钞口有钞或其他状态
        {
            //不做处理
        }
    }

    return 0;
}

HRESULT XFS_CRSImp::CDMGetStatus(LPWFSCDMSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoMutexStl(m_cStatusMutex);
    ////AutoLogFuncBeginEnd();();

    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);

    m_CDMStatus.InitData();
    ICDMStatus *pIStatus = (ICDMStatus *)&m_CDMStatus;

    pIStatus->SetDeviceSt(ComputeDeviceState(ADPStatus, TRUE));
    pIStatus->SetSafeDoorSt(m_StatusConvertor.ADP2XFS(ADPStatus.stSafeDoor, SCT_SAFEDOOR, TRUE));
    pIStatus->SetIntermediateStackerSt(m_StatusConvertor.ADP2XFS(ADPStatus.stStacker, SCT_STACKER, TRUE));

    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    WFSCDMOUTPOS outpos;
    outpos.fwPosition = CDMCaps.fwPositions;
    outpos.fwPositionStatus = m_StatusConvertor.ADP2XFS(ADPStatus.stOutPutPosition, SCT_OUTPOS, TRUE);
    outpos.fwShutter = m_StatusConvertor.ADP2XFS(ADPStatus.stOutShutter, SCT_OUTSHUTTER, TRUE);
    outpos.fwTransport = TransportADPToXFS(ADPStatus.stTransport, TRUE);
    outpos.fwTransportStatus = m_StatusConvertor.ADP2XFS(ADPStatus.stTransport, SCT_TRANSPORT, TRUE);
    pIStatus->AddPositionSt(outpos);

    //处理Dispense状态
    pIStatus->SetDispenserSt(ComputeDispenseState(ADPStatus));

    //处理扩展状态
    AddExtraStOrCap(pIStatus, (PAddExtraFunc)AddExtraStCDM);
    lpStatus = (LPWFSCDMSTATUS)&m_CDMStatus;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CDMGetCapabilities(LPWFSCDMCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);

    m_CDMCaps.InitData();
    ICDMCaps *lpCap = (ICDMCaps *)&m_CDMCaps;

    lpCap->SetType(WFS_CDM_SELFSERVICEBILL);
    lpCap->SetMaxDispenseItems(CDMCaps.wMaxDispenseItems);
    lpCap->SetCompound(m_Param.GetCashUnitType() == 0 ? TRUE : FALSE);
    lpCap->SetShutter(CDMCaps.bShutter);
    lpCap->SetShutterControl(CDMCaps.bShutterControl);
    lpCap->SetRetractAreas(CDMCaps.fwRetractAreas);
    lpCap->SetRetractTransportActions(CDMCaps.fwRetractTransportActions);
    lpCap->SetRetractStackerActions(CDMCaps.fwRetractStackerActions);
    lpCap->SetSafeDoor(CDMCaps.bSafeDoor);
    lpCap->SetCashBox(FALSE);
    lpCap->SetIntermediateStacker(CDMCaps.bIntermediateStacker);
    lpCap->SetItemsTakenSensor(CDMCaps.bItemsTakenSensor);
    lpCap->SetPositions(CDMCaps.fwPositions);
    lpCap->SetMoveItems(CDMCaps.fwMoveItems);
    lpCap->SetExchangeType(WFS_CDM_EXBYHAND);
    AddExtraStOrCap(lpCap, (PAddExtraFunc)AddExtraCapCDM);
    lpCaps = (LPWFSCDMCAPS)&m_CDMCaps;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CDMGetCashUnitInfo(LPWFSCDMCUINFO &lpCUInfor)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    assert(m_pCUManager != NULL);

    ICUInterface *pCDMCU = m_pCUManager->GetCUInterface_CDM();
    assert(pCDMCU != NULL);

    lpCUInfor = pCDMCU->BuildByXFSCDMFormat();
    if (lpCUInfor == NULL)
    {
        Log(ThisModule, -1,
            "pCDMCU->BuildByXFSCDMFormat()失败");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CDMGetMixType(LPWFSCDMMIXTYPE *&lppMixType)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_MixTypes.Clear();
    m_MixTypes.AddMixTape(WFS_CDM_MIX_MINIMUM_NUMBER_OF_BILLS, WFS_CDM_MIXALGORITHM, WFS_CDM_MIX_MINIMUM_NUMBER_OF_BILLS, (LPSTR)"MinNumberOfBills");
    m_MixTypes.AddMixTape(WFS_CDM_MIX_EQUAL_EMPTYING_OF_CASH_UNITS, WFS_CDM_MIXALGORITHM, WFS_CDM_MIX_EQUAL_EMPTYING_OF_CASH_UNITS, (LPSTR)"EqualEmptyingCashUnits");
    m_MixTypes.AddMixTape(WFS_CDM_MIX_MAXIMUM_NUMBER_OF_CASH_UNITS, WFS_CDM_MIXALGORITHM, WFS_CDM_MIX_MAXIMUM_NUMBER_OF_CASH_UNITS, (LPSTR)"MaxNumberOfCashUnits");

    lppMixType = (LPWFSCDMMIXTYPE *)m_MixTypes;

    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CDMGetMixTable(USHORT usMixNumber, LPWFSCDMMIXTABLE &lpMixTable)
{
    return WFS_ERR_UNSUPP_CATEGORY;
}

HRESULT XFS_CRSImp::CDMGetCurrencyEXP(LPWFSCDMCURRENCYEXP *&lppCurrencyExp)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    //CCDMCurrencyExp      m_CurrencyExp;   //所有出钞机支持的币种信息
    lppCurrencyExp = m_Param.GetCurrencyExp();
    return 0;
}

HRESULT XFS_CRSImp::CDMGetPresentStatus(WORD wPos, LPWFSCDMPRESENTSTATUS &lpPresentStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_PresentStatus.ClearData();
    ICDMPresentStatus *pPresentStatus = (ICDMPresentStatus *)&m_PresentStatus;
    if (m_PresentInfor.GetAvailable())
    {
        pPresentStatus->SetPresentState(m_PresentInfor.GetPresentState());
        WFSCDMDENOMINATION deno;
        m_PresentInfor.GetDenomination(&deno);
        pPresentStatus->SetDenomination(&deno);
        //CExtraInforManager &Extra = m_PresentInfor.m_Extra;
        CExtraInforManager &Extra = m_PresentInfor.m_Extra;
        pair<string, string> sp;
        long lRet = Extra.GetFirstRecord(sp);
        while (lRet == 0)
        {
            pPresentStatus->AddExtra(sp.first.c_str(), sp.second.c_str());
            lRet = Extra.GetNextRecord(sp);
        }
    }
    else
    {
        WFSCDMDENOMINATION deno;
        m_PresentInfor.GetDenomination(&deno);
        memcpy(deno.cCurrencyID, "CNY", 3);
        deno.ulAmount = 0;
        deno.usCount = m_pCUManager->GetCUInterface_CDM()->GetCUCount();
        deno.lpulValues = new ULONG[deno.usCount];
        memset(deno.lpulValues, 0, sizeof(ULONG) * deno.usCount);
        pPresentStatus->SetDenomination(&deno);
        pPresentStatus->SetPresentState(WFS_CDM_UNKNOWN);
        if (deno.lpulValues != NULL)
        {
            delete[] deno.lpulValues;
        }
    }

    lpPresentStatus = (LPWFSCDMPRESENTSTATUS)&m_PresentStatus;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::OnDenominate(USHORT usTellerID, USHORT usMixNumber, ICDMDenomination *pDenoInOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    //校验币种代码
    long iRet = VerifyDenominateCurrency(pDenoInOut);
    if (iRet < 0)
    {
        Log(ThisModule, -1, "Currency(% 3.3s)无效或者金额[%d]不为0", pDenoInOut->GetCurrency(), pDenoInOut->GetAmount());
        return iRet;
    }

    //首先查看wDispense状态，如果不可出钞，直接返回
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);
    iRet = VerifyCanOperatoByStackerTransportShutter(ADPStatus, ThisModule, TRUE);
    if (iRet < 0)
    {
        if (iRet == WFS_ERR_CDM_ITEMSLEFT)
        {
            iRet = WFS_ERR_CDM_NOTDISPENSABLE;
        }
        return iRet;
    }
    BOOL bStopByHardwareError = FALSE;
    WORD wDispense = ComputeDispenseState(ADPStatus, &bStopByHardwareError, TRUE);
    if (wDispense != WFS_CDM_DISPOK &&
        wDispense != WFS_CDM_DISPCUSTATE)
    {
        Log(ThisModule, -1,
            "wDispense(%hd)指示不可出钞(bStopByHardwareError=%d)",
            wDispense, bStopByHardwareError);
        if (bStopByHardwareError)
        {
            return WFS_ERR_HARDWARE_ERROR;
        }
        return WFS_ERR_CDM_NOTDISPENSABLE;
    }

    unsigned long iAmountCalced = 0;        //计算得到的金额
    ULONG ulCountAlloced = 0;   //已分配的张数
    iRet = VerifyDenominate(usMixNumber, pDenoInOut, iAmountCalced, ulCountAlloced, ThisModule);
    if (iRet <= 0)
        return iRet;
    assert(iAmountCalced >= 0);

    //校验计算得到的已分配张数小于最大出钞张数
    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    ULONG ulLeftAmount = pDenoInOut->GetAmount() - iAmountCalced;
    if (ulCountAlloced == (ULONG)CDMCaps.wMaxDispenseItems)
    {
        Log(ThisModule, -1,
            "计算得到的已分配张数(%d)等于能够出钞的总张数(%d)，但剩余金额(%d)不为0",
            ulCountAlloced, (ULONG)CDMCaps.wMaxDispenseItems, ulLeftAmount);
        return WFS_ERR_CDM_TOOMANYITEMS;
    }

    return MixByAlgorithmAndLog(ulLeftAmount, usMixNumber,
                                (ULONG)CDMCaps.wMaxDispenseItems - ulCountAlloced, pDenoInOut, ThisModule);
}

HRESULT XFS_CRSImp::CDMDenominate(const LPWFSCDMDENOMINATE lpDenoData, LPWFSCDMDENOMINATION &lpDenoOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    VERIFY_NOT_IN_CASH_IN();

    m_bCdmCmd = true;
    assert(m_pCUManager != NULL);
    assert(m_pMixManager != NULL);

    HRESULT hRet = WFS_SUCCESS;
   /* HRESULT hRet = CheckDenominateParam(lpDenoData);                //30-00-00-00(FT#0026)
    if(hRet != WFS_SUCCESS){                                        //30-00-00-00(FT#0026)
        return hRet;                                                //30-00-00-00(FT#0026)
    }                            */                                   //30-00-00-00(FT#0026)

    ICDMDenomination *pDenoInOut = (ICDMDenomination *)&m_Denomination;
    m_Denomination.ClearData();
    pDenoInOut->SetAmount(lpDenoData->lpDenomination->ulAmount);
    pDenoInOut->SetCashBox(lpDenoData->lpDenomination->ulCashBox);
    pDenoInOut->SetCurrency(lpDenoData->lpDenomination->cCurrencyID);
    if (pDenoInOut->SetValues(lpDenoData->lpDenomination->usCount,
                              lpDenoData->lpDenomination->lpulValues) == WFS_ERR_INVALID_DATA)
    {
        Log(ThisModule, WFS_ERR_INVALID_DATA, "Cmd=CDMDenominate, Error=SetValues\
					(Count != 0 && Values == NULL)");
        return WFS_ERR_INVALID_DATA;
    }

    hRet = OnDenominate(lpDenoData->usTellerID, lpDenoData->usMixNumber, pDenoInOut);        //30-00-00-00(FT#0026)
    lpDenoOut = (LPWFSCDMDENOMINATION)&m_Denomination;
    return hRet;
}

HRESULT XFS_CRSImp::CDMDispense(const LPWFSCDMDISPENSE lpDisData, LPWFSCDMDENOMINATION &lpDenoOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_bCdmCmd = true;
    m_pAdapter->SetLFSCmdId(ADP_DISPENSE);            //test#8
    //检查参数
    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    if (lpDisData->bPresent)
    {
        CHECK_OUTPUT_POS(lpDisData->fwPosition, CDMCaps.fwPositions);
    }

    HRESULT hRes = WFS_SUCCESS;
   /* HRESULT hRes = CheckDispenseParam(lpDisData);                   //30-00-00-00(FT#0026)
    if(hRes != WFS_SUCCESS){                                        //30-00-00-00(FT#0026)
        return hRes;                                                //30-00-00-00(FT#0026)
    }            */                                                   //30-00-00-00(FT#0026)

    //首先查看wDispense状态，如果不可出钞，直接返回
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);
    VERIFY_SAFEDOOR_OPEN(ADPStatus.stSafeDoor, TRUE);          //30-00-00-00(FS#0004)
    long iRet = VerifyCanOperatoByStackerTransportShutter(ADPStatus, ThisModule, TRUE);
    if (iRet < 0)
    {
        if (iRet == WFS_ERR_CDM_ITEMSLEFT)
        {
            iRet = WFS_ERR_CDM_NOTDISPENSABLE;
        }
        return iRet;
    }

    //先尝试配钞
    if (lpDisData->lpDenomination == NULL)
    {
        Log(ThisModule, -1, "Cmd=CDMDispense,\
					LPWFSCDMDISPENSE->LPWFSCDMDENOMINATION=NULL");
        return WFS_ERR_INVALID_DATA;
    }
    m_Denomination.ClearData();
    ICDMDenomination *pDenoInOut = (ICDMDenomination *)&m_Denomination;
    pDenoInOut->SetAmount(lpDisData->lpDenomination->ulAmount);
    pDenoInOut->SetCashBox(lpDisData->lpDenomination->ulCashBox);
    pDenoInOut->SetCurrency(lpDisData->lpDenomination->cCurrencyID);
    if (pDenoInOut->SetValues(lpDisData->lpDenomination->usCount,
                              lpDisData->lpDenomination->lpulValues) == WFS_ERR_INVALID_DATA)
    {
        Log(ThisModule, WFS_ERR_INVALID_DATA, "Cmd=CDMDenominate, Error=SetValues\
					(Count != 0 && Values == NULL)");
        return WFS_ERR_INVALID_DATA;
    }
    hRes = OnDenominate(lpDisData->usTellerID, lpDisData->usMixNumber, pDenoInOut);         //30-00-00-00(FT#0026)
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, hRes,
            "CDMDenominate(usMixNumber=%hd)失败",
            lpDisData->usMixNumber);
        return hRes;
    }

    //计算各种钞箱的出钞数
    ULONG ulCount[ADP_MAX_CU_SIZE];         //保存各钞箱出钞数
    memset(ulCount, 0, sizeof(ulCount));
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    assert(pCDM != NULL);
    USHORT usCount;
    LPULONG lpValues = pDenoInOut->GetValues(usCount);
    USHORT i = 0;
    for (i = 0; i < usCount; i++)
    {
        ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
        USHORT usIndex = pCU->GetIndex();
        if (usIndex < 1 ||
            usIndex > ADP_MAX_CU_SIZE)
        {
            Log(ThisModule, -1, "第%hd个钞箱的物理钞箱索引(%hd)不正确", i + 1, usIndex);
            return WFS_ERR_INTERNAL_ERROR;
        }
        usIndex--;

        if (ulCount[usIndex] > 0) //重复引用相同物理钞箱
        {
            Log(ThisModule, -1, "第%hd个张数被索引为%hd的物理重复引用", i + 1, usIndex);
            return WFS_ERR_INTERNAL_ERROR;
        }
        ulCount[usIndex] = lpValues[i];
    }

    //禁用未用的钞箱
    VERIFY_DISABLE_UNUSED_CASSETTE(TRUE); //禁用不相关的钞箱

    //更新PRESENT_STATUS为WFS_CDM_NOTPRESENTED
    m_PresentInfor.SetData(TRUE, WFS_CDM_NOTPRESENTED, pDenoInOut->GetCurrency(),
                           pDenoInOut->GetAmount(), usCount, lpValues);

    //出钞
    ULONG ulDispenseCounts[ADP_MAX_CU_SIZE];
    ULONG ulRejectCounts[ADP_MAX_CU_SIZE];
    ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE];
    NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(ulDispenseCounts);
    CLEAR_ARRAY(ulRejectCounts);
    CLEAR_ARRAY(wCUErrors);
//30-00-00-00(FS#0020)    AUTO_UPDATE_CASS_DATA(ulDispenseCounts, ulRejectCounts, \
//30-00-00-00(FS#0020)                          m_Param.GetCashUnitType() == 0 ? NoteIDCountArray : NULL, TRUE, TRUE);
//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(); //忽略一次ITEM插入取出事件
    memset(m_ulLastDispenseCount, 0, sizeof(m_ulLastDispenseCount));
    DisableUnusedCassettes(TRUE);
    iRet = m_pAdapter->Dispense(ulCount, ulDispenseCounts, ulRejectCounts, wCUErrors);
//30-00-00-00(FS#0020)    FireCUErrorAndSetErrorNum(wCUErrors, TRUE);

    if (m_Param.GetCashUnitType() == 0)
    {
        char arycResult[256];
        ULONG ulRejCount(0);
        long lRs = m_pAdapter->GetRetractNoteIdAndCountPairs(arycResult, ulRejCount);
        if (lRs != 0)
        {
            Log(ThisModule, lRs, "调用m_pAdapter->GetRetractNoteIdAndCountPairs()失败");
        }

        USHORT usIndex = 1;
        if (FindCassetteADPIndex(FCAIF_REJECT, TRUE, usIndex) < 0)
        {
            usIndex = 1;
            if (FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usIndex) < 0)
            {
                Log(ThisModule, -1, "没有拒收钞和回收箱");
                return WFS_ERR_INTERNAL_ERROR;
            }
        }

        lRs = AnalyseCountResult(arycResult, NoteIDCountArray[usIndex - 1], TRUE, FALSE);
        if (lRs < 0)
        {
            Log(ThisModule, lRs, "AnalyseCashInResult(%s)失败", arycResult);
        }
        //end
    }

    //30-00-00-00(FS#0020) add start
    //更新钞箱计数
    {
        AUTO_UPDATE_CASS_DATA(ulDispenseCounts, ulRejectCounts, \
                          m_Param.GetCashUnitType() == 0 ? NoteIDCountArray : NULL, TRUE, TRUE);
    }

    //钞箱发生Missfeed和Empty错误的特殊处理
    {
        bool bUpdateCount = false;
        NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE];
        ULONG ulCUEmptyCstDelCounts[ADP_MAX_CU_SIZE] = {0};
        ULONG ulCURejectCounts[ADP_MAX_CU_SIZE] = {0};
        ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
        //获取钞箱的最新状态
        m_pAdapter->UpdateStatus();

        //获取回收箱物理位置下标
        USHORT usABPhyIndex = 1;
        if (FindCassetteADPIndex(FCAIF_REJECT, TRUE, usABPhyIndex) < 0)
        {
            if (FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usABPhyIndex) < 0)
            {
                Log(ThisModule, -1, "没有拒收钞和回收箱");
                return WFS_ERR_INTERNAL_ERROR;
            }
        }

        //获取状态为空但逻辑计数不为0钞箱单元计数信息，合成改变钞箱计数预设定信息
        for(int i = 0; i < pCDM->GetCUCount(); i++){
            ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
            if(pCU->GetType() == ADP_CASSETTE_RECYCLING ||
                    pCU->GetType() == ADP_CASSETTE_BILL){
                CASHUNIT_STATUS eCstStatus;
                ULONG ulCount = pCU->GetCount();

                //获取物理位置下标
                int iPhyIdx = pCU->GetIndex();

                //获取钞箱状态
                char cCassID[128] = {0};
                m_pAdapter->GetCassetteInfo(iPhyIdx, eCstStatus, cCassID);
                if(eCstStatus == ADP_CASHUNIT_EMPTY && ulCount > 0){
                    USHORT aryusNoteIDs[MAX_NOTEID_SIZE] = {0};
                    pCU->GetNoteIDs(aryusNoteIDs);

                    ulCUEmptyCstDelCounts[iPhyIdx - 1] = ulCount;
                    ulCURejectCounts[iPhyIdx - 1] = ulCount;
                    //TBD:支持多个NoteId
                    NoteIDCountArray[usABPhyIndex - 1].push_back(NOTEID_COUNT((USHORT)aryusNoteIDs[0], ulCount));  //回收箱NoteNumberList
                    bUpdateCount = true;
                }
            }
        }

        for(int i = 0; i < ADP_MAX_CU_SIZE - 1; i++){
            /*不使用底层错误来判断，按照金种别出金时，命令算正常结束，无该异常信息
             if(wCUErrors[i] == ADP_CUERROR_EMPTY){
                ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
                ULONG ulCount = pCU->GetCount();
                if(ulCount > 0){
                    ulCUEmptyErrDelCounts[i] = ulCount;
                    ulCURejectCounts[i] = ulCount;
                    ulCURejectCounts[0] += ulCount;
                    bUpdateCount = true;
                }
            } else
            */
           if(wCUErrors[i] == ADP_CUERROR_MISSFEED){
                //设置朝钞箱状态为INOP
                ICashUnit *pCU = pCDM->GetFirstCUByPHIndex(i + 1);
                pCU->SetStatus(ADP_CASHUNIT_INOP, true);
                if(pCU->GetType() == ADP_CASSETTE_RECYCLING){
                    pCU = pCIM->GetFirstCUByPHIndex(i + 1);
                    pCU->SetStatus(ADP_CASHUNIT_INOP, true);
                }
            }
        }

        if(bUpdateCount){
            AUTO_UPDATE_CASS_DATA(ulCUEmptyCstDelCounts, ulCURejectCounts, NoteIDCountArray, TRUE, TRUE);
        }
    }
    FireCUErrorAndSetErrorNum(wCUErrors, TRUE);
    //30-00-00-00(FS#0020) add end

    if (iRet < 0)
    {
        Log(ThisModule, iRet, "m_pAdapter->Dispense()失败");
        return iRet;
    }

    if (!VerifyDispenseCount(ulCount, ulDispenseCounts, pDenoInOut->GetCurrency()))
    {
        Log(ThisModule, -1, "VerifyDispenseCount失败");
        return WFS_ERR_INTERNAL_ERROR;
    }

    m_bCashOutActive = TRUE;
    memcpy(m_ulLastDispenseCount, ulDispenseCounts, sizeof(ulDispenseCounts));

    //保存数据到返回结果中
    for (i = 0; i < usCount; i++)
    {
        ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
        USHORT usIndex = pCU->GetIndex() - 1;
        lpValues[i] = ulDispenseCounts[usIndex];
    }

    //记录结果
    LogDispenseRejectCount(ThisModule, "Dispense", ulDispenseCounts, ulRejectCounts);

    //更新PRESENT_STATUS
//    m_PresentInfor.SetData(TRUE, WFS_CDM_NOTPRESENTED, pDenoInOut->GetCurrency(),
//                           pDenoInOut->GetAmount(), usCount, lpValues);


    GetUpdatedStatus(ADPStatus);
    if ((ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN) &&
        (m_Param.GetStopWhenSafeDoorOpen() == 1))
    {
        Log(ThisModule, -1, "ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN");
        return WFS_ERR_CDM_SAFEDOOROPEN;
    }

    //送钞
    if (lpDisData->bPresent)
    {
        iRet = CDMPresent(lpDisData->fwPosition);
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "Present失败");
            return iRet;
        }
    }
    lpDenoOut = (LPWFSCDMDENOMINATION)&m_Denomination;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CDMPresent(WORD wPos)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_bCdmCmd = true;
    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    VERIFY_NOT_IN_CASH_IN();
    CHECK_OUTPUT_POS(wPos, CDMCaps.fwPositions);

    //检查STAKER是否为空
    ADPSTATUSINFOR ADPStatus;
    UpdateStatus(ThisModule, ADPStatus, TRUE);
//30-00-00-00(FT#0011)    if (CDMCaps.bIntermediateStacker &&
//30-00-00-00(FT#0011)         ADPStatus.stStacker == ADP_STACKER_EMPTY)
    if(ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY)      //30-00-00-00(FT#0011)
    {
        Log(ThisModule, -1, "ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY");  //30-00-00-00(FT#0011)
        return WFS_ERR_CDM_NOITEMS;
    }

    //检查钞门是否打开
    if (ADPStatus.stOutShutter == ADP_OUTSHUTTER_OPEN)
    {
        Log(ThisModule, -1, "ADPStatus.stShutter == ADP_SHUTTER_OPEN");
        return WFS_ERR_CDM_SHUTTEROPEN;
    }

    if ((ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN) &&
        (m_Param.GetStopWhenSafeDoorOpen() == 1))
    {
        Log(ThisModule, -1, "ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN");
        return WFS_ERR_CDM_SAFEDOOROPEN;
    }

    //物理送钞
//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(TRUE, FALSE); //忽略一次ITEM插入取出事件
    long iRet = m_pAdapter->Present();
    if (iRet < 0)
    {
        //更新PRESENT_STATUS
        m_PresentInfor.SetPresentState(WFS_CDM_UNKNOWN);

        Log(ThisModule, iRet, "m_pAdapter->Present()失败");
        return iRet;
    }

    //更新PRESENT_STATUS
    m_PresentInfor.SetPresentState(WFS_CDM_PRESENTED);

    //设置检视钞票取走关门标志
//30-00-00-00(FT#0005)    if (SHUTTER_CONTROL_BY_SP(CDMCaps) &&
//30-00-00-00(FT#0005)        CDMCaps.bItemsTakenSensor)
//30-00-00-00(FT#0060)    if(CDMCaps.bItemsTakenSensor && m_Param.GetPresentSPControlCloseShutter())           //30-00-00-00(FT#0005)
    if(CDMCaps.bItemsTakenSensor)                           //30-00-00-00(FT#0060)
    {
        if(m_Param.GetPresentSPControlCloseShutter()){      //30-00-00-00(FT#0060)
            m_eWaitTaken = WTF_CDM;
            m_dwCloseShutterCount = 0;
        } else {                                            //30-00-00-00(FT#0060)
            m_eWaitTaken = WTF_CDM_TK_EVT;                  //30-00-00-00(FT#0060)
        }                                                   //30-00-00-00(FT#0060)
    }

    m_bCashOutActive = TRUE;
    Log(ThisModule, 1, "CDMPresent()成功");
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CDMReject()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    VERIFY_NOT_IN_CASH_IN();
    m_pAdapter->SetLFSCmdId(ADP_OTHER);            //test#8

    m_bCdmCmd = true;
    if (m_Param.IsCheckTSEmptyBeforeReject())
    {
        //检查STAKER是否为空
        ADPSTATUSINFOR ADPStatus;
        UpdateStatus(ThisModule, ADPStatus, TRUE);
        if ((ADPStatus.stStacker == ADP_STACKER_EMPTY) && (ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY)) //test#15
        {
            Log(ThisModule, -1, "ADPStatus.stStacker == ADP_STACKER_EMPTY");
            return WFS_ERR_CDM_NOITEMS;
        }
    }

    if (!m_bCashOutActive)
    {
        Log(ThisModule, -1, "m_bCashOutActive == FALSE");
        return WFS_ERR_CDM_NOITEMS;
    }

    //禁用未用的钞箱
    VERIFY_DISABLE_UNUSED_CASSETTE(TRUE); //禁用不相关的钞箱

    //执行物理动作
    long iRet;
    ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(wCUErrors);
    USHORT usIndex = 1;
    if (FindCassetteADPIndex(FCAIF_REJECT, TRUE, usIndex) < 0)
    {
        usIndex = 1;
        if (FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usIndex) < 0)
        {
            Log(ThisModule, -1, "没有拒收钞和回收箱");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    m_eWaitTaken = WTF_NONE;    //当执行新命令时，清除等待取钞关门标志
    m_bCashOutActive = FALSE;

//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(); //忽略一次ITEM插入取出事件
    iRet = m_pAdapter->Reject(usIndex, wCUErrors);
    FireCUErrorAndSetErrorNum(wCUErrors, TRUE);

    if (iRet < 0)
    {
        Log(ThisModule, iRet, "CDMReject失败");
        return iRet;
    }

    ULONG ulRejectCount[ADP_MAX_CU_SIZE] = { 0 };
    memcpy(ulRejectCount, m_ulLastDispenseCount, ADP_MAX_CU_SIZE * sizeof(ULONG));

    USHORT usRejIndex = 1;
    if (m_Param.GetCashUnitType() == 0)
    {
        UpdateRetractCountToCUManager(TRUE, usIndex);
    }
    else
    {
        for (int i = 1; i < ADP_MAX_CU_SIZE; i++)
        {
            ulRejectCount[usIndex - 1] += m_ulLastDispenseCount[i];
        }
    }

    {
        AUTO_UPDATE_CASS_DATA(NULL, ulRejectCount, NULL, TRUE, FALSE);
    }
    CLEAR_ARRAY(m_ulLastDispenseCount);

    Log(ThisModule, iRet < 0 ? iRet : 1, "CDMReject成功");
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CDMRetract(const WFSCDMRETRACT &stData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_bCdmCmd = true;
    m_pAdapter->SetLFSCmdId(ADP_CDM_RETRACT);            //test#8
    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);

    VERIFY_NOT_IN_CASH_IN();
    CHECK_OUTPUT_POS(stData.fwOutputPosition, CDMCaps.fwPositions);

    return Retract(ThisModule, stData.fwOutputPosition, stData.usRetractArea, stData.usIndex, TRUE);
}

HRESULT XFS_CRSImp::CDMOpenShutter(WORD wPos)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_bCdmCmd = true;
    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    CHECK_OUTPUT_POS(wPos, CDMCaps.fwPositions);
    VERIFY_SUPPORT_SHUTER(CDMCaps);

    long iRet;
    ADPSTATUSINFOR ADPStatus;
    iRet = UpdateStatus(ThisModule, ADPStatus, TRUE);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "UpdateStatus()失败");
        return iRet;
    }

    //检查钞门是否打开
    if (ADPStatus.stOutShutter == ADP_OUTSHUTTER_OPEN)
    {
        Log(ThisModule, -1, "OUTSHUTTER钞门已经打开");
        return m_Param.GetShutterOpenedErrorSupp() ? WFS_ERR_CDM_SHUTTEROPEN : WFS_SUCCESS;
    }

    return OpenShutter(ThisModule, FALSE, TRUE);
}

HRESULT XFS_CRSImp::CDMCloseShutter(WORD wPos)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_bCdmCmd = true;
    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    CHECK_OUTPUT_POS(wPos, CDMCaps.fwPositions);
    VERIFY_SUPPORT_SHUTER(CDMCaps);

    long iRet;
    ADPSTATUSINFOR ADPStatus;
    iRet = UpdateStatus(ThisModule, ADPStatus, TRUE);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "UpdateStatus()失败");
        return iRet;
    }

    //检查钞门是否打开
    if (ADPStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED)
    {
        Log(ThisModule, -1, "OUTSHUTTER钞门已经关闭");
        return m_Param.GetShutterClosedErrorSupp() ? WFS_ERR_CDM_SHUTTERCLOSED : WFS_SUCCESS;
    }
    USHORT usIndex = 1;
    if (FindCassetteADPIndex(FCAIF_REJECT, TRUE, usIndex) < 0)
    {
        usIndex = 1;
        if (FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usIndex) < 0)
        {
            Log(ThisModule, -1, "没有拒收钞和回收箱");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    iRet = CloseShutter(ThisModule, FALSE, TRUE);
    return iRet;
}

HRESULT XFS_CRSImp::CDMStartEXChange(const LPWFSCDMSTARTEX lpData, LPWFSCDMCUINFO &lpCUInfor)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_bCdmCmd = true;
    if (lpData->fwExchangeType != WFS_CDM_EXBYHAND)
    {
        if (lpData->fwExchangeType == WFS_CDM_EXTOCASSETTES)
        {
            Log(ThisModule, -1, "lpStartEx->fwExchangeType(%hd) != WFS_CDM_EXBYHAND", lpData->fwExchangeType);
            return WFS_ERR_UNSUPP_DATA;
        }
        else
        {
            Log(ThisModule, -1, "lpStartEx->fwExchangeType(%hd)的值在XFS规范中未定义", lpData->fwExchangeType);
            return WFS_ERR_INVALID_DATA;
        }
    }

    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    assert(pCDM != NULL);

    HRESULT hRes = StartExchange(ThisModule, lpData->usCount, lpData->lpusCUNumList, pCDM);
    if (hRes == WFS_SUCCESS)
    {
        return CDMGetCashUnitInfo(lpCUInfor);
    }
    else
    {
        return hRes;
    }
}

HRESULT XFS_CRSImp::CDMEndEXChange(const LPWFSCDMCUINFO lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_bCdmCmd = true;
    if (m_Param.IsEndExchangeAutoClearCount())
    {
        ClearCountWhenEndExchange(lpCUInfo);
    }

    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    assert(pCDM != NULL);

    return EndExchange(ThisModule, lpCUInfo, pCDM);
}


HRESULT XFS_CRSImp::SetCashUnitInfo(const char *ThisModule, LPVOID lpCUInfo,
                                    ICUInterface *pCUInterface)
{
    VERIFY_NOT_IN_EXCHANGE(pCUInterface->IsCDM());
    VERIFY_NOT_IN_CASH_IN();
    CHECK_PTR_VALID(lpCUInfo);

    //设置数据给钞箱管理模块
    long iRet;
    if (pCUInterface->IsCDM())
        iRet = pCUInterface->SetByXFSCDMFormat((LPWFSCDMCUINFO)lpCUInfo);
    else
        iRet = pCUInterface->SetByXFSCIMFormat((LPWFSCIMCASHINFO)lpCUInfo);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "pCUInterface->SetByXFSFormat失败");
        return iRet;
    }

    //在CDM和CIM间同步循环箱和回收箱数据
    ICUInterface *pDestCUInterface =
    pCUInterface->IsCDM() ? m_pCUManager->GetCUInterface_CIM() : m_pCUManager->GetCUInterface_CDM();
    pDestCUInterface->SyncDataByPhysicalIndex(pCUInterface);

    //从钞箱管理模块更新到本地数据和适配层
    iRet = UpdateADPCassInfoFromCUManager();
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "UpdateADPCassInfoFromCUManager()失败");
        return iRet;
    }    
    if(m_Param.GetSetUnitInfoRelieveManipSupp()){       //30-00-00-00(FT#0042)
        //恢复钞箱MANIP状态                               //30-00-00-00(FT#0042)
        RelieveBoxManipStatus();                        //30-00-00-00(FT#0042)
    }                                                   //30-00-00-00(FT#0042)

    //根据情况进行复位
    if (m_pAdapter->IsNeedReset())
    {
        iRet = Reset(ThisModule, NULL, pCUInterface, TRUE);
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "Reset()失败");
            return iRet;
        }
    }

    Log(ThisModule, 1, "%sSetCashUnitInfo()成功", pCUInterface->IsCDM() ? "CDM" : "CIM");
    return 0;
}

HRESULT XFS_CRSImp::CDMSetCashUnitInfo(const LPWFSCDMCUINFO lpCUInfo)
{

    THISMODULE(__FUNCTION__);
    m_bCdmCmd = true;
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    assert(pCDM != NULL);

    return SetCashUnitInfo(ThisModule, lpCUInfo, pCDM);
}

//30-00-00-00(FS#0007)
HRESULT XFS_CRSImp::CDMTestCashUnits(const LPWFSCDMITEMPOSITION lpItemPosition, LPWFSCDMCUINFO &lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    HRESULT hRes = WFS_SUCCESS;

    //状态检查
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);
    if(ADPStatus.stOutShutter == ADP_OUTSHUTTER_OPEN){
        return WFS_ERR_CDM_SHUTTEROPEN;
    }

    //检查参数
    USHORT  usRetractArea = WFS_CDM_RA_RETRACT;
    USHORT usNumber = lpItemPosition->usNumber;
    if(usNumber == 0){
        //CS口
        usRetractArea = WFS_CDM_RA_TRANSPORT;
    } else {
        if(lpItemPosition->lpRetractArea){
            usRetractArea = lpItemPosition->lpRetractArea->usRetractArea;
            if(usRetractArea == WFS_CDM_RA_TRANSPORT){
                return WFS_ERR_CDM_UNSUPPOSITION;
            }
        }
    }

    //执行命令
    TESTDISPINFO stTestDispInfo = {0};
    ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE] = {ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK, ADP_CUERROR_OK};

    int iPhyIndex = 0;
    int iCanDispenseBoxCnt = 0;
    BYTE byRoomExclusion[2][5] = {0};
    memset(byRoomExclusion, 1, sizeof(byRoomExclusion));
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    for(int i = 0; i < pCDM->GetCUCount(); i++){        
        BOOL bSet = FALSE;
        ICashUnit *pCashUnit = pCDM->GetCUByNumber(i + 1);
        iPhyIndex = pCashUnit->GetIndex() - 1;
        //可以出钞的只有出金箱和循环箱
        if(pCashUnit->GetType() == ADP_CASSETTE_BILL || pCashUnit->GetType() == ADP_CASSETTE_RECYCLING){
            if(pCashUnit->GetCount() > 0 && pCashUnit->GetStatus() != ADP_CASHUNIT_EMPTY){
               bSet = TRUE;
               iCanDispenseBoxCnt++;
            }
            wCUErrors[iPhyIndex] = ADP_CUERROR_EMPTY;
        } else if(pCashUnit->GetType() == ADP_CASSETTE_RETRACT || pCashUnit->GetType() == ADP_CASSETTE_REJECT){
            bSet = TRUE;
        }
        if(bSet){
            //0:可用 1:不可用
            byRoomExclusion[0][iPhyIndex] = 0;
        }
    }
    if(iCanDispenseBoxCnt == 0){
        FireCUErrorAndSetErrorNum(wCUErrors, TRUE);
        return WFS_ERR_CDM_CASHUNITERROR;
    }

    CLEAR_ARRAY(wCUErrors);
    hRes = m_pAdapter->TestCashUnitsHT(byRoomExclusion, stTestDispInfo, wCUErrors);
    //更新钞箱计数
    ULONG ulDispenseCounts[ADP_MAX_CU_SIZE] = {0};
    ULONG ulRejectCounts[ADP_MAX_CU_SIZE] = {0};
    ULONG ulRetractCount[ADP_MAX_CU_SIZE] = {0};
    NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE];
    {
        for(int i = 0; i < pCDM->GetCUCount(); i++){
            iPhyIndex = pCDM->GetCUByNumber(i + 1)->GetIndex() - 1;
            ulDispenseCounts[iPhyIndex] = stTestDispInfo.wCountFromRoom[0][iPhyIndex];
            ulRejectCounts[iPhyIndex] = stTestDispInfo.wRBBoxRJCount[0][iPhyIndex];
        }

        USHORT usBoxIndex = 1;
        if(FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usBoxIndex) == 0){
            ulRetractCount[usBoxIndex - 1] = stTestDispInfo.wCountToRoom[0][3];

            for(int i = 0; i < 128; i++){
                if(stTestDispInfo.stPerDenomiNumofRJ[i].byNoteID != 0){
                    NoteIDCountArray[usBoxIndex - 1].push_back(NOTEID_COUNT(stTestDispInfo.stPerDenomiNumofRJ[i].byNoteID,
                                                                stTestDispInfo.stPerDenomiNumofRJ[i].ulCount));
                }
            }
        }

        CAutoUpdateCassData data(this, ulDispenseCounts, ulRejectCounts, ulRetractCount, NoteIDCountArray, FALSE);
    }    

    if(hRes == WFS_ERR_CDM_CASHUNITERROR){
        //CashUnitError事件上报
        FireCUErrorAndSetErrorNum(wCUErrors, TRUE);
    }

    //命令执行完成后，钞票搬送到出金口，再根据要求的位置进行二次搬送
    do {
        if(usRetractArea == WFS_CDM_RA_RETRACT || usRetractArea == WFS_CDM_RA_REJECT ||
           usRetractArea == WFS_CDM_RA_STACKER){
            //检查钞口是否有钞
            ADPSTATUSINFOR ADPStatus;
            GetUpdatedStatus(ADPStatus);
            if(ADPStatus.stOutPutPosition == ADP_INPOS_EMPTY){
                break;
            }

            //转换回收区域，计算索引
            ADP_RETRACTEARE eRetractArea;
            USHORT usIndex = 1;
            long lRet = GetRetractAreaAndIndex(usRetractArea, eRetractArea, usIndex, TRUE);
            if (lRet < 0)
            {
                Log(ThisModule, lRet,   "GetRetractAreaAndIndex()失败");
                hRes = (hRes == WFS_SUCCESS) ? WFS_ERR_CDM_UNSUPPOSITION : hRes;
                break;
            }

            //执行回收动作
            ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE];
            CLEAR_ARRAY(wCUErrors);
            lRet = m_pAdapter->Retract(eRetractArea, usIndex, wCUErrors);
            FireCUErrorAndSetErrorNum(wCUErrors, TRUE);

            //update cassette info
            UpdateRetractCountToCUManager(TRUE, usIndex);

            if (lRet < 0)
            {
                Log(ThisModule, lRet, "m_pAdapter->Retract(usRetractArea=%hd, usIndex=%hd)失败",
                    usRetractArea, usIndex);
                hRes = (hRes == WFS_SUCCESS) ? lRet : hRes;
            }
        }
    } while(false);

    //结果数据
    lpCUInfo = m_pCUManager->GetCUInterface_CDM()->BuildByXFSCDMFormat();    

    return hRes;
}

HRESULT XFS_CRSImp::CIMSetCashUnitInfo(const LPWFSCIMCASHINFO lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    m_bCdmCmd = false;
    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    assert(pCIM != NULL);

    return SetCashUnitInfo(ThisModule, lpCUInfo, pCIM);
}


HRESULT XFS_CRSImp::CDMReset(const LPWFSCDMITEMPOSITION lpResetIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = true;
    m_pAdapter->SetLFSCmdId(ADP_CDM_RESET);            //test#8
    return Reset(ThisModule, lpResetIn, m_pCUManager->GetCUInterface_CDM());
}

//////////////////////////////////////////////////////////////////////////
HRESULT XFS_CRSImp::CIMGetStatus(LPWFSCIMSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);

    m_CIMStatus.InitData();
    ICIMStatus *pIStatus = (ICIMStatus *)&m_CIMStatus;

    pIStatus->SetDeviceSt(ComputeDeviceState(ADPStatus, FALSE));
    pIStatus->SetSafeDoorSt(m_StatusConvertor.ADP2XFS(ADPStatus.stSafeDoor, SCT_SAFEDOOR, FALSE));
    pIStatus->SetIntermediateStackerSt(m_StatusConvertor.ADP2XFS(ADPStatus.stStacker, SCT_STACKER, FALSE));

    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);

    //处理StackerItem状态
    if (ADPStatus.stDevice != ADP_DEVICE_ONLINE ||
        /*(ADPStatus.stInPutPosition != ADP_INPOS_EMPTY && ADPStatus.stInPutPosition != ADP_INPOS_NOTEMPTY) ||*/
        (ADPStatus.stOutPutPosition != ADP_OUTPOS_EMPTY && ADPStatus.stOutPutPosition != ADP_OUTPOS_NOTEMPTY) ||
        (ADPStatus.stStacker != ADP_STACKER_EMPTY && ADPStatus.stStacker != ADP_STACKER_NOTEMPTY && ADPStatus.stStacker != ADP_STACKER_FULL))           //30-00-00-00(FT#0029)
    {
        pIStatus->SetStackerItemsSt(WFS_CIM_ACCESSUNKNOWN);
    }
    else
    {
        if (ADPStatus.stStacker == ADP_STACKER_NOTEMPTY || ADPStatus.stStacker == ADP_STACKER_FULL)            //30-00-00-00(FT#0029)
        {
            pIStatus->SetStackerItemsSt(WFS_CIM_NOCUSTOMERACCESS);
        }
        else
        {
            if (/*ADPStatus.stInPutPosition == ADP_INPOS_EMPTY &&*/
            ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY)
            {
                pIStatus->SetStackerItemsSt(WFS_CIM_NOITEMS);
            }
            else
            {
                if (ADPStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED)
                    pIStatus->SetStackerItemsSt(WFS_CIM_NOCUSTOMERACCESS);
                else
                {
                    WORD iRet = m_bCSItemsFromTS ? WFS_CIM_CUSTOMERACCESS : WFS_CIM_NOITEMS;
                    pIStatus->SetStackerItemsSt(iRet);
                }
            }
        }
    }

    WFSCIMINPOS InOutpos[2];
    InOutpos[0].fwPosition = CIM_INPUT_POSITION(CIMCaps.fwPositions);
    InOutpos[0].fwPositionStatus = m_StatusConvertor.ADP2XFS(ADPStatus.stInPutPosition, SCT_INPOS, FALSE);
    InOutpos[0].fwShutter = m_StatusConvertor.ADP2XFS(ADPStatus.stInShutter, SCT_INSHUTTER, FALSE);
    InOutpos[0].fwTransport = TransportADPToXFS(ADPStatus.stTransport, FALSE);
    InOutpos[0].fwTransportStatus = m_StatusConvertor.ADP2XFS(ADPStatus.stTransport, SCT_TRANSPORT, FALSE);

    InOutpos[1] = InOutpos[0];
    InOutpos[1].fwPosition = CIM_OUTPUT_POSITION(CIMCaps.fwPositions);
    InOutpos[1].fwPositionStatus = m_StatusConvertor.ADP2XFS(ADPStatus.stOutPutPosition, SCT_OUTPOS, FALSE);
    InOutpos[1].fwShutter = m_StatusConvertor.ADP2XFS(ADPStatus.stOutShutter, SCT_OUTSHUTTER, FALSE);
    pIStatus->AddPositionSt(InOutpos);

    pIStatus->SetDropBox(FALSE); //todo: Drop Box
    pIStatus->SetBanknoteReaderSt(m_StatusConvertor.ADP2XFS(ADPStatus.stBanknoteReader, SCT_NOTE_READER, FALSE));
    //处理Acceptor状态
    pIStatus->SetAcceptorSt(ComputeAcceptorState(ADPStatus));

    //处理扩展状态
    AddExtraStOrCap(pIStatus, (PAddExtraFunc)AddExtraStCIM);

    lpStatus = (LPWFSCIMSTATUS)&m_CIMStatus;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMGetCapabilities(LPWFSCIMCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);

    m_CIMCaps.InitData();
    ICIMCaps *lpCap = (ICIMCaps *)&m_CIMCaps;

    lpCap->SetType(WFS_CIM_SELFSERVICEBILL);
    lpCap->SetMaxCashInItems(CIMCaps.wMaxCashInItems);
    lpCap->SetCompound(TRUE);
    lpCap->SetShutter(CIMCaps.bShutter);
    lpCap->SetShutterControl(CIMCaps.bShutterControl);
    lpCap->SetRetractAreas(CIMCaps.fwRetractAreas);
    lpCap->SetRetractTransportActions(CIMCaps.fwRetractTransportActions);
    lpCap->SetRetractStackerActions(CIMCaps.fwRetractStackerActions);
    lpCap->SetSafeDoor(CIMCaps.bSafeDoor);
    lpCap->SetCashBox(FALSE);
    lpCap->SetRefill(FALSE);
    lpCap->SetIntermediateStacker(CIMCaps.fwIntermediateStacker);
    lpCap->SetItemsTakenSensor(CIMCaps.bItemsTakenSensor);
    lpCap->SetItemsInsertedSensor(CIMCaps.bItemsInsertedSensor);
    lpCap->SetPositions(CIMCaps.fwPositions);
    lpCap->SetExchangeType(WFS_CIM_EXBYHAND);
    AddExtraStOrCap(lpCap, (PAddExtraFunc)AddExtraCapCIM);

    lpCaps = (LPWFSCIMCAPS)&m_CIMCaps;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMGetCashUnitInfo(LPWFSCIMCASHINFO &lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    ICUInterface *pCIMCU = m_pCUManager->GetCUInterface_CIM();
    assert(pCIMCU != NULL);

    lpCUInfo = pCIMCU->BuildByXFSCIMFormat();
    if (lpCUInfo == NULL)
    {
        Log(ThisModule, -1, "pCIMCU->BuildByXFSCIMFormat()失败");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMGetCurrencyEXP(LPWFSCIMCURRENCYEXP *&lppCurrencyExp)
{
    THISMODULE(__FUNCTION__);
    lppCurrencyExp = (LPWFSCIMCURRENCYEXP *)m_Param.GetCurrencyExp();
    return 0;
}

HRESULT XFS_CRSImp::CIMGetBankNoteType(LPWFSCIMNOTETYPELIST &lpNoteList)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
//30-00-00-00(FS#0021)    LPWFSCIMNOTETYPELIST pNoteTypeList =
//30-00-00-00(FS#0021)    GCNTCDM_GET_OLD_STATUS(m_Param.GetCfgNoteTypeCantDepositMode()) ? //给应用原状态
//30-00-00-00(FS#0021)    m_Param.GetNoteTypeList() : GetNoteTypeListDipositable();
    LPWFSCIMNOTETYPELIST pNoteTypeList = m_Param.GetNoteTypeList();
    m_BanknoteTypes.ClearData();
    ICIMBanknoteTypes *lpType = (ICIMBanknoteTypes *)&m_BanknoteTypes;
    for (USHORT i = 0; i < pNoteTypeList->usNumOfNoteTypes; i++)
    {
        assert(pNoteTypeList->lppNoteTypes[i] != NULL);
        lpType->AddBanknoteTypes(pNoteTypeList->lppNoteTypes[i]);
    }

    lpNoteList = (LPWFSCIMNOTETYPELIST)&m_BanknoteTypes;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMGetCashInStatus(LPWFSCIMCASHINSTATUS &lpCashInStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();

    m_CashInStatus.ClearData();
    ICIMCashInStatus *lpStatus = (ICIMCashInStatus *)&m_CashInStatus;
    lpStatus->SetStatus(
    m_CashInInfor.GetAvailable() ? m_CashInInfor.GetState() : WFS_CIM_CIUNKNOWN);
    lpStatus->SetNumOfRefused(m_CashInInfor.GetRefusedCount());
    if (m_CashInInfor.GetAvailable())
    {
        for (USHORT i = 0; i < m_CashInInfor.GetNoteNumberCount(); i++)
        {
            lpStatus->SetCountByID(
            m_CashInInfor.GetIDOfNoteNumber(i),
            m_CashInInfor.GetCountOfNoteNumber(i));
        }
    }
    else
    {
        LPWFSCIMNOTETYPELIST lpNumberList = m_Param.GetNoteTypeList();
        lpStatus->SetCountByID(lpNumberList->lppNoteTypes[0]->usNoteID, 0);
    }

    lpCashInStatus = (LPWFSCIMCASHINSTATUS)&m_CashInStatus;

    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMCashInStart(const LPWFSCIMCASHINSTART lpData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);

    CHECK_CIM_OUTPUT_POS(lpData->fwOutputPosition, CIMCaps.fwPositions);
    CHECK_CIM_INPUT_POS(lpData->fwInputPosition, CIMCaps.fwPositions);

    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_NOT_IN_CASH_IN();

    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);
    VERIFY_SAFEDOOR_OPEN(ADPStatus.stSafeDoor, FALSE);          //30-00-00-00(FS#0004)

    //处理STACKER、通道、门口不为空或SHUTTER异常的情况
    long iRet = VerifyCanOperatoByStackerTransportShutter(ADPStatus, ThisModule, FALSE);
    if (iRet < 0)
        return iRet;

    WORD wAcceptorState = ComputeAcceptorState(ADPStatus, TRUE);
    if (wAcceptorState != WFS_CIM_ACCOK &&
        wAcceptorState != WFS_CIM_ACCCUSTATE)
    {
        Log(ThisModule, -1,
            "wAcceptorState(%hd) != WFS_CIM_ACCOK && != WFS_CIM_ACCCUSTATE",
            wAcceptorState);
        return WFS_ERR_HARDWARE_ERROR;
    }

    iRet = m_pAdapter->SetSupportNoteTypes(
           GCNTCDM_DEPOSIT_ALLOWED(m_Param.GetCfgNoteTypeCantDepositMode()) ?
           m_Param.GetNoteTypeList() :
           GetNoteTypeListDipositable());
    VERIFY_RESULT("m_pAdapter->SetSupportNoteTypes()", iRet);

    m_eWaitTaken = WTF_NONE;    //当执行新命令时，清除等待取钞关门标志
    m_bCashOutActive = FALSE;

    //执行物理动作
    iRet = m_pAdapter->StartCashIn();
    if (iRet < 0)
    {
        Log(ThisModule, iRet,
            "m_pAdapter->StartCashIn()失败");
        return iRet;
    }

    m_bProhibitRecycleBox = !lpData->bUseRecycleUnits;

    //implicit control shutter
    if (SHUTTER_CONTROL_BY_SP(CIMCaps))
    {
        long iRet = OpenShutter(ThisModule, TRUE, FALSE);
        if (iRet < 0)
        {
            return iRet;
        }
    }

    //设置CASH_IN_STATUS和CASH_IN事务标志
    m_CashInInfor.CashInStart();
    m_bCashInActive = TRUE;

    //入金限制数据初始化
    m_ulMaxNumber = 0xFFFFFFFF;                  //30-00-00-00(FS#0002)
    m_ulMaxAmount = 0xFFFFFFFF;                  //30-00-00-00(FS#0002)
    m_bCashInExecuted = false;                   //30-00-00-00(FS#0002)

    Log(ThisModule, 1, "CIMCashInStart()成功");
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMCashIn(LPWFSCIMNOTENUMBERLIST &lpResult)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_IN_CASH_IN();
    m_pAdapter->SetLFSCmdId(ADP_CASHIN);            //test#8

    m_bCashInExecuted = true;                       //30-00-00-00(FS#0002)
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);
    VERIFY_SAFEDOOR_OPEN(ADPStatus.stSafeDoor, FALSE);          //30-00-00-00(FS#0004)

    m_pAdapter->SetResultFlg();  //test#7 initialize

    m_CashInResult.ClearData();

    if (SHUTTER_CONTROL_BY_SP(CIMCaps))
    {
        long iRet;
        AUTO_FIRE_SHUTTER_STATUS();
        iRet =  CloseShutter(ThisModule, TRUE, FALSE);
        if (iRet < 0)
        {
            return iRet;
        }
    } else {            //AP控制开关门                               //30-00-00-00(FT#0006)
        if (m_Param.IsCheckDoorCloseBeforeCashIn())
        {
            ADPSTATUSINFOR ADPStatus;
            GetUpdatedStatus(ADPStatus);
            if ((m_adapterinfo.bHaveInShutter &&
                 ADPStatus.stInShutter == ADP_INSHUTTER_CLOSED) ||
                    (m_adapterinfo.bHaveOutShutter &&
                     ADPStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED))
            {
                Log(ThisModule, -1, "SHUTTER is CLOSE");
                return WFS_ERR_CIM_SHUTTERNOTOPEN;
            }
        }

        //关闭钞门
        GetUpdatedStatus(ADPStatus);                                //30-00-00-00(FT#0006)
        if(ADPStatus.stInShutter != ADP_INSHUTTER_CLOSED){          //30-00-00-00(FT#0006)
            if(CloseShutter(ThisModule, TRUE, FALSE) != 0){         //30-00-00-00(FT#0006)
                return WFS_ERR_CIM_SHUTTERNOTCLOSED;                //30-00-00-00(FT#0006)
            }                                                       //30-00-00-00(FT#0006)
        }                                                           //30-00-00-00(FT#0006)

        //检查入钞口是纸币(在关门时纸币可能被取走)
        GetUpdatedStatus(ADPStatus);                                //30-00-00-00(FT#0006)
        if(ADPStatus.stInPutPosition == ADP_INPOS_EMPTY){           //30-00-00-00(FT#0006)
            return WFS_ERR_CIM_NOITEMS;                             //30-00-00-00(FT#0006)
        }                                                           //30-00-00-00(FT#0006)
    }

    long iRet = m_pAdapter->SetSupportNoteTypes(
                GCNTCDM_DEPOSIT_ALLOWED(m_Param.GetCfgNoteTypeCantDepositMode()) ?
                m_Param.GetNoteTypeList() :
                GetNoteTypeListDipositable());
    VERIFY_RESULT("m_pAdapter->SetSupportNoteTypes()", iRet);

    //物理验钞
    ULONG ulRejectCount = 0;
    ADP_ERR_REJECT eRejectCause = ADP_ERR_REJECT_OK;    //回收原因
    char arycResult[256];
    CLEAR_ARRAY(arycResult);
//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(); //忽略一次ITEM插入取出事件
    iRet = m_pAdapter->ValidateAndCounting(arycResult, ulRejectCount, eRejectCause, m_ulMaxNumber, m_ulMaxAmount);      //30-00-00-00(FS#0002)
    if (iRet < 0)
    {
        Log(ThisModule, iRet,
            "m_pAdapter->ValidateAndCounting()失败");
        return iRet;
    }
    Log(ThisModule, 1,
        "m_pAdapter->ValidateAndCounting()成功: ulRejectCount=%d, eRejectCause=%d, arycResult=%s",
        ulRejectCount, eRejectCause, arycResult);

    //检验并保存结果数据
    if(m_Param.GetCashinRefuseCountAddFlag()){                          //30-00-00-00(FT#0034)
        m_CashInInfor.AddRefusedCount((USHORT)ulRejectCount);
    } else {                                                            //30-00-00-00(FT#0034)
        m_CashInInfor.SetRefusedCount((USHORT)ulRejectCount);           //30-00-00-00(FT#0034)
    }                                                                   //30-00-00-00(FT#0034)
    NOTEID_COUNT_ARRAY NoteIDCount;
    iRet = AnalyseCountResult(arycResult, NoteIDCount, FALSE, TRUE);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "AnalyseCashInResult(%s)失败", arycResult);
        return iRet;
    }

    ULONG ulCount = 0;
    ICIMCashInResult *lpCashInRes = (ICIMCashInResult *)&m_CashInResult;
    for (NOTEID_COUNT_ARRAY::iterator it = NoteIDCount.begin(); it != NoteIDCount.end(); it++)
    {
        lpCashInRes->SetCountByID(it->first, it->second);
        m_CashInInfor.AddNoteNumber(it->first, it->second);
        ulCount += it->second;
    }

    if (m_pAdapter->GetNoteSeiralNumbersCount() > 0 &&
        m_Param.GetInfoAvailableAfterCashIn() && ulCount != 0)
    {
        Log(ThisModule, 1, "发送事件WFS_EXEE_CIM_INFO_AVAILABLE");
        //FireInfoAvailable(m_pAdapter->GetNoteSeiralNumbersCount());
    }

    //FIRE拒钞事件
    USHORT usRejectCause = RejectCauseADP2XFS(eRejectCause);
    if (usRejectCause != 0)
    {
        Log(ThisModule, 1, "CIMFireInputRefuse(%hd)", usRejectCause);
        CIMFireInputRefuse(usRejectCause);
        CIMFireItemPresented();

        //由SP控制门的情况处理
        ADPCIMCAPS CIMCaps;
        m_pAdapter->GetCIMCapabilities(&CIMCaps);
        if (SHUTTER_CONTROL_BY_SP(CIMCaps))
        {
            if (ADPStatus.stOutShutter != ADP_OUTSHUTTER_OPEN
                && m_adapterinfo.bHaveOutShutter)
            {
                long iRet = OpenShutter(ThisModule, FALSE, FALSE);
                if (iRet < 0)
                {
                    return iRet;
                }
            }
            m_eWaitTaken = WTF_CIM;
        } else {                                        //30-00-00-00(FT#0060)
            m_eWaitTaken = WTF_CIM_TK_EVT;              //30-00-00-00(FT#0060)
        }                                               //30-00-00-00(FT#0060)
    }

    //检查是否为整捆钞票
    UpdateStatus(ThisModule, ADPStatus, TRUE);           //30-00-00-00(FT#0054)
    if (ulCount == 0 && ulRejectCount == 0 &&            //30-00-00-00(FT#0054)
        ADPStatus.stStacker == ADP_STACKER_EMPTY &&      //30-00-00-00(FT#0054)
        m_Param.GetCashInBoxReturnNOITEMS())             //30-00-00-00(FT#0054)
    {                                                                                      //30-00-00-00(FT#0054)
        Log(ThisModule, 1, "放入整捆钞票,已验钞数: %d, 已拒钞数: %d", ulCount, ulRejectCount);  //30-00-00-00(FT#0054)
        return WFS_ERR_CIM_NOITEMS;                                                        //30-00-00-00(FT#0054)
    }                                                                                      //30-00-00-00(FT#0054)

    //设置CASH_IN_STATUS
    m_CashInInfor.SetState(WFS_CIM_CIACTIVE);

    Log(ThisModule, 1, "CIMCashIn()成功");
    lpResult = (LPWFSCIMNOTENUMBERLIST)&m_CashInResult;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMCashInEnd(LPWFSCIMCASHINFO &lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_IN_CASH_IN();
    m_pAdapter->SetLFSCmdId(ADP_CASHINEND);            //test#8

    //修改CASHIN事务标志
    m_bCashInActive = FALSE;

    m_UpdateOfCU.ClearData();
    //处理无钞的情况
    if (m_CashInInfor.GetTotalNumber() == 0)
    {
        m_CashInInfor.SetState(WFS_CIM_CIOK);
        Log(ThisModule, -1,
            "没有钞票：m_CashInInfor.GetTotalNumber() == 0");
        //用户做存款但不放入钞票，如果这里返回则下一个用户做取款时会导致设备报571040A故障码
        //return WFS_ERR_CIM_NOITEMS;
    }

    //由SP控制门的情况处理
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);
    VERIFY_SAFEDOOR_OPEN(ADPStatus.stSafeDoor, FALSE);          //30-00-00-00(FS#0004)

    if (SHUTTER_CONTROL_BY_SP(CIMCaps))
    {
        if (ADPStatus.stOutShutter != ADP_INSHUTTER_CLOSED
            && m_adapterinfo.bHaveOutShutter)
        {
            AUTO_FIRE_SHUTTER_STATUS();
            long iRet = CloseShutter(ThisModule, FALSE, FALSE);
            if (iRet < 0)
            {
                return iRet;
            }
        }
    }

    //更新状态，以读取钞口状态
    long iRet;
    iRet = UpdateStatus(ThisModule, ADPStatus, TRUE);
    if (iRet < 0)
    {
        m_CashInInfor.SetState(WFS_CIM_CIUNKNOWN);
        Log(ThisModule, iRet,
            "UpdateStatus()失败");
        return iRet;
    }

    //处理门口有钞的情况
    if (ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY
        || ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)
    {
        //30-00-00-00(FT#0014) add start
        WORD wStatus;
        switch(m_Param.GetCashInStatusWhenCSNotEmpty()){
        case 0:
            wStatus = WFS_CIM_CIOK;
            break;
        case 1:
            wStatus = WFS_CIM_CIUNKNOWN;
            break;
        case 2:
            wStatus = WFS_CIM_CIACTIVE;
            break;
        default:
            wStatus = WFS_CIM_CIOK;
            break;
        }
        m_CashInInfor.SetState(wStatus);
        //30-00-00-00(FT#0014) add end
        Log(ThisModule, -1, "门口有钞InPutPosition:%s,OutPutPosition:%s",
            ADPStatus.stInPutPosition == 1 ? "NotEmpty" : "", ADPStatus.stOutPutPosition == 1 ? "NotEmpty" : "");
        return WFS_ERR_CIM_POSITION_NOT_EMPTY;
    }

    VERIFY_DISABLE_UNUSED_CASSETTE(FALSE); //禁用不相关的钞箱

    //执行物理存钞
    char ppNoteCounts[ADP_MAX_CU_SIZE][256];
    ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(ppNoteCounts);
    CLEAR_ARRAY(arywCUError);
//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(FALSE, TRUE); //忽略一次ITEM插入取出事件
    DisableUnusedCassettes(FALSE);
    int iResult = m_pAdapter->StoreCash(ppNoteCounts, arywCUError);
    FireCUErrorAndSetErrorNum(arywCUError, FALSE);

    //存款失败的情况下仍更新钞箱数据 by tucy-2013.5.17
    if (m_pAdapter->GetNoteSeiralNumbersCount() > 0 &&
        m_Param.GetInfoAvailableAfterCashInEnd() &&
        m_CashInInfor.GetTotalNumber() != 0)
    {
        Log(ThisModule, 1, "发送事件WFS_EXEE_CIM_INFO_AVAILABLE");
        //FireInfoAvailable(m_pAdapter->GetNoteSeiralNumbersCount());
    }
    //分析进钞数据
    NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE];
    for (USHORT i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        iRet = AnalyseCountResult(ppNoteCounts[i], NoteIDCountArray[i], TRUE, FALSE);
        if (iRet < 0)
        {
            Log(ThisModule, iRet,
                "AnalyseCashInResult(ppNoteCounts[%hd]=%s)失败", i, ppNoteCounts[i]);
            return iRet;
        }
        if (NoteIDCountArray[i].size() == 0)
            continue;

        //确保钞票ID与钞箱类型匹配
        ICashUnit *pCU = m_pCUManager->GetCUInterface_CIM()->GetFirstCUByPHIndex(i + 1);
        if (pCU == NULL)
        {
            Log(ThisModule, iRet,
                "在CIM中查找第%hd个物理钞箱失败(NoteID.size=%d)",
                i + 1, NoteIDCountArray[i].size());
            return WFS_ERR_INTERNAL_ERROR;
        }
        LPUSHORT pNoteIDs = FormatCUNoteIDs(pCU, m_Param.GetNoteTypeList());
        for (NOTEID_COUNT_ARRAY::iterator it = NoteIDCountArray[i].begin();
             it != NoteIDCountArray[i].end(); it++)
        {
            if (!FindNoteIDInList(it->first, pNoteIDs))
            {
                Log(ThisModule, iRet,
                    "第%hd个物理钞箱返回第%d个币种ID(%hd)是钞箱未定义的(usNumber=%hd)",
                    i + 1, it - NoteIDCountArray[i].begin() + 1, it->first, pCU->GetNumber());
                //return WFS_ERR_INTERNAL_ERROR;
            }
        }
    }

    //更新钞箱数据
    CAutoUpdateCassData _auto_update_cass_data(this, NULL, NULL, NULL, NoteIDCountArray, FALSE);

    ICIMSetUpdatesOfCU *pUpdates = (ICIMSetUpdatesOfCU *)&m_UpdateOfCU;
    //保存数据到输出接口中
    iRet = SaveCashInEndResult(NoteIDCountArray, pUpdates);
    VERIFY_RESULT("SaveCashInEndResult", iRet);

    if (iResult < 0)
    {
        m_CashInInfor.SetState(WFS_CIM_CIUNKNOWN);
        Log(ThisModule, iResult, "m_pAdapter->StoreCash()失败");
        return iResult;
    }

    Log(ThisModule, 1, "m_pAdapter->StoreCash()成功:[0]=%s,[1]=%s,[2]=%s,[3]=%s,[4]=%s,[5]=%s",
        ppNoteCounts[0], ppNoteCounts[1], ppNoteCounts[2], ppNoteCounts[3], ppNoteCounts[4], ppNoteCounts[5]);

    m_CashInInfor.SetState(WFS_CIM_CIOK);   //修改CASHINSTATUS状态

    Log(ThisModule, 1, "CIMCashInEnd()成功");
    lpCUInfo = (LPWFSCIMCASHINFO)m_UpdateOfCU;
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMCashInRollBack(LPWFSCIMCASHINFO &lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_IN_CASH_IN();
    m_pAdapter->SetLFSCmdId(ADP_OTHER);            //test#8

    //由SP控制门的情况处理
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);

    if (SHUTTER_CONTROL_BY_SP(CIMCaps))
    {
        AUTO_FIRE_SHUTTER_STATUS();
        if (ADPStatus.stOutShutter != ADP_OUTSHUTTER_CLOSED
            && m_adapterinfo.bHaveOutShutter)
        {
            long iRet = CloseShutter(ThisModule, FALSE, FALSE);
            if (iRet < 0)
            {
                m_CashInInfor.SetState(WFS_CIM_CIUNKNOWN);
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    }

    if (m_Param.IsCheckCSTSEmptyWhenRollBack())
    {
        //处理无钞的情况
        if (m_CashInInfor.GetTotalNumber() == 0)
        {
            m_CashInInfor.SetState(WFS_CIM_CIROLLBACK);
            Log(ThisModule, -1,
                "没有钞票：m_CashInInfor.GetTotalNumber() == 0");
            return WFS_ERR_CIM_NOITEMS;
        }

        //更新状态，以读取钞口状态
        long iRet;
        ADPSTATUSINFOR ADPStatus;
        iRet = UpdateStatus(ThisModule, ADPStatus, TRUE);
        if (iRet < 0)
        {
            m_CashInInfor.SetState(WFS_CIM_CIUNKNOWN);
            Log(ThisModule, iRet,
                "UpdateStatus()失败");
            return iRet;
        }

        //处理门口有钞的情况
        /*if (ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY ||
            ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)
        {
            Log(ThisModule, -1, "门口有钞: InPutPosition:%s,OutPutPosition:%s",
                ADPStatus.stInPutPosition == 1 ? "NotEmpty" : "", ADPStatus.stOutPutPosition == 1 ? "NotEmpty" : "");
            return WFS_ERR_CIM_POSITION_NOT_EMPTY;
        }*/
    }

    m_eWaitTaken = WTF_NONE;    //当执行新命令时，清除等待取钞关门标志

    USHORT usIndex = 1;
    if (FindCassetteADPIndex(FCAIF_REJECT, TRUE, usIndex) < 0)
    {
        usIndex = 1;
        if (FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usIndex) < 0)
        {
            Log(ThisModule, -1, "没有拒收钞和回收箱");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    //执行物理退钞
    char ppNoteCounts[ADP_MAX_CU_SIZE][256];
    ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(ppNoteCounts);
    CLEAR_ARRAY(arywCUError);
//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(TRUE, FALSE); //忽略一次ITEM插入取出事件
    long iRet = m_pAdapter->RollBack(arywCUError);
    FireCUErrorAndSetErrorNum(arywCUError, FALSE);
    UpdateRetractCountToCUManager(FALSE, usIndex);
    GetUpdatedStatus(ADPStatus);
    if (ADPStatus.stStacker == ADP_STACKER_EMPTY)
    {
        m_bCashInActive = FALSE;
    }

    if (iRet < 0)
    {
        m_CashInInfor.SetState(WFS_CIM_CIUNKNOWN);
        Log(ThisModule, iRet,
            "m_pAdapter->RollBack()失败");
        return iRet;
    }


    m_CashInInfor.SetState(WFS_CIM_CIROLLBACK); //修改CASHINSTATUS状态
    //由SP控制门的情况处理
    if (SHUTTER_CONTROL_BY_SP(CIMCaps))
    {
        if (ADPStatus.stOutShutter != ADP_OUTSHUTTER_OPEN
            && m_adapterinfo.bHaveOutShutter)
        {
            long iRet;
            iRet = OpenShutter(ThisModule, FALSE, FALSE);
            if (iRet < 0)
            {
                return WFS_ERR_CIM_SHUTTERNOTOPEN;
            }
        }

        m_dwCloseShutterCount = 0;
        m_eWaitTaken = WTF_CIM;
    } else {                                //30-00-00-00(FT#0060)
        m_eWaitTaken = WTF_CIM_TK_EVT;      //30-00-00-00(FT#0060)
    }                                       //30-00-00-00(FT#0060)
    CIMFireItemPresented();

    iRet = CIMGetCashUnitInfo(lpCUInfo);
    if (iRet < 0)
    {
        return iRet;
    }
    Log(ThisModule, 1, "CIMRollback()成功");

    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMRetract(const LPWFSCIMRETRACT lpData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    m_pAdapter->SetLFSCmdId(ADP_CIM_RETRACT);            //test#8
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    CHECK_CIM_OUTPUT_POS(lpData->fwOutputPosition, CIMCaps.fwPositions);

    m_bCashInActive = FALSE;

    HRESULT hRes = Retract(ThisModule, lpData->fwOutputPosition, lpData->usRetractArea, lpData->usIndex, FALSE);
    if (hRes != WFS_SUCCESS)
    {
        if (m_CashInInfor.GetAvailable())
        {
            m_CashInInfor.SetState(WFS_CIM_CIUNKNOWN);
        }
        return hRes;
    }

    //WFS_CIM_CIRETRACT是否包括Rollback后的Retract
    if (m_CashInInfor.GetAvailable())
    {
        m_CashInInfor.SetState(WFS_CIM_CIRETRACT);
    }
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMOpenShutter(WORD wPos)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    CHECK_CIM_POS(wPos, CIMCaps.fwPositions);
    VERIFY_SUPPORT_SHUTER(CIMCaps);


    //更新状态
    long iRet;
    ADPSTATUSINFOR ADPStatus;
    iRet = UpdateStatus(ThisModule, ADPStatus, TRUE);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "UpdateStatus()失败");
        return iRet;
    }

    //检查钞门是否打开
    if ((ADPStatus.stInShutter == ADP_INSHUTTER_OPEN)
        && (ADPStatus.stOutShutter == ADP_OUTSHUTTER_OPEN))
    {
        Log(ThisModule, -1, "INSHUTTER钞门已经打开");
        return m_Param.GetShutterOpenedErrorSupp() ? WFS_ERR_CIM_SHUTTEROPEN : WFS_SUCCESS;
    }

//30-00-00-00(FT#0053)    HRESULT hRet = OpenShutter(ThisModule, FALSE, FALSE);
//30-00-00-00(FT#0053)    if (hRet < 0)
//30-00-00-00(FT#0053)    {
//30-00-00-00(FT#0053)        return hRet;
//30-00-00-00(FT#0053)    }

    HRESULT hRet = OpenShutter(ThisModule, TRUE, FALSE);            //30-00-00-00(FT#0053)
    return hRet;
}

HRESULT XFS_CRSImp::CIMCloseShutter(WORD wPos)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    CHECK_CIM_POS(wPos, CIMCaps.fwPositions);
    VERIFY_SUPPORT_SHUTER(CIMCaps);

    //更新状态
    long iRet;
    ADPSTATUSINFOR ADPStatus;
    iRet = UpdateStatus(ThisModule, ADPStatus, TRUE);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "UpdateStatus失败");
        return iRet;
    }

    //检查钞门是否关闭
    if ((ADPStatus.stInShutter == ADP_INSHUTTER_CLOSED) &&
        (ADPStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED))
    {
        Log(ThisModule, -1, "INSHUTTER钞门已经关闭");
        return m_Param.GetShutterClosedErrorSupp() ? WFS_ERR_CIM_SHUTTERCLOSED : WFS_SUCCESS;
    }

//30-00-00-00(FT#0053)    HRESULT hRet = CloseShutter(ThisModule, FALSE, FALSE);
//30-00-00-00(FT#0053)    if (hRet < 0)
//30-00-00-00(FT#0053)    {
//30-00-00-00(FT#0053)        return hRet;
//30-00-00-00(FT#0053)    }

    HRESULT hRet = CloseShutter(ThisModule, TRUE, FALSE);               //30-00-00-00(FT#0053)
    return hRet;
}

HRESULT XFS_CRSImp::CIMStartEXChange(const LPWFSCIMSTARTEX lpStartEx, LPWFSCIMCASHINFO &lpCUInfor)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    do
    {
        if (lpStartEx->fwExchangeType == WFS_CIM_EXBYHAND)
        {
            break;
        }
        else if (lpStartEx->fwExchangeType == WFS_CIM_EXTOCASSETTES ||
                 lpStartEx->fwExchangeType == WFS_CIM_CLEARRECYCLER ||
                 lpStartEx->fwExchangeType == WFS_CIM_DEPOSITINTO)
        {
            Log(ThisModule, -1,
                "lpStartEx->fwExchangeType(%hd) != WFS_CIM_EXBYHAND",
                lpStartEx->fwExchangeType);
            return WFS_ERR_UNSUPP_DATA;
        }
        else
        {
            Log(ThisModule, -1,
                "lpStartEx->fwExchangeType(%hd)的值在XFS规范中未定义",
                lpStartEx->fwExchangeType);
            return WFS_ERR_INVALID_DATA;
        }
    } while (0);

    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    assert(pCIM != NULL);

    HRESULT hRet = StartExchange(ThisModule, lpStartEx->usCount, lpStartEx->lpusCUNumList, pCIM);
    if (hRet == WFS_SUCCESS)
    {
        return CIMGetCashUnitInfo(lpCUInfor);
    }
    else
    {
        return hRet;
    }

    return hRet;
}

HRESULT XFS_CRSImp::CIMEndEXChange(const LPWFSCIMCASHINFO lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    if (m_Param.IsEndExchangeAutoClearCount())
    {
        ClearCountWhenEndExchange(lpCUInfo);
    }

    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    assert(pCIM != NULL);

    return EndExchange(ThisModule, lpCUInfo, pCIM);
}

HRESULT XFS_CRSImp::CIMReset(const LPWFSCIMITEMPOSITION lpResetIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    m_pAdapter->SetLFSCmdId(ADP_CIM_RESET);            //test#8
    return Reset(ThisModule, (LPWFSCDMITEMPOSITION)lpResetIn, m_pCUManager->GetCUInterface_CIM());
}

HRESULT XFS_CRSImp::CIMConfigureCashInUnits(const LPWFSCIMCASHINTYPE *lppCashInType)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;

    CHECK_PTR_VALID(lppCashInType);
    CHECK_PTR_VALID(lppCashInType[0]);

    VERIFY_NOT_IN_EXCHANGE(FALSE);


    //保存输入数据到钞箱管理模块中
    assert(m_pCUManager != NULL);
    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    assert(pCIM != NULL);

    LPWFSCIMCASHINTYPE pType = *lppCashInType;
    while (pType != NULL)
    {
        CHECK_PTR_VALID(pType->lpusNoteIDs);
        ICashUnit *pCU = pCIM->GetCUByNumber(pType->usNumber);
        if (pCU == NULL)
        {
            Log(ThisModule, -1,     "pCIM->GetCUByNumber(pType->usNumber=%hd)失败", pType->usNumber);
            return WFS_ERR_CIM_INVALIDCASHUNIT;
        }
        if (pCU->GetType() != ADP_CASSETTE_CASHIN &&
            pCU->GetType() != ADP_CASSETTE_RECYCLING)
        {
            Log(ThisModule, -1,     "钞箱类型(%d)不为CASHIN或RECYCLING(pType->usNumber=%hd)",
                pCU->GetType(),
                pType->usNumber);
            return WFS_ERR_CIM_INVALIDCASHUNIT;
        }
        if (pCU->GetCount() > 0)
        {
            Log(ThisModule, -1,     "钞箱(pType->usNumber=%hd)不为空: pCU->GetCount()=%d",
                pType->usNumber, pCU->GetCount());
            return WFS_ERR_INVALID_DATA;
        }

        if ((pType->dwType & WFS_CIM_CITYPINDIVIDUAL) != 0)
        {
            if (pType->lpusNoteIDs == NULL)
            {
                Log(ThisModule, -1,     "pType->lpusNoteIDs == NULL(usNumber=%hd)", pType->usNumber);
                return WFS_ERR_INVALID_DATA;
            }
            USHORT usNoteIDCount = BRMCASHUNITINFOR::GetNoteIDCount(pType->lpusNoteIDs);
            if (usNoteIDCount == 0)
            {
                Log(ThisModule, -1,     "usNoteIDCount==0(usNumber=%hd, dwType=%d)",
                    pType->usNumber, pType->dwType);
                return WFS_ERR_INVALID_DATA;
            }
            char cCurrencyID[3];
            pCU->GetCurrencyID(cCurrencyID);
            if (pCU->GetType() == ADP_CASSETTE_RECYCLING &&
                (usNoteIDCount == 0 || !NoteIDsAreSameCurrencyValue(pType->lpusNoteIDs, usNoteIDCount,
                                                                    m_Param.GetNoteTypeList(), cCurrencyID, pCU->GetValues())))
            {
                Log(ThisModule, -1,
                    "RECYCLING cassette's Currency or Value is different with NoteID Or NoteID isn't exist(usNoteIDCount=%hd)(Number=%hd)",
                    usNoteIDCount, pType->usNumber);
                return WFS_ERR_INVALID_DATA;
            }
            long iRet = pCU->SetNoteIDs(pType->lpusNoteIDs, usNoteIDCount);
            if (iRet < 0)
            {
                Log(ThisModule, iRet, "pCU->SetNoteIDs(pType->usNumber=%hd)失败", pType->usNumber);
                return iRet;
            }
            pCU->ClearNoteNumbers();
        }
        pCU->SetItemType(pType->dwType);

        lppCashInType++;
        pType = *lppCashInType;
    }

    //从钞箱管理模块中更新数据到适配层中
    long iRet = UpdateADPCassInfoFromCUManager();
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "UpdateADPCassInfoFromCUManager()失败");
        return iRet;
    }

    //根据情况进行复位
    if (m_pAdapter->IsNeedReset())
    {
        iRet = Reset(ThisModule, NULL, m_pCUManager->GetCUInterface_CIM(), TRUE);
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "Reset()失败");
            return iRet;
        }
    }
    Log(ThisModule, 1, "CIMConfCashInUnit()成功");
    return WFS_SUCCESS;
}

HRESULT XFS_CRSImp::CIMConfigureNoteTypes(const LPUSHORT lpusNoteIDs)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    m_bCdmCmd = false;
    CHECK_PTR_VALID(lpusNoteIDs);

    VERIFY_NOT_IN_EXCHANGE(FALSE);

    //组织NOTE TYPE数据
    LPWFSCIMNOTETYPELIST pList = m_Param.GetNoteTypeList();
    assert(pList != NULL);
    assert(pList->usNumOfNoteTypes > 0 && pList->lppNoteTypes != NULL);

    //校验输入的币种列种有效
    USHORT usIndex = 0;
    USHORT i = 0;
    while (lpusNoteIDs[usIndex] != 0)
    {
        for (i = 0; i < pList->usNumOfNoteTypes; i++)
        {
            assert(pList->lppNoteTypes[i] != NULL);
            if (pList->lppNoteTypes[i]->usNoteID == lpusNoteIDs[usIndex])
                break;
        }
        if (i == pList->usNumOfNoteTypes)
        {
            Log(ThisModule, -1, "第%hd个NoteID(%hd)无效", usIndex + 1, lpusNoteIDs[usIndex]);
            return WFS_ERR_UNSUPP_DATA;
        }

        usIndex++;
    }

    //设置到配置列表中
    for (USHORT i = 0; i < pList->usNumOfNoteTypes; i++)
    {
        assert(pList->lppNoteTypes[i] != NULL);
        if (FindNoteIDInList(pList->lppNoteTypes[i]->usNoteID, lpusNoteIDs))
        {
            pList->lppNoteTypes[i]->bConfigured = TRUE;
        }
        else
        {
            pList->lppNoteTypes[i]->bConfigured = FALSE;
        }
    }

    //保存到配置文件
    m_Param.SaveNoteTypeList();
    Log(ThisModule, 1, "CIMConfNoteTypes()成功");
    return WFS_SUCCESS;
}

bool XFS_CRSImp::UpdatePostionStatus()
{
    THISMODULE(__FUNCTION__);
    CAutoSaveDataAndFireEvent _auto_save_data_and_fire_event(this);

    long iRet = m_pAdapter->UpdatePostionStatus();
    if (iRet < 0)
    {
        MLog(ThisModule, iRet, "m_pAdapter->UpdatePostionStatus()失败");
    }
    return iRet >= 0;
}

bool XFS_CRSImp::UpdateStatus()
{
    THISMODULE(__FUNCTION__);
    CAutoSaveDataAndFireEvent _auto_save_data_and_fire_event(this);
    //更新状态
    long iRet = m_pAdapter->UpdateStatus();
    if (iRet < 0)
    {
        MLog(ThisModule, iRet, "m_pAdapter->UpdateStatus()失败");
    }
    return iRet >= 0;
}

//////////////////////////////////////////////////////////////////////////

long XFS_CRSImp::UpdateADPCassInfoFromCUManager()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();();
    USHORT i;

    //首先清除aryCUInfor钞箱信息
    BRMCASHUNITINFOR aryCUInfor[ADP_MAX_CU_SIZE];
    for (i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        aryCUInfor[i].SetType(ADP_CASSETTE_UNKNOWN);
        aryCUInfor[i].SetCurrencyID("   ");
        aryCUInfor[i].SetNoteIDs(NULL);
        aryCUInfor[i].SetValue(0);
        aryCUInfor[i].SetCount(0);
    }

    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    assert(pCDM != NULL && pCIM != NULL);
    ICUInterface *pCUInterfaces[2] = { pCDM, pCIM };
    //save ADP cassette info to aryCUInfor
    for (USHORT usInterface = 0; usInterface < 2; usInterface++)
    {
        ICUInterface *pCUInterface = pCUInterfaces[usInterface];
        USHORT usCount;
        usCount = pCUInterface->GetCUCount();
        for (i = 0; i < usCount; i++)
        {
            ICashUnit *pCU = pCUInterface->GetCUByNumber(i + 1);
            assert(pCU != NULL);
            USHORT usIndex = pCU->GetIndex();
            if (usIndex < 1 || usIndex > ADP_MAX_CU_SIZE)
            {
                Log(ThisModule, -1, "%s第%hd逻辑钞箱(usNumber=%hd)的物理钞箱索引(%hd)非法",
                    (usInterface == 0 ? "CDM" : "CIM"), i + 1, i + 1, usIndex);
                return WFS_ERR_INTERNAL_ERROR;
            }
            usIndex--;

            aryCUInfor[usIndex].SetType(pCU->GetType());
            char cCurrencyID[3];
            pCU->GetCurrencyID(cCurrencyID);
            aryCUInfor[usIndex].SetCurrencyID(cCurrencyID);
            aryCUInfor[usIndex].SetValue(pCU->GetValues());
            aryCUInfor[usIndex].SetCount(pCU->GetCount());
            if (usInterface == 1)
            {
                SetNoteIDToADPCashInfo(pCU, aryCUInfor[usIndex]);
            }
        }
    }

    //设置适配层的钞箱信息
    for (i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        switch (aryCUInfor[i].GetType())
        {
        case ADP_CASSETTE_RETRACT:
            if (aryCUInfor[i].GetNoteIDCount() == 0)
            {
                aryCUInfor[i].AddNoteID(NOTE_ID_UNKNOWN);
            }
            break;

        case ADP_CASSETTE_CASHIN:
            if (aryCUInfor[i].GetNoteIDCount() == 0)
            {
                Log(ThisModule, -1, "第%hd个物理钞箱是进钞箱NoteID个数为0", i + 1);
                aryCUInfor[i].SetType(ADP_CASSETTE_UNKNOWN);
            }
            break;

        case ADP_CASSETTE_RECYCLING:
            USHORT usNoteIDCount = aryCUInfor[i].GetNoteIDCount();
            if (usNoteIDCount == 0 || !NoteIDsAreSameCurrencyValue((LPUSHORT)aryCUInfor[i].GetNoteIDs(), usNoteIDCount,
                                                                   m_Param.GetNoteTypeList(), aryCUInfor[i].GetCurrencyID(), aryCUInfor[i].GetValue()))
            {
                Log(ThisModule, -1,
                    "第%hd个物理钞箱是循环箱且NoteID对应的币种面值与钞箱的币种面值不一致或币种ID不存在(usNoteIDCount=%hd)", i + 1, usNoteIDCount);
                aryCUInfor[i].SetType(ADP_CASSETTE_UNKNOWN);
            }
            break;
        }

        long iRet = m_pAdapter->SetCassetteInfo(
                    i + 1,                          //Hopper ID: 1 ~ ADP_MAX_CU_SIZE
                    (char *)aryCUInfor[i].GetCurrencyID(),
                    aryCUInfor[i].GetType(),
                    aryCUInfor[i].GetValue(),
                    (USHORT *)aryCUInfor[i].GetNoteIDs(),
                    aryCUInfor[i].GetCount());
        char szBuf[100];
        sprintf(szBuf, "m_pAdapter->SetCassetteInfo(%d)", i + 1);
        VERIFY_RESULT(szBuf, iRet);
    }
    return 0;
}

void XFS_CRSImp::GetUpdatedStatus(ADPSTATUSINFOR &ADPStatus)
{
    m_pAdapter->GetStatus(ADPStatus);
    if (m_Param.ErrWhenStackNotEmpty() &&
        !m_bCDMExchangeActive && !m_bCIMExchangeActive &&
        !m_bCashInActive && !m_bCashOutActive &&
        m_PresentInfor.GetPresentState() != WFS_CDM_NOTPRESENTED &&
        ADPStatus.stStacker == ADP_STACKER_NOTEMPTY)
    {
        ADPStatus.stDevice = ADP_DEVICE_HWERROR;
    }

    if (m_dwCloseShutterCount > m_Param.GetCloseShutterCountBeforeJammed())
    {
        ADPStatus.stDevice = ADP_DEVICE_HWERROR;
        ADPStatus.stOutShutter = ADP_OUTSHUTTER_JAMMED;
    }
}

//test#7
int XFS_CRSImp::GetTooManeyFlag()
{
    return m_pAdapter->GetResultFlg();
}

void XFS_CRSImp::SetExeFlag(DWORD dwCmd, BOOL bInExe)
{
    m_bInCmdExecute = bInExe;
    m_dwCurrentCommand = dwCmd;
    CINIFileReader ConfigFile;
    ConfigFile.LoadINIFile(m_strConfigFile);
    CINIWriter cINI = ConfigFile.GetWriterSection("BRMInfo");
    string strName = "ExCmdCode";
    //cINI.SetValue(strName, m_dwCurrentCommand);
}

WORD XFS_CRSImp::ComputeDeviceState(const ADPSTATUSINFOR &ADPStatus, BOOL bCDM)
{

    WORD fwDevice;
    if (m_pAdapter->IsNeedReset() && ADPStatus.stDevice == ADP_DEVICE_ONLINE)// 有些情况下如修改了钞箱类型，IsNeedReset()满足且机芯状态也正常，但机芯必须复位后才能工作，此时逻辑设定机芯故障
    {
        fwDevice = bCDM ? WFS_CDM_DEVHWERROR : WFS_CIM_DEVHWERROR;
    }
    else if (m_bInCmdExecute)
    {
        fwDevice = bCDM ? WFS_CDM_DEVBUSY : WFS_CIM_DEVBUSY;
    }
    else if (bCDM && m_Param.IsCDMStatusBusyWhenCashInActive() && m_bCashInActive)  // 在处于进钞事务时才设置为BUSY
    {
        fwDevice = WFS_CDM_DEVBUSY;
    }
    else
    {
        fwDevice = m_StatusConvertor.ADP2XFS(ADPStatus.stDevice, SCT_DEVICE, bCDM);
    }
    return fwDevice;
}

WORD XFS_CRSImp::ComputeDispenseState(const ADPSTATUSINFOR &ADPStatus, BOOL *pbStopByHardwareError, BOOL bLog)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();
    STOPDENOMINATE_BY_HARDWARE(TRUE);
    //处理安全门打开的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN, WFS_CDM_DISPCUSTOP);         //30-00-00-00(FS#0004)

    //处理门状态不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stOutShutter == ADP_OUTSHUTTER_UNKNOWN, WFS_CDM_DISPCUSTOP);

    //处理传送通道不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stTransport == ADP_TRANSPORT_UNKNOWN, WFS_CDM_DISPCUSTOP);

    //处理钞门状态不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stOutPutPosition == ADP_OUTPOS_UNKNOWN, WFS_CDM_DISPCUSTOP);

    STOPDENOMINATE_BY_HARDWARE(FALSE);
    //处理当STACKER、通道、门口不为空或SHUTTER打开时是否停止存取款
    if (AcceptorDispenseStopByStackerTransportShutter(ADPStatus, TRUE))
    {
        if (bLog)
        {
            Log(ThisModule, -1, "AcceptorDispenseStopByStackerTransportShutter(ADPStatus)=%d",
                AcceptorDispenseStopByStackerTransportShutter(ADPStatus, TRUE));
        }
        return WFS_CDM_DISPCUSTOP;
    }
    //根据钞箱状态判定DISPENSE状态
    BOOL bHasCassBoxWarning = FALSE; //是否有出钞箱故障、警告
    BOOL bAllOutCassBoxBad = TRUE;  //是否所有出钞箱故障
    BOOL bHasRetractBox = FALSE;//是否有回收箱
    BOOL bRejectBoxBad = TRUE;  //拒钞箱是否故障
    BOOL bHasRejectBox = FALSE; //是否有拒钞箱
    BOOL bRetractBoxBad = TRUE; //回收箱是否故障
    ICUInterface *pCDMCU = m_pCUManager->GetCUInterface_CDM();
    USHORT usCount = pCDMCU->GetCUCount();
    for (int i = 0; i < usCount; i++)
    {
        ICashUnit *pCU = pCDMCU->GetCUByNumber(i + 1);
        if (pCU->GetAppLock() &&
            pCU->GetType() != ADP_CASSETTE_REJECT &&
            pCU->GetType() != ADP_CASSETTE_RETRACT)
            continue;

        switch (pCU->GetType())
        {
        case ADP_CASSETTE_BILL:
        case ADP_CASSETTE_RECYCLING:
            if (CUCanDispense(pCU))
            {
                bAllOutCassBoxBad = FALSE;
                if (pCU->GetStatus() == ADP_CASHUNIT_LOW)
                {
                    bHasCassBoxWarning = TRUE;
                }
            }
            else
            {
                bHasCassBoxWarning = TRUE;
            }
            break;
        case ADP_CASSETTE_REJECT:
            bHasRejectBox = TRUE;
            if (CUCanAccept(pCU, m_Param.GetNoteTypeList()))
            {
                bRejectBoxBad = FALSE;
                if (pCU->GetStatus() == ADP_CASHUNIT_HIGH)
                {
                    bHasCassBoxWarning = TRUE;
                }
            }
            else
            {
                bHasCassBoxWarning = TRUE;
            }
            break;
        case ADP_CASSETTE_RETRACT:
            bHasRetractBox = TRUE;
            if (CUCanAccept(pCU, m_Param.GetNoteTypeList()))
            {
                bRetractBoxBad = FALSE;
                if (pCU->GetStatus() == ADP_CASHUNIT_HIGH)
                {
                    bHasCassBoxWarning = TRUE;
                }
            }
            else
            {
                bHasCassBoxWarning = TRUE;
            }
            break;
        }
    }

    //没有回收箱和拒钞箱
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(!bHasRetractBox && !bHasRejectBox, WFS_CDM_DISPCUSTOP);

    //没有拒钞箱并且回收箱故障
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(!bHasRejectBox && bRetractBoxBad, WFS_CDM_DISPCUSTOP);

    //没有回收箱并且拒钞箱故障
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(!bHasRetractBox && bRejectBoxBad, WFS_CDM_DISPCUSTOP);

    //回收箱和拒钞箱都故障
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(bRetractBoxBad && bRejectBoxBad, WFS_CDM_DISPCUSTOP);

    //所有出钞箱故障
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(bAllOutCassBoxBad, WFS_CDM_DISPCUSTOP);

    STOPDENOMINATE_BY_HARDWARE(TRUE);
    //处理STACKER不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stStacker == ADP_STACKER_UNKNOWN, WFS_CDM_DISPCUSTOP);

    //处理设备状态不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stDevice != ADP_DEVICE_ONLINE, WFS_CDM_DISPCUUNKNOWN);

    STOPDENOMINATE_BY_HARDWARE(FALSE);

    //根据钞箱启用情况判断是否可以出钞
    BOOL bEnables[ADP_MAX_CU_SIZE];
    GetEnableFromCUManager(TRUE, FALSE, bEnables);
    BOOL bIsOperatable = m_pAdapter->IsOperatable(TRUE, bEnables);
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(!bIsOperatable, WFS_CDM_DISPCUSTOP);

    //有钞箱故障或警告
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(bHasCassBoxWarning, WFS_CIM_ACCCUSTATE);

    return  WFS_CDM_DISPOK;
}

WORD XFS_CRSImp::ComputeAcceptorState(const ADPSTATUSINFOR &ADPStatus, BOOL bLog)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    //处理安全门打开的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN, WFS_CIM_ACCCUSTOP);  //30-00-00-00(FS#0004)

    //处理STACKER不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stStacker == ADP_STACKER_UNKNOWN, WFS_CIM_ACCCUSTOP);

    //处理SHUTTER不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stInShutter == ADP_INSHUTTER_UNKNOWN, WFS_CIM_ACCCUSTOP);
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stOutShutter == ADP_OUTSHUTTER_UNKNOWN, WFS_CIM_ACCCUSTOP);

    //处理传送通道不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stTransport == ADP_TRANSPORT_UNKNOWN, WFS_CIM_ACCCUSTOP);

    //处理钞门不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stInPutPosition == ADP_INPOS_UNKNOWN, WFS_CIM_ACCCUSTOP);
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stOutPutPosition == ADP_OUTPOS_UNKNOWN, WFS_CIM_ACCCUSTOP);

    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stBanknoteReader != ADP_BR_OK &&
                                        ADPStatus.stBanknoteReader != ADP_BR_NOTSUPPORTED, WFS_CIM_ACCCUSTOP);

    //处理当STACKER、通道、门口不为空或SHUTTER打开时是否停止存取款
    if (AcceptorDispenseStopByStackerTransportShutter(ADPStatus, FALSE))
    {
        if (bLog)
        {
            Log(ThisModule, -1, "AcceptorDispenseStopByStackerTransportShutter(ADPStatus)=%d",
                AcceptorDispenseStopByStackerTransportShutter(ADPStatus, FALSE));
        }
        return WFS_CIM_ACCCUSTOP;
    }
    //根据钞箱状态返回ACCEPTOR状态
    BOOL bHasCassBoxWarning = FALSE;//是否有钞箱故障或警告
    BOOL bAllInCassBoxBad = TRUE;   //是否所有进钞箱故障
    BOOL bHasRetractBox = FALSE;    //是否有回收箱
    BOOL bRetractBoxBad = TRUE;     //回收箱是否故障
    ICUInterface *pCIMCU = m_pCUManager->GetCUInterface_CIM();
    USHORT usCount = pCIMCU->GetCUCount();
    for (int i = 0; i < usCount; i++)
    {
        ICashUnit *pCU = pCIMCU->GetCUByNumber(i + 1);
        switch (pCU->GetType())
        {
        case ADP_CASSETTE_CASHIN:
        case ADP_CASSETTE_RECYCLING:
            if (CUCanAccept(pCU, m_Param.GetNoteTypeList()))
            {
                bAllInCassBoxBad = FALSE;
                if (pCU->GetStatus() == ADP_CASHUNIT_HIGH)
                {
                    bHasCassBoxWarning = TRUE;
                }
            }
            else
            {
                bHasCassBoxWarning = TRUE;
            }
            break;
        case ADP_CASSETTE_RETRACT:
            bHasRetractBox = TRUE;
            if (CUCanAccept(pCU, m_Param.GetNoteTypeList()))
            {
                bRetractBoxBad = FALSE;
                if (pCU->GetStatus() == ADP_CASHUNIT_HIGH)
                {
                    bHasCassBoxWarning = TRUE;
                }
            }
            else
            {
                bHasCassBoxWarning = TRUE;
            }
            break;
        }
    }

    //没有回收箱
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(!bHasRetractBox, WFS_CIM_ACCCUSTOP);

    //回收箱故障
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(bRetractBoxBad, WFS_CIM_ACCCUSTOP)

    //所有进钞箱故障
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(bAllInCassBoxBad &&
                                        m_Param.IsCashInBoxFullAccptStop(), WFS_CIM_ACCCUSTOP);

    //处理设备状态不正常的情况
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(ADPStatus.stDevice != ADP_DEVICE_ONLINE, WFS_CIM_ACCCUUNKNOWN);

    //根据钞箱启用情况判断是否可以进钞
    BOOL bEnables[ADP_MAX_CU_SIZE];
    GetEnableFromCUManager(FALSE, FALSE, bEnables);
    BOOL bIsOperatable = m_pAdapter->IsOperatable(FALSE, bEnables);
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(!bIsOperatable, WFS_CIM_ACCCUSTOP);

    //有钞箱故障或警告
    CHECK_DISPENSE_ACCEPTOR_STATE_ERROR(bHasCassBoxWarning || bAllInCassBoxBad, WFS_CIM_ACCCUSTATE);

    return WFS_CIM_ACCOK;
}

long XFS_CRSImp::DenominateIsDispensable(ICDMDenomination *pDenoIn, ULONG &ulCountAlloced)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    assert(m_pCUManager != NULL);
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    assert(pCDM != NULL);

    USHORT usCount;
    LPULONG pValues = pDenoIn->GetValues(usCount);
    assert(usCount > 0);
    assert(pValues != NULL);

    //校验DENO的张数数组的元素个数与出钞机的逻辑钞箱个数相同
    if (pCDM->GetCUCount() != usCount)      //钞箱个数应该与本地相同
    {
        Log(ThisModule, -1,
            "pDenoIn的usCount(%hd) != pCDM->GetCUCount()[%d]",
            usCount, pCDM->GetCUCount());
        return WFS_ERR_INVALID_DATA;
    }

    //校验DENO的各个张数不大于钞箱能出的张数（1.是出钞箱；2.未锁定；3.张数够；4.状态正常）
    ULONG ulAmountCalced = 0;   //计算得到的金额
    ulCountAlloced = 0;
    for (USHORT i = 0; i < usCount; i++)
    {
        if (pValues[i] == 0)                    //输入张数为0的不校验
            continue;

        ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);

        char cCurrencyID[3];
        pCU->GetCurrencyID(cCurrencyID);
        if (strncmp(pDenoIn->GetCurrency(), "   ", 3) != 0 &&
            strncmp(pDenoIn->GetCurrency(), cCurrencyID, 3) != 0)
        {
            Log(ThisModule, -1, "Currency(%.3s)无效或与第%d钞箱币种ID(%.3s)不符",
                pDenoIn->GetCurrency(), i + 1, cCurrencyID);
            return WFS_ERR_CDM_INVALIDCURRENCY;
        }

        if (CUCanDispense(pCU))
        {
            if (pCU->GetCount() < pValues[i])   //不够出
            {
                Log(ThisModule, -1,
                    "第%hd个逻辑钞箱剩余数(%d)小于要求出钞数(%d)",
                    i + 1, pCU->GetCount(), pValues[i]);
                return WFS_ERR_CDM_NOTDISPENSABLE;
            }
        }
        else
        {
            Log(ThisModule, -1, "第%hd个逻辑钞箱要求出钞%d张，实际不可出(Status=%hd, Lock=%d, Type=%hd)",
                i + 1, pValues[i], pCU->GetStatus(), pCU->GetAppLock(), pCU->GetType());
            if (FireDenoIsDispCUError(pCU) != 0)
                return WFS_ERR_CDM_CASHUNITERROR;

            return WFS_ERR_CDM_NOTDISPENSABLE;
        }
        ulAmountCalced += pValues[i] * pCU->GetValues();
        ulCountAlloced += pValues[i];
    }

    assert(ulAmountCalced >= 0);
    return ulAmountCalced;
}

long XFS_CRSImp::VerifyDenominateCurrency(ICDMDenomination *pDenoIn) const
{
    if (memcmp(pDenoIn->GetCurrency(), "   ", 3) == 0)
    {
        if (pDenoIn->GetAmount() != 0)
        {
            return WFS_ERR_CDM_INVALIDCURRENCY;
        }
        return WFS_SUCCESS;
    }

    if (!CurrencyCodeFitISOList(pDenoIn->GetCurrency()))
    {
        return WFS_ERR_CDM_INVALIDCURRENCY;
    }
    LPWFSCDMCURRENCYEXP *ppExp = m_Param.GetCurrencyExp();
    assert(ppExp != NULL);
    while (*ppExp != NULL)
    {
        if (memcmp(pDenoIn->GetCurrency(), (*ppExp)->cCurrencyID, 3) == 0)
        {
            return WFS_SUCCESS;
        }
        ppExp++;
    }

    return WFS_ERR_UNSUPP_DATA;
}

long XFS_CRSImp::VerifyCanOperatoByStackerTransportShutter(
const ADPSTATUSINFOR &ADPStatus, const char *ThisModule, BOOL bCDM)
{
    if (bCDM && m_Param.GetStopWhenShutterAbnormal() && ADPStatus.stOutShutter != ADP_OUTSHUTTER_CLOSED)
    {
        Log(ThisModule, -1, "CDM SHUTTER(%d)未关闭", ADPStatus.stOutShutter);
        return WFS_ERR_CDM_SHUTTEROPEN;
    }

    if (!bCDM && m_Param.GetStopWhenShutterAbnormal() &&
        ((ADPStatus.stInShutter != ADP_INSHUTTER_CLOSED) ||
         (ADPStatus.stOutShutter != ADP_OUTSHUTTER_CLOSED)))
    {
        Log(ThisModule, -1, "CIM SHUTTER(%d)未关闭", ADPStatus.stOutShutter);
        return WFS_ERR_CDM_SHUTTEROPEN;
    }

    if ((m_Param.GetStopWhenStackerNotEmpty() && (ADPStatus.stStacker == ADP_STACKER_NOTEMPTY || ADPStatus.stStacker == ADP_STACKER_FULL)) ||            //30-00-00-00(FT#0029)
        (m_Param.GetStopWhenTransportNotEmpty() && ADPStatus.stTransport == ADP_TRANSPORT_NOTEMPTY) ||
        (m_Param.GetStopWhenPositionNotEmpty() && (ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY ||
                                                   ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)))
    {
        Log(ThisModule, -1,
            "Stacker(%d)、Transport(%d)、OutPosition(%d)、或InPosition(%d)不为空",
            ADPStatus.stStacker, ADPStatus.stTransport, ADPStatus.stOutPutPosition, ADPStatus.stInPutPosition);
        if (bCDM)
        {
            return WFS_ERR_CDM_ITEMSLEFT;
        }
        else
        {
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    return 0;
}

long XFS_CRSImp::MixByAlgorithmAndLog(ULONG ulLeftAmount, USHORT usMixNumber, ULONG ulMaxAllocCount,
                                      ICDMDenomination *pDenoInOut, const char *ThisModule)
{
    USHORT usCount;
    USHORT i = 0;
    LPULONG lpValues;
    lpValues = pDenoInOut->GetValues(usCount);

    //组织MIX参数: Amount、输入张数为0的钞箱数据
    //把以下情况钞箱张数置为0：
    //  1. 不是出钞箱；2. 被锁定；3. 状态不能出钞；4. 应用已指定张数
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    usCount = pCDM->GetCUCount();
    CAutoDeleteArrayEx<MIXCUINFOR> pMixCUInfo(usCount);
    for (i = 0; i < usCount; i++)
    {
        ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
        char arycCurrencyID[3];
        pCU->GetCurrencyID(arycCurrencyID);
        pMixCUInfo[i].ulValue = pCU->GetValues();
        pMixCUInfo[i].ulCount = pCU->GetCount();
        if (lpValues != NULL)
        {
            pMixCUInfo[i].ulCount -= lpValues[i];
        }
        if (!CUCanDispense(pCU) ||
            memcmp(pDenoInOut->GetCurrency(), arycCurrencyID, 3) != 0)
        {
            pMixCUInfo[i].ulCount = 0;
        }
        if (pMixCUInfo[i].ulCount > m_Param.GetCassMiniGuarantee())
        {
            pMixCUInfo[i].ulCount -= m_Param.GetCassMiniGuarantee();
        }
        else
        {
            pMixCUInfo[i].ulCount = 0;
        }
    }

    //调用配钞算法库进行配钞
    CAutoDeleteArrayEx<ULONG> pResult(usCount);
    HRESULT hRes;
    if (usMixNumber == WFS_CDM_MIX_EQUAL_EMPTYING_OF_CASH_UNITS)
    {
        usMixNumber = m_Param.GetMixEqualEmptyMode() == 1 ? MIX_EQUAL_EMPTYING_EX : MIX_EQUAL_EMPTYING;
    }
    hRes = m_pMixManager->MixByAlgorithm(
           pResult, pMixCUInfo, usCount, ulLeftAmount, (MIXALGORITHM)usMixNumber,
           ulMaxAllocCount);
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, hRes,
            "m_pMixManager->MixByAlgorithm(usMixNumber=%hd, ulAmount=%d)失败",
            usMixNumber, ulLeftAmount);
        if (hRes == ERR_MIX_MORE_ITEM)
            return WFS_ERR_CDM_TOOMANYITEMS;
        return WFS_ERR_CDM_NOTDISPENSABLE;
    }

    //把原传入的张数写回到pResult对应位置，并设置回pDenoInOut中
    if (lpValues != NULL)
    {
        for (i = 0; i < usCount; i++)
        {
            if (lpValues[i] != 0)
            {
                //              if (pResult[i] != 0)
                //              {
                //                  Log(ThisModule, -1,
                //                      "MixByAlgorithm(usMixNumber=%hd)结果: 第%d个张数外部(%d)不为0但内部Mix结果(%d)也不为0",
                //                      usMixNumber, i + 1, lpValues[i], pResult[i]);
                //                  return WFS_ERR_INTERNAL_ERROR;
                //              }
                pResult[i] += lpValues[i];
            }
        }
    }
    pDenoInOut->SetValues(usCount, pResult);
    //计算结果金额
    ULONG iAmountCalced = 0;
    ULONG ulTotalNum = 0;  //总张数
    for (i = 0; i < usCount; i++)
    {
        iAmountCalced += pResult[i] * pMixCUInfo[i].ulValue;
        ulTotalNum += pResult[i];
    }

    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    assert(ulTotalNum <= CDMCaps.wMaxDispenseItems);
    //校验金额相符
    if (iAmountCalced != pDenoInOut->GetAmount())
    {
        Log(ThisModule, hRes,
            "最后计算的金额(%d)与外部传入的金额(%d)不相等",
            iAmountCalced, pDenoInOut->GetAmount());
        return WFS_ERR_INTERNAL_ERROR;
    }

    //记录结果
    char szMixResultBuf[1024];
    CLEAR_ARRAY(szMixResultBuf);
    char *p = szMixResultBuf;
    for (i = 0; i < usCount; i++)
    {
        sprintf(p, "[%3d]=%d ", pMixCUInfo[i].ulValue, pResult[i]);
        p += strlen(p);
    }
    Log(ThisModule, 1,
        "配钞结果: %s", szMixResultBuf);
    return 0;
}

long XFS_CRSImp::VerifyDenominate(USHORT usMixNumber, ICDMDenomination *pDenoInOut,
                                  unsigned long &iAmountCalced, ULONG &ulCountAlloced, const char *ThisModule)
{
    //校验传入的金额和张数，对于传入的张数数组长度不为0的才校验
    iAmountCalced = 0;  //计算得到的金额
    ulCountAlloced = 0; //已分配的张数
    USHORT usCount;
    LPULONG lpValues;
    lpValues = pDenoInOut->GetValues(usCount);
    if (lpValues != NULL &&
        usCount > 0)
    {
        long lAmountCalced = DenominateIsDispensable(pDenoInOut, ulCountAlloced);
        if (lAmountCalced < 0)
        {
            Log(ThisModule, -1,
                "DenominateIsDispensable()=%d", lAmountCalced);
            return lAmountCalced;
        }
        else
        {
            iAmountCalced = (unsigned long)lAmountCalced;
        }
    }

    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    if (ulCountAlloced > (ULONG)CDMCaps.wMaxDispenseItems)
    {
        Log(ThisModule, -1,
            "计算得到的已分配张数(%d)大于能够出钞的总张数(%d)",
            ulCountAlloced, (ULONG)CDMCaps.wMaxDispenseItems);
        return WFS_ERR_CDM_TOOMANYITEMS;
    }

    //处理usMixNumber为WFS_CDM_INDIVIDUAL的情况
    //如果张数数组长度为0，非法
    //如果传入的金额不为0，必须与计算得到的金额一致
    if (usMixNumber == WFS_CDM_INDIVIDUAL)          //校验DENO是否可以出钞
    {
        if (usCount == 0)
        {
            Log(ThisModule, -1,
                "usMixNumber == WFS_CDM_INDIVIDUAL, 但usCount = 0");
            return WFS_ERR_INVALID_DATA;
        }

        if (pDenoInOut->GetAmount() != 0)
        {
            if (iAmountCalced != pDenoInOut->GetAmount())
            {
                Log(ThisModule, -1,
                    "iAmountCalced(%d) != pDenoInOut->GetAmount()[%d]",
                    iAmountCalced, pDenoInOut->GetAmount());
                return WFS_ERR_CDM_INVALIDDENOMINATION;
            }
        }
        pDenoInOut->SetAmount(iAmountCalced);
        return WFS_SUCCESS;
    }

    //校验MixNumber，只能是标准的三种算法
    if (usMixNumber != WFS_CDM_MIX_MINIMUM_NUMBER_OF_BILLS &&
        usMixNumber != WFS_CDM_MIX_EQUAL_EMPTYING_OF_CASH_UNITS &&
        usMixNumber != WFS_CDM_MIX_MAXIMUM_NUMBER_OF_CASH_UNITS)
    {
        Log(ThisModule, -1,
            "usMixNumber(%d)错误",
            usMixNumber);
        return WFS_ERR_CDM_INVALIDMIXNUMBER;
    }

    //校验币种必须有效
    if (memcmp(pDenoInOut->GetCurrency(), "   ", 3) == 0)
    {
        Log(ThisModule, -1,
            "currency(%.3s) error(usMixNumber=%hd)",
            pDenoInOut->GetCurrency(), usMixNumber);
        return WFS_ERR_CDM_INVALIDCURRENCY;
    }

    //校验金额：金额不可为0，不可小于计算金额
    if (pDenoInOut->GetAmount() == 0) //金额不能为0
    {
        Log(ThisModule, -1,
            "pDenoInOut->GetAmount() == 0(usMixNumber=%hd)",
            usMixNumber);
        return WFS_ERR_INVALID_DATA;
    }

    if (pDenoInOut->GetAmount() < iAmountCalced) //金额不能小于已经分配的金额
    {
        Log(ThisModule, -1,
            "pDenoInOut->GetAmount()[%d] < iAmountCalced[%d](usMixNumber=%hd)",
            pDenoInOut->GetAmount(), iAmountCalced, usMixNumber);
        return WFS_ERR_CDM_NOTDISPENSABLE;
    }

    //处理金额已经分配的情况：如果金额全部分配，直接返回成功
    if (pDenoInOut->GetAmount() == iAmountCalced)
    {
        pDenoInOut->SetAmount(iAmountCalced);
        return WFS_SUCCESS;
    }

    return 1; //返回大于0的值，表示检验成功，但没有分配完
}

BOOL XFS_CRSImp::AcceptorDispenseStopByStackerTransportShutter(const ADPSTATUSINFOR &ADPStatus, BOOL bCDM) const
{
    if (!m_bCashInActive && !m_bCashOutActive)
    {
        if ((m_Param.GetStopWhenStackerNotEmpty() == 1 && (ADPStatus.stStacker == ADP_STACKER_NOTEMPTY || ADPStatus.stStacker == ADP_STACKER_FULL)) ||      //30-00-00-00(FT#0029)
            (m_Param.GetStopWhenTransportNotEmpty() == 1 && ADPStatus.stTransport == ADP_TRANSPORT_NOTEMPTY) ||
            (m_Param.GetStopWhenPositionNotEmpty() == 1 && (ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY ||
                                                            ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)) ||
            (m_Param.GetStopWhenSafeDoorOpen() == 1 && ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN))
        {
            return TRUE;
        }

        if (bCDM && ((m_Param.GetStopWhenShutterAbnormal() == 1) && (ADPStatus.stOutShutter != ADP_OUTSHUTTER_CLOSED)))
        {
            return TRUE;
        }

        if (!bCDM && ((m_Param.GetStopWhenShutterAbnormal() == 1)
                      && ((ADPStatus.stInShutter != ADP_INSHUTTER_CLOSED)
                          || (ADPStatus.stOutShutter != ADP_OUTSHUTTER_CLOSED))))
        {
            return TRUE;
        }
    }
    return FALSE;
}

//在结束交换时，当ulCount=0时清除CDM的ulCount类数据
void XFS_CRSImp::ClearCountWhenEndExchange(LPWFSCDMCUINFO pCDMCUInfo)
{
    for (USHORT i = 0; i < pCDMCUInfo->usCount; i++)
    {
        LPWFSCDMCASHUNIT pCU = pCDMCUInfo->lppList[i];
        if (pCU == NULL)
            continue;

        if (pCU->ulCount == 0)
        {
            pCU->ulInitialCount = pCU->ulRejectCount = 0;
            for (USHORT j = 0; j < pCU->usNumPhysicalCUs; j++)
            {
                LPWFSCDMPHCU lpPhysical = pCU->lppPhysical[j];
                lpPhysical->ulInitialCount = lpPhysical->ulCount = lpPhysical->ulRejectCount = 0;
            }
        }
    }
}

//在结束交换时，当ulCount=0时清除CDM的ulCount类数据
void XFS_CRSImp::ClearCountWhenEndExchange(LPWFSCIMCASHINFO pCIMCUInfo)
{
    for (USHORT i = 0; i < pCIMCUInfo->usCount; i++)
    {
        LPWFSCIMCASHIN pCU = pCIMCUInfo->lppCashIn[i];
        if (pCU == NULL)
            continue;

        if (pCU->ulCount == 0)
        {
            pCU->ulCashInCount = 0;
            for (USHORT j = 0; j < pCU->usNumPhysicalCUs; j++)
            {
                LPWFSCIMPHCU lpPhysical = pCU->lppPhysical[j];
                lpPhysical->ulCashInCount = lpPhysical->ulCount = 0;
            }
            LPWFSCIMNOTENUMBERLIST lpNoteNumberList = pCU->lpNoteNumberList;
            if (lpNoteNumberList != NULL)
            {
                for (USHORT j = 0; j < lpNoteNumberList->usNumOfNoteNumbers; j++)
                {
                    LPWFSCIMNOTENUMBER pNoteNumber = lpNoteNumberList->lppNoteNumber[j];
                    if (pNoteNumber == NULL)
                        continue;
                    pNoteNumber->ulCount = 0;
                }
            }
        }
    }
}

void XFS_CRSImp::FireCUErrorAndSetErrorNum(ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE], BOOL bCDM)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    ICUInterface *pCUInterface =
    bCDM ? m_pCUManager->GetCUInterface_CDM() : m_pCUManager->GetCUInterface_CIM();
    USHORT usCount = pCUInterface->GetCUCount();
    for (USHORT i = 0; i < usCount; i++)
    {
        ICashUnit *pCU = pCUInterface->GetCUByNumber(i + 1);
        USHORT usIndex = pCU->GetIndex();
        if (usIndex < 1 || usIndex > ADP_MAX_CU_SIZE)
            continue;

        usIndex--;

        if (wCUErrors[usIndex] == ADP_CUERROR_NOTUSED)
            continue;

        if (wCUErrors[usIndex] != ADP_CUERROR_OK)
        {
            if (wCUErrors[usIndex] == ADP_CUERROR_FATAL)
            {
                pCU->SetErrorCount(-1);
            }
            else if (wCUErrors[usIndex] == ADP_CUERROR_RETRYABLE)
            {
                pCU->SetErrorCount(pCU->GetErrorCount() + 1);
            }

            if (bCDM)
            {
                CDMFireCUError(ADPCUError2XFS_CDM(wCUErrors[usIndex]), pCU->BuildByXFSCDMFormat());
            }
            else
            {
                CIMFireCUError(ADPCUError2XFS_CIM(wCUErrors[usIndex]), pCU->BuildByXFSCIMFormat());
            }
        }
        else
        {
            pCU->SetErrorCount(0);
        }
    }
}

long XFS_CRSImp::FireDenoIsDispCUError(ICashUnit *pCU)
{
    if (CUIsOutBox(pCU->GetType()))
    {
        if (pCU->GetStatus() == ADP_CASHUNIT_EMPTY)
        {
            CDMFireCUError(WFS_CDM_CASHUNITEMPTY, pCU->BuildByXFSCDMFormat());
            return WFS_ERR_CDM_CASHUNITERROR;
        }
        if (pCU->GetAppLock())
        {
            CDMFireCUError(WFS_CDM_CASHUNITLOCKED, pCU->BuildByXFSCDMFormat());
            return WFS_ERR_CDM_CASHUNITERROR;
        }
    }
    return 0;
}


HRESULT XFS_CRSImp::OpenAllShutter(const char *ThisModule, BOOL bCDM)
{
    VERIFY_NOT_IN_EXCHANGE(bCDM);
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);

    if ((ADPStatus.stInShutter == ADP_INSHUTTER_OPEN)
        && (ADPStatus.stOutShutter == ADP_OUTSHUTTER_OPEN))
    {
        return WFS_SUCCESS;
    }
    //执行物理开门
    long lRet = 0;
    if (ADPStatus.stInShutter == ADP_INSHUTTER_CLOSED
        && m_adapterinfo.bHaveInShutter)
    {
        lRet = OpenShutter(ThisModule, TRUE, bCDM);
        if (lRet < 0)
        {
            return lRet;
        }
    }

    if (ADPStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED
        && m_adapterinfo.bHaveOutShutter)
    {
        lRet = OpenShutter(ThisModule, FALSE, bCDM);
        if (lRet < 0)
        {
            return lRet;
        }
    }

    m_dwCloseShutterCount = 0;
    return 0;
}

HRESULT XFS_CRSImp::CloseAllShutter(const char *ThisModule, BOOL bCDM)
{
    VERIFY_NOT_IN_EXCHANGE(bCDM);
    ADPSTATUSINFOR ADPStatus;
    GetUpdatedStatus(ADPStatus);

    if ((ADPStatus.stInShutter == ADP_INSHUTTER_CLOSED)
        && (ADPStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED))
    {
        return WFS_SUCCESS;
    }
    //执行物理开门
    long lRet = 0;
    if (ADPStatus.stInShutter == ADP_INSHUTTER_OPEN
        && m_adapterinfo.bHaveInShutter)
    {
        lRet = CloseShutter(ThisModule, TRUE, bCDM);
        if (lRet < 0)
        {
            return lRet;
        }
    }

    if (ADPStatus.stOutShutter == ADP_OUTSHUTTER_OPEN
        && m_adapterinfo.bHaveOutShutter)
    {
        lRet = CloseShutter(ThisModule, FALSE, bCDM);
        if (lRet < 0)
        {
            return lRet;
        }
    }

    m_dwCloseShutterCount = 0;
    return 0;
}


HRESULT XFS_CRSImp::OpenShutter(const char *ThisModule, BOOL bInShutter, BOOL bCDM)
{
    ////AutoLogFuncBeginEnd();();
    VERIFY_NOT_IN_EXCHANGE(bCDM);

    ADPDEVINFOR devinfo;
    //m_pAdapter->GetDevConfig(devinfo);

    //  if ((!devinfo.bHaveInShutter && bInShutter)
    //      ||(!devinfo.bHaveOutShutter && !bInShutter))
    //  {

    //      return 0;
    //  }

    //执行物理开门
    long lRet = m_pAdapter->OpenShutter(bInShutter);
    if (lRet < 0)
    {
        Log(ThisModule, lRet, "m_pAdapter->OpenShutter(%s)失败", bInShutter ? "In" : "OUT");
        return WFS_ERR_HARDWARE_ERROR;
    }
    m_dwCloseShutterCount = 0;
    Log(ThisModule, 1, "m_pAdapter->OpenShutter(%s)成功", bInShutter ? "In" : "OUT");
    return 0;
}

HRESULT XFS_CRSImp::CloseShutter(const char *ThisModule, BOOL bInShutter, BOOL bCDM)
{
    VERIFY_NOT_IN_EXCHANGE(bCDM);

    ADPDEVINFOR devinfo;
    m_pAdapter->GetDevConfig(devinfo);

    if ((!devinfo.bHaveInShutter && bInShutter)
        || (!devinfo.bHaveOutShutter && !bInShutter))
    {
        return 0;
    }

    //执行物理开门
    //CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce();
    //CAutoSaveDataAndFireEvent _auto_save_data_and_fire_event(this);
    int iRet = m_pAdapter->CloseShutter(bInShutter);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "m_pAdapter->CloseShutter(%s)失败", bInShutter ? "In" : "OUT");
        return WFS_ERR_HARDWARE_ERROR;
    }
    m_dwCloseShutterCount = 0;
    Log(ThisModule, 1, "m_pAdapter->CloseShutter(%s)成功", bInShutter ? "In" : "OUT");
    return 0;
}

long XFS_CRSImp::FindCassetteADPIndex(FIND_CASS_ADP_INDEX_FLAG eType, BOOL bCDM, USHORT &usIndex) const
{
    USHORT usFoundCount = 0;
    USHORT i;
    if (bCDM)
    {
        ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
        USHORT usCount = pCDM->GetCUCount();
        for (i = 0; i < usCount; i++)
        {
            ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
            if (eType == FCAIF_RETRACT &&
                pCU->GetType() == ADP_CASSETTE_RETRACT)
            {
                usFoundCount++;
            }
            else if (eType == FCAIF_REJECT &&
                     pCU->GetType() == ADP_CASSETTE_REJECT)
            {
                usFoundCount++;
            }
            if (usFoundCount != 0 && usFoundCount == usIndex)
            {
                usIndex = pCU->GetIndex();
                return 0;
            }
        }
    }
    else
    {
        ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
        USHORT usCount = pCIM->GetCUCount();
        for (i = 0; i < usCount; i++)
        {
            ICashUnit *pCU = pCIM->GetCUByNumber(i + 1);
            if (eType == FCAIF_RETRACT &&
                pCU->GetType() == ADP_CASSETTE_RETRACT)
            {
                usFoundCount++;
            }
            else if (eType == FCAIF_RECYCLE_CASHIN &&
                     (pCU->GetType() == ADP_CASSETTE_RECYCLING || pCU->GetType() == ADP_CASSETTE_CASHIN))
            {
                usFoundCount++;
            }
            if (usFoundCount != 0 && usFoundCount == usIndex)
            {
                usIndex = pCU->GetIndex();
                return 0;
            }
        }
    }

    if (usFoundCount == 0)
        return WFS_ERR_CIM_NOTRETRACTAREA;
    else
        return WFS_ERR_CIM_INVALIDRETRACTPOSITION;
}

long XFS_CRSImp::GetRetractAreaAndIndex(USHORT usRetractArea,
                                        ADP_RETRACTEARE &eRetractArea, USHORT &usIndex, BOOL bCDM, BOOL bReset)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    long iRet;
    switch (usRetractArea)
    {
    case WFS_CDM_RA_RETRACT: //WFS_CIM_RA_RETRACT
        if (usIndex == 0)
        {
            if (m_Param.IsSupportIllegalRetractIndex())
            {
                eRetractArea = ADP_RETRACTEARE_NOSET;
                usIndex = 1;
                return 0;
            }
            Log(ThisModule, -1, "usRetractArea=WFS_CDM_RA_RETRACT, usIndex=0");
            iRet = bReset ? WFS_ERR_INVALID_DATA : WFS_ERR_CDM_INVALIDRETRACTPOSITION;
            return iRet;
        }
        eRetractArea = ADP_RETRACTEARE_RETRACTBOX;
        break;

    case WFS_CDM_RA_TRANSPORT: //WFS_CIM_RA_TRANSPORT
        eRetractArea = ADP_RETRACTEARE_TRANSPORT;
        usIndex = 0;
        break;

    case WFS_CDM_RA_STACKER: //WFS_CIM_RA_STACKER
        eRetractArea = ADP_RETRACTEARE_STACKER;
        usIndex = 0;
        break;

    case WFS_CDM_RA_REJECT: //WFS_CIM_RA_BILLCASSETTES
        eRetractArea =  m_Param.IsGenerRejectPosbyRetractCass() ? ADP_RETRACTEARE_RETRACTBOX : ADP_RETRACTEARE_REJECTBOX;
        usIndex = 1;
        break;

    default:
        Log(ThisModule, -1, "无效的回收位置:%hd", usRetractArea);
        iRet = bReset ? WFS_ERR_INVALID_DATA : WFS_ERR_CDM_NOTRETRACTAREA;
        return iRet;
    }

    //查找钞箱的索引
    if (usRetractArea == WFS_CDM_RA_RETRACT ||
        usRetractArea == WFS_CDM_RA_REJECT)
    {
        FIND_CASS_ADP_INDEX_FLAG eType;
        if (usRetractArea == WFS_CDM_RA_RETRACT)
        {
            eType = FCAIF_RETRACT;
        }
        else
        {
            if (bCDM)
            {
                eType = m_Param.IsGenerRejectPosbyRetractCass() ? FCAIF_RETRACT : FCAIF_REJECT;
            }
            else
            {
                eType = FCAIF_RECYCLE_CASHIN;
            }
        }
        iRet = FindCassetteADPIndex(eType, bCDM, usIndex);
        if (iRet < 0)
        {
            if (m_Param.IsSupportIllegalRetractIndex())
            {
                eRetractArea = ADP_RETRACTEARE_NOSET;
                usIndex = 1;
                return 0;
            }
            Log(ThisModule, -1,
                "FindCassetteADPIndex(eType=%d, bCDM=%d, usIndex=%hd)失败(usRetractArea=%hd)",
                eType, bCDM, usIndex, usRetractArea);
            iRet = bReset ? WFS_ERR_CDM_UNSUPPOSITION : iRet;
            return iRet;
        }
    }

    return eRetractArea;
}

void XFS_CRSImp::GetResetDefaultAreaAndIndex(ADP_RETRACTEARE eRetractArea, USHORT &usRetractArea,
                                             USHORT &usIndex, BOOL bCDM) const
{
    switch (eRetractArea)
    {
    case ADP_RETRACTEARE_RETRACTBOX:
        usRetractArea = bCDM ? WFS_CDM_RA_RETRACT : WFS_CIM_RA_RETRACT;
        break;
    case ADP_RETRACTEARE_STACKER:
        usRetractArea = bCDM ? WFS_CDM_RA_STACKER : WFS_CIM_RA_STACKER;
        break;
        /*case ADP_RETRACTEARE_REJECTBOX:  XFS310
            usRetractArea = bCDM ? WFS_CDM_RA_REJECT : WFS_CIM_RA_REJECT;
            usIndex = 1;
            break;*/
    }
}

void XFS_CRSImp::GetXFSRetractAreaFromADP(ADP_RETRACTEARE eRetractArea, USHORT &usRetractArea, BOOL bCDM) const
{
    switch (eRetractArea)
    {
    case ADP_RETRACTEARE_RETRACTBOX:
        usRetractArea = bCDM ? WFS_CDM_RA_RETRACT : WFS_CIM_RA_RETRACT;
        break;
    case ADP_RETRACTEARE_TRANSPORT:
        usRetractArea = bCDM ? WFS_CDM_RA_TRANSPORT : WFS_CIM_RA_TRANSPORT;
        break;
    case ADP_RETRACTEARE_STACKER:
        usRetractArea = bCDM ? WFS_CDM_RA_STACKER : WFS_CIM_RA_STACKER;
        break;
    case ADP_RETRACTEARE_REJECTBOX:
        usRetractArea = WFS_CDM_RA_REJECT;
        break;
    case ADP_RETRACTEARE_RECYCLEBOX:
        usRetractArea = WFS_CIM_RA_BILLCASSETTES;
        break;
    }
}

HRESULT XFS_CRSImp::Retract(const char *ThisModule, WORD fwOutputPosition,
                            USHORT usRetractArea, USHORT usIndex, BOOL bCDM)
{
    VERIFY_NOT_IN_EXCHANGE(bCDM);
    ADPCDMCAPS CDMCaps;
    m_pAdapter->GetCDMCapabilities(&CDMCaps);
    ADPCIMCAPS CIMCaps;
    m_pAdapter->GetCIMCapabilities(&CIMCaps);
    CHECK_RETRACT_AREA(usRetractArea, bCDM ? CDMCaps.fwRetractAreas : CIMCaps.fwRetractAreas, FALSE);

    VERIFY_DISABLE_UNUSED_CASSETTE(bCDM);   //禁用不相关的钞箱

    m_eWaitTaken = WTF_NONE;    //当执行新命令时，清除等待取钞关门标志
    m_bCashOutActive = FALSE;

    long iRet;

    //清掉上次记录
    //bCDM ? m_PresentInfor.m_Extra.Clear() : m_CashInInfor.m_Extra.Clear();

    if (m_Param.IsCheckCSEmptyBeforeRetract())
    {
        //确保钞门有钞
        ADPSTATUSINFOR ADPStatus;
        UpdateStatus(ThisModule, ADPStatus, TRUE);
        if (/*(fwOutputPosition != WFS_CIM_POSNULL || bCDM)&&*/
        bCDM  && (ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY))

        {
            Log(ThisModule, -1, "门口没有钞票");
            return WFS_ERR_CDM_NOITEMS;
        }
        else if (!bCDM && ((ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY) || (ADPStatus.stInPutPosition == ADP_INPOS_EMPTY)))
        {
            Log(ThisModule, -1, "进出钞门口没有钞票");
            return WFS_ERR_CIM_NOITEMS;
        }
    }
    //转换回收区域，计算索引
    ADP_RETRACTEARE eRetractArea;
    iRet = GetRetractAreaAndIndex(usRetractArea, eRetractArea, usIndex, bCDM);
    if (iRet < 0)
    {
        Log(ThisModule, iRet,   "GetRetractAreaAndIndex()失败");
        return iRet;
    }

    //执行回收动作
    ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(wCUErrors);
//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(); //忽略一次ITEM插入取出事件
    iRet = m_pAdapter->Retract(eRetractArea, usIndex, wCUErrors);
    FireCUErrorAndSetErrorNum(wCUErrors, bCDM);

    //update cassette info
    UpdateRetractCountToCUManager(bCDM, usIndex);

    if (iRet < 0)
    {
        Log(ThisModule, iRet, "m_pAdapter->Retract(usRetractArea=%hd, usIndex=%hd)失败",
            usRetractArea, usIndex);
        return iRet;
    }

    Log(ThisModule, 1, "%sRetract成功", bCDM ? "CDM" : "CIM");
    return 0;
}

HRESULT XFS_CRSImp::StartExchange(const char *ThisModule,
                                  USHORT usCount, LPUSHORT lpusCUNumList,
                                  ICUInterface *pCUInterface)
{
    VERIFY_NOT_IN_EXCHANGE(pCUInterface->IsCDM());
    USHORT i = 0;
    USHORT aryTemp[128];
    if (usCount == 0 || lpusCUNumList == nullptr)           //30-00-00-00(FT#0027)
    {
        usCount = pCUInterface->GetCUCount();
        lpusCUNumList = aryTemp;
        for (USHORT i = 0; i < usCount; i++)
        {
            lpusCUNumList[i] = i + 1;
        }
    }

    //组织交换标志
    BOOL bExFlag[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(bExFlag);
    for (i = 0; i < usCount; i++)
    {
        ICashUnit *pCU;
        pCU = pCUInterface->GetCUByNumber(lpusCUNumList[i]);
        if (pCU == NULL)
        {
            Log(ThisModule, -1,
                "pCUInterface->GetCUByNumber(lpusCUNumList[%hd]) = NULL(Number=%hd)",
                i, lpusCUNumList[i]);
            return WFS_ERR_INVALID_DATA;
        }
        USHORT usIndex = pCU->GetIndex();
        if (usIndex < 1 ||
            usIndex > ADP_MAX_CU_SIZE)
        {
            Log(ThisModule, -1,
                "第%hd个钞箱(usNumber=%hd)的物理钞箱索引(%hd)不正确",
                i + 1, lpusCUNumList[i], usIndex);
            return WFS_ERR_INTERNAL_ERROR;
        }
        usIndex--;
        bExFlag[usIndex] = TRUE;
    }

    m_eWaitTaken = WTF_NONE;    //当执行新命令时，清除等待取钞关门标志
    m_bCashOutActive = FALSE;

    //执行物理动作
    long iRet;
    iRet = m_pAdapter->StartExchange(bExFlag);
    if (iRet < 0)
    {
        Log(ThisModule, iRet,
            "m_pAdapter->StartExchange()失败");
        return iRet;
    }

    //设置钞箱交换状态
    for (i = 0; i < usCount; i++)
    {
        ICashUnit *pCU;
        pCU = pCUInterface->GetCUByNumber(lpusCUNumList[i]);
        pCU->SetExchangeState(TRUE);
    }

    if (pCUInterface->IsCDM())
        m_bCDMExchangeActive = TRUE;
    else
        m_bCIMExchangeActive = TRUE;

    Log(ThisModule, 1,
        "%s:StartExchange()成功", pCUInterface->IsCDM() ? "CDM" : "CIM");
    return 0;
}

HRESULT XFS_CRSImp::EndExchange(const char *ThisModule, LPVOID lpCUInfo, ICUInterface *pCUInterface)
{
    VERIFY_IN_EXCHANGE();

    m_bCDMExchangeActive = FALSE;
    m_bCIMExchangeActive = FALSE;

    //设置数据
    long iRet = 0;
    if (lpCUInfo != NULL)
    {
        if (pCUInterface->IsCDM())
            iRet = pCUInterface->SetByXFSCDMFormat((LPWFSCDMCUINFO)lpCUInfo);
        else
            iRet = pCUInterface->SetByXFSCIMFormat((LPWFSCIMCASHINFO)lpCUInfo);
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "pCUInterface->SetByXFSFormat失败");
        }
    }

    //在CDM和CIM间同步循环箱和回收箱数据
    if (iRet == 0)
    {
        ICUInterface *pDestCUInterface =
        pCUInterface->IsCDM() ? m_pCUManager->GetCUInterface_CIM() : m_pCUManager->GetCUInterface_CDM();
        pDestCUInterface->SyncDataByPhysicalIndex(pCUInterface, TRUE);
    }

    //清除交换状态
    USHORT usCount = pCUInterface->GetCUCount();
    for (USHORT i = 0; i < usCount; i++)
    {
        ICashUnit *pCU;
        pCU = pCUInterface->GetCUByNumber(i + 1);
        if (pCU == NULL)
        {
            Log(ThisModule, -1, "pCUInterface->GetCUByNumber(%hd)失败", i + 1);
            return WFS_ERR_INTERNAL_ERROR;
        }
        pCU->SetExchangeState(FALSE);
    }

    {
        //执行物理动作
        long iRet = m_pAdapter->EndExchange();
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "m_pAdapter->EndExchange()失败");
            return iRet;
        }
    }

    if (iRet < 0)
    {
        return iRet;
    }

    //从钞箱管理模块更新到本地数据和适配层
    iRet = UpdateADPCassInfoFromCUManager();
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "UpdateADPCassInfoFromCUManager()失败");
        return iRet;
    }

    //处理交换时复位
    if (m_pAdapter->IsNeedReset())
    {
        iRet = Reset(ThisModule, NULL, pCUInterface, TRUE);
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "Reset()失败");
            return iRet;
        }
    }

    Log(ThisModule, 1, "%s:EndExchange()成功", pCUInterface->IsCDM() ? "CDM" : "CIM");
    return 0;
}

HRESULT XFS_CRSImp::Reset(const char *ThisModule,
                          LPWFSCDMITEMPOSITION lpResetIn, ICUInterface *pCUInterface, BOOL bInnerCall)
{
    VERIFY_NOT_IN_EXCHANGE(pCUInterface->IsCDM());
    //VERIFY_NOT_IN_CASH_IN();根据XFS3.10规范说明，在Cashin状态为Active时，调用reset须执行复位

    m_bCashInActive = FALSE;

    m_bCashOutActive = FALSE;

    m_CashInInfor.SetState(WFS_CIM_CIUNKNOWN);
    m_PresentInfor.SetPresentState(WFS_CDM_UNKNOWN);

    BOOL bOutput = FALSE;

    long iRet;

	if (!m_bAdapterInited)
	{
		iRet = InitAdapterSetting();
		if (iRet < 0)
		{
			return iRet;
		}
        m_bAdapterInited = TRUE;
	}

    ADP_RETRACTEARE eRetractArea;
    USHORT usIndex;
    BOOL bCDM = pCUInterface->IsCDM();
    if (lpResetIn == NULL) //RESET参数为NULL，由适配层决定决定，有两种情况： 1）外部调用为NULL，由内部决定；2）内部调用
    {
        eRetractArea = ADP_RETRACTEARE_NOSET;
        usIndex = 1;
    }
    else
    {
        ADPCDMCAPS CDMCaps;
        m_pAdapter->GetCDMCapabilities(&CDMCaps);
        ADPCIMCAPS CIMCaps;
        m_pAdapter->GetCIMCapabilities(&CIMCaps);
        if (lpResetIn->usNumber == 0) //到门口
        {
            eRetractArea = ADP_RETRACTEARE_CASHSLOT;
            usIndex = 1;
            CHECK_CDM_OR_CIM_OUTPUT_POS(bCDM, lpResetIn->fwOutputPosition, CDMCaps.fwPositions, CIMCaps.fwPositions);
        }
        else
        {
            assert(lpResetIn->usNumber != 0);
            //校验回收的钞箱NUMBER
            USHORT usNumber = lpResetIn->usNumber;
            ICashUnit *pCU = pCUInterface->GetCUByNumber(usNumber);
            if (pCU == NULL)
            {
                Log(ThisModule, -1, "pCUInterface->GetCUByNumber(%hd) = NULL", usNumber);
                return WFS_ERR_CDM_INVALIDCASHUNIT;
            }

            //形成回收钞箱索引
            usIndex = pCU->GetIndex();
            if (usIndex < 1 ||
                usIndex > ADP_MAX_CU_SIZE)
            {
                Log(ThisModule, -1, "钞箱(usNumber=%hd)的物理钞箱索引(%hd)不正确", usNumber, usIndex);
                return WFS_ERR_INTERNAL_ERROR;
            }

            //形成回收位置
            if ((pCU->GetType() == ADP_CASSETTE_RETRACT)
                || ((pCU->GetType() == ADP_CASSETTE_REJECT) && m_Param.IsGenerRejectPosbyRetractCass()))
            {
                CHECK_PTR_VALID(lpResetIn->lpRetractArea);
                eRetractArea = ADP_RETRACTEARE_RETRACTBOX;
                CHECK_CDM_OR_CIM_OUTPUT_POS(bCDM, lpResetIn->lpRetractArea->fwOutputPosition, \
                                            CDMCaps.fwPositions, CIMCaps.fwPositions);
                CHECK_RETRACT_AREA(lpResetIn->lpRetractArea->usRetractArea, \
                                   bCDM ? CDMCaps.fwRetractAreas : CIMCaps.fwRetractAreas, TRUE);

                usIndex = lpResetIn->lpRetractArea->usIndex;
                iRet = GetRetractAreaAndIndex(lpResetIn->lpRetractArea->usRetractArea, eRetractArea,
                                              usIndex, pCUInterface->IsCDM(), TRUE);
                if (iRet < 0)
                {
                    Log(ThisModule, iRet, "GetRetractAreaAndIndex()失败");
                    return iRet;
                }
            }
            else
            {
                Log(ThisModule, -1, "钞箱(usNumber=%hd)不是回收箱(Type=%d)",    usNumber, pCU->GetType());
                return WFS_ERR_CDM_INVALIDCASHUNIT;
            }
        }
    }

    VERIFY_DISABLE_UNUSED_CASSETTE(pCUInterface->IsCDM());

    BOOL bMediaDetected = FALSE;
    m_eWaitTaken = WTF_NONE;    //当执行新命令时，清除等待取钞关门标志
    ADP_CUERROR wCUErrors[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(wCUErrors);

    bCDM ? m_PresentInfor.m_Extra.Clear() : m_CashInInfor.m_Extra.Clear();
//    CAutoFireDeviceStatus::IgnoreItemTakenInsertedOnce(); //忽略一次ITEM插入取出事件
    USHORT usTemp = usIndex;
    iRet = m_pAdapter->Reset(eRetractArea, usIndex, bMediaDetected, wCUErrors);
    //m_dwLastRecoverTime = GetTickCount();
    FireCUErrorAndSetErrorNum(wCUErrors, pCUInterface->IsCDM());
    UpdateRetractCountToCUManager(pCUInterface->IsCDM(), usIndex);

    //从配置文件读取是否RESET有回收时设置CASH_IN_STATUS状态为WFS_CIM_CIRETRACT
    //更新CASH_IN_STATUS
    if (m_Param.IsSetCIRetractAfterReset() &&
        m_CashInInfor.GetAvailable() &&
        m_CashInInfor.GetState() == WFS_CIM_CIACTIVE)
    {
        m_CashInInfor.SetState(WFS_CIM_CIRETRACT);
    }

    if (iRet < 0)
    {
        m_dwAutoRecoverNumber++;
        Log(ThisModule, iRet,
            "m_pAdapter->Reset(bOutput=%d, eRetractArea=%hd, usIndex=%hd)失败",
            bOutput, eRetractArea, usIndex);
        return iRet;
    }

    m_dwAutoRecoverNumber = 0;
    m_dwCloseShutterCount = 0;

    if (bMediaDetected && !bInnerCall)
    {
        ADPCDMCAPS CDMCaps;
        m_pAdapter->GetCDMCapabilities(&CDMCaps);
        ADPCIMCAPS CIMCaps;
        m_pAdapter->GetCIMCapabilities(&CIMCaps);

        WFSCDMRETRACT RetractInfo;
        RetractInfo.fwOutputPosition =
        pCUInterface->IsCDM() ? CDMCaps.fwPositions : CIM_OUTPUT_POSITION(CIMCaps.fwPositions);
        if (lpResetIn == NULL)
        {
            RetractInfo.usIndex = usIndex;
            GetResetDefaultAreaAndIndex(eRetractArea, RetractInfo.usRetractArea, RetractInfo.usIndex, pCUInterface->IsCDM());
        }
        else if (lpResetIn->usNumber != 0)
        {
            GetXFSRetractAreaFromADP(eRetractArea, RetractInfo.usRetractArea, pCUInterface->IsCDM());
            RetractInfo.usIndex = usTemp;
        }
        else
        {
            RetractInfo.usRetractArea = WFS_CDM_RA_NOTSUPP;
        }

        WFSCDMITEMPOSITION pos;
        pos.fwOutputPosition =
        pCUInterface->IsCDM() ? CDMCaps.fwPositions : CIM_OUTPUT_POSITION(CIMCaps.fwPositions);

        pos.usNumber = lpResetIn != NULL ? lpResetIn->usNumber : 1;
        if (lpResetIn != NULL && lpResetIn->usNumber == 0)
            pos.lpRetractArea = NULL;
        else
            pos.lpRetractArea = &RetractInfo;
        if (RetractInfo.usRetractArea == WFS_CDM_RA_REJECT)
        {
            if (pCUInterface->IsCDM())
                CDMFireMediaDetected(&pos);
            else
                CIMFireMediaDetected((LPWFSCIMITEMPOSITION)&pos);
        }
        else
        {
            CDMFireMediaDetected(&pos);
            CIMFireMediaDetected((LPWFSCIMITEMPOSITION)&pos);
        }
    }

    if(m_Param.GetResetRelieveManipSupp()){         //30-00-00-00(FT#0042)
        RelieveBoxManipStatus();                    //30-00-00-00(FT#0042)
    }                                               //30-00-00-00(FT#0042)
    Log(ThisModule, 1, "%sReset()成功", bCDM ? "CDM" : "CIM");
    return 0;
}

long XFS_CRSImp::UpdateStatus(const char *ThisModule, ADPSTATUSINFOR &ADPStatus, BOOL bUpdateFromHardware)
{

    if (bUpdateFromHardware)
    {
        //AutoMutexStl(m_cStatusMutex);
        long iRet = m_pAdapter->UpdateStatus();
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "m_pAdapter->UpdateStatus()失败");
            return iRet;
        }
    }

    //save data to ADPStatus
    GetUpdatedStatus(ADPStatus);

    if(bUpdateFromHardware){                                    //30-00-00-00(FT#0018)
        CAutoFireDeviceStatus::SetADPStatusInfor(ADPStatus);    //30-00-00-00(FT#0018)
    }                                                           //30-00-00-00(FT#0018)

    return 0;
}

void XFS_CRSImp::UpdateRetractCountToCUManager(BOOL bCDM, USHORT usPCUIndex)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    ULONG aryulRetractCount[ADP_MAX_CU_SIZE];
    CLEAR_ARRAY(aryulRetractCount);
    m_pAdapter->GetRetractCount(aryulRetractCount);
    ULONG ulCount = 0;
    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        ulCount += aryulRetractCount[i];
    }
    if (ulCount <= 0)
    {
        Log(ThisModule, 0, "获取到的回收钞票张数为0");
        return;
    }

    if (m_pAdapter->GetNoteSeiralNumbersCount() > 0 &&
        m_Param.GetInfoAvailableAfterRetractCount() &&
        ulCount != 0)
    {
        Log(ThisModule, 1, "发送事件WFS_EXEE_CIM_INFO_AVAILABLE");
        //FireInfoAvailable(m_pAdapter->GetNoteSeiralNumbersCount());
    }

    //处理按张计数还是按次数计数
    if (m_Param.GetRetractCountMode() == 1) //按次数计数
    {
        BOOL bSetOne = FALSE;
        for (UINT i = 0; i < ADP_MAX_CU_SIZE; i++)
        {
            if (bSetOne) //如果已经设置过1，其他钞箱设置为0
            {
                aryulRetractCount[i] = 0;
            }
            else if (aryulRetractCount[i] != 0) //回收数不为0的钞箱的张数设置为1
            {
                bSetOne = TRUE;
                aryulRetractCount[i] = 1;
            }
        }
    }

    if (m_Param.GetRetractCountMode() == 2) //按上次挖钞张数计数
    {
        USHORT usIndex = 1;
        if (FindCassetteADPIndex(FCAIF_RETRACT, TRUE, usIndex) < 0)
        {
            Log(ThisModule, -1, "没有拒收钞和回收箱");
            return ;
        }

        UINT i = 0;
        for (; i < ADP_MAX_CU_SIZE; i++)
        {
            if (aryulRetractCount[i] > 0)
            {
                break;
            }
        }

        if (i == ADP_MAX_CU_SIZE)
        {
            Log(ThisModule, -1, "回收数为0");
            return;
        }

        if ((i + 1) != usIndex)
        {
            Log(ThisModule, -1, "钞箱[%d]回收数不为0，且钞箱[%d]为回收箱", i + 1, usIndex);
            return;
        }

        CLEAR_ARRAY(aryulRetractCount);
        for (UINT i = 0; i < ADP_MAX_CU_SIZE; i++)
        {
            if (m_ulLastDispenseCount[i] > 0)
            {
                aryulRetractCount[usIndex - 1] += m_ulLastDispenseCount[i];
            }
        }
        CLEAR_ARRAY(m_ulLastDispenseCount);
    }

    //更新钞票列表
    char arycResult[256];
    ULONG ulRejCount(0);
    NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE];
    if (m_Param.GetRetractCountMode() == 0 && m_Param.GetCashUnitType() == 0)
    {
        long lRs = m_pAdapter->GetRetractNoteIdAndCountPairs(arycResult, ulRejCount);
        if (lRs != 0)
        {
            Log(ThisModule, lRs, "调用m_pAdapter->GetRetractNoteIdAndCountPairs()失败");
        }

        lRs = AnalyseCountResult(arycResult, NoteIDCountArray[usPCUIndex - 1], TRUE, FALSE);
        if (lRs < 0)
        {
            Log(ThisModule, lRs, "AnalyseCashInResult(%s)失败", arycResult);
            return;
        }
    }

    if (m_Param.GetRetractCountMode() == 1 || m_Param.GetRetractCountMode() == 2)
    {
        CAutoUpdateCassData _auto_update_cass_data(this, NULL, NULL, aryulRetractCount, NULL, bCDM);
    }
    else
    {
        CAutoUpdateCassData _auto_update_cass_data(this, NULL, NULL, NULL, NoteIDCountArray, bCDM);
    }
}

long XFS_CRSImp::AnalyseCountResult(const char arycResult[256],
                                    NOTEID_COUNT_ARRAY &NoteIDCountArray,
                                    BOOL bAddUnknownIDToArray,
                                    BOOL bVerifyNoteID)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();
    //计算NOTEID的个数
    size_t usCount = strlen(arycResult);
    if (usCount % 12 != 0)
    {
        Log(ThisModule, -1,
            "进钞结果(%s)长度(%hd)不是12的整数倍",
            arycResult, usCount);
        return WFS_ERR_INTERNAL_ERROR;
    }
    usCount = usCount / 12;

    //读NOTEID
    LPWFSCIMNOTETYPELIST pNoteTypeList =
    GCNTCDM_DEPOSIT_ALLOWED(m_Param.GetCfgNoteTypeCantDepositMode()) ? //允许进
    m_Param.GetNoteTypeList() : GetNoteTypeListDipositable();
    for (USHORT i = 0; i < usCount; i++)
    {
        int nID = 0, nCount = 0;
        int iScanResult = sscanf(arycResult + i * 12, "%06x%06d", &nID, &nCount);
        if (iScanResult != 2)
        {
            Log(ThisModule, -1,
                "处理第%hd个数据时sscanf(%s)失败(iScanResult=%d)",
                i + 1, arycResult + i * 12, iScanResult);
            return WFS_ERR_INTERNAL_ERROR;
        }
        if (nID <= 0)
        {
            Log(ThisModule, -1,
                "第%hd项ID非法(ID=%d)", i + 1, nID);
            return WFS_ERR_INTERNAL_ERROR;
        }

        if (nCount == 0)
            continue;

        if (nID != NOTE_ID_UNKNOWN)
        {
            int iRet = FindNoteIDInNoteTypeList((USHORT)nID, pNoteTypeList);
            assert(iRet < (int)pNoteTypeList->usNumOfNoteTypes);
            if (iRet < 0)
            {
                Log(ThisModule, -1,
                    "第%hd项ID(%d)未在币种类型列表中定义", i + 1, nID);
                return WFS_ERR_INTERNAL_ERROR;
            }
            if (!pNoteTypeList->lppNoteTypes[iRet]->bConfigured && bVerifyNoteID)
            {
                Log(ThisModule, -1,
                    "第%hd项ID(%d)在币种类型列表中未启用", i + 1, nID);
                return WFS_ERR_INTERNAL_ERROR;
            }
            NoteIDCountArray.push_back(NOTEID_COUNT((USHORT)nID, (ULONG)nCount));
        }
        else if (bAddUnknownIDToArray)
        {
            NoteIDCountArray.push_back(NOTEID_COUNT((USHORT)nID, (ULONG)nCount));
        }
    }
    return 0;
}

long XFS_CRSImp::SaveCashInEndResult(
const NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE], ICIMSetUpdatesOfCU *pUpdates)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    long iRet;

    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    iRet = pUpdates->SetSourceInfor(pCIM->BuildByXFSCIMFormat());
    VERIFY_RESULT("pUpdates->SetSourceInfor", iRet);
    for (USHORT i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        //计算总张数
        ULONG ulCount = 0;
        for (NOTEID_COUNT_ARRAY::const_iterator it = NoteIDCountArray[i].begin();
             it != NoteIDCountArray[i].end();
             it++)
        {
            ulCount += it->second;
        }

        //如果张数大于0，设置到对应的逻辑钞箱的张数和NoteNumber中
        if (ulCount > 0)
        {
            USHORT usNumber = MapIndex2NumberCIM(i + 1);
            if (usNumber == (USHORT) - 1)
            {
                Log(ThisModule, -1, "MapIndex2NumberCIM(%hd)失败",
                    i + 1);
                return WFS_ERR_INTERNAL_ERROR;
            }

            ICashUnit *pCU = pCIM->GetCUByNumber(usNumber);
            assert(pCU != NULL);
            pUpdates->SetCount(usNumber, ulCount, ulCount);

            for (NOTEID_COUNT_ARRAY::const_iterator it = NoteIDCountArray[i].begin();
                 it != NoteIDCountArray[i].end();
                 it++)
            {
                pUpdates->SetNoteNumber(usNumber, it->first, it->second);
            }
        }
    }

    return 0;
}

long XFS_CRSImp::DisableUnusedCassettes(BOOL bCDM)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    //首先清除标志
    BOOL bEnables[ADP_MAX_CU_SIZE];

    GetEnableFromCUManager(bCDM, m_bProhibitRecycleBox, bEnables);

    //设置到适配层
    long iRet = m_pAdapter->EnableCashUnit(bEnables);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "m_pAdapter->EnableCashUnit()失败");
        return iRet;
    }

    return 0;
}

void XFS_CRSImp::GetEnableFromCUManager(BOOL bCDM, BOOL bProhibitRecycleBox, BOOL bEnables[ADP_MAX_CU_SIZE]) const
{
    memset(bEnables, 0, sizeof(BOOL) * ADP_MAX_CU_SIZE);

    USHORT i;
    USHORT usCount;

    //查找钞箱管理器中各个钞箱对应适配层钞箱
    //对CDM，如果它没有锁定、1）出钞箱、状态可出钞并且剩余张数大于0，2）其他类型钞箱、状态可接收，则允许使用它。
    //对CIM，只要没有锁定、状态状态可接收，则允许使用它
    if (bCDM)   //CDM
    {
        ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
        usCount = pCDM->GetCUCount();
        for (i = 0; i < usCount; i++)
        {
            ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
            assert(pCU != NULL);
            USHORT usIndex = pCU->GetIndex();
            if (usIndex == 0 || usIndex > ADP_MAX_CU_SIZE)
                continue;
            usIndex--;
            if (CUIsOutBox(pCU->GetType()) && CUCanDispense(pCU))
            {
                bEnables[usIndex] = TRUE;
            }
//            else if (CUIsAcceptBox(pCU->GetType()) && CUCanAccept(pCU, m_Param.GetNoteTypeList()))
            else if ((pCU->GetType() == ADP_CASSETTE_RETRACT || pCU->GetType() == ADP_CASSETTE_REJECT)
                     && CUCanAccept(pCU, m_Param.GetNoteTypeList()))
            {
                bEnables[usIndex] = TRUE;
            }
        }
    }
    else    //CIM
    {
        DWORD dwModeOfUseRecycle = m_Param.GetModeOfUseRecycleUnitsOfCashInStart();
        ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
        usCount = pCIM->GetCUCount();
        for (i = 0; i < usCount; i++)
        {
            ICashUnit *pCU = pCIM->GetCUByNumber(i + 1);
            assert(pCU != NULL);
            USHORT usIndex = pCU->GetIndex();
            if (usIndex == 0 || usIndex > ADP_MAX_CU_SIZE)
                continue;

            if (pCU->GetType() == ADP_CASSETTE_RECYCLING)
            {
                if (dwModeOfUseRecycle == 0)    //正常模式，由标示决定
                {
                    if (bProhibitRecycleBox)
                    {
                        continue;
                    }
                }
                else if (dwModeOfUseRecycle != 1)   //2或其他: 忽略标志，直接禁用
                    continue;
                //1: 忽略标志，不禁用
            }

            usIndex--;
            if (CUCanAccept(pCU, m_Param.GetNoteTypeList()))
            {
                bEnables[usIndex] = TRUE;
            }
        }
    }
}

void XFS_CRSImp::LogDispenseRejectCount(const char *ThisModule, const char *lpszActionName,
                                        const ULONG ulDispenseCounts[ADP_MAX_CU_SIZE], const ULONG ulRejectCounts[ADP_MAX_CU_SIZE])
{
    char arycDispenseResult[1024];
    char *p = arycDispenseResult;
    //记录原始出钞数据
    int i = 0;
    for (i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        sprintf(p, "[%d]=D%2d R%2d ", i + 1, ulDispenseCounts[i], ulRejectCounts[i]);
        p += strlen(p);
    }
    Log(ThisModule, 1,
        "%s物理出钞数据: %s", lpszActionName, arycDispenseResult);

    //保存数据到返回结果，并记录Dispense结果
    p = arycDispenseResult;
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    ULONG ulCout = 0;
    for (i = 0; i < pCDM->GetCUCount(); i++)
    {
        ICashUnit *pCU = pCDM->GetCUByNumber(i + 1);
        USHORT usIndex = pCU->GetIndex() - 1;

        sprintf(p, "[%d]=%d ", pCU->GetValues(), ulDispenseCounts[usIndex]);
        p += strlen(p);
        ulCout += ulDispenseCounts[usIndex];
    }
    //Fire序列号事件
    if (m_pAdapter->GetNoteSeiralNumbersCount() > 0 && ulCout != 0 && m_Param.GetInfoAvailableAfterCashOut())
    {
        Log(ThisModule, 1, "发送事件WFS_EXEE_CIM_INFO_AVAILABLE");
        //FireInfoAvailable(m_pAdapter->GetNoteSeiralNumbersCount());
    }
    Log(ThisModule, 1,
        "%s成功: %s", lpszActionName, arycDispenseResult);
}

/******* internal function member ******/
// 根据适配层使用的钞箱数组下标得到对应的CIM逻辑钞箱number, 返回0xffff表示失败
USHORT XFS_CRSImp::MapIndex2NumberCIM(USHORT usIndex)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    USHORT usCount = pCIM->GetCUCount();
    for (USHORT i = 0; i < usCount; i++)
    {
        ICashUnit *pCU = pCIM->GetCUByNumber(i + 1);
        if (pCU == NULL)
        {
            Log(ThisModule, -1,
                "pCIM->GetCUByNumber(usNumber=%hd)失败", i + 1);
            return -1;
        }

        if (pCU->GetIndex() == usIndex)
            return i + 1;
    }

    return -1;
}

BOOL XFS_CRSImp::FindCassByADPIndex(USHORT usIndex, ICashUnit *&pCU)
{
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    USHORT usCount = pCDM->GetCUCount();
    for (USHORT i = 0; i < usCount; i++)
    {
        pCU = pCDM->GetCUByNumber(i + 1);
        if (pCU == NULL)
        {
            continue;
        }

        if (pCU->GetIndex() == usIndex)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void XFS_CRSImp::SetNoteIDToADPCashInfo(ICashUnit *pCU, BRMCASHUNITINFOR &stCUInfor)
{
    LPUSHORT pNoteIDs = FormatCUNoteIDs(pCU, m_Param.GetNoteTypeList());
    stCUInfor.SetNoteIDs(pNoteIDs);
}

//得到钞箱币种ID数组（不可在多线程中调用）
//返回以0结束的币种ID数组
LPUSHORT XFS_CRSImp::FormatCUNoteIDs(ICashUnit *pCU, const LPWFSCIMNOTETYPELIST pNoteTypeList)
{
    static USHORT aryusNoteIDs[100];
    CLEAR_ARRAY(aryusNoteIDs);
    DWORD dwNoteIDCount = 0;
    DWORD dwItemType = pCU->GetItemType();
    if ((dwItemType & WFS_CIM_CITYPINDIVIDUAL) != 0)
    {
        dwNoteIDCount = pCU->GetNoteIDs(aryusNoteIDs);
        aryusNoteIDs[dwNoteIDCount] = 0;
    }

    if ((dwItemType & WFS_CIM_CITYPALL) != 0)
    {
        for (USHORT i = 0; i < pNoteTypeList->usNumOfNoteTypes; i++)
        {
            LPWFSCIMNOTETYPE pType = pNoteTypeList->lppNoteTypes[i];
            if (!FindNoteIDInList(pType->usNoteID, aryusNoteIDs))
            {
                aryusNoteIDs[dwNoteIDCount++] = pType->usNoteID;
            }
        }
    }

    if ((dwItemType & WFS_CIM_CITYPUNFIT) != 0)
    {
        aryusNoteIDs[dwNoteIDCount++] = NOTE_ID_UNKNOWN;
    }
    return aryusNoteIDs;
}

//更新实际可存的币种列表
void XFS_CRSImp::UpdateNoteTypeListDepositable()
{
//30-00-00-00(FS#0021)    assert(m_NoteTypeListDepositable.usNumOfNoteTypes ==
    assert(m_NoteTypeListDepositable.usNumOfNoteTypes <=            //30-00-00-00(FS#0021)
           m_Param.GetNoteTypeList()->usNumOfNoteTypes);
    ICUInterface *pCIM = m_pCUManager->GetCUInterface_CIM();
    for (USHORT i = 0; i < m_NoteTypeListDepositable.usNumOfNoteTypes; i++)
    {
        LPWFSCIMNOTETYPE pNoteType = m_NoteTypeListDepositable.lppNoteTypes[i];
        assert(pNoteType != NULL);
        pNoteType->bConfigured = FALSE; //首先，清为未配置

        BOOL bOrigConfig = m_Param.GetNoteTypeList()->lppNoteTypes[i]->bConfigured; //原配置值
        if (!bOrigConfig)   //只修改原配置为TRUE的币种类型
            continue;

        //循环检查CIM钞箱，如可进该ID币，则设置为TRUE
        for (UINT j = 0; j < pCIM->GetCUCount(); j++)
        {
            ICashUnit *pCU = pCIM->GetCUByNumber(j + 1);
            assert(pCU != NULL);
            if (!CUCanAccept(pCU, m_Param.GetNoteTypeList()))
                continue;
            if (pCU->GetType() == ADP_CASSETTE_RETRACT)
                continue;

            LPUSHORT pNoteIDs = FormatCUNoteIDs(pCU, m_Param.GetNoteTypeList());
            if (FindNoteIDInList(pNoteType->usNoteID, pNoteIDs) ||
                FindNoteIDInList(NOTE_ID_UNKNOWN, pNoteIDs))
            {
                pNoteType->bConfigured = TRUE;
                break;
            }
        }
    }
}

//校验出钞数与要求出钞数一致
BOOL XFS_CRSImp::VerifyDispenseCount(const ULONG ulCount[ADP_MAX_CU_SIZE],
                                     const ULONG ulDispenseCounts[ADP_MAX_CU_SIZE], const char *pCurrencyID)
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();();

    //首先比较是否完全按输入张数出钞
    if (memcmp(ulCount, ulDispenseCounts, sizeof(ULONG) * ADP_MAX_CU_SIZE) == 0)
        return TRUE;

    if (memcmp(pCurrencyID, "   ", 3) == 0)
    {
        Log(ThisModule, -1,
            "多币种出钞时出钞张数与要求张数不一致");
        return FALSE;
    }

    //计算要求出钞金额
    ICUInterface *pCDM = m_pCUManager->GetCUInterface_CDM();
    ULONG ulAmount = 0;
    USHORT i = 0;
    for (i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        if (ulCount[i] > 0)
        {
            ICashUnit *pCU = pCDM->GetFirstCUByPHIndex(i + 1);
            assert(pCU != NULL);
            assert(pCU->GetValues() > 0);
            ulAmount += ulCount[i] * pCU->GetValues();
        }
    }

    //计算实际出钞金额
    ULONG ulDispenseAmount = 0;
    for (i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        if (ulDispenseCounts[i] > 0)
        {
            ICashUnit *pCU = pCDM->GetFirstCUByPHIndex(i + 1);
            if (pCU == NULL)
            {
                Log(ThisModule, -1,
                    "pCDM->GetFirstCUByPHIndex(%hd) failed", i + 1);
                return FALSE;
            }
            char cCurrencyID[3];
            pCU->GetCurrencyID(cCurrencyID);
            if (!CUIsOutBox(pCU->GetType()) ||
                pCU->GetValues() == 0 ||
                memcmp(pCurrencyID, cCurrencyID, 3) != 0)
            {
                Log(ThisModule, -1,
                    "CU Type(%d) or currency(%.3s) or Values(%d) error(usNumber=%hd, currency=%.3s)",
                    pCU->GetType(), cCurrencyID, pCU->GetValues(), pCU->GetNumber(), pCurrencyID);
                return FALSE;
            }
            ulDispenseAmount += ulDispenseCounts[i] * pCU->GetValues();
        }
    }

    //比较金额是否相等
    if (ulAmount == ulDispenseAmount)
        return TRUE;

    return FALSE;
}

BOOL  XFS_CRSImp::IsOnWaitTaken()
{
    return FALSE;
}

void XFS_CRSImp::SetFlagOfCSItemFromTS(BOOL bCSItemsFromTS)
{
    m_bCSItemsFromTS = bCSItemsFromTS;
}

bool XFS_CRSImp::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
    return TRUE;
}

void XFS_CRSImp::FireDeviceHardWare()
{
    m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_pAdapter->GetLastError());
    return ;
}

void XFS_CRSImp::CDMFireSafeDoorOpen()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_SAFEDOOROPEN, nullptr);
}

void XFS_CRSImp::CDMFireSafeDoorClose()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_SAFEDOORCLOSED, nullptr);
}

void XFS_CRSImp::CDMFireCashUnitThreshold(const LPWFSCDMCASHUNIT lpCU)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_CDM_CASHUNITTHRESHOLD, lpCU);
}

void XFS_CRSImp::CDMFireCUInfoChanged(const LPWFSCDMCASHUNIT lpCU)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_CASHUNITINFOCHANGED, lpCU);
}

void XFS_CRSImp::CDMFireCUError(WORD wFail, const LPWFSCDMCASHUNIT lpCU)
{
    WFSCDMCUERROR err;
    err.wFailure = wFail;
    err.lpCashUnit = lpCU;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CDM_CASHUNITERROR, &err);
}

void XFS_CRSImp::CDMFireItemTaken(WORD wPos)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_ITEMSTAKEN, &wPos);
}

void XFS_CRSImp::CDMFireCountsChanged(LPUSHORT lpusCUNumList, USHORT usCount)
{
    WFSCDMCOUNTSCHANGED CashChangeInfo;
    CashChangeInfo.usCount = usCount;
    CashChangeInfo.lpusCUNumList = lpusCUNumList;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_COUNTS_CHANGED, &CashChangeInfo);
}

void XFS_CRSImp::CDMFireNoteError(USHORT usReason)
{

}

void XFS_CRSImp::CDMFireItemPresented()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_ITEMSPRESENTED, nullptr);
}

void XFS_CRSImp::CDMFireMediaDetected(const LPWFSCDMITEMPOSITION lpItemPosition)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_MEDIADETECTED, lpItemPosition);
}

void XFS_CRSImp::CIMFireSafeDoorOpen()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_SAFEDOOROPEN, nullptr);
}

void XFS_CRSImp::CIMFireSafeDoorClose()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_SAFEDOORCLOSED, nullptr);
}

void XFS_CRSImp::CIMFireCashUnitThreshold(const LPWFSCIMCASHIN lpCashIn)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_CIM_CASHUNITTHRESHOLD, lpCashIn);
}

void XFS_CRSImp::CIMFireCUInfoChanged(const LPWFSCIMCASHIN lpCashIn)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_CASHUNITINFOCHANGED, lpCashIn);
}

void XFS_CRSImp::CIMFireTellerInfoChanged(USHORT usTellerID)
{

}

void XFS_CRSImp::CIMFireCUError(WORD wFail, const LPWFSCIMCASHIN lpCashIn)
{
    WFSCIMCUERROR CashUnitError;
    CashUnitError.wFailure = wFail;
    CashUnitError.lpCashUnit = lpCashIn;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CIM_CASHUNITERROR,  &CashUnitError);
}

void XFS_CRSImp::CIMFireItemTaken()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_ITEMSTAKEN, nullptr);
}

void XFS_CRSImp::CIMFireCountsChanged(LPUSHORT lpusCUNumList, USHORT usCount)
{
    WFSCIMCOUNTSCHANGED CountChanged;
    CountChanged.lpusCUNumList = lpusCUNumList;
    CountChanged.usCount = usCount;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_COUNTS_CHANGED, &CountChanged);
}

void XFS_CRSImp::CIMFireInputRefuse(USHORT usReason)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CIM_INPUTREFUSE, &usReason);
}

void XFS_CRSImp::CIMFireNoteError(USHORT usReason)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CIM_NOTEERROR, &usReason);
}

void XFS_CRSImp::CIMFireItemPresented()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_ITEMSPRESENTED, nullptr);
}

void XFS_CRSImp::CIMFireItemsInserted()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_ITEMSINSERTED, nullptr);
}

void XFS_CRSImp::CIMFireSubCashin(const LPWFSCIMNOTENUMBERLIST lpNoteNumberList)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CIM_SUBCASHIN, lpNoteNumberList);
}

void XFS_CRSImp::CIMFireMediaDetected(const LPWFSCIMITEMPOSITION lpItemPosition)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_MEDIADETECTED, lpItemPosition);
}

/*310
//void CIMFireInfoAvailable(LPWFSCIMITEMINFOSUMMARY *lppItemInfoSummary);
void XFS_CRSImp::CIMFireInsertItems()
{

}
*/
