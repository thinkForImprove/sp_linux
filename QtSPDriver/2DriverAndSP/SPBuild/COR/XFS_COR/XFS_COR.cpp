#include "XFS_COR.h"

static const char *DEVTYPE = "COR";
static const char *ThisModule = "XFS_COR.cpp";

#define SP_ERR_SUCCESS                      0
#define SP_ERR_LOAD_LIBRARY_FAIL            1
#define SP_ERR_CONFIG_ERROR                 2
#define SP_ERR_OPEN_FAIL                    3
#define SP_ERR_RESET_FAIL                   4
#define SP_ERR_DATA_UNMATCH                 5
#define SP_ERR_DEV_ERROR                    6
#define SP_ERR_PARAM_ERROR                  7
#define SP_ERR_NO_ENOUGH_COIN               8
#define SP_ERR_CODE_MAX                     9

static const char *sp_errors[SP_ERR_CODE_MAX][2] = {
    {"00000000", "SUCCESS"},
    {"00000001", "加载程序库文件失败"},
    {"00000002", "配置文件异常"},
    {"00000003", "打开设备失败"},
    {"00000004", "复位设备失败"},
    {"00000005", "配置文件数据和设备不挥发数据不一致"},
    {"00000006", "设备故障"},
    {"00000007", "参数异常"},
    {"00000008", "钞箱硬币不足"}
};

//////////////////////////////////////////////////////////////////////////
//校验错误条件
//如bErrCondition为真，记录pErrorDesc日志，并返回ErrorCode错误码
#define VERIFY_ERR_CONDITION(bErrCondition, ErrorCode, pErrorDesc) \
    {\
        HRESULT hRes = CheckAndLog((bErrCondition), ThisModule, (ErrorCode), (pErrorDesc));\
        if (hRes != 0)\
            return hRes;\
    }

//检查条件并记录日志
HRESULT CXFS_COR::CheckAndLog(BOOL bErrCondition, const char *ThisModule,
                                HRESULT hRes, const char *pLogDesc)
{
    if (bErrCondition)
    {
        Log(ThisModule, -1, pLogDesc);
        return hRes;
    }
    return 0;
}

//校验正常打开设备
#define VERIFY_DEVICE_OPENED()     VERIFY_ERR_CONDITION(!m_bOpened, \
                                                                WFS_ERR_HARDWARE_ERROR, "Device is not open")

//校验正常打开设备
#define VERIFY_CUINFO_INITIALIZED()     VERIFY_ERR_CONDITION(!m_bCUInfoInitialized, \
                                                                WFS_ERR_HARDWARE_ERROR, "Cash unit info is not initialized")

//校验不在交换状态
#define VERIFY_NOT_IN_EXCHANGE(bCDM)    VERIFY_ERR_CONDITION(m_bCDMExchangeActive || m_bCIMExchangeActive, \
                                                                bCDM ? WFS_ERR_CDM_EXCHANGEACTIVE:WFS_ERR_CIM_EXCHANGEACTIVE, "Exchange is active")

//校验处于交换状态
#define VERIFY_IN_EXCHANGE(bCDM)        VERIFY_ERR_CONDITION((!m_bCDMExchangeActive && bCDM) || (!m_bCIMExchangeActive && !bCDM), \
                                                                bCDM ? WFS_ERR_CDM_NOEXCHANGEACTIVE:WFS_ERR_CIM_NOEXCHANGEACTIVE, "Exchange is NOT active")

//校验不在CASH_IN事务中
#define VERIFY_NOT_IN_CASH_IN(bCDM)     VERIFY_ERR_CONDITION(m_bCashInActive, \
                                                                bCDM ? WFS_ERR_DEV_NOT_READY:WFS_ERR_CIM_CASHINACTIVE, "It is in CashIn transaction")

//校验处于CASH_IN事件中
#define VERIFY_IN_CASH_IN()         VERIFY_ERR_CONDITION(!m_bCashInActive, WFS_ERR_CIM_NOCASHINACTIVE, "It is NOT in CashIn transaction")

CXFS_COR::CXFS_COR()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisModule, DEVTYPE);

    m_strConfigFile = string(SPETCPATH) + "/CORConfig.ini";

    m_bCanceled = FALSE;

    memset(m_cFirmwareVer, 0, sizeof(m_cFirmwareVer));
    memset(m_cStatusExtraInfo, 0, sizeof(m_cStatusExtraInfo));
    memset(m_cErrorCode, 0, sizeof(m_cErrorCode));
    sprintf(m_cErrorCode, "%s", "00000000");
    memset(m_cErrorDetail, 0, sizeof(m_cErrorDetail));
    sprintf(m_cErrorDetail, "%s", "SUCCESS");
    memset(m_cNullExtra, 0, sizeof(m_cNullExtra));

    m_bCDMExchangeActive = FALSE;
    m_bCIMExchangeActive = FALSE;
    m_bCashInActive = FALSE;
    m_bCashOutActive = FALSE;
    m_bInCmdExecute = FALSE;
    m_bOpened = FALSE;
    m_bCUInfoInitialized = FALSE;
    m_wPresentState = WFS_CDM_UNKNOWN;
    m_wCashInStatus = WFS_CIM_CIUNKNOWN;

    memset(m_cCashUnitNameArray, 0, sizeof(m_cCashUnitNameArray));
    memset(m_cPHPositionNameArray, 0, sizeof(m_cPHPositionNameArray));
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        sprintf(m_cPHPositionNameArray[i], "%d", i + 1);
    }
    sprintf(m_cPHPositionNameArray[MAX_COINCYLINDER_NUM], "%d", MAX_COINCYLINDER_NUM + 1);

    memset(m_ulCountBK, 0, sizeof(m_ulCountBK));
    memset(m_CUStatus, 0, sizeof(m_CUStatus));

    m_usCoinEnable = 0;
    m_usCoinEnable |= COIN_CN010B | COIN_CN010C | COIN_CN010D | COIN_CN050B | COIN_CN050C | COIN_CN050D | COIN_CN100B | COIN_CN100C;
}

CXFS_COR::~CXFS_COR()
{
}

long CXFS_COR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseCOR
    if (0 != m_pBase.Load("SPBaseCOR.dll", "CreateISPBaseCOR", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseCOR失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_COR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpLogicalName)

    HRESULT hRes = 0;
    m_bOpened = FALSE;

    //装载配置参数
    hRes = m_Param.LoadParam(m_strConfigFile.c_str());
    if (hRes != WFS_SUCCESS)
    {
        SetSPErrorInfo(SP_ERR_CONFIG_ERROR);
        return hRes;
    }

    std::string strPort = m_Param.getPortMode();
    std::string strDevDll = m_Param.getDevDllName();
    if (strDevDll.empty() || strPort.empty())
    {
        SetSPErrorInfo(SP_ERR_CONFIG_ERROR);
        Log(ThisModule, __LINE__, "SP=%s的DdevDllName或PortMode配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 加载DEV
    hRes = m_pDev.Load(strDevDll.c_str(), "CreateIDevCOR", DEVTYPE);
    if (0 != hRes)
    {
        SetSPErrorInfo(SP_ERR_LOAD_LIBRARY_FAIL);
        Log(ThisModule, __LINE__, "加载%s失败, hRes=%d", strDevDll.c_str(), hRes);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 打开连接
    hRes = m_pDev->Open(m_strConfigFile.c_str());
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_OPEN_FAIL);
        Log(ThisModule, __LINE__, "打开设备连接失败！, Port=%s", strPort.c_str());
        return WFS_ERR_HARDWARE_ERROR;
    }

    // set open flag.
    m_bOpened = TRUE;

    // 复位设备
    hRes = m_pDev->Reset();
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_RESET_FAIL);

        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);

    // 更新扩展状态(FirmwareViersion/Production info)
    memset(m_cFirmwareVer, 0, sizeof(m_cFirmwareVer));
    hRes = m_pDev->GetFWVersion(m_cFirmwareVer);
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, __LINE__, "设备固件版本信息取得失败！");
    }

    memset(&m_stDevInfo, 0, sizeof(m_stDevInfo));
    hRes = m_pDev->GetDevInfo(m_stDevInfo);
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, __LINE__, "设备信息取得失败！");
    }

    // 比对并初始化钞箱信息
    hRes = InitializeCashUnitInfo();
    if (hRes != WFS_SUCCESS)
    {
        m_bCUInfoInitialized = FALSE;
        Log(ThisModule, __LINE__, "初始化钞箱信息失败！");
    }
    else {
        m_bCUInfoInitialized = TRUE;
    }
    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    // 更新一次状态
    OnStatus();

    Log(ThisModule, 1, "打开设备连接成功");

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
    {
        m_pDev->Close();
    }

    // set open flag.
    m_bOpened = FALSE;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::OnStatus()
{
    THISMODULE(__FUNCTION__);

    // 空闲更新状态
    UpdateStatus(ThisModule);

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_pDev->Cancel();
    m_bCanceled = TRUE;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //TOFO

    return WFS_SUCCESS;
}

void CXFS_COR::SetExeFlag(DWORD dwCmd, BOOL bInExe)
{
    m_bInCmdExecute = bInExe;
    m_dwCurrentCommand = dwCmd;
}

HRESULT CXFS_COR::CDMGetStatus(LPWFSCDMSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);

    UpdateStatus(ThisModule);

    memset(&m_CDMStatus, 0, sizeof(m_CDMStatus));
    m_CDMStatus.fwDevice = m_stCorStatus.fwDevice;
    if (m_bCUInfoInitialized == FALSE)
    {
        m_CDMStatus.fwDevice = WFS_CDM_DEVHWERROR;
    }
    if ((m_CDMStatus.fwDevice != WFS_CDM_DEVHWERROR) && m_bInCmdExecute)
    {
        m_CDMStatus.fwDevice = WFS_CDM_DEVBUSY;
    }

    m_CDMStatus.fwDispenser = m_stCorStatus.fwDispenser;
    m_CDMStatus.fwIntermediateStacker = m_stCorStatus.fwIntermediateStacker;
    m_CDMStatus.fwSafeDoor = m_stCorStatus.fwSafeDoor;

    memset(m_CDMOutPosArray, 0, sizeof(m_CDMOutPosArray));
    memset(&m_CDMOutPos, 0, sizeof(m_CDMOutPos));
    m_CDMOutPos.fwPosition = m_stCorStatus.fwPosition;
    m_CDMOutPos.fwShutter = m_stCorStatus.fwShutter;
    m_CDMOutPos.fwPositionStatus = m_stCorStatus.fwPositionStatus;
    m_CDMOutPos.fwTransport = m_stCorStatus.fwTransport;
    m_CDMOutPos.fwTransportStatus = m_stCorStatus.fwTransportStatus;
    m_CDMOutPosArray[0] = &m_CDMOutPos;
    m_CDMStatus.lppPositions = m_CDMOutPosArray;

    // Extra info.
    EditStatusExtraInfo();
    m_CDMStatus.lpszExtra = m_cStatusExtraInfo;

    // assign output parameter.
    lpStatus = &m_CDMStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMGetCapabilities(LPWFSCDMCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRes;
    ST_COR_CAPS stCorCaps;
    hRes = m_pDev->GetDeviceCaps(&stCorCaps);
    if (hRes != 0)
    {
        lpCaps = NULL;
        Log(ThisModule, __LINE__, "GetDeviceCaps fail(%d)", hRes);
        return WFS_ERR_HARDWARE_ERROR;
    }

    ST_CDMCAPS* pstCorCaps = (ST_CDMCAPS*)&stCorCaps;

    memset(&m_CDMCaps, 0, sizeof(m_CDMCaps));
    m_CDMCaps.wClass = WFS_SERVICE_CLASS_CDM;
    m_CDMCaps.fwType = WFS_CDM_SELFSERVICECOIN;
    m_CDMCaps.wMaxDispenseItems = pstCorCaps->wMaxDispenseItems;
    m_CDMCaps.bCompound = TRUE;
    m_CDMCaps.bShutter = pstCorCaps->bShutter;
    m_CDMCaps.bShutterControl = pstCorCaps->bShutterControl;
    m_CDMCaps.fwRetractAreas = pstCorCaps->fwRetractAreas;
    m_CDMCaps.fwRetractTransportActions = pstCorCaps->fwRetractTransportActions;
    m_CDMCaps.fwRetractStackerActions = pstCorCaps->fwRetractStackerActions;
    m_CDMCaps.bSafeDoor = pstCorCaps->bSafeDoor;
    m_CDMCaps.bCashBox = FALSE;
    m_CDMCaps.bIntermediateStacker = pstCorCaps->bIntermediateStacker;
    m_CDMCaps.bItemsTakenSensor = pstCorCaps->bItemsTakenSensor;
    m_CDMCaps.fwPositions = pstCorCaps->fwPositions;
    m_CDMCaps.fwMoveItems = pstCorCaps->fwMoveItems;
    m_CDMCaps.fwExchangeType = WFS_CDM_EXBYHAND;
    m_CDMCaps.lpszExtra = m_cNullExtra;

    // assign output parameter.
    lpCaps = &m_CDMCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMGetCashUnitInfo(LPWFSCDMCUINFO &lpCUInfor)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRes;
    hRes = EditCDMCashUnitInfo();

    // assign output parameter.
    lpCUInfor = &m_CDMCUInfo;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMGetMixType(LPWFSCDMMIXTYPE *&lppMixType)
{
    THISMODULE(__FUNCTION__);

    lppMixType = m_Param.GetMixTypes();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMGetMixTable(USHORT usMixNumber, LPWFSCDMMIXTABLE &lpMixTable)
{
    Q_UNUSED(usMixNumber)
    Q_UNUSED(lpMixTable)
    return WFS_ERR_UNSUPP_CATEGORY;
}

HRESULT CXFS_COR::CDMGetPresentStatus(WORD wPos, LPWFSCDMPRESENTSTATUS &lpPresentStatus)
{
    THISMODULE(__FUNCTION__);
    Q_UNUSED(wPos)

    memset(&m_CDMPresentStatus, 0, sizeof(m_CDMPresentStatus));
    m_CDMPresentStatus.wPresentState = m_wPresentState;
    m_CDMPresentStatus.lpDenomination = NULL;
    if (m_CDMPresentStatus.wPresentState == WFS_CDM_PRESENTED)
    {
        m_CDMPresentStatus.lpDenomination = &m_CDMDenomination;
    }
    m_CDMPresentStatus.lpszExtra = NULL;

    // assign output parameter.
    lpPresentStatus = &m_CDMPresentStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMGetCurrencyEXP(LPWFSCDMCURRENCYEXP *&lppCurrencyExp)
{
    THISMODULE(__FUNCTION__);
    lppCurrencyExp = m_Param.GetCurrencyExp();
    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMDenominate(const LPWFSCDMDENOMINATE lpDenoData, LPWFSCDMDENOMINATION &pDenoInOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRes = 0;
    ULONG ulAmount = 0;
    ULONG ulRequestItems[MAX_COINCYLINDER_NUM];

    // check Exchange/Cash in status.
    VERIFY_DEVICE_OPENED();
    VERIFY_CUINFO_INITIALIZED();
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    VERIFY_NOT_IN_CASH_IN(TRUE);

    // Clear error.
    SetSPErrorInfo(SP_ERR_SUCCESS);

    memset(ulRequestItems, 0, sizeof(ulRequestItems));
    memset(&m_CDMDenomination, 0, sizeof(m_CDMDenomination));
    hRes = DoDenomination(lpDenoData->usMixNumber, lpDenoData->lpDenomination, &ulAmount, ulRequestItems);
    if (hRes == WFS_SUCCESS)
    {
        m_CDMDenomination.ulAmount = ulAmount;
        memcpy(m_CDMDenomination.cCurrencyID, lpDenoData->lpDenomination->cCurrencyID, 3);
        m_CDMDenomination.usCount = MAX_COINCYLINDER_NUM;
        memcpy(m_ulValues, ulRequestItems, sizeof(ulRequestItems));
        m_CDMDenomination.lpulValues = m_ulValues;
        m_CDMDenomination.ulCashBox = 0;

        // assign result to output parameter.
        pDenoInOut = &m_CDMDenomination;
    }

    return hRes;
}

HRESULT CXFS_COR::CDMDispense(const LPWFSCDMDISPENSE lpDisData, LPWFSCDMDENOMINATION &lpDenoOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check Exchange/Cash in status.
    VERIFY_DEVICE_OPENED();
    VERIFY_CUINFO_INITIALIZED();
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    VERIFY_NOT_IN_CASH_IN(TRUE);

    // Clear error.
    SetSPErrorInfo(SP_ERR_SUCCESS);

    HRESULT hRes = 0;
    ULONG ulAmount = 0;
    ULONG ulRequestItems[MAX_COINCYLINDER_NUM];
    ULONG ulDispensedItems[MAX_COINCYLINDER_NUM];
    DEV_CUERROR devCUErrors[MAX_COINCYLINDER_NUM];

    memset(ulRequestItems, 0, sizeof(ulRequestItems));
    memset(ulDispensedItems, 0, sizeof(ulDispensedItems));
    memset(devCUErrors, 0, sizeof(devCUErrors));
    memset(&m_CDMDenomination, 0, sizeof(m_CDMDenomination));
    hRes = DoDenomination(lpDisData->usMixNumber, lpDisData->lpDenomination, &ulAmount, ulRequestItems);
    if (hRes != WFS_SUCCESS)
    {
        m_wPresentState = WFS_CDM_NOTPRESENTED;
        return hRes;
    }

    // record count before cash out.
    m_Param.LogCashUnitCount(ACTION_CASHOUT, TIMING_BEFORE);

    // Edit denomination.
    memcpy(m_CDMDenomination.cCurrencyID, lpDisData->lpDenomination->cCurrencyID, 3);
    m_CDMDenomination.ulCashBox = 0;

    hRes = m_pDev->Dispense(ulRequestItems, ulDispensedItems, devCUErrors);
    if (hRes == DEV_SUCCESS)
    {
        hRes = WFS_SUCCESS;
        m_CDMDenomination.ulAmount = ulAmount;
        m_CDMDenomination.usCount = MAX_COINCYLINDER_NUM;
        memcpy(m_ulValues, ulDispensedItems, sizeof(ulDispensedItems));
        m_CDMDenomination.lpulValues = m_ulValues;

        // update cash unit count.
        UpdateCashUnitCount(ulDispensedItems, MAX_COINCYLINDER_NUM, MONEY_COUNTERS_MONEYOUT);

        // assign output parameter.
        lpDenoOut = &m_CDMDenomination;
        m_wPresentState = WFS_CDM_PRESENTED;
    }
    else
    {
        Log(ThisModule, __LINE__, "Dispense fail(%d)", hRes);

        ulAmount = 0;
        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            ulAmount += m_stCoinCylinderInfos[i].iCashValue * ulDispensedItems[i];

            // check hopper error.
            switch (devCUErrors[i]) {
            case DEV_CUERROR_OK:
                {
                    hRes = WFS_ERR_CDM_NOTDISPENSABLE;
                }
                break;
            case DEV_CUERROR_EMPTY:
                {
                    hRes = WFS_CDM_CASHUNITERROR;

                    // post cash unit error.
                    CDMFireCUError(WFS_CDM_CASHUNITEMPTY, &m_CDMCashUnitArray[i]);
                }
                break;
            default:
                {
                    SetSPErrorInfo(SP_ERR_DEV_ERROR);
                    hRes = WFS_CDM_CASHUNITERROR;

                    // post cash unit error.
                    CDMFireCUError(WFS_CDM_CASHUNITERROR, &m_CDMCashUnitArray[i]);
                }
                break;
            }
        }

        if (ulAmount == 0)
        {
            Log(ThisModule, __LINE__, "no coin dispensed.");

            lpDenoOut = NULL;
            m_wPresentState = WFS_CDM_NOTPRESENTED;
        }
        else
        {
            m_CDMDenomination.ulAmount = ulAmount;
            m_CDMDenomination.usCount = MAX_COINCYLINDER_NUM;
            memcpy(m_ulValues, ulDispensedItems, sizeof(ulDispensedItems));
            m_CDMDenomination.lpulValues = m_ulValues;

            // update cash unit count.
            UpdateCashUnitCount(ulDispensedItems, MAX_COINCYLINDER_NUM, MONEY_COUNTERS_MONEYOUT);

            // assign output parameter.
            lpDenoOut = &m_CDMDenomination;
            m_wPresentState = WFS_CDM_PRESENTED;
        }
    }

    // record count after cash out.
    m_Param.LogCashUnitCount(ACTION_CASHOUT, TIMING_AFTER);

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    CheckCashUnitThreshold(MONEY_COUNTERS_MONEYOUT);
    CheckCashUnitInfoChanged();
    BackupCashUnitInfo();

    return hRes;
}

HRESULT CXFS_COR::CDMCount(LPWFSCDMPHYSICALCU lpPhysicalCU, LPWFSCDMCOUNT &lpCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check Exchange/Cash in status.
    VERIFY_DEVICE_OPENED();
    VERIFY_CUINFO_INITIALIZED();
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    VERIFY_NOT_IN_CASH_IN(TRUE);

    // Clear error.
    SetSPErrorInfo(SP_ERR_SUCCESS);

    HRESULT hRes = 0;
    int iCylinderNo = 0;
    int iHopperIndex = 0;
    ULONG ulDispensedCount[MAX_COINCYLINDER_NUM];
    ULONG ulCountedCount[MAX_COINCYLINDER_NUM];
    DEV_CUERROR devCUErrors[MAX_COINCYLINDER_NUM];

    if (lpPhysicalCU->bEmptyAll)
    {
        iCylinderNo = 255;
    }
    else if (lpPhysicalCU->lpPhysicalPositionName == NULL) {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "lpPhysicalCU->lpPhysicalPositionName is null");
        return WFS_ERR_INVALID_DATA;
    }
    else {
        BOOL bFind = FALSE;
        char cPositionName[8];
        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            sprintf(cPositionName, "%d", i + 1);
            if (strcmp(lpPhysicalCU->lpPhysicalPositionName, cPositionName) == 0)
            {
                bFind = TRUE;
                iCylinderNo = m_stCoinCylinderInfos[i].iCylinderNO;
                iHopperIndex = i;
                break;
            }
        }

        // invalid physical position name.
        if (bFind == FALSE)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "lpPhysicalCU->lpPhysicalPositionName(%s) is invalid",
                lpPhysicalCU->lpPhysicalPositionName);
            return WFS_ERR_INVALID_DATA;
        }
    }

    // record count before cash out.
    m_Param.LogCashUnitCount(ACTION_CASHOUT, TIMING_BEFORE);

    memset(ulDispensedCount, 0, sizeof(ulDispensedCount));
    memset(ulCountedCount, 0, sizeof(ulCountedCount));
    memset(devCUErrors, 0, sizeof(devCUErrors));
    hRes = m_pDev->Count(iCylinderNo, ulDispensedCount, ulCountedCount, devCUErrors);
    if (hRes != DEV_SUCCESS)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, -1,
            "m_pDev->Count fail(%d)",
            hRes);
        return WFS_CDM_DEVHWERROR;
    }

    memset(&m_CDMCount, 0, sizeof(m_CDMCount));
    memset(m_CDMCountedPhysCUPtrArray, 0, sizeof(m_CDMCountedPhysCUPtrArray));
    memset(m_CDMCountedPhysCUs, 0, sizeof(m_CDMCountedPhysCUs));
    m_CDMCount.lppCountedPhysCUs = m_CDMCountedPhysCUPtrArray;
    if (lpPhysicalCU->bEmptyAll)
    {
        m_CDMCount.usNumPhysicalCUs = MAX_COINCYLINDER_NUM;
        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            m_CDMCountedPhysCUPtrArray[i] = &m_CDMCountedPhysCUs[i];
            strcpy(m_CDMCountedPhysCUs[i].cUnitId, m_stCoinCylinderInfos[i].szId);
            m_CDMCountedPhysCUs[i].lpPhysicalPositionName = m_cPHPositionNameArray[i];
            m_CDMCountedPhysCUs[i].ulCounted = ulCountedCount[i];
            m_CDMCountedPhysCUs[i].ulDispensed = ulDispensedCount[i];
            m_CDMCountedPhysCUs[i].usPStatus = WFS_CDM_STATCUEMPTY;
        }
    }
    else {
        m_CDMCount.usNumPhysicalCUs = 1;
        m_CDMCountedPhysCUPtrArray[0] = &m_CDMCountedPhysCUs[iHopperIndex];
        strcpy(m_CDMCountedPhysCUs[iHopperIndex].cUnitId, m_stCoinCylinderInfos[iHopperIndex].szId);
        m_CDMCountedPhysCUs[iHopperIndex].lpPhysicalPositionName = m_cPHPositionNameArray[iHopperIndex];
        m_CDMCountedPhysCUs[iHopperIndex].ulCounted = ulCountedCount[iHopperIndex];
        m_CDMCountedPhysCUs[iHopperIndex].ulDispensed = ulDispensedCount[iHopperIndex];
        m_CDMCountedPhysCUs[iHopperIndex].usPStatus = WFS_CDM_STATCUEMPTY;
    }

    // update cash unit count.
    UpdateCashUnitCount(ulDispensedCount, MAX_COINCYLINDER_NUM, MONEY_COUNTERS_MONEYOUT);

    // assign output parameter.
    lpCount = &m_CDMCount;

    // record count after cash out.
    m_Param.LogCashUnitCount(ACTION_CASHOUT, TIMING_AFTER);
    BackupCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMPresent(WORD wPos)
{
    Q_UNUSED(wPos)
    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMReject()
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CDMRetract(const WFSCDMRETRACT &stData)
{
    Q_UNUSED(stData);
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CDMOpenShutter(WORD wPos)
{
    Q_UNUSED(wPos)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CDMCloseShutter(WORD wPos)
{
    Q_UNUSED(wPos)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CDMStartEXChange(const LPWFSCDMSTARTEX lpData, LPWFSCDMCUINFO &lpCUInfor)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check exchange status.
    VERIFY_DEVICE_OPENED();
    VERIFY_NOT_IN_EXCHANGE(TRUE);

    HRESULT hRes;

    if (lpData->fwExchangeType != WFS_CDM_EXBYHAND)
    {
        Log(ThisModule, -1,
            "invalid fwExchangeType(%d)",
            lpData->fwExchangeType);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (m_Param.GetStartExchangeIgnoreCUNumberList() == 0)
    {
        if (lpData->lpusCUNumList == NULL)
        {
            Log(ThisModule, -1,
                "invalid lpusCUNumList(NULL)");
            return WFS_ERR_INVALID_POINTER;
        }

        for (int i = 0; i < lpData->usCount; i++)
        {
            if ((lpData->lpusCUNumList[i] <= 0) ||
                    (lpData->lpusCUNumList[i] > (MAX_COINCYLINDER_NUM +1)))
            {
                Log(ThisModule, -1,
                    "invalid logical number(%d)",
                    lpData->lpusCUNumList[i]);
                return WFS_ERR_CIM_CASHUNITERROR;
            }
        }
    }

    // edit output parameter.
    hRes = EditCDMCashUnitInfo();
    lpCUInfor = &m_CDMCUInfo;

    // set exchange active flag.
    m_bCDMExchangeActive = TRUE;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMEndEXChange(const LPWFSCDMCUINFO lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check exchange status.
    VERIFY_DEVICE_OPENED();
    VERIFY_IN_EXCHANGE(TRUE);

    HRESULT hRes;
    LPWFSCDMCASHUNIT lpWFSCDMCashUnit = NULL;
    LPWFSCDMPHCU lpWFSCDMPHCU = NULL;
    LPWFSCIMNOTENUMBERLIST lpLocalNoteNumberList = NULL;

    ULONG ulInitialCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulRejectCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMinimum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMaximum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulPHMaximum[MAX_COINCYLINDER_NUM + 1];
    m_bCDMExchangeActive = FALSE;

    if (lpCUInfo == NULL)
    {
        return WFS_SUCCESS;
    }

    // check cash unit counts.
    if (lpCUInfo->usCount != (MAX_COINCYLINDER_NUM + 1))
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "invalid lpCUInfo->usCount(%d)",
            lpCUInfo->usCount);
        return WFS_ERR_UNSUPP_DATA;
    }

    // update recycling cash unit.
    memset(ulInitialCount, 0, sizeof(ulInitialCount));
    memset(ulCount, 0, sizeof(ulCount));
    memset(ulRejectCount, 0, sizeof(ulRejectCount));
    memset(ulMinimum, 0, sizeof(ulMinimum));
    memset(ulMaximum, 0, sizeof(ulMaximum));
    memset(ulPHMaximum, 0, sizeof(ulPHMaximum));

    for (int i = 0; i < lpCUInfo->usCount; i++)
    {
        lpWFSCDMCashUnit = lpCUInfo->lppList[i];

        // check usType(Recycling(1-6) / Last Cash Box for Retract)
        if (((i < (lpCUInfo->usCount - 1)) && (lpWFSCDMCashUnit->usType != WFS_CDM_TYPERECYCLING)) ||
                ((i == (lpCUInfo->usCount - 1)) && (lpWFSCDMCashUnit->usType != WFS_CDM_TYPERETRACTCASSETTE)))
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usType(%d) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->usType,
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCDMCashUnit->ulMaximum < lpWFSCDMCashUnit->ulMinimum)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid ulMinimum/ulMaximum(%d/%d) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->ulMinimum,
                lpWFSCDMCashUnit->ulMaximum,
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCDMCashUnit->usNumPhysicalCUs != 1)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usNumPhysicalCUs(%d) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->usNumPhysicalCUs,
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCDMCashUnit->lppPhysical == NULL)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid lppPhysical(NULL) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_POINTER;
        }

        // Retrieve count relative info.
        lpWFSCDMPHCU = lpWFSCDMCashUnit->lppPhysical[0];
        if ((lpWFSCDMPHCU->ulCount == 0) &&
                (lpWFSCDMPHCU->ulRejectCount == 0))
        {
            ulInitialCount[i] = lpWFSCDMCashUnit->ulInitialCount;
            ulCount[i] = lpWFSCDMCashUnit->ulCount;
            ulRejectCount[i] = lpWFSCDMCashUnit->ulRejectCount;
        }
        else
        {
            ulInitialCount[i] = lpWFSCDMPHCU->ulInitialCount;
            ulCount[i] = lpWFSCDMPHCU->ulCount;
            ulRejectCount[i] = lpWFSCDMPHCU->ulRejectCount;
        }
        ulMinimum[i] = lpWFSCDMCashUnit->ulMinimum;
        ulMaximum[i] = lpWFSCDMCashUnit->ulMaximum;
        ulPHMaximum[i] = lpWFSCDMPHCU->ulMaximum;

        // Update note id specific count for CIM CashUnitInfo.
        lpLocalNoteNumberList = m_Param.getNoteNumberList(i);

        //NOTE! only clear note id specific count.
        if ((lpLocalNoteNumberList->usNumOfNoteNumbers > 0) && (ulCount[i] == 0))
        {
            for (int j = 0; j < lpLocalNoteNumberList->usNumOfNoteNumbers; j++) {
                lpLocalNoteNumberList->lppNoteNumber[j]->ulCount = 0;
            }
        }
    }

    // Set device hopper no if failed last time.
    if ((m_bCUInfoInitialized == FALSE) && (m_stCoinCylinderInfos[0].iCylinderNO == 0))
    {
        ST_COINCYLINDER_INFO stCoinCylinderInfos[MAX_COINCYLINDER_NUM];
        hRes = m_pDev->GetCoinCylinderList(stCoinCylinderInfos);
        if (DEV_SUCCESS != hRes)
        {
            SetSPErrorInfo(SP_ERR_DEV_ERROR);
            Log(ThisModule, -1,
                "GetCoinCylinderList() failed(%ld)",
                hRes);
            return WFS_ERR_HARDWARE_ERROR;
        }

        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            m_stCoinCylinderInfos[i].iCylinderNO = stCoinCylinderInfos[i].iCylinderNO;
        }
    }

    // update cash unit info(recycling).
    ST_COINCYLINDER_INFO stCyliInfo;
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        stCyliInfo = m_stCoinCylinderInfos[i];
        stCyliInfo.lCount = ulCount[i];

        // update device balance.
        hRes = m_pDev->SetCoinCylinderInfo(stCyliInfo);
        if (hRes == SOFT_ERROR_PARAMS)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            return WFS_ERR_INVALID_DATA;
        }
        else if (hRes == ERR_COR_CASHUNITERROR)
        {
            SetSPErrorInfo(SP_ERR_DEV_ERROR);
            Log(ThisModule, -1,
                "GetCoinCylinderInfo occur CU missing(usNumber:%d, iCylinderNO:%d)",
                i + 1,
                stCyliInfo.iCylinderNO);
            return WFS_ERR_HARDWARE_ERROR;
        }
        else if (hRes != DEV_SUCCESS)
        {
            SetSPErrorInfo(SP_ERR_DEV_ERROR);
            Log(ThisModule, -1,
                "SetCoinCylinderInfo fail(usNumber:%d, iCylinderNO:%d, hRes:%d)",
                i + 1,
                stCyliInfo.iCylinderNO, hRes);
            return WFS_ERR_HARDWARE_ERROR;
        }

        // update config info(local nonvolatile data).
        m_Param.setInitialCount(i, ulInitialCount[i]);
        m_Param.setCount(i, ulCount[i]);
        m_Param.setRejectCount(i, ulRejectCount[i]);
        m_stCoinCylinderInfos[i].lCount = ulCount[i];
        m_Param.setMinimum(i, ulMinimum[i]);
        m_Param.setMaximum(i, ulMaximum[i]);
        m_Param.setPHMaximum(i, ulPHMaximum[i]);
    }

    // update AB cash unit.
    m_Param.setInitialCount(MAX_COINCYLINDER_NUM, ulInitialCount[MAX_COINCYLINDER_NUM]);
    m_Param.setCount(MAX_COINCYLINDER_NUM, ulCount[MAX_COINCYLINDER_NUM]);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount = ulCount[MAX_COINCYLINDER_NUM];
    m_Param.setRejectCount(MAX_COINCYLINDER_NUM, 0);
    m_Param.setMinimum(MAX_COINCYLINDER_NUM, ulMinimum[MAX_COINCYLINDER_NUM]);
    m_Param.setMaximum(MAX_COINCYLINDER_NUM, ulMaximum[MAX_COINCYLINDER_NUM]);
    m_Param.setPHMaximum(MAX_COINCYLINDER_NUM, ulPHMaximum[MAX_COINCYLINDER_NUM]);

    // save nonvolatile data.
    m_Param.SaveCashUnitInfo(SAVE_MODE_CDM_EXCHANGE);

    // 复位设备
    hRes = m_pDev->Reset();
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_RESET_FAIL);

        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }
    else {
        SetSPErrorInfo(SP_ERR_SUCCESS);
    }

    hRes = m_pDev->SetSupportCoinTypes(m_usCoinEnable);
    if (hRes != DEV_SUCCESS)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, __LINE__, "m_pDev->SetSupportCoinTypes() fail(%d)", hRes);
        return WFS_ERR_UNSUPP_DATA;
    }

    // Set cash unit info initialized flag.
    m_bCUInfoInitialized = TRUE;

    // Reset present/cashin status.
    m_wPresentState = WFS_CDM_UNKNOWN;
    m_wCashInStatus = WFS_CIM_CIUNKNOWN;

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    // Check cash unit info.
    CheckCashUnitThreshold(MONEY_COUNTERS_MONEYOUT);
    CheckCashUnitInfoChanged();
    BackupCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMOpenSafeDoor()
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CDMReset(const LPWFSCDMITEMPOSITION lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpResetIn)

    // check cash in status.
    VERIFY_NOT_IN_EXCHANGE(TRUE);
    VERIFY_NOT_IN_CASH_IN(TRUE);

    // clear exchange status.
    m_bCDMExchangeActive = FALSE;
    m_bCIMExchangeActive = FALSE;

    // Reset present/cashin status.
    m_wPresentState = WFS_CDM_UNKNOWN;
    m_wCashInStatus = WFS_CIM_CIUNKNOWN;

    HRESULT hRes;
    hRes = m_pDev->Reset();
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_RESET_FAIL);

        // 复位失败
        Log(ThisModule, __LINE__, "设备复位失败！");
        return WFS_ERR_HARDWARE_ERROR;
    }
    else
    {
        SetSPErrorInfo(SP_ERR_SUCCESS);

        // 如果钞箱信息初始化失败再次执行初始化处理。
        if (m_bCUInfoInitialized == FALSE)
        {
            // 比对并初始化钞箱信息
            hRes = InitializeCashUnitInfo();
            if (hRes != WFS_SUCCESS)
            {
                m_bCUInfoInitialized = FALSE;
                Log(ThisModule, __LINE__, "初始化钞箱信息失败！");
            }
            else {
                m_bCUInfoInitialized = TRUE;
            }
        }
    }

    // initialize device status.
    UpdateStatus(ThisModule);

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMSetCashUnitInfo(const LPWFSCDMCUINFO lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check exchange status.
    VERIFY_DEVICE_OPENED();
    VERIFY_NOT_IN_EXCHANGE(TRUE);

    HRESULT hRes;
    LPWFSCDMCASHUNIT lpWFSCDMCashUnit = NULL;
    LPWFSCDMPHCU lpWFSCDMPHCU = NULL;
    LPWFSCIMNOTENUMBERLIST lpLocalNoteNumberList = NULL;

    ULONG ulInitialCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulRejectCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMinimum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMaximum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulPHMaximum[MAX_COINCYLINDER_NUM + 1];

    if (lpCUInfo == NULL)
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "in parameter is null");
        return WFS_ERR_INVALID_POINTER;
    }

    // check cash unit counts.
    if (lpCUInfo->usCount != (MAX_COINCYLINDER_NUM + 1))
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "invalid lpCUInfo->usCount(%d)",
            lpCUInfo->usCount);
        return WFS_ERR_UNSUPP_DATA;
    }

    // update recycling cash unit.
    memset(ulInitialCount, 0, sizeof(ulInitialCount));
    memset(ulCount, 0, sizeof(ulCount));
    memset(ulRejectCount, 0, sizeof(ulRejectCount));
    memset(ulMinimum, 0, sizeof(ulMinimum));
    memset(ulMaximum, 0, sizeof(ulMaximum));
    memset(ulPHMaximum, 0, sizeof(ulPHMaximum));

    for (int i = 0; i < lpCUInfo->usCount; i++)
    {
        lpWFSCDMCashUnit = lpCUInfo->lppList[i];

        // check usType(Recycling(1-6) / Last Cash Box for Retract)
        if (((i < (lpCUInfo->usCount - 1)) && (lpWFSCDMCashUnit->usType != WFS_CDM_TYPERECYCLING)) ||
                ((i == (lpCUInfo->usCount - 1)) && (lpWFSCDMCashUnit->usType != WFS_CDM_TYPERETRACTCASSETTE)))
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usType(%d) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->usType,
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCDMCashUnit->ulMaximum < lpWFSCDMCashUnit->ulMinimum)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid ulMinimum/ulMaximum(%d/%d) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->ulMinimum,
                lpWFSCDMCashUnit->ulMaximum,
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCDMCashUnit->usNumPhysicalCUs != 1)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usNumPhysicalCUs(%d) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->usNumPhysicalCUs,
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCDMCashUnit->lppPhysical == NULL)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid lppPhysical(NULL) for cash unit(usNumber:%d)",
                lpWFSCDMCashUnit->usNumber);
            return WFS_ERR_INVALID_POINTER;
        }

        // Retrieve count relative info.
        lpWFSCDMPHCU = lpWFSCDMCashUnit->lppPhysical[0];
        if ((lpWFSCDMPHCU->ulCount == 0) &&
                (lpWFSCDMPHCU->ulRejectCount == 0))
        {
            ulInitialCount[i] = lpWFSCDMCashUnit->ulInitialCount;
            ulCount[i] = lpWFSCDMCashUnit->ulCount;
            ulRejectCount[i] = lpWFSCDMCashUnit->ulRejectCount;
        }
        else
        {
            ulInitialCount[i] = lpWFSCDMPHCU->ulInitialCount;
            ulCount[i] = lpWFSCDMPHCU->ulCount;
            ulRejectCount[i] = lpWFSCDMPHCU->ulRejectCount;
        }
        ulMinimum[i] = lpWFSCDMCashUnit->ulMinimum;
        ulMaximum[i] = lpWFSCDMCashUnit->ulMaximum;
        ulPHMaximum[i] = lpWFSCDMPHCU->ulMaximum;

        // Update note id specific count for CIM CashUnitInfo.
        lpLocalNoteNumberList = m_Param.getNoteNumberList(i);

        //NOTE! only clear note id specific count.
        if ((lpLocalNoteNumberList->usNumOfNoteNumbers > 0) && (ulCount[i] == 0))
        {
            for (int j = 0; j < lpLocalNoteNumberList->usNumOfNoteNumbers; j++) {
                lpLocalNoteNumberList->lppNoteNumber[j]->ulCount = 0;
            }
        }
    }

    // update cash unit info(recycling).
    ST_COINCYLINDER_INFO stCyliInfo;
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        stCyliInfo = m_stCoinCylinderInfos[i];
        stCyliInfo.lCount = ulCount[i];

        // update device balance.
        hRes = m_pDev->SetCoinCylinderInfo(stCyliInfo);
        if (hRes == SOFT_ERROR_PARAMS)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            return WFS_ERR_INVALID_DATA;
        }
        else if (hRes != DEV_SUCCESS)
        {
            SetSPErrorInfo(SP_ERR_DEV_ERROR);
            return WFS_ERR_HARDWARE_ERROR;
        }

        // update config info(local nonvolatile data).
        m_Param.setInitialCount(i, ulInitialCount[i]);
        m_Param.setCount(i, ulCount[i]);
        m_Param.setRejectCount(i, ulRejectCount[i]);
        m_stCoinCylinderInfos[i].lCount = ulCount[i];
        m_Param.setMinimum(i, ulMinimum[i]);
        m_Param.setMaximum(i, ulMaximum[i]);
        m_Param.setPHMaximum(i, ulPHMaximum[i]);
    }

    // update AB cash unit.
    m_Param.setInitialCount(MAX_COINCYLINDER_NUM, ulInitialCount[MAX_COINCYLINDER_NUM]);
    m_Param.setCount(MAX_COINCYLINDER_NUM, ulCount[MAX_COINCYLINDER_NUM]);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount = ulCount[MAX_COINCYLINDER_NUM];
    m_Param.setRejectCount(MAX_COINCYLINDER_NUM, 0);
    m_Param.setMinimum(MAX_COINCYLINDER_NUM, ulMinimum[MAX_COINCYLINDER_NUM]);
    m_Param.setMaximum(MAX_COINCYLINDER_NUM, ulMaximum[MAX_COINCYLINDER_NUM]);
    m_Param.setPHMaximum(MAX_COINCYLINDER_NUM, ulPHMaximum[MAX_COINCYLINDER_NUM]);

    // save nonvolatile data.
    m_Param.SaveCashUnitInfo(SAVE_MODE_CDM_COUNT);

    // 复位设备
    hRes = m_pDev->Reset();
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_RESET_FAIL);

        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }
    else {
        SetSPErrorInfo(SP_ERR_SUCCESS);
    }

    // Set cash unit info initialized flag.
    m_bCUInfoInitialized = TRUE;

    // Reset present/cashin status.
    m_wPresentState = WFS_CDM_UNKNOWN;
    m_wCashInStatus = WFS_CIM_CIUNKNOWN;

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    // Check cash unit info.
    CheckCashUnitThreshold(MONEY_COUNTERS_MONEYOUT);
    CheckCashUnitInfoChanged();
    BackupCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CDMTestCashUnits(LPWFSCDMITEMPOSITION lpPosition, LPWFSCDMCUINFO &lpCUInfo)
{
    Q_UNUSED(lpPosition)
    Q_UNUSED(lpCUInfo)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CDMSetGuidanceLight(LPWFSCDMSETGUIDLIGHT lpSetGuidLight)
{
    Q_UNUSED(lpSetGuidLight)
    return WFS_ERR_UNSUPP_COMMAND;
}

// CIM类型接口
// 查询命令
HRESULT CXFS_COR::CIMGetStatus(LPWFSCIMSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);

    UpdateStatus(ThisModule);

    memset(&m_CIMStatus, 0, sizeof(m_CIMStatus));
    m_CIMStatus.fwDevice = m_stCorStatus.fwDevice;
    if (m_bCUInfoInitialized == FALSE)
    {
        m_CIMStatus.fwDevice = WFS_CIM_DEVHWERROR;
    }
    if ((m_CIMStatus.fwDevice != WFS_CIM_DEVHWERROR) && m_bInCmdExecute)
    {
        m_CIMStatus.fwDevice = WFS_CIM_DEVBUSY;
    }

    m_CIMStatus.fwSafeDoor = m_stCorStatus.fwSafeDoor;
    m_CIMStatus.fwAcceptor= m_stCorStatus.fwAcceptor;
    m_CIMStatus.fwIntermediateStacker = m_stCorStatus.fwIntermediateStacker;
    m_CIMStatus.fwStackerItems = m_stCorStatus.fwStackerItems;
    m_CIMStatus.fwBanknoteReader = m_stCorStatus.fwBanknoteReader;
    m_CIMStatus.bDropBox = m_stCorStatus.bDropBox;

    memset(m_CIMInPosArray, 0, sizeof(m_CIMInPosArray));
    memset(&m_CIMInPos, 0, sizeof(m_CIMInPos));
    m_CIMInPos.fwPosition = m_stCorStatus.fwPosition;
    m_CIMInPos.fwShutter = m_stCorStatus.fwShutter;
    m_CIMInPos.fwPositionStatus = m_stCorStatus.fwPositionStatus;
    m_CIMInPos.fwTransport = m_stCorStatus.fwTransport;
    m_CIMInPos.fwTransportStatus = m_stCorStatus.fwTransportStatus;
    m_CIMInPosArray[0] = &m_CIMInPos;
    m_CIMStatus.lppPositions = m_CIMInPosArray;

    // Extra info.
    EditStatusExtraInfo();
    m_CIMStatus.lpszExtra = m_cStatusExtraInfo;

    // assign output parameter.
    lpStatus = &m_CIMStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMGetCapabilities(LPWFSCIMCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRes;
    ST_COR_CAPS stCorCaps;
    hRes = m_pDev->GetDeviceCaps(&stCorCaps);
    if (hRes != 0)
    {
        lpCaps = NULL;
        Log(ThisModule, __LINE__, "GetDeviceCaps fail(%d)", hRes);
        return WFS_ERR_HARDWARE_ERROR;
    }

    ST_CIMCAPS* pstCorCaps = (ST_CIMCAPS*)&stCorCaps;

    memset(&m_CIMCaps, 0, sizeof(m_CIMCaps));
    m_CIMCaps.wClass = WFS_SERVICE_CLASS_CIM;
    m_CIMCaps.fwType = WFS_CIM_SELFSERVICECOIN;
    m_CIMCaps.wMaxCashInItems = pstCorCaps->wMaxCashInItems;
    m_CIMCaps.bCompound = TRUE;
    m_CIMCaps.bShutter = pstCorCaps->bShutter;
    m_CIMCaps.bShutterControl = pstCorCaps->bShutterControl;
    m_CIMCaps.bSafeDoor = pstCorCaps->bSafeDoor;
    m_CIMCaps.bCashBox = FALSE;
    m_CIMCaps.fwIntermediateStacker = pstCorCaps->fwIntermediateStacker;
    m_CIMCaps.bItemsTakenSensor = pstCorCaps->bItemsTakenSensor;
    m_CIMCaps.bItemsInsertedSensor = pstCorCaps->bItemsInsertedSensor;
    m_CIMCaps.fwPositions = pstCorCaps->fwPositions;
    m_CIMCaps.fwExchangeType = WFS_CDM_EXBYHAND;
    m_CIMCaps.fwRetractAreas = pstCorCaps->fwRetractAreas;
    m_CIMCaps.fwRetractTransportActions = pstCorCaps->fwRetractTransportActions;
    m_CIMCaps.fwRetractStackerActions = pstCorCaps->fwRetractStackerActions;
    m_CIMCaps.lpszExtra = m_cNullExtra;

    // assign output parameter.
    lpCaps = &m_CIMCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMGetCashUnitInfo(LPWFSCIMCASHINFO &lpCUInfo)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRes;
    hRes = EditCIMCashUnitInfo();

    // assign output parameter.
    lpCUInfo = &m_CIMCashInfo;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMGetBankNoteType(LPWFSCIMNOTETYPELIST &lpNoteList)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpNoteList = m_Param.GetNoteTypes();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMGetCashInStatus(LPWFSCIMCASHINSTATUS &lpCashInStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    memset(&m_CIMCashInStatus, 0, sizeof(m_CIMCashInStatus));
    m_CIMCashInStatus.wStatus = m_wCashInStatus;
    m_CIMCashInStatus.usNumOfRefused = 0;
    m_CIMCashInStatus.lpszExtra = m_cNullExtra;
    if ((m_wCashInStatus == WFS_CIM_CIOK) ||
        (m_wCashInStatus == WFS_CIM_CIACTIVE))
    {
        LPWFSCIMNOTENUMBER lpNoteNumber = NULL;
        USHORT usNoteID;
        ULONG ulNoteIDSpecCount[MAX_SUPP_COIN_TYPE_NUM];
        memset(ulNoteIDSpecCount, 0, sizeof(ulNoteIDSpecCount));
        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            // Aggregate the cash in count.
            switch (m_stCoinCylinderInfos[i].iCoinCode)
            {
            case COIN_CODE_CN010C:
            {
                ulNoteIDSpecCount[2] = m_ulCashInCount[i];
            }
                break;
            case COIN_CODE_CN050C:
            {
                ulNoteIDSpecCount[1] = m_ulCashInCount[i];
            }
                break;
            case COIN_CODE_CN100B:
            {
                ulNoteIDSpecCount[0] = m_ulCashInCount[i];
            }
                break;
            default:
                break;
            }
        }

        // Aggregate the retract bin coin count.
        ulNoteIDSpecCount[2] += m_stRetractBin.ulCN010;
        ulNoteIDSpecCount[1] += m_stRetractBin.ulCN050;
        ulNoteIDSpecCount[0] += m_stRetractBin.ulCN100;

        memset(&m_cashinStatusNoteNumberList, 0, sizeof(m_cashinStatusNoteNumberList));
        memset(m_cashinStatusNoteNumberPtrArray, 0, sizeof(m_cashinStatusNoteNumberPtrArray));
        memset(m_cashinStatusNoteNumberArray, 0, sizeof(m_cashinStatusNoteNumberArray));
        m_cashinStatusNoteNumberList.lppNoteNumber = m_cashinStatusNoteNumberPtrArray;
        for (int i = 0; i < MAX_SUPP_COIN_TYPE_NUM; i++)
        {
            if (ulNoteIDSpecCount[i] == 0)
            {
                continue;
            }
            usNoteID = i + 1;
            lpNoteNumber = &m_cashinStatusNoteNumberArray[m_cashinStatusNoteNumberList.usNumOfNoteNumbers];
            m_cashinStatusNoteNumberPtrArray[m_cashinStatusNoteNumberList.usNumOfNoteNumbers] = lpNoteNumber;
            lpNoteNumber->usNoteID = usNoteID;
            lpNoteNumber->ulCount = ulNoteIDSpecCount[i];
            m_cashinStatusNoteNumberList.usNumOfNoteNumbers++;
        }

        // details of inserted coins.
        m_CIMCashInStatus.lpNoteNumberList = &m_cashinStatusNoteNumberList;
    }

    // assign output parameter.
    lpCashInStatus = &m_CIMCashInStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMGetCurrencyEXP(LPWFSCIMCURRENCYEXP *&lppCurrencyExp)
{
    THISMODULE(__FUNCTION__);
    lppCurrencyExp = (LPWFSCIMCURRENCYEXP *)m_Param.GetCurrencyExp();
    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMCashInStart(const LPWFSCIMCASHINSTART lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpData)

    // check Exchange/Cash in status.
    VERIFY_DEVICE_OPENED();
    VERIFY_CUINFO_INITIALIZED();
    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_NOT_IN_CASH_IN(FALSE);

    // set cash in flag.
    m_bCashInActive = TRUE;
    m_wCashInStatus = WFS_CIM_CIACTIVE;
    memset(m_ulCashInCount, 0, sizeof(m_ulCashInCount));
    memset(&m_stRetractBin, 0, sizeof(m_stRetractBin));

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMCashIn(LPWFSCIMNOTENUMBERLIST &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check Exchange/Cash in status.
    VERIFY_DEVICE_OPENED();
    VERIFY_CUINFO_INITIALIZED();
    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_IN_CASH_IN();

    // set cash in flag.
    m_bCanceled = FALSE;
    m_bCashInActive = TRUE;
    DWORD dwCashInTimeOut = m_Param.GetCoinInTimeOut() * 1000;
    ULONG ulNoteIDSpecCount[MAX_SUPP_COIN_TYPE_NUM];
    LPWFSCIMNOTENUMBER lpNoteNumber;
    LPWFSCIMNOTENUMBERLIST lpLocalNoteNumberList = NULL;
    USHORT usNoteID;

    // record count before cash in.
    m_Param.LogCashUnitCount(ACTION_CASHIN, TIMING_BEFORE);

    HRESULT hRes;
    ULONG ulSumCount;
    ULONG ulRecvItems[MAX_COINCYLINDER_NUM];
    ST_RETRACTBIN_COUNT stRetractBin;
    DEV_CUERROR eCUErrors[MAX_COINCYLINDER_NUM];
    memset(ulRecvItems, 0, sizeof(ulRecvItems));
    memset(&stRetractBin, 0, sizeof(stRetractBin));
    memset(eCUErrors, 0, sizeof(eCUErrors));
    hRes = m_pDev->CashIn(dwCashInTimeOut, ulRecvItems, stRetractBin, eCUErrors, NULL);
    if (m_bCanceled)
    {
        m_bCanceled = FALSE;
        hRes = WFS_ERR_CANCELED;
    }
    else if (hRes != DEV_SUCCESS)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        hRes = WFS_ERR_HARDWARE_ERROR;
    }
    else
    {
        hRes = WFS_SUCCESS;
    }

    memset(ulNoteIDSpecCount, 0, sizeof(ulNoteIDSpecCount));
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        m_Param.addInitialCount(i, ulRecvItems[i]);
        m_Param.addCount(i, ulRecvItems[i]);
        m_Param.addCashInCount(i, ulRecvItems[i]);
        m_stCoinCylinderInfos[i].lCount += ulRecvItems[i];

        // Update note id specific count.
        lpLocalNoteNumberList = m_Param.getNoteNumberList(i);
        if (lpLocalNoteNumberList->usNumOfNoteNumbers > 0)
        {
            lpLocalNoteNumberList->lppNoteNumber[0]->ulCount += ulRecvItems[i];
        }

        // Aggregate the cash in count.
        m_ulCashInCount[i] += ulRecvItems[i];
        switch (m_stCoinCylinderInfos[i].iCoinCode)
        {
        case COIN_CODE_CN010C:
        {
            ulNoteIDSpecCount[2] += ulRecvItems[i];
        }
            break;
        case COIN_CODE_CN050C:
        {
            ulNoteIDSpecCount[1] += ulRecvItems[i];
        }
            break;
        case COIN_CODE_CN100B:
        {
            ulNoteIDSpecCount[0] += ulRecvItems[i];
        }
            break;
        default:
            break;
        }
    }

    // Aggregate the retract bin coin count.
    ulSumCount = stRetractBin.ulCN010 + stRetractBin.ulCN050 + stRetractBin.ulCN100;
    m_Param.addCount(MAX_COINCYLINDER_NUM, ulSumCount);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount += ulSumCount;

    // Update note id specific count.
    lpLocalNoteNumberList = m_Param.getNoteNumberList(MAX_COINCYLINDER_NUM);
    if (lpLocalNoteNumberList->usNumOfNoteNumbers > 0)
    {
        for (int i = 0; i < lpLocalNoteNumberList->usNumOfNoteNumbers; i++)
        {
            lpNoteNumber = lpLocalNoteNumberList->lppNoteNumber[i];
            if (lpNoteNumber->usNoteID == 1)
            {
                lpNoteNumber->ulCount += stRetractBin.ulCN100;
            }
            else if (lpNoteNumber->usNoteID == 2)
            {
                lpNoteNumber->ulCount += stRetractBin.ulCN050;
            }
            else if (lpNoteNumber->usNoteID == 3)
            {
                lpNoteNumber->ulCount += stRetractBin.ulCN010;
            }
        }
    }

    // Aggregate the retract bin cash in count.
    m_stRetractBin.ulCN010 += stRetractBin.ulCN010;
    m_stRetractBin.ulCN050 += stRetractBin.ulCN050;
    m_stRetractBin.ulCN100 += stRetractBin.ulCN100;

    ulNoteIDSpecCount[2] += stRetractBin.ulCN010;
    ulNoteIDSpecCount[1] += stRetractBin.ulCN050;
    ulNoteIDSpecCount[0] += stRetractBin.ulCN100;

    memset(&m_cashinCIMNoteNumberList, 0, sizeof(m_cashinCIMNoteNumberList));
    memset(m_cashinCIMNoteNumberPtrArray, 0, sizeof(m_cashinCIMNoteNumberPtrArray));
    memset(m_cashinCIMNoteNumberArray, 0, sizeof(m_cashinCIMNoteNumberArray));
    m_cashinCIMNoteNumberList.lppNoteNumber = m_cashinCIMNoteNumberPtrArray;
    for (int i = 0; i < MAX_SUPP_COIN_TYPE_NUM; i++)
    {
        if (ulNoteIDSpecCount[i] == 0)
        {
            continue;
        }
        usNoteID = i + 1;
        lpNoteNumber = &m_cashinCIMNoteNumberArray[m_cashinCIMNoteNumberList.usNumOfNoteNumbers];
        m_cashinCIMNoteNumberPtrArray[m_cashinCIMNoteNumberList.usNumOfNoteNumbers] = lpNoteNumber;
        lpNoteNumber->usNoteID = usNoteID;
        lpNoteNumber->ulCount = ulNoteIDSpecCount[i];
        m_cashinCIMNoteNumberList.usNumOfNoteNumbers++;
    }

    // record count after cash in.
    m_Param.LogCashUnitCount(ACTION_CASHIN, TIMING_AFTER);

    // save count to configuration file.
    m_Param.SaveCashUnitInfo(SAVE_MODE_CIM_CASHIN);

    // assign output parameter(return NULL if no valid items)
    if (m_cashinCIMNoteNumberList.usNumOfNoteNumbers == 0)
    {
        hRes = WFS_ERR_CIM_NOITEMS;
        lpResult = NULL;
    }
    else
    {
        lpResult = &m_cashinCIMNoteNumberList;
    }

    //return WFS_SUCCESS;
    return hRes;
}

HRESULT CXFS_COR::CIMCashInEnd(LPWFSCIMCASHINFO &lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check Exchange/Cash in status.
    VERIFY_DEVICE_OPENED();
    VERIFY_CUINFO_INITIALIZED();
    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_IN_CASH_IN();

    // set cash in flag.
    m_bCashInActive = FALSE;
    m_wCashInStatus = WFS_CIM_CIOK;
    memset(&m_endCIMCashInfo, 0, sizeof(m_endCIMCashInfo));
    memset(m_endCIMCashInPtrList, 0, sizeof(m_endCIMCashInPtrList));
    memset(m_endCIMCashInArray, 0, sizeof(m_endCIMCashInArray));
    memset(m_endCIMPHCashUnitPtrList, 0, sizeof(m_endCIMPHCashUnitPtrList));
    memset(m_endCIMPHCashUnitArray, 0, sizeof(m_endCIMPHCashUnitArray));
    memset(m_endCIMNoteNumberListArray, 0, sizeof(m_endCIMNoteNumberListArray));
    memset(m_endCIMNoteNumberPtrArrayArray, 0, sizeof(m_endCIMNoteNumberPtrArrayArray));
    memset(m_endCIMNoteNumberArrayArray, 0, sizeof(m_endCIMNoteNumberArrayArray));

    LPWFSCIMCASHIN lpCashIn;
    LPWFSCIMPHCU lpPhCU;
    LPWFSCIMNOTENUMBER lpNoteNumber;
    m_endCIMCashInfo.usCount = MAX_COINCYLINDER_NUM + 1;
    m_endCIMCashInfo.lppCashIn = m_endCIMCashInPtrList;
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        lpCashIn = &m_endCIMCashInArray[i];
        lpPhCU = &m_endCIMPHCashUnitArray[i];
        m_endCIMCashInfo.lppCashIn[i] = lpCashIn;
        lpCashIn->usNumber = i + 1;
        lpCashIn->fwType = WFS_CIM_TYPERECYCLING;
        lpCashIn->fwItemType = WFS_CIM_CITYPINDIVIDUAL;
        strcpy(lpCashIn->cUnitID, m_stCoinCylinderInfos[i].szId);
        strcpy(lpCashIn->cCurrencyID, "CNY");
        lpCashIn->ulValues = m_stCoinCylinderInfos[i].iCashValue;
        lpCashIn->ulCashInCount = m_ulCashInCount[i];
        lpCashIn->ulCount = m_ulCashInCount[i];
        lpCashIn->ulMaximum = m_Param.getMaximum(i);
        lpCashIn->usStatus = WFS_CIM_STATCUOK;
        lpCashIn->bAppLock = FALSE;
        lpCashIn->lpNoteNumberList = &m_endCIMNoteNumberListArray[i];
        lpCashIn->lpNoteNumberList->usNumOfNoteNumbers = 1;
        lpCashIn->lpNoteNumberList->lppNoteNumber = m_endCIMNoteNumberPtrArrayArray[i];
        lpNoteNumber = &m_endCIMNoteNumberArrayArray[i][0];
        lpCashIn->lpNoteNumberList->lppNoteNumber[0] = lpNoteNumber;
        switch (m_stCoinCylinderInfos[i].iCoinCode)
        {
        case COIN_CODE_CN010C:
        {
            lpNoteNumber->usNoteID = 3;
        }
            break;
        case COIN_CODE_CN050C:
        {
            lpNoteNumber->usNoteID = 2;
        }
            break;
        case COIN_CODE_CN100B:
        {
            lpNoteNumber->usNoteID = 1;
        }
            break;
        default:
            break;
        }
        lpNoteNumber->ulCount = m_ulCashInCount[i];

        // Physical cash unit info.
        lpCashIn->usNumPhysicalCUs = 1;
        lpCashIn->lppPhysical = &m_endCIMPHCashUnitPtrList[i];
        lpCashIn->lppPhysical[0] = lpPhCU;
        lpPhCU->lpPhysicalPositionName = m_cPHPositionNameArray[i];
        strcpy(lpPhCU->cUnitID, m_stCoinCylinderInfos[i].szId);
        lpPhCU->ulCashInCount = m_ulCashInCount[i];
        lpPhCU->ulCount = m_ulCashInCount[i];
        lpPhCU->ulMaximum = m_Param.getPHMaximum(i);
        lpPhCU->usPStatus = WFS_CIM_STATCUOK;
        lpPhCU->bHardwareSensors = FALSE;
        lpPhCU->lpszExtra = m_cNullExtra;

        // nothing but two null characters.
        lpCashIn->lpszExtra = m_cNullExtra;
    }

    // Retract cash unit.
    ULONG ulCashInCount = m_stRetractBin.ulCN010 + m_stRetractBin.ulCN050 + m_stRetractBin.ulCN100;
    lpCashIn = &m_endCIMCashInArray[MAX_COINCYLINDER_NUM];
    lpPhCU = &m_endCIMPHCashUnitArray[MAX_COINCYLINDER_NUM];
    m_endCIMCashInfo.lppCashIn[MAX_COINCYLINDER_NUM] = lpCashIn;
    lpCashIn->usNumber = MAX_COINCYLINDER_NUM + 1;
    lpCashIn->fwType = WFS_CIM_TYPERETRACTCASSETTE;
    lpCashIn->fwItemType = WFS_CIM_CITYPALL;
    strcpy(lpCashIn->cUnitID, m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].szId);
    strcpy(lpCashIn->cCurrencyID, "CNY");
    lpCashIn->ulValues = m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iCashValue;
    lpCashIn->ulCashInCount = ulCashInCount;
    lpCashIn->ulCount = ulCashInCount;
    lpCashIn->ulMaximum = m_Param.getMaximum(MAX_COINCYLINDER_NUM);
    lpCashIn->usStatus = WFS_CIM_STATCUOK;
    lpCashIn->bAppLock = FALSE;
    lpCashIn->lpNoteNumberList = &m_endCIMNoteNumberListArray[MAX_COINCYLINDER_NUM];
    lpCashIn->lpNoteNumberList->usNumOfNoteNumbers = 3;
    lpCashIn->lpNoteNumberList->lppNoteNumber = m_endCIMNoteNumberPtrArrayArray[MAX_COINCYLINDER_NUM];
    lpNoteNumber = &m_endCIMNoteNumberArrayArray[MAX_COINCYLINDER_NUM][0];
    lpCashIn->lpNoteNumberList->lppNoteNumber[0] = lpNoteNumber;
    lpNoteNumber->usNoteID = 1;
    lpNoteNumber->ulCount = m_stRetractBin.ulCN100;
    lpNoteNumber = &m_endCIMNoteNumberArrayArray[MAX_COINCYLINDER_NUM][1];
    lpCashIn->lpNoteNumberList->lppNoteNumber[1] = lpNoteNumber;
    lpNoteNumber->usNoteID = 2;
    lpNoteNumber->ulCount = m_stRetractBin.ulCN050;
    lpNoteNumber = &m_endCIMNoteNumberArrayArray[MAX_COINCYLINDER_NUM][2];
    lpCashIn->lpNoteNumberList->lppNoteNumber[2] = lpNoteNumber;
    lpNoteNumber->usNoteID = 3;
    lpNoteNumber->ulCount = m_stRetractBin.ulCN010;

    // Physical cash unit info.
    lpCashIn->usNumPhysicalCUs = 1;
    lpCashIn->lppPhysical = &m_endCIMPHCashUnitPtrList[MAX_COINCYLINDER_NUM];
    lpCashIn->lppPhysical[0] = lpPhCU;
    lpPhCU->lpPhysicalPositionName = m_cPHPositionNameArray[MAX_COINCYLINDER_NUM];
    strcpy(lpPhCU->cUnitID, m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].szId);
    lpPhCU->ulCashInCount = ulCashInCount;
    lpPhCU->ulCount = ulCashInCount;
    lpPhCU->ulMaximum = m_Param.getPHMaximum(MAX_COINCYLINDER_NUM);
    lpPhCU->usPStatus = WFS_CIM_STATCUOK;
    lpPhCU->bHardwareSensors = FALSE;
    lpPhCU->lpszExtra = m_cNullExtra;

    // nothing but two null characters.
    lpCashIn->lpszExtra = m_cNullExtra;

    // assign output parameter.
    lpCUInfo = &m_endCIMCashInfo;

    // Update cash unit status.
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        CheckCashUnitThreshold(i, m_stCoinCylinderInfos[i], MONEY_COUNTERS_MONEYIN);
    }

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    // Check cash unit info.
    CheckCashUnitThreshold(MONEY_COUNTERS_MONEYIN);
    CheckCashUnitInfoChanged();
    BackupCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMCashInRollBack(LPWFSCIMCASHINFO &lpCUInfo)
{
    Q_UNUSED(lpCUInfo)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CIMRetract(const LPWFSCIMRETRACT lpData)
{
    Q_UNUSED(lpData)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CIMOpenShutter(WORD wPos)
{
    Q_UNUSED(wPos)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CIMCloseShutter(WORD wPos)
{
    Q_UNUSED(wPos)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CIMStartEXChange(const LPWFSCIMSTARTEX lpStartEx, LPWFSCIMCASHINFO &lpCUInfor)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check exchange status.
    VERIFY_DEVICE_OPENED();
    VERIFY_NOT_IN_EXCHANGE(FALSE);

    HRESULT hRes;

    if (lpStartEx->fwExchangeType != WFS_CIM_EXBYHAND)
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "invalid fwExchangeType(%d)",
            lpStartEx->fwExchangeType);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (m_Param.GetStartExchangeIgnoreCUNumberList() == 0)
    {
        if (lpStartEx->lpusCUNumList == NULL)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid lpusCUNumList(NULL)");
            return WFS_ERR_INVALID_POINTER;
        }

        for (int i = 0; i < lpStartEx->usCount; i++)
        {
            if ((lpStartEx->lpusCUNumList[i] <= 0) ||
                    (lpStartEx->lpusCUNumList[i] > (MAX_COINCYLINDER_NUM +1)))
            {
                SetSPErrorInfo(SP_ERR_PARAM_ERROR);
                Log(ThisModule, -1,
                    "invalid logical number(%d)",
                    lpStartEx->lpusCUNumList[i]);
                return WFS_ERR_CDM_CASHUNITERROR;
            }
        }
    }

    // edit output parameter.
    hRes = EditCIMCashUnitInfo();
    lpCUInfor = &m_CIMCashInfo;

    // set exchange active flag.
    m_bCIMExchangeActive = TRUE;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMEndEXChange(const LPWFSCIMCASHINFO lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check exchange status.
    VERIFY_DEVICE_OPENED();
    VERIFY_IN_EXCHANGE(FALSE);

    HRESULT hRes;
    LPWFSCIMCASHIN lpWFSCIMCashIn = NULL;
    LPWFSCIMPHCU lpWFSCIMPHCU = NULL;
    LPWFSCIMNOTENUMBERLIST lpWFSNoteNumberList = NULL;
    LPWFSCIMNOTENUMBERLIST lpLocalNoteNumberList = NULL;
    LPWFSCIMNOTENUMBER lpWFSNoteNumber = NULL;

    ULONG ulCashInCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMinimum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMaximum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulPHMaximum[MAX_COINCYLINDER_NUM + 1];

    // clear exchange flag.
    m_bCIMExchangeActive = FALSE;

    if (lpCUInfo == NULL)
    {
        return WFS_SUCCESS;
    }

    // check cash unit counts.
    if (lpCUInfo->usCount != (MAX_COINCYLINDER_NUM + 1))
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "invalid lpCUInfo->usCount(%d)",
            lpCUInfo->usCount);
        return WFS_ERR_UNSUPP_DATA;
    }

    // update recycling cash unit.
    memset(ulCashInCount, 0, sizeof(ulCashInCount));
    memset(ulCount, 0, sizeof(ulCount));
    memset(ulMinimum, 0, sizeof(ulMinimum));
    memset(ulMaximum, 0, sizeof(ulMaximum));
    memset(ulPHMaximum, 0, sizeof(ulPHMaximum));

    for (int i = 0; i < lpCUInfo->usCount; i++)
    {
        lpWFSCIMCashIn = lpCUInfo->lppCashIn[i];

        // check usType(Recycling(1-6) / Last Cash Box for Retract)
        if (((i < (lpCUInfo->usCount - 1)) && (lpWFSCIMCashIn->fwType != WFS_CIM_TYPERECYCLING)) ||
                ((i == (lpCUInfo->usCount - 1)) && (lpWFSCIMCashIn->fwType != WFS_CIM_TYPERETRACTCASSETTE)))
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usType(%d) for cash unit(usNumber:%d)",
                lpWFSCIMCashIn->fwType,
                lpWFSCIMCashIn->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCIMCashIn->usNumPhysicalCUs != 1)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usNumPhysicalCUs(%d) for cash unit(usNumber:%d)",
                lpWFSCIMCashIn->usNumPhysicalCUs,
                lpWFSCIMCashIn->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCIMCashIn->lppPhysical == NULL)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid lppPhysical(NULL) for cash unit(usNumber:%d)",
                lpWFSCIMCashIn->usNumber);
            return WFS_ERR_INVALID_POINTER;
        }

        // Retrieve count relative info.
        lpWFSCIMPHCU = lpWFSCIMCashIn->lppPhysical[0];
        if ((lpWFSCIMPHCU->ulCount == 0) &&
                (lpWFSCIMPHCU->ulCashInCount == 0))
        {
            ulCount[i] = lpWFSCIMCashIn->ulCount;
            ulCashInCount[i] = lpWFSCIMCashIn->ulCashInCount;
        }
        else
        {
            ulCount[i] = lpWFSCIMCashIn->ulCount;
            ulCashInCount[i] = lpWFSCIMCashIn->ulCashInCount;
        }
        ulMaximum[i] = lpWFSCIMCashIn->ulMaximum;
        ulPHMaximum[i] = lpWFSCIMPHCU->ulMaximum;

        lpWFSNoteNumberList = lpWFSCIMCashIn->lpNoteNumberList;
        lpLocalNoteNumberList = m_Param.getNoteNumberList(i);

        //NOTE! only clear note id specific count.
        if ((lpLocalNoteNumberList->usNumOfNoteNumbers > 0) && (ulCount[i] == 0))
        {
            for (int j = 0; j < lpLocalNoteNumberList->usNumOfNoteNumbers; j++) {
                lpLocalNoteNumberList->lppNoteNumber[j]->ulCount = 0;
            }
        }
    }

    // Set device hopper no if failed last time.
    if ((m_bCUInfoInitialized == FALSE) && (m_stCoinCylinderInfos[0].iCylinderNO == 0))
    {
        ST_COINCYLINDER_INFO stCoinCylinderInfos[MAX_COINCYLINDER_NUM];
        hRes = m_pDev->GetCoinCylinderList(stCoinCylinderInfos);
        if (DEV_SUCCESS != hRes)
        {
            SetSPErrorInfo(SP_ERR_DEV_ERROR);
            Log(ThisModule, -1,
                "GetCoinCylinderList() failed(%ld)",
                hRes);
            return WFS_ERR_HARDWARE_ERROR;
        }

        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            m_stCoinCylinderInfos[i].iCylinderNO = stCoinCylinderInfos[i].iCylinderNO;
        }
    }

    // update cash unit info(recycling).
    ST_COINCYLINDER_INFO stCyliInfo;
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        stCyliInfo = m_stCoinCylinderInfos[i];
        stCyliInfo.lCount = ulCount[i];

        // update device balance.
        hRes = m_pDev->SetCoinCylinderInfo(stCyliInfo);
        if (hRes == SOFT_ERROR_PARAMS)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            return WFS_ERR_INVALID_DATA;
        }
        else if (hRes == ERR_COR_CASHUNITERROR)
        {
            SetSPErrorInfo(SP_ERR_DEV_ERROR);
            Log(ThisModule, -1,
                "GetCoinCylinderInfo occur CU missing(usNumber:%d, iCylinderNO:%d)",
                i + 1,
                stCyliInfo.iCylinderNO);
            return WFS_ERR_HARDWARE_ERROR;
        }
        else if (hRes != DEV_SUCCESS)
        {
            SetSPErrorInfo(SP_ERR_DEV_ERROR);
            Log(ThisModule, -1,
                "SetCoinCylinderInfo fail(usNumber:%d, iCylinderNO:%d, hRes:%d)",
                i + 1,
                stCyliInfo.iCylinderNO, hRes);
            return WFS_ERR_HARDWARE_ERROR;
        }

        // update config info(local nonvolatile data).
        m_Param.setCashInCount(i, ulCashInCount[i]);
        m_Param.setCount(i, ulCount[i]);
        m_stCoinCylinderInfos[i].lCount = ulCount[i];
        m_Param.setMaximum(i, ulMaximum[i]);
        m_Param.setPHMaximum(i, ulPHMaximum[i]);
    }

    // update AB cash unit.
    m_Param.setCashInCount(MAX_COINCYLINDER_NUM, ulCashInCount[MAX_COINCYLINDER_NUM]);
    m_Param.setCount(MAX_COINCYLINDER_NUM, ulCount[MAX_COINCYLINDER_NUM]);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount = ulCount[MAX_COINCYLINDER_NUM];
    m_Param.setMaximum(MAX_COINCYLINDER_NUM, ulMaximum[MAX_COINCYLINDER_NUM]);
    m_Param.setPHMaximum(MAX_COINCYLINDER_NUM, ulPHMaximum[MAX_COINCYLINDER_NUM]);

    // save nonvolatile data.
    m_Param.SaveCashUnitInfo(SAVE_MODE_CIM_EXCHANGE);

    // 复位设备
    hRes = m_pDev->Reset();
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_RESET_FAIL);

        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }
    else {
        SetSPErrorInfo(SP_ERR_SUCCESS);
    }

    hRes = m_pDev->SetSupportCoinTypes(m_usCoinEnable);
    if (hRes != DEV_SUCCESS)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, __LINE__, "m_pDev->SetSupportCoinTypes() fail(%d)", hRes);
        return WFS_ERR_UNSUPP_DATA;
    }

    // Set cash unit info initialized flag.
    m_bCUInfoInitialized = TRUE;

    // Reset present/cashin status.
    m_wPresentState = WFS_CDM_UNKNOWN;
    m_wCashInStatus = WFS_CIM_CIUNKNOWN;

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    // Check cash unit info.
    CheckCashUnitThreshold(MONEY_COUNTERS_MONEYIN);
    CheckCashUnitInfoChanged();
    BackupCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMOpenSafeDoor()
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CIMReset(const LPWFSCIMITEMPOSITION lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpResetIn)

    // check cash in status.
    VERIFY_NOT_IN_EXCHANGE(FALSE);
    VERIFY_NOT_IN_CASH_IN(FALSE);

    // clear exchange status.
    m_bCDMExchangeActive = FALSE;
    m_bCIMExchangeActive = FALSE;

    // Reset present/cashin status.
    m_wPresentState = WFS_CDM_UNKNOWN;
    m_wCashInStatus = WFS_CIM_CIUNKNOWN;

    HRESULT hRes;
    hRes = m_pDev->Reset();
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_RESET_FAIL);

        // 复位失败
        Log(ThisModule, __LINE__, "设备复位失败！");
        return WFS_ERR_HARDWARE_ERROR;
    }
    else
    {
        SetSPErrorInfo(SP_ERR_SUCCESS);

        // 如果钞箱信息初始化失败再次执行初始化处理。
        if (m_bCUInfoInitialized == FALSE)
        {
            // 比对并初始化钞箱信息
            hRes = InitializeCashUnitInfo();
            if (hRes != WFS_SUCCESS)
            {
                m_bCUInfoInitialized = FALSE;
                Log(ThisModule, __LINE__, "初始化钞箱信息失败！");
            }
            else {
                m_bCUInfoInitialized = TRUE;
            }
        }
    }

    // initialize device status.
    UpdateStatus(ThisModule);

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMConfigureCashInUnits(const LPWFSCIMCASHINTYPE *lppCashInType)
{
    Q_UNUSED(lppCashInType)
    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMConfigureNoteTypes(const LPUSHORT lpusNoteIDs)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRes;

    // check exchange status.
    VERIFY_DEVICE_OPENED();
    VERIFY_NOT_IN_EXCHANGE(FALSE);

    if (lpusNoteIDs == NULL)
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, __LINE__, "lpusNoteIDs is null");
        return WFS_ERR_INVALID_POINTER;
    }

    LPUSHORT usNoteIDs = m_Param.getNoteIDs();
    LPUSHORT p = lpusNoteIDs;
    while (*p)
    {
        BOOL bFind = FALSE;
        for (int i = 0; i < MAX_SUPP_COIN_TYPE_NUM; i++)
        {
            if (usNoteIDs[i] == *p)
            {
                bFind = TRUE;
                break;
            }
        }
        if (bFind == FALSE)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, __LINE__, "invalid note id(%d)", *p);
            return WFS_ERR_UNSUPP_DATA;
        }

        p++;
    }

    // reset enable coins.
    m_usCoinEnable = 0;
    p = lpusNoteIDs;
    while (*p)
    {
        switch (*p)
        {
        case 1: //1Yuan
        {
            m_usCoinEnable |= COIN_CN100B | COIN_CN100C;
        }
            break;
        case 2: //5Jiao
        {
            m_usCoinEnable |= COIN_CN050B | COIN_CN050C | COIN_CN050D;
        }
            break;
        case 3: //1Jiao
        {
            m_usCoinEnable |= COIN_CN010B | COIN_CN010C | COIN_CN010D;
        }
            break;
        default:
            break;
        }

        p++;
    }

    hRes = m_pDev->SetSupportCoinTypes(m_usCoinEnable);
    if (hRes != DEV_SUCCESS)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, __LINE__, "m_pDev->SetSupportCoinTypes() fail(%d)", hRes);
        return WFS_ERR_UNSUPP_DATA;
    }

    // Save configured note ids.
    m_Param.ConfigureNoteTypes(lpusNoteIDs);

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMSetCashUnitInfo(const LPWFSCIMCASHINFO lpCUInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // check exchange status.
    VERIFY_DEVICE_OPENED();
    VERIFY_NOT_IN_EXCHANGE(FALSE);

    HRESULT hRes;
    LPWFSCIMCASHIN lpWFSCIMCashIn = NULL;
    LPWFSCIMPHCU lpWFSCIMPHCU = NULL;
    LPWFSCIMNOTENUMBERLIST lpWFSNoteNumberList = NULL;
    LPWFSCIMNOTENUMBERLIST lpLocalNoteNumberList = NULL;
    LPWFSCIMNOTENUMBER lpWFSNoteNumber = NULL;

    ULONG ulCashInCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulCount[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMinimum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulMaximum[MAX_COINCYLINDER_NUM + 1];
    ULONG ulPHMaximum[MAX_COINCYLINDER_NUM + 1];

    if (lpCUInfo == NULL)
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "in parameter is null");
        return WFS_ERR_INVALID_POINTER;
    }

    // check cash unit counts.
    if (lpCUInfo->usCount != (MAX_COINCYLINDER_NUM + 1))
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "invalid lpCUInfo->usCount(%d)",
            lpCUInfo->usCount);
        return WFS_ERR_UNSUPP_DATA;
    }

    // update recycling cash unit.
    memset(ulCashInCount, 0, sizeof(ulCashInCount));
    memset(ulCount, 0, sizeof(ulCount));
    memset(ulMinimum, 0, sizeof(ulMinimum));
    memset(ulMaximum, 0, sizeof(ulMaximum));
    memset(ulPHMaximum, 0, sizeof(ulPHMaximum));

    for (int i = 0; i < lpCUInfo->usCount; i++)
    {
        lpWFSCIMCashIn = lpCUInfo->lppCashIn[i];

        // check usType(Recycling(1-6) / Last Cash Box for Retract)
        if (((i < (lpCUInfo->usCount - 1)) && (lpWFSCIMCashIn->fwType != WFS_CIM_TYPERECYCLING)) ||
                ((i == (lpCUInfo->usCount - 1)) && (lpWFSCIMCashIn->fwType != WFS_CIM_TYPERETRACTCASSETTE)))
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usType(%d) for cash unit(usNumber:%d)",
                lpWFSCIMCashIn->fwType,
                lpWFSCIMCashIn->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCIMCashIn->usNumPhysicalCUs != 1)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usNumPhysicalCUs(%d) for cash unit(usNumber:%d)",
                lpWFSCIMCashIn->usNumPhysicalCUs,
                lpWFSCIMCashIn->usNumber);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpWFSCIMCashIn->lppPhysical == NULL)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid lppPhysical(NULL) for cash unit(usNumber:%d)",
                lpWFSCIMCashIn->usNumber);
            return WFS_ERR_INVALID_POINTER;
        }

        // Retrieve count relative info.
        lpWFSCIMPHCU = lpWFSCIMCashIn->lppPhysical[0];
        if ((lpWFSCIMPHCU->ulCount == 0) &&
                (lpWFSCIMPHCU->ulCashInCount == 0))
        {
            ulCount[i] = lpWFSCIMCashIn->ulCount;
            ulCashInCount[i] = lpWFSCIMCashIn->ulCashInCount;
        }
        else
        {
            ulCount[i] = lpWFSCIMCashIn->ulCount;
            ulCashInCount[i] = lpWFSCIMCashIn->ulCashInCount;
        }
        ulMaximum[i] = lpWFSCIMCashIn->ulMaximum;
        ulPHMaximum[i] = lpWFSCIMPHCU->ulMaximum;

        lpWFSNoteNumberList = lpWFSCIMCashIn->lpNoteNumberList;
        lpLocalNoteNumberList = m_Param.getNoteNumberList(i);

        //NOTE! only clear note id specific count.
        if ((lpLocalNoteNumberList->usNumOfNoteNumbers > 0) && (ulCount[i] == 0))
        {
            for (int j = 0; j < lpLocalNoteNumberList->usNumOfNoteNumbers; j++) {
                lpLocalNoteNumberList->lppNoteNumber[j]->ulCount = 0;
            }
        }
    }

    // update cash unit info(recycling).
    ST_COINCYLINDER_INFO stCyliInfo;
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        stCyliInfo = m_stCoinCylinderInfos[i];
        stCyliInfo.lCount = ulCount[i];

        // update device balance.
        hRes = m_pDev->SetCoinCylinderInfo(stCyliInfo);
        if (hRes == SOFT_ERROR_PARAMS)
        {
            return WFS_ERR_INVALID_DATA;
        }
        else if (hRes != DEV_SUCCESS)
        {
            return WFS_ERR_HARDWARE_ERROR;
        }

        // update config info(local nonvolatile data).
        m_Param.setCashInCount(i, ulCashInCount[i]);
        m_Param.setCount(i, ulCount[i]);
        m_stCoinCylinderInfos[i].lCount = ulCount[i];
        m_Param.setMaximum(i, ulMaximum[i]);
        m_Param.setPHMaximum(i, ulPHMaximum[i]);
    }

    // update AB cash unit.
    m_Param.setCashInCount(MAX_COINCYLINDER_NUM, ulCashInCount[MAX_COINCYLINDER_NUM]);
    m_Param.setCount(MAX_COINCYLINDER_NUM, ulCount[MAX_COINCYLINDER_NUM]);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount = ulCount[MAX_COINCYLINDER_NUM];
    m_Param.setMaximum(MAX_COINCYLINDER_NUM, ulMaximum[MAX_COINCYLINDER_NUM]);
    m_Param.setPHMaximum(MAX_COINCYLINDER_NUM, ulPHMaximum[MAX_COINCYLINDER_NUM]);

    // save nonvolatile data.
    m_Param.SaveCashUnitInfo(SAVE_MODE_CIM_EXCHANGE);

    // 复位设备
    hRes = m_pDev->Reset();
    if (hRes != 0)
    {
        SetSPErrorInfo(SP_ERR_RESET_FAIL);

        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }
    else {
        SetSPErrorInfo(SP_ERR_SUCCESS);
    }

    // Set cash unit info initialized flag.
    m_bCUInfoInitialized = TRUE;

    // Reset present/cashin status.
    m_wPresentState = WFS_CDM_UNKNOWN;
    m_wCashInStatus = WFS_CIM_CIUNKNOWN;

    EditCDMCashUnitInfo();
    EditCIMCashUnitInfo();

    // Check cash unit info.
    CheckCashUnitThreshold(MONEY_COUNTERS_MONEYIN);
    CheckCashUnitInfoChanged();
    BackupCashUnitInfo();

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::CIMSetGuidanceLight(const LPWFSCIMSETGUIDLIGHT lpSetGuidLight)
{
    Q_UNUSED(lpSetGuidLight)
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_COR::CIMCashInLimit(const LPWFSCIMCASHINLIMIT lpCUInfo)
{
    Q_UNUSED(lpCUInfo)
    return WFS_ERR_UNSUPP_COMMAND;
}

////////////////////////////////////////////////////////////////////////////////////////////////



HRESULT CXFS_COR::InitializeCashUnitInfo()
{
    THISMODULE(__FUNCTION__);
    HRESULT hRes;

    memset(m_stCoinCylinderInfos, 0, sizeof(m_stCoinCylinderInfos));
    string strUnitID;
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        m_stCoinCylinderInfos[i].iCylinderNO = 0;
        m_stCoinCylinderInfos[i].iType = COR_UT_TYPERECYCLING;
        strUnitID = m_Param.getUnitID(i);
        memcpy(m_stCoinCylinderInfos[i].szId, strUnitID.c_str(), strUnitID.length());
        m_stCoinCylinderInfos[i].iCashValue = m_Param.getCashValue(i);
        m_stCoinCylinderInfos[i].iCoinCode = (COIN_CODE)m_Param.getCoinCode(i);
        m_stCoinCylinderInfos[i].lCount = m_Param.getCount(i);
        m_stCoinCylinderInfos[i].iStatus = COR_UNIT_STATCUOK;

        m_ulCountBK[i] = m_Param.getCount(i);
        m_CUStatus[i] = COR_UNIT_STATCUOK;
    }

    // initialize retract cash unit.
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iCylinderNO = 0;
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iType = COR_UT_TYPERETRACTCASSETTE;
    strUnitID = m_Param.getUnitID(MAX_COINCYLINDER_NUM);
    memcpy(m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].szId, strUnitID.c_str(), strUnitID.length());
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount = m_Param.getCount(MAX_COINCYLINDER_NUM);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iCashValue = m_Param.getCashValue(MAX_COINCYLINDER_NUM);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iCoinCode = (COIN_CODE)m_Param.getCoinCode(MAX_COINCYLINDER_NUM);
    m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iStatus = COR_UNIT_STATCUOK;

    m_ulCountBK[MAX_COINCYLINDER_NUM] = m_Param.getCount(MAX_COINCYLINDER_NUM);
    m_CUStatus[MAX_COINCYLINDER_NUM] = COR_UNIT_STATCUOK;

    // evaluate support coins.
    LPWFSCIMNOTETYPELIST lpNoteTypeList = m_Param.GetNoteTypes();
    LPWFSCIMNOTETYPE lpNoteType = NULL;
    m_usCoinEnable = 0;
    for (int i = 0; i < lpNoteTypeList->usNumOfNoteTypes; i++)
    {
        lpNoteType = lpNoteTypeList->lppNoteTypes[i];
        if (lpNoteType->bConfigured == FALSE)
        {
            continue;
        }

        switch (lpNoteType->usNoteID)
        {
        case 1: //1元
        {
            m_usCoinEnable |= COIN_CN100B | COIN_CN100C;
        }
            break;
        case 2: //5角
        {
            m_usCoinEnable |= COIN_CN050B | COIN_CN050C | COIN_CN050D;
        }
            break;
        case 3: //1角
        {
            m_usCoinEnable |= COIN_CN010B | COIN_CN010C | COIN_CN010D;
        }
            break;
        default:
            break;
        }
    }

    ST_COINCYLINDER_INFO stCoinCylinderInfos[MAX_COINCYLINDER_NUM];
    hRes = m_pDev->GetCoinCylinderList(stCoinCylinderInfos);
    if (DEV_SUCCESS != hRes)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, -1,
            "GetCoinCylinderList() failed(%ld)",
            hRes);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // initialize the [hopper no.] for cash unit.
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        m_stCoinCylinderInfos[i].iCylinderNO = stCoinCylinderInfos[i].iCylinderNO;
    }

    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        if (stCoinCylinderInfos[i].lCount != m_stCoinCylinderInfos[i].lCount)
        {
            SetSPErrorInfo(SP_ERR_DATA_UNMATCH);
            Log(ThisModule, -1,
                "count does not match(index:%d, device:%ld, config:%ld)",
                i,
                stCoinCylinderInfos[i].lCount,
                m_Param.getCount(i));
            return WFS_ERR_INTERNAL_ERROR;
        }

        // copy to local variable.
        m_stCoinCylinderInfos[i].lCount = stCoinCylinderInfos[i].lCount;
        m_stCoinCylinderInfos[i].iStatus = stCoinCylinderInfos[i].iStatus;

        m_ulCountBK[i] = stCoinCylinderInfos[i].lCount;
        m_CUStatus[i] = stCoinCylinderInfos[i].iStatus;
    }

    hRes = m_pDev->SetSupportCoinTypes(m_usCoinEnable);
    if (hRes != DEV_SUCCESS)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, __LINE__, "m_pDev->SetSupportCoinTypes() fail(%d)", hRes);
        return WFS_ERR_UNSUPP_DATA;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::DoDenomination(USHORT usMixNumber, LPWFSCDMDENOMINATION lpDenomination, LPULONG lpulAmount, LPULONG lpulValues)
{
    THISMODULE(__FUNCTION__);
    HRESULT hRes;

    ST_COINCYLINDER_INFO stCoinCylinderInfos[MAX_COINCYLINDER_NUM];

    if ((usMixNumber != WFS_CDM_INDIVIDUAL) &&
            (usMixNumber != WFS_CDM_MIX_MINIMUM_NUMBER_OF_BILLS))
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "invalid usMixNumber(%d)",
            usMixNumber);
        return WFS_ERR_CDM_INVALIDMIXNUMBER;
    }

    if (lpDenomination == NULL)
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "lpDenomination is null");
        return WFS_ERR_INVALID_DATA;
    }

    if (memcmp(lpDenomination->cCurrencyID, "CNY", 3) != 0)
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        char cBuf[8];
        memset(cBuf, 0, sizeof(cBuf));
        memcpy(cBuf, lpDenomination->cCurrencyID, 3);
        Log(ThisModule, -1,
            "invalid cCurrencyID(%s)",
            cBuf);
        return WFS_ERR_CDM_INVALIDCURRENCY;
    }

    if ((lpDenomination->ulCashBox != 0) && (m_Param.GetDiscardCheck_ulCashBox() != 1))
    {
        SetSPErrorInfo(SP_ERR_PARAM_ERROR);
        Log(ThisModule, -1,
            "invalid ulCashBox(%d)",
            lpDenomination->ulCashBox);
        return WFS_ERR_INVALID_DATA;
    }

    // Get coin cylinder info.
    hRes = m_pDev->GetCoinCylinderList(stCoinCylinderInfos);
    if (DEV_SUCCESS != hRes)
    {
        SetSPErrorInfo(SP_ERR_DEV_ERROR);
        Log(ThisModule, -1,
            "GetCoinCylinderList() fail(%ld)",
            hRes);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // Check denomination.
    ULONG ulAmount = 0;
    if (usMixNumber == WFS_CDM_INDIVIDUAL)
    {
        if (lpDenomination->usCount != MAX_COINCYLINDER_NUM)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid usCount(%d)",
                lpDenomination->usCount);
            return WFS_ERR_INVALID_DATA;
        }

        if (lpDenomination->lpulValues == NULL)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid lpulValues");
            return WFS_ERR_INVALID_DATA;
        }

        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            // Copy dispense value.
            lpulValues[i] = lpDenomination->lpulValues[i];

            // Skip zero value.
            if (lpDenomination->lpulValues[i] == 0)
            {
                continue;
            }

            if ((stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUEMPTY) ||
                (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUINOP) ||
                (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUMISSING) ||
                (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUNOVAL) ||
                (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUNOREF) ||
                (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUMANIP))
            {
                SetSPErrorInfo(SP_ERR_PARAM_ERROR);
                Log(ThisModule, -1,
                    "invalid coin cylinder status(index:%d, status:%d)",
                    i, stCoinCylinderInfos[i].iStatus);
                return WFS_ERR_CDM_NOTDISPENSABLE;
            }

            if (stCoinCylinderInfos[i].lCount < lpDenomination->lpulValues[i])
            {
                SetSPErrorInfo(SP_ERR_NO_ENOUGH_COIN);
                Log(ThisModule, -1,
                    "no enough coin in cylinder(index:%d, count:%d)",
                    i, stCoinCylinderInfos[i].lCount);
                return WFS_ERR_CDM_NOTDISPENSABLE;
            }

            // Accumulate the total amount.
            ulAmount += stCoinCylinderInfos[i].iCashValue * lpDenomination->lpulValues[i];
        }

        // Check amount if input parameter set.
        if ((lpDenomination->ulAmount != 0) &&
                (lpDenomination->ulAmount != ulAmount))
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "amount does not match denomination(in:%ld, total:%l)",
                lpDenomination->ulAmount, ulAmount);
            return WFS_ERR_CDM_INVALIDDENOMINATION;
        }

        // Set amount.
        *lpulAmount = ulAmount;
    }
    else
    {
        if (lpDenomination->ulAmount == 0)
        {
            SetSPErrorInfo(SP_ERR_PARAM_ERROR);
            Log(ThisModule, -1,
                "invalid ulAmount(%d)",
                lpDenomination->ulAmount);
            return WFS_ERR_INVALID_DATA;
        }

        ulAmount = lpDenomination->ulAmount;
        BOOL    bDoneFlags[MAX_COINCYLINDER_NUM];
        ULONG   ulRemainAmount = ulAmount;
        ULONG   ulDispenseCount;
        USHORT  usCoinValue;
        LPUSHORT lpusCoinValues = m_Param.getCoinValues();

        memset(bDoneFlags, 0, sizeof(bDoneFlags));
        for (USHORT usCVIndex = 0; usCVIndex < m_Param.getCoinValuesCount(); usCVIndex++)
        {
            usCoinValue = lpusCoinValues[usCVIndex];
            ulDispenseCount = 0;

            for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
            {
                if ((stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUEMPTY) ||
                    (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUINOP) ||
                    (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUMISSING) ||
                    (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUNOVAL) ||
                    (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUNOREF) ||
                    (stCoinCylinderInfos[i].iStatus == COR_UNIT_STATCUMANIP))
                {
                    continue;
                }

                if (stCoinCylinderInfos[i].iCashValue != usCoinValue)
                {
                    continue;
                }

                // Skip the coin cylinder which has been used.
                if (bDoneFlags[i])
                {
                    continue;
                }

                ulDispenseCount = ulRemainAmount / usCoinValue;
                if (stCoinCylinderInfos[i].lCount < (long)ulDispenseCount)
                {
                    // There aren't enough coins to dispense.
                    ulRemainAmount -= stCoinCylinderInfos[i].lCount * usCoinValue;
                    lpulValues[i] += stCoinCylinderInfos[i].lCount;
                }
                else
                {
                    // There are enough coins to dispense.
                    ulRemainAmount -= ulDispenseCount * usCoinValue;
                    lpulValues[i] += ulDispenseCount;
                    break;
                }

                // Set used flag.
                bDoneFlags[i] = true;
            }
        }

        if (ulRemainAmount != 0)
        {
            SetSPErrorInfo(SP_ERR_NO_ENOUGH_COIN);
            Log(ThisModule, -1,
                "no enough coin in all cylinders(remain:%ld)",
                ulRemainAmount);
            return WFS_ERR_CDM_NOTDISPENSABLE;
        }

        // Set amount.
        *lpulAmount = ulAmount;
    }

    // Save coin cylinder info.
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        m_stCoinCylinderInfos[i].lCount = stCoinCylinderInfos[i].lCount;
        m_stCoinCylinderInfos[i].iStatus = stCoinCylinderInfos[i].iStatus;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::EditCDMCashUnitInfo()
{
    THISMODULE(__FUNCTION__);

    LPWFSCDMCASHUNIT lpWFSCDMCashUnit = NULL;
    LPWFSCDMPHCU     lpWFSCDMPHCU = NULL;

    memset(&m_CDMCUInfo, 0, sizeof(m_CDMCUInfo));
    memset(m_lpCDMCashUnitList, 0, sizeof(m_lpCDMCashUnitList));
    memset(m_CDMCashUnitArray, 0, sizeof(m_CDMCashUnitArray));
    memset(m_lpCDMPHCashUnitList, 0, sizeof(m_lpCDMPHCashUnitList));
    memset(m_CDMPHCashUnitArray, 0, sizeof(m_CDMPHCashUnitArray));

    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        lpWFSCDMCashUnit = &m_CDMCashUnitArray[i];
        lpWFSCDMPHCU = &m_CDMPHCashUnitArray[i];
        m_lpCDMCashUnitList[i] = lpWFSCDMCashUnit;
        m_lpCDMPHCashUnitList[i] = lpWFSCDMPHCU;

        // set cash unit details.
        lpWFSCDMCashUnit->usNumber = i + 1;
        lpWFSCDMCashUnit->usType = WFS_CDM_TYPERECYCLING;
        lpWFSCDMCashUnit->lpszCashUnitName = m_cCashUnitNameArray[i];
        strcpy(lpWFSCDMCashUnit->cUnitID,  m_stCoinCylinderInfos[i].szId);
        memcpy(lpWFSCDMCashUnit->cCurrencyID, "CNY", 3);
        lpWFSCDMCashUnit->ulValues = m_stCoinCylinderInfos[i].iCashValue;
        lpWFSCDMCashUnit->ulInitialCount = m_Param.getInitialCount(i);
        lpWFSCDMCashUnit->ulCount = m_stCoinCylinderInfos[i].lCount;
        lpWFSCDMCashUnit->ulRejectCount = m_Param.getRejectCount(i);
        lpWFSCDMCashUnit->ulMinimum = m_Param.getMinimum(i);
        lpWFSCDMCashUnit->ulMaximum = m_Param.getMaximum(i);
        lpWFSCDMCashUnit->bAppLock = FALSE;
        lpWFSCDMCashUnit->usStatus = m_stCoinCylinderInfos[i].iStatus;
        lpWFSCDMCashUnit->usNumPhysicalCUs = 1;
        lpWFSCDMCashUnit->lppPhysical = &m_lpCDMPHCashUnitList[i];

        // set physical cash unit details.
        lpWFSCDMPHCU->lpPhysicalPositionName = m_cPHPositionNameArray[i];
        strcpy(lpWFSCDMPHCU->cUnitID,  m_stCoinCylinderInfos[i].szId);
        lpWFSCDMPHCU->ulInitialCount = m_Param.getInitialCount(i);
        lpWFSCDMPHCU->ulCount = m_stCoinCylinderInfos[i].lCount;
        lpWFSCDMPHCU->ulRejectCount = m_Param.getRejectCount(i);
        lpWFSCDMPHCU->ulMaximum = m_Param.getPHMaximum(i);
        lpWFSCDMPHCU->usPStatus = m_stCoinCylinderInfos[i].iStatus;
        lpWFSCDMPHCU->bHardwareSensor = TRUE;
    }

    // retract cash unit.
    lpWFSCDMCashUnit = &m_CDMCashUnitArray[MAX_COINCYLINDER_NUM];
    lpWFSCDMPHCU = &m_CDMPHCashUnitArray[MAX_COINCYLINDER_NUM];
    m_lpCDMCashUnitList[MAX_COINCYLINDER_NUM] = lpWFSCDMCashUnit;
    m_lpCDMPHCashUnitList[MAX_COINCYLINDER_NUM] = lpWFSCDMPHCU;

    // set cash unit details.
    lpWFSCDMCashUnit->usNumber = MAX_COINCYLINDER_NUM + 1;
    lpWFSCDMCashUnit->usType = WFS_CDM_TYPERETRACTCASSETTE;
    lpWFSCDMCashUnit->lpszCashUnitName = m_cCashUnitNameArray[MAX_COINCYLINDER_NUM];
    strcpy(lpWFSCDMCashUnit->cUnitID, m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].szId);
    memcpy(lpWFSCDMCashUnit->cCurrencyID, "CNY", 3);
    lpWFSCDMCashUnit->ulValues = 0;
    lpWFSCDMCashUnit->ulInitialCount = m_Param.getInitialCount(MAX_COINCYLINDER_NUM);
    lpWFSCDMCashUnit->ulCount = m_Param.getCount(MAX_COINCYLINDER_NUM);
    lpWFSCDMCashUnit->ulRejectCount = 0;
    lpWFSCDMCashUnit->ulMinimum = m_Param.getMinimum(MAX_COINCYLINDER_NUM);
    lpWFSCDMCashUnit->ulMaximum = m_Param.getMaximum(MAX_COINCYLINDER_NUM);
    lpWFSCDMCashUnit->bAppLock = FALSE;
    lpWFSCDMCashUnit->usStatus = m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iStatus;
    lpWFSCDMCashUnit->usNumPhysicalCUs = 1;
    lpWFSCDMCashUnit->lppPhysical = &m_lpCDMPHCashUnitList[MAX_COINCYLINDER_NUM];

    // set physical cash unit details.
    lpWFSCDMPHCU->lpPhysicalPositionName = m_cPHPositionNameArray[MAX_COINCYLINDER_NUM];
    strcpy(lpWFSCDMPHCU->cUnitID, m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].szId);
    lpWFSCDMPHCU->ulInitialCount = 0;
    lpWFSCDMPHCU->ulCount = m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount;
    lpWFSCDMPHCU->ulRejectCount = 0;
    lpWFSCDMPHCU->ulMaximum = m_Param.getPHMaximum(MAX_COINCYLINDER_NUM);
    lpWFSCDMPHCU->usPStatus = m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iStatus;
    lpWFSCDMPHCU->bHardwareSensor = TRUE;

    m_CDMCUInfo.usTellerID = 0;
    m_CDMCUInfo.usCount = MAX_COINCYLINDER_NUM + 1;
    m_CDMCUInfo.lppList = m_lpCDMCashUnitList;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::EditCIMCashUnitInfo()
{
    THISMODULE(__FUNCTION__);

    LPWFSCIMCASHIN lpWFSCIMCashIn = NULL;
    LPWFSCIMPHCU lpWFSCIMPHCU = NULL;
    LPWFSCIMNOTENUMBERLIST lpWFSNoteNumberList = NULL;
    LPWFSCIMNOTENUMBER lpWFSNoteNumber = NULL;

    memset(&m_CIMCashInfo, 0, sizeof(WFSCIMCASHINFO));
    memset(m_CIMCashInPtrList, 0, sizeof(m_CIMCashInPtrList));
    memset(m_CIMCashInArray, 0, sizeof(m_CIMCashInArray));
    memset(m_CIMPHCashUnitPtrList, 0, sizeof(m_CIMPHCashUnitPtrList));
    memset(m_CIMPHCashUnitArray, 0, sizeof(m_CIMPHCashUnitArray));
    memset(m_CIMNoteNumberListArray, 0, sizeof(m_CIMNoteNumberListArray));
    memset(m_CIMNoteNumberPtrArrayArray, 0, sizeof(m_CIMNoteNumberPtrArrayArray));
    memset(m_CIMNoteNumberArrayArray, 0, sizeof(m_CIMNoteNumberArrayArray));

    m_CIMCashInfo.usCount = MAX_COINCYLINDER_NUM + 1;
    m_CIMCashInfo.lppCashIn = m_CIMCashInPtrList;

    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        lpWFSCIMCashIn = &m_CIMCashInArray[i];
        lpWFSCIMPHCU = &m_CIMPHCashUnitArray[i];
        m_CIMCashInfo.lppCashIn[i] = lpWFSCIMCashIn;
        m_CIMPHCashUnitPtrList[i] = lpWFSCIMPHCU;

        lpWFSCIMCashIn->usNumber = i + 1;
        lpWFSCIMCashIn->fwType = WFS_CIM_TYPERECYCLING;
        lpWFSCIMCashIn->fwItemType = WFS_CIM_CITYPINDIVIDUAL;
        strcpy(lpWFSCIMCashIn->cUnitID, m_stCoinCylinderInfos[i].szId);
        memcpy(lpWFSCIMCashIn->cCurrencyID, "CNY", 3);
        lpWFSCIMCashIn->ulValues = m_stCoinCylinderInfos[i].iCashValue;
        lpWFSCIMCashIn->ulCashInCount = m_Param.getCashInCount(i);
        lpWFSCIMCashIn->ulCount = m_stCoinCylinderInfos[i].lCount;
        lpWFSCIMCashIn->ulMaximum = m_Param.getMaximum(i);
        lpWFSCIMCashIn->usStatus = m_stCoinCylinderInfos[i].iStatus;
        lpWFSCIMCashIn->bAppLock = FALSE;
        lpWFSCIMCashIn->lpNoteNumberList = &m_CIMNoteNumberListArray[i];

        lpWFSNoteNumberList = m_Param.getNoteNumberList(i);
        m_CIMNoteNumberListArray[i].usNumOfNoteNumbers = lpWFSNoteNumberList->usNumOfNoteNumbers;
        m_CIMNoteNumberListArray[i].lppNoteNumber = m_CIMNoteNumberPtrArrayArray[i];

        for (int j = 0; j < lpWFSNoteNumberList->usNumOfNoteNumbers; j++)
        {
            m_CIMNoteNumberListArray[i].lppNoteNumber[j] = &m_CIMNoteNumberArrayArray[i][j];
            lpWFSNoteNumber = m_CIMNoteNumberListArray[i].lppNoteNumber[j];
            lpWFSNoteNumber->usNoteID = lpWFSNoteNumberList->lppNoteNumber[j]->usNoteID;
            lpWFSNoteNumber->ulCount = lpWFSNoteNumberList->lppNoteNumber[j]->ulCount;
        }

        lpWFSCIMCashIn->usNumPhysicalCUs = 1;
        lpWFSCIMCashIn->lppPhysical = &m_CIMPHCashUnitPtrList[i];
        lpWFSCIMPHCU->lpPhysicalPositionName = m_cPHPositionNameArray[i];
        strcpy(lpWFSCIMPHCU->cUnitID, m_stCoinCylinderInfos[i].szId);

        // Physical cash unit info.
        lpWFSCIMPHCU->ulCashInCount = lpWFSCIMCashIn->ulCashInCount;
        lpWFSCIMPHCU->ulCount = lpWFSCIMCashIn->ulCount;
        lpWFSCIMPHCU->ulMaximum = m_Param.getPHMaximum(i);
        lpWFSCIMPHCU->bHardwareSensors = FALSE;
        lpWFSCIMPHCU->usPStatus = WFS_CIM_STATCUOK;

        // nothing but two null characters.
        lpWFSCIMPHCU->lpszExtra = m_cNullExtra;

        // nothing but two null characters.
        lpWFSCIMCashIn->lpszExtra = m_cNullExtra;
    }

    // retract cash unit.
    lpWFSCIMCashIn = &m_CIMCashInArray[MAX_COINCYLINDER_NUM];
    lpWFSCIMPHCU = &m_CIMPHCashUnitArray[MAX_COINCYLINDER_NUM];
    m_CIMCashInfo.lppCashIn[MAX_COINCYLINDER_NUM] = lpWFSCIMCashIn;
    m_CIMPHCashUnitPtrList[MAX_COINCYLINDER_NUM] = lpWFSCIMPHCU;

    lpWFSCIMCashIn->usNumber = MAX_COINCYLINDER_NUM + 1;
    lpWFSCIMCashIn->fwType = WFS_CIM_TYPERETRACTCASSETTE;
    lpWFSCIMCashIn->fwItemType = WFS_CIM_CITYPINDIVIDUAL;
    strcpy(lpWFSCIMCashIn->cUnitID, m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].szId);
    memcpy(lpWFSCIMCashIn->cCurrencyID, "CNY", 3);
    lpWFSCIMCashIn->ulValues = m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iCashValue;

    // CashIn relative info.
    lpWFSCIMCashIn->ulCashInCount = m_Param.getCashInCount(MAX_COINCYLINDER_NUM);
    lpWFSCIMCashIn->ulCount = m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].lCount;
    lpWFSCIMCashIn->ulMaximum = m_Param.getMaximum(MAX_COINCYLINDER_NUM);
    lpWFSCIMCashIn->usStatus = m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].iStatus;
    lpWFSCIMCashIn->bAppLock = FALSE;
    lpWFSCIMCashIn->lpNoteNumberList = &m_CIMNoteNumberListArray[MAX_COINCYLINDER_NUM];

    lpWFSNoteNumberList = m_Param.getNoteNumberList(MAX_COINCYLINDER_NUM);
    m_CIMNoteNumberListArray[MAX_COINCYLINDER_NUM].usNumOfNoteNumbers = lpWFSNoteNumberList->usNumOfNoteNumbers;
    m_CIMNoteNumberListArray[MAX_COINCYLINDER_NUM].lppNoteNumber = m_CIMNoteNumberPtrArrayArray[MAX_COINCYLINDER_NUM];

    for (int j = 0; j < lpWFSNoteNumberList->usNumOfNoteNumbers; j++)
    {
        m_CIMNoteNumberListArray[MAX_COINCYLINDER_NUM].lppNoteNumber[j] = &m_CIMNoteNumberArrayArray[MAX_COINCYLINDER_NUM][j];
        lpWFSNoteNumber = m_CIMNoteNumberListArray[MAX_COINCYLINDER_NUM].lppNoteNumber[j];
        lpWFSNoteNumber->usNoteID = lpWFSNoteNumberList->lppNoteNumber[j]->usNoteID;
        lpWFSNoteNumber->ulCount = lpWFSNoteNumberList->lppNoteNumber[j]->ulCount;
    }

    lpWFSCIMCashIn->usNumPhysicalCUs = 1;
    lpWFSCIMCashIn->lppPhysical = &m_CIMPHCashUnitPtrList[MAX_COINCYLINDER_NUM];
    lpWFSCIMPHCU->lpPhysicalPositionName = m_cPHPositionNameArray[MAX_COINCYLINDER_NUM];
    strcpy(lpWFSCIMPHCU->cUnitID, m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM].szId);

    // Physical cash unit info.
    lpWFSCIMPHCU->ulCashInCount = lpWFSCIMCashIn->ulCashInCount;
    lpWFSCIMPHCU->ulCount = lpWFSCIMCashIn->ulCount;
    lpWFSCIMPHCU->ulMaximum = m_Param.getPHMaximum(MAX_COINCYLINDER_NUM);
    lpWFSCIMPHCU->bHardwareSensors = FALSE;
    lpWFSCIMPHCU->usPStatus = WFS_CIM_STATCUOK;
    lpWFSCIMPHCU->lpszExtra = m_cNullExtra;

    lpWFSCIMCashIn->lpszExtra = m_cNullExtra;

    return WFS_SUCCESS;
}

HRESULT CXFS_COR::UpdateCashUnitCount(LPULONG ulValues, int count, int type)
{
    HRESULT hRes = WFS_SUCCESS;
    LPWFSCIMNOTENUMBERLIST lpLocalNoteNumberList = NULL;

    switch (type)
    {
    case MONEY_COUNTERS_MONEYOUT:
    {
        for (int i = 0; i < count; i++)
        {
            m_Param.subtractCount(i, ulValues[i]);
            m_stCoinCylinderInfos[i].lCount -= ulValues[i];

            // Update note id specific count for CIM CashUnitInfo.
            lpLocalNoteNumberList = m_Param.getNoteNumberList(i);

            //NOTE! only clear note id specific count.
            if (lpLocalNoteNumberList->usNumOfNoteNumbers > 0)
            {
                lpLocalNoteNumberList->lppNoteNumber[0]->ulCount -= ulValues[i];
            }

            // check threshold.
            CheckCashUnitThreshold(i, m_stCoinCylinderInfos[i], type);
        }

        // save as nonvolatile data.
        m_Param.SaveCashUnitInfo(SAVE_MODE_CDM_CASHOUT);
    }
        break;
    default:
        break;
    }

    return hRes;
}

BOOL CXFS_COR::CheckCashUnitThreshold(int index, ST_COINCYLINDER_INFO& info, int type)
{
    BOOL bRet = FALSE;
    COIN_UNIT_STATUS status;
    switch (type)
    {
    case MONEY_COUNTERS_MONEYIN:
    {
        if (info.lCount > (LONG)m_Param.getMaximum(index))
        {
            status = COR_UNIT_STATCUHIGH;
        }
        else if (info.lCount > (LONG)m_Param.getMinimum(index))
        {
            status = COR_UNIT_STATCUOK;
        }
        else {
            status = COR_UNIT_STATCULOW;
        }

        if (status != info.iStatus)
        {
            bRet = TRUE;
            info.iStatus = status;
        }
    }
        break;
    case MONEY_COUNTERS_MONEYOUT:
    {
        if (info.lCount <= 0)
        {
            status = COR_UNIT_STATCUEMPTY;
        }
        else if (info.lCount < (LONG)m_Param.getMinimum(index))
        {
            status = COR_UNIT_STATCULOW;
        }
        else if (info.lCount < (LONG)m_Param.getMaximum(index))
        {
            status = COR_UNIT_STATCUOK;
        }
        else {
            status = COR_UNIT_STATCUHIGH;
        }

        if (status != info.iStatus)
        {
            bRet = TRUE;
            info.iStatus = status;
        }
    }
        break;
    default:
        break;
    }

    return bRet;
}

void CXFS_COR::CheckCashUnitThreshold(int type)
{
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        if (m_CUStatus[i] != m_stCoinCylinderInfos[i].iStatus)
        {
            switch (type)
            {
            case MONEY_COUNTERS_MONEYIN:
            {
                CIMFireCashUnitThreshold(&m_CIMCashInArray[i]);
            }
                break;
            case MONEY_COUNTERS_MONEYOUT:
            {
                CDMFireCashUnitThreshold(&m_CDMCashUnitArray[i]);
            }
                break;
            default:
                break;
            }
        }
    }
}

void CXFS_COR::CheckCashUnitInfoChanged()
{
    for (int i = 0; i < MAX_COINCYLINDER_NUM + 1; i++)
    {
        if (m_ulCountBK[i] != (ULONG) m_stCoinCylinderInfos[i].lCount)
        {
            CDMFireCUInfoChanged(&m_CDMCashUnitArray[i]);
            CIMFireCUInfoChanged(&m_CIMCashInArray[i]);
        }
    }
}

void CXFS_COR::BackupCashUnitInfo()
{
    for (int i = 0; i < MAX_COINCYLINDER_NUM + 1; i++)
    {
        m_ulCountBK[i] = m_stCoinCylinderInfos[i].lCount;
        m_CUStatus[i] = m_stCoinCylinderInfos[i].iStatus;
    }
}

void CXFS_COR::EditStatusExtraInfo()
{
    int iLen;
    char* pPos;

    memset(m_cStatusExtraInfo, 0, sizeof(m_cStatusExtraInfo));
    pPos = m_cStatusExtraInfo;
    iLen = sprintf(pPos, "LastErrorCode=%s", m_cErrorCode);
    pPos += iLen;
    *pPos = '\0';
    pPos++;
    iLen = sprintf(pPos, "LastErrorDetail=%s", m_cErrorDetail);
    pPos += iLen;
    *pPos = '\0';
    pPos++;
    iLen = sprintf(pPos, "FirmwareVersion=%s", m_cFirmwareVer);
    pPos += iLen;
    *pPos = '\0';
    pPos++;
    iLen = sprintf(pPos, "Manufacturer=%s", m_stDevInfo.pszManufacturer);
    pPos += iLen;
    *pPos = '\0';
    pPos++;
    iLen = sprintf(pPos, "ProductName=%s", m_stDevInfo.pszProductName);
}

void CXFS_COR::SetSPErrorInfo(int iErrCode)
{
    strcpy(m_cErrorCode, sp_errors[iErrCode][0]);
    strcpy(m_cErrorDetail, sp_errors[iErrCode][1]);
}

long CXFS_COR::UpdateStatus(const char *ThisModule)
{
    HRESULT hRes;

    hRes = m_pDev->GetStatus(&m_stCorStatus);
    if (hRes != DEV_SUCCESS)
    {
        Log(ThisModule, hRes, "m_pDev->GetStatus()失败");
        return hRes;
    }

    return DEV_SUCCESS;
}

void CXFS_COR::CDMFireCashUnitThreshold(const LPWFSCDMCASHUNIT lpCU)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_CDM_CASHUNITTHRESHOLD, lpCU);
}

void CXFS_COR::CDMFireCUInfoChanged(const LPWFSCDMCASHUNIT lpCU)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CDM_CASHUNITINFOCHANGED, lpCU);
}

void CXFS_COR::CDMFireCUError(WORD wFail, const LPWFSCDMCASHUNIT lpCU)
{
    WFSCDMCUERROR err;
    err.wFailure = wFail;
    err.lpCashUnit = lpCU;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CDM_CASHUNITERROR, &err);
}

void CXFS_COR::CIMFireCashUnitThreshold(const LPWFSCIMCASHIN lpCashIn)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_CIM_CASHUNITTHRESHOLD, lpCashIn);
}

void CXFS_COR::CIMFireCUInfoChanged(const LPWFSCIMCASHIN lpCashIn)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CIM_CASHUNITINFOCHANGED, lpCashIn);
}

void CXFS_COR::CIMFireCUError(WORD wFail, const LPWFSCIMCASHIN lpCashIn)
{
    WFSCIMCUERROR err;
    err.wFailure = wFail;
    err.lpCashUnit = lpCashIn;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CIM_CASHUNITERROR, &err);
}
