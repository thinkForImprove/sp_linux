#include "SPBaseCAM.h"

static const char *ThisFile = "SPBaseCAM.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateISPBaseCAM(LPCSTR lpDevType, ISPBaseCAM *&p)
{
    p = new CSPBaseCAM(lpDevType);
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CSPBaseCAM::CSPBaseCAM(LPCSTR lpLogType) : m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
}

CSPBaseCAM::~CSPBaseCAM()
{
}

void CSPBaseCAM::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

void CSPBaseCAM::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBaseCAM::StartRun()
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

void CSPBaseCAM::GetSPBaseData(SPBASEDATA &stData)
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

bool CSPBaseCAM::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
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

bool CSPBaseCAM::FireStatusChanged(DWORD dwStatus)
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

bool CSPBaseCAM::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
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

HRESULT CSPBaseCAM::OnOpen(LPCSTR lpLogicalName)
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

HRESULT CSPBaseCAM::OnClose()
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

HRESULT CSPBaseCAM::OnStatus()
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

HRESULT CSPBaseCAM::OnWaitTaken()
{
    //因为没有要等待取走什么的，直接返回取消
    return WFS_ERR_CANCELED;
}

HRESULT CSPBaseCAM::OnCancelAsyncRequest()
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

HRESULT CSPBaseCAM::OnUpdateDevPDL()
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


HRESULT CSPBaseCAM::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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
    case WFS_INF_CAM_STATUS:
        {
            lpQueryDetails = nullptr;
            LPWFSCAMSTATUS lpstStatus = nullptr;
            hRet = m_pCmdFunc->GetStatus(lpstStatus);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSCAMSTATUS(lpstStatus, lpResult);
        }
        break;
    case WFS_INF_CAM_CAPABILITIES:
        {
            LPWFSCAMCAPS lpstCaps = nullptr;
            hRet = m_pCmdFunc->GetCapabilities(lpstCaps);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSCAMCAPS(lpstCaps, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBaseCAM::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
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
    case WFS_CMD_CAM_DISPLAY:
        {
            auto lpDispInput = static_cast<LPWFSCAMDISP>(lpCmdData);
            if (lpDispInput == nullptr)
                break;
            hRet = m_pCmdFunc->Display(*lpDispInput, dwTimeOut);
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_CAM_TAKE_PICTURE:
        {
            auto lpTakePictExInput = static_cast<LPWFSCAMTAKEPICT>(lpCmdData);
            if (lpTakePictExInput == nullptr)
                break;

            hRet = m_pCmdFunc->TakePicture(*lpTakePictExInput, dwTimeOut);
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_CAM_TAKE_PICTURE_EX:
        {
            auto lpTakePictExInput = static_cast<LPWFSCAMTAKEPICTEX>(lpCmdData);
            if (lpTakePictExInput == nullptr)
                break;

            hRet = m_pCmdFunc->TakePictureEx(*lpTakePictExInput, dwTimeOut);
            if (hRet != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_CAM_RESET:
        hRet = m_pCmdFunc->Reset();
        break;
    default:
        hRet = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRet;
}

HRESULT CSPBaseCAM::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CSPBaseCAM::Fmt_WFSCAMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSCAMSTATUS>(lpData);
        if (lpStatus == nullptr)
            break;

        LPWFSCAMSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCAMSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSCAMSTATUS));
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

HRESULT CSPBaseCAM::Fmt_WFSCAMCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSCAMCAPS>(lpData);
        if (lpCaps == nullptr)
            break;

        LPWFSCAMCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCAMCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSCAMCAPS));
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


