#include "SPBaseIDC.h"

static const char *ThisFile = "SPBaseIDC.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateISPBaseIDC(LPCSTR lpDevType, ISPBaseIDC *&p)
{
    p = new CSPBaseIDC(lpDevType);
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CSPBaseIDC::CSPBaseIDC(LPCSTR lpLogType) : m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
}

CSPBaseIDC::~CSPBaseIDC() {}

void CSPBaseIDC::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

void CSPBaseIDC::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBaseIDC::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载Base类
    if (0 != m_pBase.Load("SPBaseClass.dll", "CreateISPBaseClass", m_szLogType))
    {
        Log(ThisModule, __LINE__, "加载SPBaseClass类失败！");
        return false;
    }

    // 加载共享内存类
    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
    {
        Log(ThisModule, __LINE__, "加载库失败: WFMShareMenory.dll");
        return false;
    }

    // 注册回调
    m_pBase->RegisterICmdRun(this);
    return m_pBase->StartRun();
}

void CSPBaseIDC::GetSPBaseData(SPBASEDATA &stData)
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

bool CSPBaseIDC::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }
    return m_pBase->FireEvent(uMsgID, dwEventID, lpData);
}

bool CSPBaseIDC::FireStatusChanged(DWORD dwStatus)
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

bool CSPBaseIDC::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
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
        stStatus.dwSize = strlen(lpDescription);
        stStatus.lpbDescription = (LPBYTE)lpDescription;
    }
    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_HARDWARE_ERROR, &stStatus);
}

HRESULT CSPBaseIDC::OnOpen(LPCSTR lpLogicalName)
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

HRESULT CSPBaseIDC::OnClose()
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

HRESULT CSPBaseIDC::OnStatus()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnStatus();
}

HRESULT CSPBaseIDC::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnWaitTaken();
}

HRESULT CSPBaseIDC::OnCancelAsyncRequest()
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

HRESULT CSPBaseIDC::OnUpdateDevPDL()
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

HRESULT CSPBaseIDC::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCategory)
    {
    case WFS_INF_IDC_STATUS:
    {
        LPWFSIDCSTATUS lpstStatus = nullptr;
        hRet = m_pCmdFunc->GetStatus(lpstStatus);
        if (hRet != WFS_SUCCESS)
            break;

        hRet = Fmt_WFSIDCSTATUS(lpstStatus, lpResult);
    }
    break;
    case WFS_INF_IDC_CAPABILITIES:
    {
        LPWFSIDCCAPS lpstCaps = nullptr;
        hRet = m_pCmdFunc->GetCapabilities(lpstCaps);
        if (hRet != WFS_SUCCESS)
            break;

        hRet = Fmt_WFSIDCCAPS(lpstCaps, lpResult);
    }
    break;
    case WFS_INF_IDC_FORM_LIST:
    {
        LPSTR lpszFormList = nullptr;
        hRet = m_pCmdFunc->GetFormList(lpszFormList);
        if (hRet != WFS_SUCCESS)
            break;
        hRet = Fmt_WFSIDCFORMLIST(lpszFormList, lpResult);
    }
    break;
    case WFS_INF_IDC_QUERY_FORM:
    {
        LPWFSIDCFORM lpForm = nullptr;
        auto lpQDetails = static_cast<LPCSTR>(lpQueryDetails);
        hRet = m_pCmdFunc->GetForm(lpQDetails, lpForm);
        if (hRet != WFS_SUCCESS)
            break;

        hRet = Fmt_WFSIDCFORM(lpForm, lpResult);
    }
    break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBaseIDC::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRes = WFS_ERR_INTERNAL_ERROR;
    switch (dwCommand)
    {
    case WFS_CMD_IDC_EJECT_CARD:
        hRes = m_pCmdFunc->EjectCard(dwTimeOut);
        break;
    case WFS_CMD_IDC_RETAIN_CARD:
    {
        LPWFSIDCRETAINCARD lpCardRetain;
        hRes = m_pCmdFunc->RetainCard(lpCardRetain);
        if (hRes != WFS_SUCCESS)
            break;
        hRes = Fmt_WFSIDCRETAINCARD(lpCardRetain, lpResult);
    }
    break;
    case WFS_CMD_IDC_RESET_COUNT:
        hRes = m_pCmdFunc->ResetCount();
        break;
    case WFS_CMD_IDC_READ_RAW_DATA:
    {
        LPWFSIDCCARDDATA *lppCardData;
        hRes = m_pCmdFunc->ReadRawData((LPWORD)lpCmdData, lppCardData, dwTimeOut);
        if (hRes != WFS_SUCCESS)
            break;
        hRes = Fmt_WFSIDCCARDDATAARY(lppCardData, lpResult);
    }
    break;
    case WFS_CMD_IDC_CHIP_IO:
    {
        LPWFSIDCCHIPIO lpChipIO;
        hRes = m_pCmdFunc->ChipIO((LPWFSIDCCHIPIO)lpCmdData, lpChipIO);
        if (hRes != WFS_SUCCESS)
            break;
        hRes = Fmt_WFSIDCCHIPIO(lpChipIO, lpResult);
    }
    break;
    case WFS_CMD_IDC_CHIP_POWER:
    {
        LPWFSIDCCHIPPOWEROUT lpChipPowerOut;
        hRes = m_pCmdFunc->ChipPower((LPWORD)lpCmdData, lpChipPowerOut);
        if (hRes != WFS_SUCCESS)
            break;
        hRes = Fmt_WFSIDCCHIPPOWEROUT(lpChipPowerOut, lpResult);
    }
    break;
    case WFS_CMD_IDC_RESET:
    {
        hRes = m_pCmdFunc->Reset((LPWORD)lpCmdData);
    }
    break;
#ifdef CARD_REJECT_GD_MODE
    case WFS_CMD_IDC_CMEJECT_CARD:
    {
        hRes = m_pCmdFunc->CMEejctCard((LPCSTR)lpCmdData);
    }
    break;
    case WFS_CMD_IDC_CMSTATUS:
    {
        BYTE bufStatus[1024] = {0};
        hRes = m_pCmdFunc->CMStatus((BYTE *)lpCmdData, bufStatus);
        if (hRes == WFS_SUCCESS)
        {
            if (bufStatus[1023] != 0x00)
            {
                Log(__FUNCTION__, __LINE__, "赋值超出空间范围");
                return WFS_ERR_OUT_OF_MEMORY;
            }
            hRes = Fmt_WFSIDCCMSTRING(bufStatus, 118, lpResult);
        }
    }
    break;
    case WFS_CMD_IDC_SETCARDDATA:
    {
        hRes = m_pCmdFunc->CMSetCardData((LPCSTR)lpCmdData);
    }
    break;
    case WFS_CMD_IDC_CMRETAIN_CARD:
    {
        hRes = m_pCmdFunc->CMRetain();
    }
    break;
    case WFS_CMD_IDC_CMREDUCE_COUNT:
    {
        hRes = m_pCmdFunc->CMReduceCount();
    }
    break;
    case WFS_CMD_IDC_CMSET_COUNT:
    {
        hRes = m_pCmdFunc->CMSetCount((LPWORD)lpCmdData);
    }
    break;
    case WFS_CMD_CMEMPTY_CARD:
    {
        hRes = m_pCmdFunc->CMEmptyCard((LPCSTR)lpCmdData);
    }
    break;
    case WFS_CMD_IDC_GETCARDINFO:
    {
        char bufCardInfo[1024] = {0};
        hRes = m_pCmdFunc->CMGetCardInfo((LPCSTR)lpCmdData, bufCardInfo);
        if (hRes == WFS_SUCCESS)
        {
            if (bufCardInfo[1023] != 0x00)
            {
                Log(__FUNCTION__, __LINE__, "赋值超出空间范围");
                return WFS_ERR_OUT_OF_MEMORY;
            }
            DWORD dwLen = strlen(bufCardInfo);
            hRes = Fmt_WFSIDCCMSTRING((BYTE *)bufCardInfo, dwLen + 1, lpResult);
        }
    }
    break;
    case WFS_CMD_IDC_CMRESET:
    {
        hRes = m_pCmdFunc->CMReset();
    }
    case WFS_CMD_IDC_REDUCE_COUNT:
    {
        hRes = m_pCmdFunc->ReduceCount();
    }
    case WFS_CMD_IDC_SET_COUNT:
    {
        hRes = m_pCmdFunc->SetCount((LPWORD)lpCmdData);
    }
    case WFS_CMD_IDC_INTAKE_CARD_BACK:
    {
        hRes = m_pCmdFunc->IntakeCardBack();
    }
    break;
#endif
    default:
        hRes = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRes;
}
#ifdef CARD_REJECT_GD_MODE
HRESULT CSPBaseIDC::Fmt_WFSIDCCMSTRING(LPBYTE lpData, DWORD dwLen, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpInfo = static_cast<LPBYTE>(lpData);
        if (lpInfo == nullptr)
            break;

        LPWFSIDCSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(dwLen, lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpInfo, dwLen);
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}
#endif

HRESULT CSPBaseIDC::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
    {
        switch (dwEventID)
        {
        case WFS_EXEE_IDC_INVALIDTRACKDATA:
            hRet = Fmt_WFSIDCTRACKEVENT(lpData, lpResult);
            break;
        case WFS_EXEE_IDC_MEDIARETAINED:
            hRet = WFS_SUCCESS;
            break;
        case WFS_EXEE_IDC_MEDIAINSERTED:
            hRet = WFS_SUCCESS;
            break;
        case WFS_EXEE_IDC_INVALIDMEDIA:
            hRet = WFS_SUCCESS;
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
        case WFS_SRVE_IDC_MEDIADETECTED:
            hRet = Fmt_MEDIADETECTED(lpData, lpResult);
            break;
        case WFS_SRVE_IDC_MEDIAREMOVED:
            hRet = WFS_SUCCESS;
            break;
        case WFS_SRVE_IDC_CARDACTION:
            hRet = Fmt_WFSIDCCARDACT(lpData, lpResult);
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
        case WFS_USRE_IDC_RETAINBINTHRESHOLD:
            hRet = Fmt_RETAINBINTHRESHOLD(lpData, lpResult);
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

HRESULT CSPBaseIDC::Fmt_WFSIDCSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSIDCSTATUS>(lpData);
        if (lpStatus == nullptr)
            break;

        LPWFSIDCSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSIDCSTATUS));
        lpNewData->lpszExtra = nullptr;
        if (lpStatus->lpszExtra != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpStatus->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSIDCCAPS>(lpData);
        if (lpCaps == nullptr)
            break;

        LPWFSIDCCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSIDCCAPS));
        lpNewData->lpszExtra = nullptr;

        // 有扩展状态
        if (lpCaps->lpszExtra != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpCaps->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCFORM(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpFormList = static_cast<LPWFSIDCFORM>(lpData);
        if (lpFormList == nullptr)
            break;

        LPWFSIDCFORM lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCFORM), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpFormList, sizeof(WFSIDCFORM));
        lpNewData->lpszFormName = nullptr;
        if (lpFormList->lpszFormName != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(strlen(lpFormList->lpszFormName) + 1, lpResult, (LPVOID *)&lpNewData->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpszFormName);
            memcpy(lpNewData->lpszFormName, lpFormList->lpszFormName, strlen(lpFormList->lpszFormName) + 1);
        }

        if (lpFormList->lpszTracks != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(strlen(lpFormList->lpszTracks) + 1, lpResult, (LPVOID *)&lpNewData->lpszTracks);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpszTracks);
            memcpy(lpNewData->lpszFormName, lpFormList->lpszTracks, strlen(lpFormList->lpszTracks) + 1);
        }

        if (lpFormList->lpszTrack1Fields != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszTrack1Fields, lpFormList->lpszTrack1Fields);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszTrack1Fields);
        }

        if (lpFormList->lpszTrack2Fields != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszTrack2Fields, lpFormList->lpszTrack2Fields);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszTrack2Fields);
        }

        if (lpFormList->lpszTrack3Fields != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszTrack3Fields, lpFormList->lpszTrack3Fields);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszTrack3Fields);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCRETAINCARD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCardRetain = static_cast<LPWFSIDCRETAINCARD>(lpData);
        if (lpCardRetain == nullptr)
            break;

        LPWFSIDCRETAINCARD lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCRETAINCARD), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCardRetain, sizeof(WFSIDCRETAINCARD));

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCCARDDATAARY(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    LPWFSIDCCARDDATA *lppPos = (LPWFSIDCCARDDATA *)lpData;
    while (*lppPos != nullptr)
    {
        lppPos++;
    }
    USHORT usCount = lppPos - (LPWFSIDCCARDDATA *)lpData;
    if (usCount == 0)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    do
    {
        auto lppCardData = static_cast<LPWFSIDCCARDDATA *>(lpData);
        if (lppCardData == nullptr)
            break;

        LPWFSIDCCARDDATA *lppNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSIDCCARDDATA) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0, sizeof(LPWFSIDCCARDDATA) * (usCount + 1));

        for (int i = 0; i < usCount; i++)
        {
            LPWFSIDCCARDDATA lpNewCardData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCCARDDATA), lpResult, (LPVOID *)&lpNewCardData);
            if (hRet != WFS_SUCCESS)
                return hRet;

            _auto.push_back(lpNewCardData);
            memcpy(lpNewCardData, lppCardData[i], sizeof(WFSIDCCARDDATA));

            if (lppCardData[i]->ulDataLength > 0)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lppCardData[i]->ulDataLength, lpResult, (LPVOID *)&lpNewCardData->lpbData);
                if (hRet != WFS_SUCCESS)
                    return hRet;

                _auto.push_back(lpNewCardData->lpbData);
                memcpy(lpNewCardData->lpbData, lppCardData[i]->lpbData, sizeof(BYTE) * lppCardData[i]->ulDataLength);
            }
            lppNewData[i] = lpNewCardData;
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCCHIPIO(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpChipIO = static_cast<LPWFSIDCCHIPIO>(lpData);
        if (lpChipIO == nullptr)
            break;

        LPWFSIDCCHIPIO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCCHIPIO), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpChipIO, sizeof(WFSIDCCHIPIO));

        if (lpChipIO->ulChipDataLength > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpChipIO->ulChipDataLength, lpResult, (LPVOID *)&lpNewData->lpbChipData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbChipData);
            memcpy(lpNewData->lpbChipData, lpChipIO->lpbChipData, sizeof(BYTE) * lpChipIO->ulChipDataLength);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCCHIPPOWEROUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpData == nullptr)
    {
        lpResult = nullptr;
        return WFS_SUCCESS;
    }

    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpChipPowerOut = static_cast<LPWFSIDCCHIPPOWEROUT>(lpData);
        if (lpChipPowerOut == nullptr)
            break;

        LPWFSIDCCHIPPOWEROUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCCHIPPOWEROUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpChipPowerOut, sizeof(WFSIDCCHIPPOWEROUT));

        if ((lpChipPowerOut->ulChipDataLength > 0) && (lpChipPowerOut->lpbChipData != nullptr))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpChipPowerOut->ulChipDataLength, lpResult, (LPVOID *)&lpNewData->lpbChipData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbChipData);
            memcpy(lpNewData->lpbChipData, lpChipPowerOut->lpbChipData, sizeof(BYTE) * lpChipPowerOut->ulChipDataLength);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCFORMLIST(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto lpFormList = static_cast<LPSTR>(lpData);
        if (lpFormList == nullptr)
            break;

        size_t usSize = strlen(lpFormList) + 1;
        LPSTR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        memcpy(lpNewData, lpFormList, sizeof(char) * usSize);
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpEvent = static_cast<LPWFSIDCTRACKEVENT>(lpData);
        if (lpEvent == nullptr)
            break;

        LPWFSIDCTRACKEVENT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCTRACKEVENT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpEvent, sizeof(WFSIDCTRACKEVENT));
        if (lpEvent->lpstrTrack != nullptr)
        {
            size_t uLen = strlen(lpEvent->lpstrTrack);
            LPSTR lpNew = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNew);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNew);
            memcpy(lpNew, lpEvent->lpstrTrack, sizeof(char) * uLen);
            lpNewData->lpstrTrack = lpNew;
            lpNew = nullptr;

            uLen = strlen(lpEvent->lpstrData);
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNew);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNew);
            memcpy(lpNew, lpEvent->lpstrData, sizeof(char) * uLen);
            lpNewData->lpstrData = lpNew;
            lpNew = nullptr;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseIDC::Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto lpwResetOut = static_cast<LPWORD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        LPWORD lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        *lpNewData = *lpwResetOut;
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseIDC::Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto lpfwRetainBin = static_cast<LPWORD>(lpData);
        if (lpfwRetainBin == nullptr)
            break;

        LPWORD lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        *lpNewData = *lpfwRetainBin;
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseIDC::Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto lpAct = static_cast<LPWFSIDCCARDACT>(lpData);
        if (lpAct == nullptr)
            break;

        LPWFSIDCCARDACT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSIDCCARDACT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        memcpy(lpNewData, lpAct, sizeof(WFSIDCCARDACT));
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}
