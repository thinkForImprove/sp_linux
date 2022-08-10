#include "SPBaseBCR.h"

static const char *ThisFile = "SPBaseBCR.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateISPBaseBCR(LPCSTR lpDevType, ISPBaseBCR *&p)
{
    p = new CSPBaseBCR(lpDevType);
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CSPBaseBCR::CSPBaseBCR(LPCSTR lpLogType) : m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
}

CSPBaseBCR::~CSPBaseBCR()
{
}

void CSPBaseBCR::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

void CSPBaseBCR::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBaseBCR::StartRun()
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

void CSPBaseBCR::GetSPBaseData(SPBASEDATA &stData)
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

bool CSPBaseBCR::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
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

bool CSPBaseBCR::FireStatusChanged(DWORD dwStatus)
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

bool CSPBaseBCR::FireBarCodePosition(WORD wPosition)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }
    return m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_BCR_DEVICEPOSITION, (void *)wPosition);
}

bool CSPBaseBCR::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
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
        stStatus.dwSize = strlen((char *)lpDescription);
        stStatus.lpbDescription = (LPBYTE)lpDescription;
    }
    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_HARDWARE_ERROR, &stStatus);
}

HRESULT CSPBaseBCR::OnOpen(LPCSTR lpLogicalName)
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

HRESULT CSPBaseBCR::OnClose()
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

HRESULT CSPBaseBCR::OnStatus()
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

HRESULT CSPBaseBCR::OnWaitTaken()
{
    //因为没有要等待取走什么的，直接返回取消
    return WFS_ERR_CANCELED;
}

HRESULT CSPBaseBCR::OnCancelAsyncRequest()
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

HRESULT CSPBaseBCR::OnUpdateDevPDL()
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


HRESULT CSPBaseBCR::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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
    case WFS_INF_BCR_STATUS:
        {
            lpQueryDetails = nullptr;
            LPWFSBCRSTATUS lpstStatus = nullptr;
            hRet = m_pCmdFunc->GetStatus(lpstStatus);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSBCRSTATUS(lpstStatus, lpResult);
        }
        break;
    case WFS_INF_BCR_CAPABILITIES:
        {
            LPWFSBCRCAPS lpstCaps = nullptr;
            hRet = m_pCmdFunc->GetCapabilities(lpstCaps);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSBCRCAPS(lpstCaps, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBaseBCR::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCommand)
    {
    case WFS_CMD_BCR_READ:
        {
            auto lpReadInput = static_cast<LPWFSBCRREADINPUT>(lpCmdData);
            if (lpReadInput == nullptr)
                break;

            LPWFSBCRREADOUTPUT *lppReadOutput = nullptr;
            hRet = m_pCmdFunc->ReadBCR(*lpReadInput, lppReadOutput, dwTimeOut);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSBCRREADOUTPUT(lppReadOutput, lpResult);
        }
        break;
    case WFS_CMD_BCR_RESET:
        hRet = m_pCmdFunc->Reset();
        break;
    case WFS_CMD_BCR_SET_GUIDANCE_LIGHT:
        {
            auto lpLight = static_cast<LPWFSBCRSETGUIDLIGHT>(lpCmdData);
            if (lpLight == nullptr)
                break;
            hRet = m_pCmdFunc->SetGuidLight(*lpLight);
        }
        break;
    case WFS_CMD_BCR_POWER_SAVE_CONTROL:
        {
            auto lpPower = static_cast<LPWFSBCRPOWERSAVECONTROL>(lpCmdData);
            if (lpPower == nullptr)
                break;
            hRet = m_pCmdFunc->PowerSaveControl(*lpPower);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRet;
}

HRESULT CSPBaseBCR::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        break;
    case WFS_SERVICE_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_SRVE_BCR_DEVICEPOSITION:
                hRet = Fmt_WFSBCRPOSITION(lpData, lpResult);
                break;
            default:
                break;
            }
        }
        break;
    case WFS_USER_EVENT:
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

HRESULT CSPBaseBCR::Fmt_WFSBCRSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSBCRSTATUS>(lpData);
        if (lpStatus == nullptr)
            break;

        LPWFSBCRSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSBCRSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSBCRSTATUS));
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

HRESULT CSPBaseBCR::Fmt_WFSBCRCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSBCRCAPS>(lpData);
        if (lpCaps == nullptr)
            break;

        LPWFSBCRCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSBCRCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSBCRCAPS));
        lpNewData->lpszExtra = nullptr;
        if (lpCaps->lpwSymbologies != nullptr)
        {
            WORD wCount = 0;
            while (true)
            {
                if (lpCaps->lpwSymbologies[wCount++] == 0)
                {
                    LPWORD lpSymb = nullptr;
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(WORD) * wCount, lpResult, (LPVOID *)&lpSymb);
                    if (hRet != WFS_SUCCESS)
                    {
                        lpNewData->lpwSymbologies = nullptr;
                        break;
                    }
                    _auto.push_back(lpSymb);
                    memcpy(lpSymb, lpCaps->lpwSymbologies, sizeof(WORD) * wCount);
                    lpNewData->lpwSymbologies = lpSymb;
                    lpSymb = nullptr;
                    break;
                }
            }
        }
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

HRESULT CSPBaseBCR::Fmt_WFSBCRREADOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    DWORD dwSize = 0;
    auto lppOutData = static_cast<LPWFSBCRREADOUTPUT *>(lpData);
    while (true)
    {
        if (lppOutData[dwSize] == nullptr)
            break;
        dwSize++;
    }
    if (dwSize == 0)
    {
        return WFS_ERR_INVALID_POINTER;
    }

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSBCRREADOUTPUT *lppOutput = nullptr;
    LPWFSBCRREADOUTPUT lpNewOutput = nullptr;
    LPWFSBCRXDATA lpbData = nullptr;
    do
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSBCRREADOUTPUT) * (dwSize + 1), lpResult, (LPVOID *)&lppOutput);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lppOutput);
        memset(lppOutput, 0x00, sizeof(LPWFSBCRREADOUTPUT) * (dwSize + 1));

        for (DWORD i = 0; i < dwSize; i++)
        {
            lpNewOutput = nullptr;
            lpbData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSBCRREADOUTPUT), lpResult, (LPVOID *)&lpNewOutput);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewOutput);
            auto pOut = lppOutData[i];
            memcpy(lpNewOutput, pOut, sizeof(WFSBCRREADOUTPUT));

            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSBCRXDATA), lpResult, (LPVOID *)&lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpbData);
            DWORD dwNewSize = pOut->lpxBarcodeData->usLength * sizeof(BYTE);
            hRet = m_pIWFM->WFMAllocateMore(dwNewSize, lpResult, (LPVOID *)&lpbData->lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpbData->lpbData);
            lpbData->usLength = pOut->lpxBarcodeData->usLength;
            memcpy(lpbData->lpbData, pOut->lpxBarcodeData->lpbData, lpbData->usLength);
            lpNewOutput->lpxBarcodeData = lpbData;
            lppOutput[i] = lpNewOutput;
        }

        // 赋值
        if (hRet == WFS_SUCCESS)
        {
            lpResult->lpBuffer = lppOutput;
            lppOutput = nullptr;
        }
    } while (false);

    return hRet;
}

HRESULT CSPBaseBCR::Fmt_WFSBCRPOSITION(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    WORD wPosition = (WORD)(ulong)lpData;
    LPWFSBCRDEVICEPOSITION lpOutput = nullptr;
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSBCRDEVICEPOSITION), lpResult, (LPVOID *)&lpOutput);
        if (hRet != WFS_SUCCESS)
            break;
        lpOutput->wPosition = wPosition;
        lpResult->lpBuffer  = lpOutput;
    } while (false);
    return hRet;
}
