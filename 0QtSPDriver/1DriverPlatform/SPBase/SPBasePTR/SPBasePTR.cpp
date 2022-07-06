#include "SPBasePTR.h"

static const char *ThisFile = "SPBasePTR.cpp";

//////////////////////////////////////////////////////////////////////////
extern "C" SPBASEPTRSHARED_EXPORT long CreateISPBasePTR(LPCSTR lpDevType, ISPBasePTR *&p)
{
    p = new CSPBasePTR(lpDevType);
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CSPBasePTR::CSPBasePTR(LPCSTR lpLogType) : m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
}

CSPBasePTR::~CSPBasePTR()
{
}

void CSPBasePTR::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

void CSPBasePTR::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBasePTR::StartRun()
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

void CSPBasePTR::GetSPBaseData(SPBASEDATA &stData)
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

bool CSPBasePTR::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
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

bool CSPBasePTR::FireStatusChanged(DWORD dwStatus)
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

bool CSPBasePTR::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
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

HRESULT CSPBasePTR::OnOpen(LPCSTR lpLogicalName)
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

HRESULT CSPBasePTR::OnClose()
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

HRESULT CSPBasePTR::OnStatus()
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

HRESULT CSPBasePTR::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);									// 30-00-00-00(FT#0048)
    ////AutoLogFuncBeginEnd();									// 30-00-00-00(FT#0048)
    if (m_pCmdFunc == nullptr)									// 30-00-00-00(FT#0048)
    {															// 30-00-00-00(FT#0048)
        MLog(ThisModule, __LINE__, "没有注册回调类ICmdFunc");	// 30-00-00-00(FT#0048)
        return WFS_ERR_INTERNAL_ERROR;							// 30-00-00-00(FT#0048)
    }															// 30-00-00-00(FT#0048)

    return m_pCmdFunc->OnWaitTaken();							// 30-00-00-00(FT#0048)
}

HRESULT CSPBasePTR::OnCancelAsyncRequest()
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

HRESULT CSPBasePTR::OnUpdateDevPDL()
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

HRESULT CSPBasePTR::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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
    case WFS_INF_PTR_FORM_LIST:
        {
            LPSTR lpszFormList = nullptr;
            hRet               = m_pCmdFunc->GetFormList(lpszFormList);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPTRFORMLIST(lpszFormList, lpResult);
        }
        break;
    case WFS_INF_PTR_MEDIA_LIST:
        {
            LPSTR lpszMediaList = nullptr;
            hRet                = m_pCmdFunc->GetMediaList(lpszMediaList);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPTRMEDIALIST(lpszMediaList, lpResult);
        }
        break;
    case WFS_INF_PTR_QUERY_FORM:
        {
            auto lpFormName = static_cast<LPCSTR>(lpQueryDetails);
            if (lpFormName == nullptr)
                break;

            LPWFSFRMHEADER lpFrmHeader = nullptr;
            hRet                       = m_pCmdFunc->GetQueryForm(lpFormName, lpFrmHeader);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSFRMHEADER(lpFrmHeader, lpResult);
        }
        break;
    case WFS_INF_PTR_QUERY_MEDIA:
        {
            auto lpMediaName = static_cast<LPCSTR>(lpQueryDetails);
            if (lpMediaName == nullptr)
                break;

            LPWFSFRMMEDIA lpFrmMeida = nullptr;
            hRet                     = m_pCmdFunc->GetQueryMeida(lpMediaName, lpFrmMeida);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSFRMMEIDA(lpFrmMeida, lpResult);
        }
        break;
    case WFS_INF_PTR_QUERY_FIELD:
        {
            auto lpQueryField = static_cast<LPWFSPTRQUERYFIELD>(lpQueryDetails);
            if (lpQueryField == nullptr)
                break;

            LPWFSFRMFIELD *lppFrmFiled = nullptr;
            hRet                       = m_pCmdFunc->GetQueryField(lpQueryField, lppFrmFiled);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSFRMFIELD(lppFrmFiled, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBasePTR::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
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
    case WFS_CMD_PTR_CONTROL_MEDIA:
        {
            auto lpdwMeidaControl = static_cast<LPDWORD>(lpCmdData);
            if (lpdwMeidaControl == nullptr)
                break;

            hRes = m_pCmdFunc->MediaControl(lpdwMeidaControl);
            if (hRes != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_PTR_PRINT_FORM:
        {
            auto lpPrintForm = static_cast<LPWFSPTRPRINTFORM>(lpCmdData);
            if (lpPrintForm == nullptr)
                break;

            hRes = m_pCmdFunc->PrintForm(lpPrintForm, dwTimeOut);
            if (hRes != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_PTR_READ_FORM:
        {
            auto lpReadForm = static_cast<LPWFSPTRREADFORM>(lpCmdData);
            if (lpReadForm == nullptr)
                break;

            LPWFSPTRREADFORMOUT lpReadFormOut = nullptr;
            hRes                              = m_pCmdFunc->ReadForm(lpReadForm, lpReadFormOut, dwTimeOut);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = Fmt_WFSPTRREADFORMOUT(lpReadFormOut, lpResult);
        }
        break;
    case WFS_CMD_PTR_RAW_DATA:
        {
            auto lpRawData = static_cast<LPWFSPTRRAWDATA>(lpCmdData);
            if (lpRawData == nullptr)
                break;

            LPWFSPTRRAWDATAIN lpRawDataIn = nullptr;
            hRes                          = m_pCmdFunc->RawData(lpRawData, lpRawDataIn, dwTimeOut);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = Fmt_WFSPTRRAWDATAIN(lpRawDataIn, lpResult);
        }
        break;
    case WFS_CMD_PTR_MEDIA_EXTENTS:
        {
            auto lpMediaUnit = static_cast<LPWFSPTRMEDIAUNIT>(lpCmdData);
            if (lpMediaUnit == nullptr)
                break;

            LPWFSPTRMEDIAEXT lpMediaExt = nullptr;
            hRes                        = m_pCmdFunc->MediaExtents(lpMediaUnit, lpMediaExt);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPTRMEDIAEXT(lpMediaExt, lpResult);
        }
        break;
    case WFS_CMD_PTR_RESET_COUNT:
        {
            auto lpusBinNum = static_cast<LPUSHORT>(lpCmdData);
            hRes            = m_pCmdFunc->ResetCount(lpusBinNum);
            if (hRes != WFS_SUCCESS)
                break;
        }
        break;
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
    case WFS_CMD_PTR_RETRACT_MEDIA:
        {
            auto lpusBinNum = static_cast<LPUSHORT>(lpCmdData);
            if (lpusBinNum == nullptr)
                break;

            LPUSHORT lpusBinNumOut = nullptr;
            hRes                   = m_pCmdFunc->RetractMedia(lpusBinNum, lpusBinNumOut);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSRETRACTMEDIAOUT(lpusBinNumOut, lpResult);
        }
        break;
    case WFS_CMD_PTR_DISPENSE_PAPER:
        {
            auto lpPaperSource = static_cast<LPWORD>(lpCmdData);
            if (lpPaperSource == nullptr)
                break;
            hRes = m_pCmdFunc->DispensePaper(lpPaperSource);
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

HRESULT CSPBasePTR::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        switch (dwEventID)
        {
        case WFS_EXEE_PTR_NOMEDIA:
            hRet = Fmt_WFS_PTR_NOMEDIA(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_MEDIAINSERTED:
            hRet = Fmt_NODATA(lpData, lpResult);
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
    case WFS_USER_EVENT:
        switch (dwEventID)
        {
        case WFS_USRE_PTR_RETRACTBINTHRESHOLD:
            hRet = Fmt_WFS_USRE_PTR_RETRACTBINTHRESHOLD(lpData, lpResult);
            break;
        case WFS_USRE_PTR_PAPERTHRESHOLD:
            hRet = Fmt_WFS_USRE_PTR_PAPERTHRESHOLD(lpData, lpResult);
            break;
        case WFS_USRE_PTR_TONERTHRESHOLD:
        case WFS_USRE_PTR_INKTHRESHOLD:
        case WFS_USRE_PTR_LAMPTHRESHOLD:
            hRet = Fmt_DATA_WORD(lpData, lpResult);
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

HRESULT CSPBasePTR::Fmt_WFSPTRSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CSPBasePTR::Fmt_WFSPTRCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CSPBasePTR::Fmt_WFSPTRFORMLIST(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    auto lppOutData = static_cast<LPSTR>(lpData);
    if (lppOutData == nullptr)
    {
        return WFS_ERR_INVALID_POINTER;
    }

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    LPSTR lpNewData = nullptr;
    UINT  uSize     = GetLenOfSZZ(lppOutData);
    hRet            = m_pIWFM->WFMAllocateMore(sizeof(char) * uSize, lpResult, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    _auto.push_back(lpNewData);
    memcpy(lpNewData, lppOutData, sizeof(char) * uSize);

    lpResult->lpBuffer = lpNewData;
    lpNewData          = nullptr;

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSPTRMEDIALIST(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    auto lppOutData = static_cast<LPSTR>(lpData);
    if (lppOutData == nullptr)
    {
        return WFS_ERR_INVALID_POINTER;
    }

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    LPSTR lpNewData = nullptr;

    UINT uSize = GetLenOfSZZ(lppOutData);
    hRet       = m_pIWFM->WFMAllocateMore(sizeof(char) * uSize, lpResult, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    _auto.push_back(lpNewData);
    memcpy(lpNewData, lppOutData, sizeof(char) * uSize);

    lpResult->lpBuffer = lpNewData;
    lpNewData          = nullptr;

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSFRMHEADER(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpFrmHeader = static_cast<LPWFSFRMHEADER>(lpData);
        if (lpFrmHeader == nullptr)
            return WFS_ERR_INTERNAL_ERROR;

        LPWFSFRMHEADER lpNewData = nullptr;
        hRet                     = m_pIWFM->WFMAllocateMore(sizeof(WFSFRMHEADER), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpFrmHeader, sizeof(WFSFRMHEADER));

        lpNewData->lpszFormName = nullptr;
        if (lpFrmHeader->lpszFormName != nullptr)
        {
            UINT uSize = strlen(lpFrmHeader->lpszFormName) + 1;
            hRet       = m_pIWFM->WFMAllocateMore(sizeof(char) * uSize, lpResult, (LPVOID *)&lpNewData->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpszFormName);
            memcpy(lpNewData->lpszFormName, lpFrmHeader->lpszFormName, sizeof(char) * uSize);
        }

        lpNewData->lpszUserPrompt = nullptr;
        if (lpFrmHeader->lpszUserPrompt != nullptr)
        {
            UINT uSize = strlen(lpFrmHeader->lpszUserPrompt) + 1;
            hRet       = m_pIWFM->WFMAllocateMore(sizeof(char) * uSize, lpResult, (LPVOID *)&lpNewData->lpszUserPrompt);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpszUserPrompt);
            memcpy(lpNewData->lpszUserPrompt, lpFrmHeader->lpszUserPrompt, sizeof(char) * uSize);
        }

        lpNewData->lpszFields = nullptr;
        if (lpFrmHeader->lpszFields != nullptr)
        {
            UINT uSize = GetLenOfSZZ(lpFrmHeader->lpszFields);
            hRet       = m_pIWFM->WFMAllocateMore(sizeof(char) * uSize, lpResult, (LPVOID *)&lpNewData->lpszFields);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpszFields);
            memcpy(lpNewData->lpszFields, lpFrmHeader->lpszFields, sizeof(char) * uSize);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSFRMMEIDA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpFrmMeida = static_cast<LPWFSFRMMEDIA>(lpData);
        if (lpFrmMeida == nullptr)
            return WFS_ERR_INTERNAL_ERROR;

        LPWFSFRMMEDIA lpNewData = nullptr;
        hRet                    = m_pIWFM->WFMAllocateMore(sizeof(WFSFRMMEDIA), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpFrmMeida, sizeof(WFSFRMMEDIA));
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSFRMFIELD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lppField = static_cast<LPWFSFRMFIELD *>(lpData);
        if (lppField == nullptr)
            return WFS_ERR_INTERNAL_ERROR;

        USHORT usCount = 0;
        while (true)
        {
            if (lppField[usCount] == 0)
                break;
            usCount++;
        }

        if (usCount <= 0)
            return WFS_ERR_INTERNAL_ERROR;

        LPWFSFRMFIELD *lppNewData = nullptr;
        hRet                      = m_pIWFM->WFMAllocateMore(sizeof(LPWFSFRMFIELD) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;
        *(lppNewData + usCount) = nullptr;

        _auto.push_back(lppNewData);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSFRMFIELD lpFrmField = nullptr;
            hRet                     = m_pIWFM->WFMAllocateMore(sizeof(WFSFRMFIELD), lpResult, (LPVOID *)&lpFrmField);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpFrmField);
            memcpy(lpFrmField, lppField[i], sizeof(WFSFRMFIELD));
            lpFrmField->lpszFieldName = nullptr;
            if (lppField[i]->lpszFieldName != nullptr)
            {
                USHORT usSize = strlen(lppField[i]->lpszFieldName) + 1;
                hRet          = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpFrmField->lpszFieldName);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpFrmField->lpszFieldName);
                memcpy(lpFrmField->lpszFieldName, lppField[i]->lpszFieldName, sizeof(char) * usSize);
            }

            lpFrmField->lpszInitialValue = nullptr;
            if (lppField[i]->lpszInitialValue != nullptr)
            {
                USHORT usSize = strlen(lppField[i]->lpszInitialValue) + 1;
                hRet          = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpFrmField->lpszInitialValue);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpFrmField->lpszInitialValue);
                memcpy(lpFrmField->lpszInitialValue, lppField[i]->lpszInitialValue, sizeof(char) * usSize);
            }

            lpFrmField->lpszFormat = nullptr;
            if (lppField[i]->lpszFormat != nullptr)
            {
                USHORT usSize = strlen(lppField[i]->lpszFormat) + 1;
                hRet          = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpFrmField->lpszFormat);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpFrmField->lpszFormat);
                memcpy(lpFrmField->lpszFormat, lppField[i]->lpszFormat, sizeof(char) * usSize);
            }

            lpFrmField->lpszUNICODEInitialValue = nullptr;
            if (lppField[i]->lpszUNICODEInitialValue != nullptr)
            {
                LPSTR  lpUnChar = QString::fromStdWString(lppField[i]->lpszUNICODEInitialValue).toLocal8Bit().data();
                USHORT usSize   = strlen(lpUnChar) + 1;
                hRet            = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpFrmField->lpszUNICODEInitialValue);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpFrmField->lpszUNICODEInitialValue);
                memcpy(lpFrmField->lpszUNICODEInitialValue, lppField[i]->lpszUNICODEInitialValue, sizeof(char) * usSize);
            }

            lpFrmField->lpszUNICODEFormat = nullptr;
            if (lppField[i]->lpszUNICODEFormat != nullptr)
            {
                LPSTR  lpUnChar = QString::fromStdWString(lppField[i]->lpszUNICODEFormat).toLocal8Bit().data();
                USHORT usSize   = strlen(lpUnChar) + 1;
                hRet            = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpFrmField->lpszUNICODEFormat);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpFrmField->lpszUNICODEFormat);
                memcpy(lpFrmField->lpszUNICODEFormat, lppField[i]->lpszUNICODEFormat, sizeof(char) * usSize);
            }

            lppNewData[i] = lpFrmField;
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData         = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSPTRREADFORMOUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpReadFormOut = static_cast<LPWFSPTRREADFORMOUT>(lpData);
        if (lpReadFormOut == nullptr)
            return WFS_ERR_INTERNAL_ERROR;

        LPWFSPTRREADFORMOUT lpNewData = nullptr;
        hRet                          = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRREADFORMOUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpReadFormOut, sizeof(WFSPTRREADFORMOUT));

        lpNewData->lpszFields = nullptr;
        if (lpReadFormOut->lpszFields != nullptr)
        {
            USHORT usSize = GetLenOfSZZ(lpReadFormOut->lpszFields);
            hRet          = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewData->lpszFields);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpszFields);
            memcpy(lpNewData->lpszFields, lpReadFormOut->lpszFields, sizeof(char) * usSize);
        }

        lpNewData->lpszUNICODEFields = nullptr;
        if (lpReadFormOut->lpszUNICODEFields != nullptr)
        {
            LPSTR  lpUnChar = QString::fromStdWString(lpReadFormOut->lpszUNICODEFields).toLocal8Bit().data();
            USHORT usSize   = GetLenOfSZZ(lpUnChar);
            hRet            = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewData->lpszUNICODEFields);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpszUNICODEFields);
            memcpy(lpNewData->lpszUNICODEFields, lpReadFormOut->lpszUNICODEFields, sizeof(char) * usSize);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSPTRRAWDATAIN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpRawDataIn = static_cast<LPWFSPTRRAWDATAIN>(lpData);
        if (lpRawDataIn == nullptr)
        {
            lpResult->lpBuffer = nullptr;
            hRet = WFS_SUCCESS;
            break;
        }

        LPWFSPTRRAWDATAIN lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRRAWDATAIN), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpRawDataIn, sizeof(WFSPTRRAWDATAIN));

        lpNewData->lpbData = nullptr;
        if (lpRawDataIn->lpbData != nullptr && lpRawDataIn->ulSize > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpRawDataIn->ulSize, lpResult, (LPVOID *)&lpNewData->lpbData);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewData->lpbData);
            memcpy(lpNewData->lpbData, lpRawDataIn->lpbData, sizeof(BYTE) * lpRawDataIn->ulSize);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSPTRMEDIAEXT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpMediaExt = static_cast<LPWFSPTRMEDIAEXT>(lpData);
        if (lpMediaExt == nullptr)
            return WFS_ERR_INTERNAL_ERROR;

        LPWFSPTRMEDIAEXT lpNewData = nullptr;
        hRet                       = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRMEDIAEXT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpMediaExt, sizeof(WFSPTRMEDIAEXT));

        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFSPTRIMAGE(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CSPBasePTR::Fmt_WFSRETRACTMEDIAOUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpMediaNum = static_cast<LPUSHORT>(lpData);
        if (lpMediaNum == nullptr)
            return WFS_ERR_INTERNAL_ERROR;

        LPUSHORT lpNewData = nullptr;
        hRet               = m_pIWFM->WFMAllocateMore(sizeof(USHORT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpMediaNum, sizeof(USHORT));

        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFS_PTR_NOMEDIA(LPVOID lpData, LPWFSRESULT &lpResult)
{
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
    return hRet;
}
HRESULT CSPBasePTR::Fmt_WFSMEDIAPRESENTED(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CSPBasePTR::Fmt_WFS_EXEE_PTR_FIELDERROR(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CSPBasePTR::Fmt_WFS_USRE_PTR_RETRACTBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;

    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRBINTHRESHOLD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        LPWFSPTRBINTHRESHOLD lpNewData = nullptr;
        hRet                           = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRBINTHRESHOLD), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpNewData, lpwResetOut, sizeof(WFSPTRBINTHRESHOLD));
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFS_USRE_PTR_PAPERTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;

    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRPAPERTHRESHOLD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        LPWFSPTRPAPERTHRESHOLD lpNewData = nullptr;
        hRet                             = m_pIWFM->WFMAllocateMore(sizeof(WFSPTRPAPERTHRESHOLD), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpNewData, lpwResetOut, sizeof(WFSPTRPAPERTHRESHOLD));
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePTR::Fmt_DATA_WORD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;

    do
    {
        auto lpwResetOut = static_cast<LPWORD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        LPWORD lpNewData = nullptr;
        hRet             = m_pIWFM->WFMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        *lpNewData         = *lpwResetOut;
        lpResult->lpBuffer = lpNewData;
        lpNewData          = nullptr;

    } while (false);
    return hRet;
}

HRESULT CSPBasePTR::Fmt_WFS_SRVE_PTR_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CSPBasePTR::Fmt_NODATA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    lpResult->lpBuffer = nullptr;
    return WFS_SUCCESS;
}
