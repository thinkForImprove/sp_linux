#include "SPBaseCRS.h"

static const char *ThisFile = "SPBaseCRS.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateISPBaseCRS(LPCSTR lpDevType, ISPBaseCRS *&p)
{
    p = new CSPBaseCRS(lpDevType);
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CSPBaseCRS::CSPBaseCRS(LPCSTR lpLogType): m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
    m_pFMTResData = new CFMTResData(lpLogType);
    m_pCmdFunc = nullptr;
}

CSPBaseCRS::~CSPBaseCRS()
{
    Release();
}

void CSPBaseCRS::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    delete m_pFMTResData;
    m_pFMTResData = nullptr;
    return;
}

void CSPBaseCRS::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBaseCRS::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载Base类
    if (0 != m_pBase.Load("SPBaseClass.dll", "CreateISPBaseClass", m_szLogType))
    {
        Log(ThisModule, __LINE__, "加载SPBaseClass类失败！");
        return false;
    }

    if (!m_pFMTResData->LoadWFMDll())
    {
        return false;
    }

    // 注册回调
    m_pBase->RegisterICmdRun(this);
    return m_pBase->StartRun();
}

void CSPBaseCRS::GetSPBaseData(SPBASEDATA &stData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return;
    }
    return m_pBase->GetSPBaseData(stData);
}

bool CSPBaseCRS::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }
    return m_pBase->FireEvent(uMsgID, dwEventID, lpData);
}

bool CSPBaseCRS::FireStatusChanged(DWORD dwStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }

    WFSDEVSTATUS stStatus;
    memset(&stStatus, 0x00, sizeof(stStatus));
    stStatus.dwState = dwStatus;
    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_DEVICE_STATUS, &stStatus);
}

bool CSPBaseCRS::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }

    WFSHWERROR stStatus;
    memset(&stStatus, 0x00, sizeof(stStatus));
    stStatus.dwAction = dwAction;
    if (lpDescription != nullptr)
    {
        stStatus.dwSize         = strlen((char *)lpDescription);
        stStatus.lpbDescription = (LPBYTE)lpDescription;
    }
    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_HARDWARE_ERROR, &stStatus);
}

HRESULT CSPBaseCRS::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnOpen(lpLogicalName);
}

HRESULT CSPBaseCRS::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnClose();
}

HRESULT CSPBaseCRS::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnStatus();
}

HRESULT CSPBaseCRS::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        MLog(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return m_pCmdFunc->OnWaitTaken();
}

HRESULT CSPBaseCRS::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnCancelAsyncRequest();
}

HRESULT CSPBaseCRS::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnUpdateDevPDL();
}

HRESULT CSPBaseCRS::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    DWORD dwCatOffset = (dwCategory / 100) * 100;
    if (dwCatOffset == CDM_SERVICE_OFFSET)
        return CDMGetInfo(dwCategory, lpQueryDetails, lpResult);
    else
        return CIMGetInfo(dwCategory, lpQueryDetails, lpResult);
}

HRESULT CSPBaseCRS::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    m_pCmdFunc->SetExeFlag(dwCommand, TRUE);
    HRESULT hRet        = WFS_ERR_INTERNAL_ERROR;
    DWORD   dwCmdOffset = (dwCommand / 100) * 100;
    if (dwCmdOffset == CDM_SERVICE_OFFSET)
        hRet = CDMExecute(dwCommand, lpCmdData, dwTimeOut, lpResult);
    else
        hRet = CIMExecute(dwCommand, lpCmdData, dwTimeOut, lpResult);
    m_pCmdFunc->SetExeFlag(dwCommand, FALSE);
    return hRet;
}

HRESULT CSPBaseCRS::CDMGetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCategory)
    {
    case WFS_INF_CDM_STATUS:
        {
            LPWFSCDMSTATUS lpStatus = nullptr;
            hRet = m_pCmdFunc->CDMGetStatus(lpStatus);
            if (hRet != WFS_SUCCESS)
                break;
            hRet = m_pFMTResData->FmtCDMGetResultBuffer(dwCategory, lpStatus, lpResult);
        }
        break;
    case WFS_INF_CDM_CAPABILITIES:
        {
            LPWFSCDMCAPS lpCaps = nullptr;
            hRet = m_pCmdFunc->CDMGetCapabilities(lpCaps);
            if (hRet != WFS_SUCCESS)
                break;
            hRet = m_pFMTResData->FmtCDMGetResultBuffer(dwCategory, lpCaps, lpResult);
        }
        break;
    case WFS_INF_CDM_CASH_UNIT_INFO:
        {
            LPWFSCDMCUINFO lpCashInfo = nullptr;
            hRet = m_pCmdFunc->CDMGetCashUnitInfo(lpCashInfo);
            if (hRet != WFS_SUCCESS)
                break;
            hRet = m_pFMTResData->FmtCDMGetResultBuffer(dwCategory, lpCashInfo, lpResult);
        }
        break;
    case WFS_INF_CDM_MIX_TYPES:
        {
            LPWFSCDMMIXTYPE *lppMixTypes = nullptr;
            hRet = m_pCmdFunc->CDMGetMixType(lppMixTypes);
            if (hRet != WFS_SUCCESS)
                break;
            hRet = m_pFMTResData->FmtCDMGetResultBuffer(dwCategory, lppMixTypes, lpResult);
        }
        break;
    case WFS_INF_CDM_PRESENT_STATUS:
        {
            auto lpPos = static_cast<LPWORD>(lpQueryDetails);
            LPWFSCDMPRESENTSTATUS lpPresentStats = nullptr;
            hRet = m_pCmdFunc->CDMGetPresentStatus(*lpPos, lpPresentStats);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCDMGetResultBuffer(dwCategory, lpPresentStats, lpResult);
        }
        break;
    case WFS_INF_CDM_CURRENCY_EXP:
        {
            auto lpPos = static_cast<LPWORD>(lpQueryDetails);
            LPWFSCDMCURRENCYEXP *lppCurrencyExp = nullptr;
            hRet = m_pCmdFunc->CDMGetCurrencyEXP(lppCurrencyExp);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCDMGetResultBuffer(dwCategory, lppCurrencyExp, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBaseCRS::CDMExecute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRes = WFS_ERR_INTERNAL_ERROR;
    switch (dwCommand)
    {
    case WFS_CMD_CDM_OPEN_SHUTTER:
        {
            WORD wPos = WFS_CDM_POSNULL;
            if (lpCmdData != nullptr)
            {
                wPos = *(LPWORD)lpCmdData;
            }
            hRes = m_pCmdFunc->CDMOpenShutter(wPos);
        }
        break;
    case WFS_CMD_CDM_CLOSE_SHUTTER:
        {
            WORD wPos = WFS_CDM_POSNULL;
            if (lpCmdData != NULL)
            {
                wPos = *(LPWORD)lpCmdData;
            }
            hRes = m_pCmdFunc->CDMCloseShutter(wPos);
        }
        break;
    case WFS_CMD_CDM_RETRACT:
        {
            auto lpReadInput = static_cast<LPWFSCDMRETRACT>(lpCmdData);
            hRes = m_pCmdFunc->CDMRetract(*lpReadInput);
        }
        break;
    case WFS_CMD_CDM_PRESENT:
        {
            auto lpReadInput = static_cast<LPWORD>(lpCmdData);
            hRes = m_pCmdFunc->CDMPresent(*lpReadInput);
        }
        break;
    case WFS_CMD_CDM_REJECT:
        hRes = m_pCmdFunc->CDMReject();
        break;
    case WFS_CMD_CDM_START_EXCHANGE:
        {
            auto lpReadInput = static_cast<LPWFSCDMSTARTEX>(lpCmdData);
            LPWFSCDMCUINFO lpCUInfor = nullptr;
            hRes = m_pCmdFunc->CDMStartEXChange(lpReadInput, lpCUInfor);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = m_pFMTResData->FmtCDMExeResultBuffer(dwCommand, lpCUInfor, lpResult);
        }
        break;
    case WFS_CMD_CDM_END_EXCHANGE:
        {
            auto lpReadInput = static_cast<LPWFSCDMCUINFO>(lpCmdData);
            hRes = m_pCmdFunc->CDMEndEXChange(lpReadInput);
        }
        break;
    case WFS_CMD_CDM_SET_CASH_UNIT_INFO:
        {
            auto lpReadInput = static_cast<LPWFSCDMCUINFO>(lpCmdData);
            hRes = m_pCmdFunc->CDMSetCashUnitInfo(lpReadInput);
        }
        break;
    case WFS_CMD_CDM_RESET:
        {
            auto lpReadInput = static_cast<LPWFSCDMITEMPOSITION>(lpCmdData);
            hRes = m_pCmdFunc->CDMReset(lpReadInput);
        }
        break;
    case WFS_CMD_CDM_DENOMINATE:
        {
            auto lpDenominate = (LPWFSCDMDENOMINATE)lpCmdData;
            LPWFSCDMDENOMINATION lpResData = nullptr;
            hRes = m_pCmdFunc->CDMDenominate(lpDenominate, lpResData);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = m_pFMTResData->FmtCDMExeResultBuffer(dwCommand, lpResData, lpResult);
        }
        break;
    case WFS_CMD_CDM_DISPENSE:
        {
            auto pDisp = (LPWFSCDMDISPENSE)lpCmdData;
            LPWFSCDMDENOMINATION lpResData = nullptr;
            hRes = m_pCmdFunc->CDMDispense(pDisp, lpResData);
            HRESULT hResF = m_pFMTResData->FmtCDMExeResultBuffer(dwCommand, lpResData, lpResult);
            if (hRes != WFS_SUCCESS)
            {
                break;
            }
            else if (hResF != WFS_SUCCESS)
            {
                hRes = hResF;
                break;
            }
        }
        break;
    case WFS_CMD_CDM_TEST_CASH_UNITS:                                                       //30-00-00-00(FS#0007)
        {                                                                                   //30-00-00-00(FS#0007)
            LPWFSCDMITEMPOSITION lpItemPosition = (LPWFSCDMITEMPOSITION)lpCmdData;          //30-00-00-00(FS#0007)
            LPWFSCDMCUINFO lpCdmCUInfo = nullptr;                                           //30-00-00-00(FS#0007)

            hRes = m_pCmdFunc->CDMTestCashUnits(lpItemPosition, lpCdmCUInfo);               //30-00-00-00(FS#0007)
            if(lpCdmCUInfo != nullptr){                                                     //30-00-00-00(FS#0007)
                hRes = m_pFMTResData->FmtCDMExeResultBuffer(dwCommand, lpCdmCUInfo, lpResult);  //30-00-00-00(FS#0007)
            }                                                                               //30-00-00-00(FS#0007)
        }                                                                                   //30-00-00-00(FS#0007)
        break;                                                                              //30-00-00-00(FS#0007)
    default:
        hRes = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRes;
}

HRESULT CSPBaseCRS::CIMGetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCategory)
    {
    case WFS_INF_CIM_STATUS:
        {
            LPWFSCIMSTATUS lpStatus = nullptr;
            hRet = m_pCmdFunc->CIMGetStatus(lpStatus);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCIMGetResultBuffer(dwCategory, lpStatus, lpResult);
        }
        break;
    case WFS_INF_CIM_CAPABILITIES:
        {
            LPWFSCIMCAPS lpCaps = nullptr;
            hRet = m_pCmdFunc->CIMGetCapabilities(lpCaps);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCIMGetResultBuffer(dwCategory, lpCaps, lpResult);
        }
        break;
    case WFS_INF_CIM_CASH_UNIT_INFO:
        {
            LPWFSCIMCASHINFO lpCashInfo = nullptr;
            hRet = m_pCmdFunc->CIMGetCashUnitInfo(lpCashInfo);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCIMGetResultBuffer(dwCategory, lpCashInfo, lpResult);
        }
        break;
    case WFS_INF_CIM_BANKNOTE_TYPES:
        {
            LPWFSCIMNOTETYPELIST lpBankNoteType = nullptr;
            hRet = m_pCmdFunc->CIMGetBankNoteType(lpBankNoteType);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCIMGetResultBuffer(dwCategory, lpBankNoteType, lpResult);
        }
        break;
    case WFS_INF_CIM_CASH_IN_STATUS:
        {
            LPWFSCIMCASHINSTATUS lpCashInStats = nullptr;
            hRet = m_pCmdFunc->CIMGetCashInStatus(lpCashInStats);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCIMGetResultBuffer(dwCategory, lpCashInStats, lpResult);
        }
        break;
    case WFS_INF_CIM_CURRENCY_EXP:
        {
            LPWFSCIMCURRENCYEXP *lppCurrencyExp = nullptr;
            hRet = m_pCmdFunc->CIMGetCurrencyEXP(lppCurrencyExp);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = m_pFMTResData->FmtCIMGetResultBuffer(dwCategory, lppCurrencyExp, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBaseCRS::CIMExecute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRes = WFS_ERR_INTERNAL_ERROR;
    switch (dwCommand)
    {
    case WFS_CMD_CIM_CASH_IN_START:
        {
            auto lpStart = static_cast<LPWFSCIMCASHINSTART>(lpCmdData);
            hRes = m_pCmdFunc->CIMCashInStart(lpStart);
        }
        break;
    case WFS_CMD_CIM_CASH_IN:
        {
            LPWFSCIMNOTENUMBERLIST lpNoteList = nullptr;
            hRes = m_pCmdFunc->CIMCashIn(lpNoteList);

            if (hRes != WFS_SUCCESS && m_pCmdFunc->GetTooManeyFlag() != 1) //test#7
                break;
            hRes = m_pFMTResData->FmtCIMExeResultBuffer(dwCommand, lpNoteList, lpResult);
            if(m_pCmdFunc->GetTooManeyFlag() == 1) //test#7
            {//test#7
             hRes = WFS_ERR_CIM_TOOMANYITEMS;//test#7

            }//test#7
        }
        break;
    case WFS_CMD_CIM_CASH_IN_END:
        {
            LPWFSCIMCASHINFO lpCashInfo = nullptr;
            hRes = m_pCmdFunc->CIMCashInEnd(lpCashInfo);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = m_pFMTResData->FmtCIMExeResultBuffer(dwCommand, lpCashInfo, lpResult);
        }
        break;
    case WFS_CMD_CIM_CASH_IN_ROLLBACK:
        {
            LPWFSCIMCASHINFO lpCashInfo = nullptr;
            hRes = m_pCmdFunc->CIMCashInRollBack(lpCashInfo);
        }
        break;
    case WFS_CMD_CIM_OPEN_SHUTTER:
        {
            WORD wPos = WFS_CIM_POSNULL;
            if (lpCmdData != NULL)
            {
                wPos = *(LPWORD)lpCmdData;
            }
            hRes = m_pCmdFunc->CIMOpenShutter(wPos);
        }
        break;
    case WFS_CMD_CIM_CLOSE_SHUTTER:
        {
            WORD wPos = WFS_CIM_POSNULL;
            if (lpCmdData != NULL)
            {
                wPos = *(LPWORD)lpCmdData;
            }
            hRes = m_pCmdFunc->CIMCloseShutter(wPos);
        }
        break;
    case WFS_CMD_CIM_START_EXCHANGE:
        {
            auto lpStartEx = static_cast<LPWFSCIMSTARTEX>(lpCmdData);
            LPWFSCIMCASHINFO lpCUInfor = nullptr;

            hRes = m_pCmdFunc->CIMStartEXChange(lpStartEx, lpCUInfor);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = m_pFMTResData->FmtCIMExeResultBuffer(dwCommand, lpCUInfor, lpResult);
        }
        break;
    case WFS_CMD_CIM_END_EXCHANGE:
        {
            auto lpEndEx = static_cast<LPWFSCIMCASHINFO>(lpCmdData);
            hRes = m_pCmdFunc->CIMEndEXChange(lpEndEx);
        }
        break;
    case WFS_CMD_CIM_RETRACT:
        {
            auto lpData = static_cast<LPWFSCIMRETRACT>(lpCmdData);
            LPWFSCIMRETRACT pRetract = (LPWFSCIMRETRACT)lpCmdData;
            hRes = m_pCmdFunc->CIMRetract(lpData);
        }
        break;
    case WFS_CMD_CIM_RESET:
        {
            auto lpData = static_cast<LPWFSCIMITEMPOSITION>(lpCmdData);
            hRes = m_pCmdFunc->CIMReset(lpData);
        }
        break;
    case WFS_CMD_CIM_SET_CASH_UNIT_INFO:
        {
            auto lpCashInfor = static_cast<LPWFSCIMCASHINFO>(lpCmdData);
            hRes = m_pCmdFunc->CIMSetCashUnitInfo(lpCashInfor);
        }
        break;
    case WFS_CMD_CIM_SET_CASH_IN_LIMIT:
        {
            auto lpCashInLimit = static_cast<LPWFSCIMCASHINLIMIT>(lpCmdData);
            hRes = m_pCmdFunc->CIMCashInLimit(lpCashInLimit);
        }
        break;
    case WFS_CMD_CIM_CONFIGURE_NOTETYPES:
        {
            auto lpNoteIDs = static_cast<LPUSHORT>(lpCmdData);
            hRes = m_pCmdFunc->CIMConfigureNoteTypes(lpNoteIDs);
        }
        break;
    case WFS_CMD_CIM_CASH_UNIT_COUNT:                               //30-00-00-00(FS#0022)
    {                                                               //30-00-00-00(FS#0022)
        auto lpCIMCount = static_cast<LPWFSCIMCOUNT>(lpCmdData);    //30-00-00-00(FS#0022)
        hRes = m_pCmdFunc->CIMCashUnitCount(lpCIMCount);            //30-00-00-00(FS#0022)
    }                                                               //30-00-00-00(FS#0022)
        break;                                                      //30-00-00-00(FS#0022)
    default:
        hRes = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRes;
}

HRESULT CSPBaseCRS::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_EXEE_CDM_CASHUNITERROR:
                hRet = m_pFMTResData->Fmt_WFSCDMCUERROR(lpData, lpResult);
                break;
            case WFS_EXEE_CIM_CASHUNITERROR:
                hRet = m_pFMTResData->Fmt_WFSCIMCUERROR(lpData, lpResult);
                break;
            case WFS_EXEE_CIM_INPUTREFUSE:
                hRet = m_pFMTResData->Fmt_WFSCIMINPUTREFUSE(lpData, lpResult);
                break;
            case WFS_EXEE_CIM_SUBCASHIN:
                hRet = m_pFMTResData->Fmt_WFSCIMSUBCASHIN(lpData, lpResult);
                break;
            default:
                break;
            }
        }
        break;
    case WFS_SERVICE_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_SRVE_CDM_SAFEDOOROPEN:
            case WFS_SRVE_CDM_SAFEDOORCLOSED:
            case WFS_SRVE_CDM_ITEMSPRESENTED:
            case WFS_SRVE_CIM_SAFEDOOROPEN:
            case WFS_SRVE_CIM_SAFEDOORCLOSED:
            case WFS_SRVE_CIM_ITEMSTAKEN:
            case WFS_SRVE_CIM_ITEMSINSERTED:
            case WFS_SRVE_CIM_ITEMSPRESENTED:
                hRet = m_pFMTResData->Fmt_NODATA(lpResult);
                break;
            case WFS_SRVE_CDM_ITEMSTAKEN:
                hRet = m_pFMTResData->Fmt_WFSCDMITEMTAKEN(lpData, lpResult);
                break;
            case WFS_SRVE_CDM_CASHUNITINFOCHANGED:
                hRet = m_pFMTResData->Fmt_WFSCDMCASHUNIT(lpData, lpResult);
                break;
            case WFS_SRVE_CIM_CASHUNITINFOCHANGED:
                hRet = m_pFMTResData->Fmt_WFSCIMCASHIN(lpData, lpResult);
                break;
            case WFS_SRVE_CDM_COUNTS_CHANGED:
            case WFS_SRVE_CIM_COUNTS_CHANGED:
                hRet = m_pFMTResData->Fmt_WFSCDMCIMCOUNTSCHANGED(lpData, lpResult);
                break;
            case WFS_SRVE_CDM_MEDIADETECTED:
                hRet = m_pFMTResData->Fmt_WFSCDMMEDIADETECTED(lpData, lpResult);
                break;
            case WFS_SRVE_CIM_MEDIADETECTED:
                hRet = m_pFMTResData->Fmt_WFSCIMMEDIADETECTED(lpData, lpResult);
                break;
            default:
                break;
            }
        }
        break;
    case WFS_USER_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_USRE_CDM_CASHUNITTHRESHOLD:
                hRet = m_pFMTResData->Fmt_WFSCDMCASHUNIT(lpData, lpResult);
                break;
            case WFS_USRE_CIM_CASHUNITTHRESHOLD:
                hRet = m_pFMTResData->Fmt_WFSCIMCASHIN(lpData, lpResult);
                break;
            default:
                break;
            }
        }
        break;
    case WFS_SYSTEM_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_SYSE_HARDWARE_ERROR:
                hRet = m_pBase->Fmt_WFSHWERROR(lpResult, lpData);
                break;
            case WFS_SYSE_DEVICE_STATUS:
                hRet = m_pBase->Fmt_WFSDEVSTATUS(lpResult, lpData);
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
    return hRet;
}
