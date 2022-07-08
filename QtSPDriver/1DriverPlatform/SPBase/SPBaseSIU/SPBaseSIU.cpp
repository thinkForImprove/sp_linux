#include "SPBaseSIU.h"

static const char *ThisFile = "SPBaseSIU.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" SPBASESIU_EXPORT long CreateISPBaseSIU(LPCSTR lpDevType, ISPBaseSIU *&p)
{
    p = new CSPBaseSIU(lpDevType);
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CSPBaseSIU::CSPBaseSIU(LPCSTR lpLogType) : m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
}

CSPBaseSIU::~CSPBaseSIU()
{
}

void CSPBaseSIU::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

void CSPBaseSIU::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBaseSIU::StartRun()
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

void CSPBaseSIU::GetSPBaseData(SPBASEDATA &stData)
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

bool CSPBaseSIU::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
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

bool CSPBaseSIU::FireStatusChanged(DWORD dwStatus)
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

bool CSPBaseSIU::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
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

HRESULT CSPBaseSIU::OnOpen(LPCSTR lpLogicalName)
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

HRESULT CSPBaseSIU::OnClose()
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

HRESULT CSPBaseSIU::OnStatus()
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

HRESULT CSPBaseSIU::OnWaitTaken()
{
    //因为没有要等待取走什么的，直接返回取消
    return WFS_ERR_CANCELED;
}

HRESULT CSPBaseSIU::OnCancelAsyncRequest()
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

HRESULT CSPBaseSIU::OnUpdateDevPDL()
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


HRESULT CSPBaseSIU::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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
    case WFS_INF_SIU_STATUS:
        {
            LPWFSSIUSTATUS lpStatus = nullptr;
            hRet = m_pCmdFunc->GetStatus(lpStatus);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSSIUSTATUS(lpStatus, lpResult);
        }
        break;
    case WFS_INF_SIU_CAPABILITIES:
        {
            LPWFSSIUCAPS lpCaps = nullptr;
            hRet = m_pCmdFunc->GetCapabilities(lpCaps);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSSIUCAPS(lpCaps, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBaseSIU::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
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
    case WFS_CMD_SIU_ENABLE_EVENTS:
        {
            auto lpEnable = static_cast<LPWFSSIUENABLE>(lpCmdData);
            if (lpEnable == nullptr)
                break;

            hRet = m_pCmdFunc->SetEnableEvent(lpEnable);
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_SIU_SET_DOOR:
        {
            auto lpData = static_cast<LPWFSSIUSETDOOR>(lpCmdData);
            if (lpData == nullptr)
                break;

            hRet = m_pCmdFunc->SetDoor(lpData);
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_SIU_SET_INDICATOR:
        {
            auto lpData = static_cast<LPWFSSIUSETINDICATOR>(lpCmdData);
            if (lpData == nullptr)
                break;

            hRet = m_pCmdFunc->SetIndicator(lpData);
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_SIU_SET_GUIDLIGHT:
        {
            auto lpData = static_cast<LPWFSSIUSETGUIDLIGHT>(lpCmdData);
            if (lpData == nullptr)
                break;

            hRet = m_pCmdFunc->SetGuidLight(lpData);
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_SIU_RESET:
        {
            hRet = m_pCmdFunc->Reset();
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRet;
}

HRESULT CSPBaseSIU::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
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
            case WFS_EXEE_SIU_PORT_ERROR:
                hRet = Fmt_WFSSIUPORTERROR(lpData, lpResult);
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
            case WFS_SRVE_SIU_PORT_STATUS:
                hRet = Fmt_WFSSIUPORTEVENT(lpData, lpResult);
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

HRESULT CSPBaseSIU::Fmt_WFSSIUPORTEVENT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto pEvent = static_cast<LPWFSSIUPORTEVENT>(lpData);
        if (pEvent == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }

        LPWFSSIUPORTEVENT lpNew = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSSIUPORTEVENT), lpResult, (LPVOID *)&lpNew);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }

        memcpy(lpNew, pEvent, sizeof(WFSSIUPORTEVENT));
        lpNew->lpszExtra = nullptr;// 暂时不支持扩展状态
        lpResult->lpBuffer = lpNew;
        lpNew = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseSIU::Fmt_WFSSIUPORTERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto pEvent = static_cast<LPWFSSIUPORTERROR>(lpData);
        if (pEvent == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }

        LPWFSSIUPORTERROR lpNew = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSSIUPORTERROR), lpResult, (LPVOID *)&lpNew);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }

        memcpy(lpNew, pEvent, sizeof(WFSSIUPORTERROR));
        lpNew->lpszExtra = nullptr;// 暂时不支持扩展状态
        lpResult->lpBuffer = lpNew;
        lpNew = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBaseSIU::Fmt_WFSSIUSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSSIUSTATUS>(lpData);
        if (lpStatus == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }

        LPWFSSIUSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSSIUSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSSIUSTATUS));
        lpNewData->lpszExtra = nullptr;
        if (lpStatus->lpszExtra != nullptr)   // 有扩展状态
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

HRESULT CSPBaseSIU::Fmt_WFSSIUCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSSIUCAPS>(lpData);
        if (lpCaps == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }

        LPWFSSIUCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSSIUCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSSIUCAPS));
        lpNewData->lpszExtra = nullptr;
        if (lpCaps->lpszExtra != nullptr)   // 有扩展状态
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

HRESULT CSPBaseSIU::Fmt_NODATA(LPWFSRESULT &lpResult)
{
    lpResult->lpBuffer = nullptr;
    return WFS_SUCCESS;
}
