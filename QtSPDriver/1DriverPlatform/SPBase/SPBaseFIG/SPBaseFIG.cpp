#include "SPBaseFIG.h"

static const char *ThisFile = "SPBaseFIG.cpp";

//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateISPBaseFIG(LPCSTR lpDevType, ISPBaseFIG *&p)
{
    p = new CSPBaseFIG(lpDevType);
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CSPBaseFIG::CSPBaseFIG(LPCSTR lpLogType) : m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
}

CSPBaseFIG::~CSPBaseFIG()
{
}

void CSPBaseFIG::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

void CSPBaseFIG::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBaseFIG::StartRun()
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

void CSPBaseFIG::GetSPBaseData(SPBASEDATA &stData)
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

bool CSPBaseFIG::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
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

bool CSPBaseFIG::FireStatusChanged(DWORD dwStatus)
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

bool CSPBaseFIG::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
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

HRESULT CSPBaseFIG::OnOpen(LPCSTR lpLogicalName)
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

HRESULT CSPBaseFIG::OnClose()
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

HRESULT CSPBaseFIG::OnStatus()
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

HRESULT CSPBaseFIG::OnWaitTaken()
{
    //因为没有要等待取走什么的，直接返回取消
    return WFS_ERR_CANCELED;
}

HRESULT CSPBaseFIG::OnCancelAsyncRequest()
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

HRESULT CSPBaseFIG::OnUpdateDevPDL()
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

HRESULT CSPBaseFIG::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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
    case WFS_INF_PTR_STATUS:
        {
            LPWFSPTRSTATUS lpstStatus = nullptr;
            hRet                      = m_pCmdFunc->GetStatus(lpstStatus);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPTRSTATUS(lpstStatus, lpResult);
        }
        break;
    case WFS_INF_PTR_CAPABILITIES:
        {
            LPWFSPTRCAPS lpstCaps = nullptr;
            hRet                  = m_pCmdFunc->GetCapabilities(lpstCaps);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPTRCAPS(lpstCaps, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBaseFIG::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
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
    case WFS_CMD_PTR_READ_IMAGE:
        {
            auto lpImgRequest = static_cast<LPWFSPTRIMAGEREQUEST>(lpCmdData);
            if (lpImgRequest == nullptr)
                break;

            LPWFSPTRIMAGE *lppImage = nullptr;
            hRes                    = m_pCmdFunc->ReadImage(lpImgRequest, lppImage, dwTimeOut);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPTRIMAGE(lppImage, lpResult);
        }
        break;
    case WFS_CMD_PTR_RESET:
        {
            auto lpReset = static_cast<LPWFSPTRRESET>(lpCmdData);
            hRes         = m_pCmdFunc->Reset(lpReset);
            if (hRes != WFS_SUCCESS)
                break;
        }
        break;
    default:
        hRes = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRes;
}

HRESULT CSPBaseFIG::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        switch (dwEventID)
        {
        case WFS_EXEE_PTR_MEDIAINSERTED:
            hRet = Fmt_NODATA(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_NOMEDIA:
            hRet = Fmt_WFS_PTR_NOMEDIA(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_FIELDERROR:
            hRet = Fmt_WFS_EXEE_PTR_FIELDERROR(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_FIELDWARNING:
            hRet = Fmt_WFS_EXEE_PTR_FIELDERROR(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_MEDIAPRESENTED:
            hRet = Fmt_WFSMEDIAPRESENTED(lpData, lpResult);
            break;
        default:
            break;
        }
        break;
    case WFS_SERVICE_EVENT:
        switch (dwEventID)
        {
        case WFS_SRVE_PTR_MEDIATAKEN:
        case WFS_SRVE_PTR_MEDIAINSERTED:
            hRet = Fmt_NODATA(lpData, lpResult);
            break;
        case WFS_SRVE_PTR_MEDIADETECTED:
            hRet = Fmt_WFS_SRVE_PTR_MEDIADETECTED(lpData, lpResult);
            break;
        default:
            break;
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

HRESULT CSPBaseFIG::Fmt_WFSPTRSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSPTRSTATUS>(lpData);
        if (lpStatus == nullptr)
            break;

        LPWFSPTRSTATUS lpNewData = nullptr;
        hRet                     = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSPTRSTATUS));
        lpNewData->lpszExtra = nullptr;
        if (lpStatus->lpszExtra != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpStatus->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
        }

        lpNewData->lppRetractBins = nullptr;
        if (lpStatus->lppRetractBins != nullptr)
        {
            USHORT usBinCount = 0;
            while (true)
            {
                if (lpStatus->lppRetractBins[usBinCount] == nullptr)
                    break;
                usBinCount++;
            }

            if (usBinCount > 0)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPTRRETRACTBINS) * (usBinCount + 1), lpResult, (LPVOID *)&lpNewData->lppRetractBins);
                if (hRet != WFS_SUCCESS)
                    return hRet;

                _auto.push_back(lpNewData->lppRetractBins);
                for (int i = 0; i < (usBinCount + 1); i++)
                {
                    lpNewData->lppRetractBins[i] = nullptr;
                    if (lpStatus->lppRetractBins[i] != nullptr)
                    {
                        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRRETRACTBINS), lpResult, (LPVOID *)&lpNewData->lppRetractBins[i]);
                        if (hRet != WFS_SUCCESS)
                            return hRet;
                        memcpy(lpNewData->lppRetractBins[i], lpStatus->lppRetractBins[i], sizeof(WFSPTRRETRACTBINS));
                        _auto.push_back(lpNewData->lppRetractBins[i]);
                    }
                }
            }
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseFIG::Fmt_WFSPTRCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSPTRCAPS>(lpData);
        if (lpCaps == nullptr)
            break;

        LPWFSPTRCAPS lpNewData = nullptr;
        hRet                   = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSPTRCAPS));

        if (lpCaps->lpusMaxRetract != nullptr)
        {
            WORD wCount = 0;
            while (true)
            {
                if (lpCaps->lpusMaxRetract[wCount++] == 0)
                {
                    LPWORD lpSymb = nullptr;
                    hRet          = m_pIWFM->WFMAllocateMore(sizeof(USHORT) * wCount, lpResult, (LPVOID *)&lpSymb);
                    if (hRet != WFS_SUCCESS)
                    {
                        lpNewData->lpusMaxRetract = nullptr;
                        break;
                    }
                    _auto.push_back(lpSymb);
                    memcpy(lpSymb, lpCaps->lpusMaxRetract, sizeof(USHORT) * wCount);
                    lpNewData->lpusMaxRetract = lpSymb;
                    lpSymb                    = nullptr;
                    break;
                }
            }
        }

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
        lpNewData          = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseFIG::Fmt_WFSPTRIMAGE(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lppImageData = static_cast<LPWFSPTRIMAGE *>(lpData);
        if (lppImageData == nullptr)
            return WFS_ERR_INTERNAL_ERROR;

        USHORT usCount = 0;
        while (true)
        {
            if (lppImageData[usCount] == 0)
                break;
            usCount++;
        }

        if (usCount <= 0)
            return WFS_ERR_INTERNAL_ERROR;

        LPWFSPTRIMAGE *lppNewData = nullptr;
        hRet                      = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPTRIMAGE) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0x00, sizeof(LPWFSPTRIMAGE) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            LPWFSPTRIMAGE lpImage = nullptr;
            hRet                  = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRIMAGE), lpResult, (LPVOID *)&lpImage);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpImage);
            memcpy(lpImage, lppImageData[i], sizeof(WFSPTRIMAGE));
            lpImage->lpbData = nullptr;
            if ((lppImageData[i]->lpbData != nullptr) && (lppImageData[i]->ulDataLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lppImageData[i]->ulDataLength, lpResult, (LPVOID *)&lpImage->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpImage->lpbData);
                memcpy(lpImage->lpbData, lppImageData[i]->lpbData, sizeof(BYTE) * lppImageData[i]->ulDataLength);
            }
            lppNewData[i] = lpImage;
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData         = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseFIG::Fmt_WFSMEDIAPRESENTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpWfsPtrMediaPresented = static_cast<LPWFSPTRMEDIAPRESENTED>(lpData);
        if (lpWfsPtrMediaPresented == nullptr)
            break;

        LPWFSPTRMEDIAPRESENTED lpNewData = nullptr;
        hRet                             = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRMEDIAPRESENTED), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        lpNewData->usWadIndex  = lpWfsPtrMediaPresented->usWadIndex;
        lpNewData->usTotalWads = lpWfsPtrMediaPresented->usTotalWads;
        lpResult->lpBuffer     = lpNewData;
        lpNewData              = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseFIG::Fmt_WFS_EXEE_PTR_FIELDERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRFIELDFAIL>(lpData);
        if (lpwResetOut == nullptr)
            break;

        LPWFSPTRFIELDFAIL lpNewData = nullptr;
        hRet                        = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRFIELDFAIL), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpwResetOut, sizeof(WFSPTRFIELDFAIL));

        if (lpwResetOut->lpszFieldName != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(strlen(lpwResetOut->lpszFieldName) + 1, lpResult, (LPVOID *)&lpNewData->lpszFieldName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFieldName);
            memcpy(lpNewData->lpszFieldName, lpwResetOut->lpszFieldName, strlen(lpwResetOut->lpszFieldName) + 1);
        }

        if (lpwResetOut->lpszFormName != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(strlen(lpwResetOut->lpszFormName) + 1, lpResult, (LPVOID *)&lpNewData->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFormName);
            memcpy(lpNewData->lpszFormName, lpwResetOut->lpszFormName, strlen(lpwResetOut->lpszFormName) + 1);
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseFIG::Fmt_WFS_SRVE_PTR_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;

    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRMEDIADETECTED>(lpData);
        if (lpwResetOut == nullptr)
            break;

        LPWFSPTRMEDIADETECTED lpNewData = nullptr;
        hRet                            = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRMEDIADETECTED), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpNewData, lpwResetOut, sizeof(WFSPTRMEDIADETECTED));
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBaseFIG::Fmt_NODATA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    lpResult->lpBuffer = nullptr;
    return WFS_SUCCESS;
}

HRESULT CSPBaseFIG::Fmt_WFS_PTR_NOMEDIA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto lpwResetOut = static_cast<LPSTR>(lpData);
        if (lpwResetOut == nullptr)
            break;

        LPSTR lpNewData = nullptr;
        hRet            = m_pIWFM->WFMAllocateMore(strlen(lpwResetOut) + 1, lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpNewData, lpwResetOut, strlen(lpwResetOut) + 1);
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    Log(ThisModule, __LINE__, "Fmt_WFS_PTR_NOMEDIA -----------:hRet=%d", hRet);

    return hRet;
}
