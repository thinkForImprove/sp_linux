#include "SPBaseVDM.h"

static const char *ThisFile = "SPBaseVDM.cpp";
///////////////////////////////////////////////////////////////////////////////
extern "C" long CreateISPBaseVDM(LPCSTR lpDevType, ISPBaseVDM *&p)
{
    p = new CSPBaseVDM(lpDevType);
    return (p != nullptr) ? 0 : -1;
}

///////////////////////////////////////////////////////////////////////////////
CSPBaseVDM::CSPBaseVDM(LPCSTR lpLogType)
{
    memset(m_szLogType, 0, sizeof(m_szLogType));
    m_pCmdFunc = nullptr;

    if(lpLogType != nullptr){
        strcpy(m_szLogType, lpLogType);
    }
    SetLogFile(LOGFILE, ThisFile, m_szLogType);
}

CSPBaseVDM::~CSPBaseVDM()
{

}

void CSPBaseVDM::Release()
{
    return;
}

void CSPBaseVDM::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBaseVDM::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //加载BAse类
    if(m_pBase.Load("SPBaseClass.dll", "CreateISPBaseClass", m_szLogType) != 0){
        Log(ThisModule, __LINE__, "加载库失败：SPBaseClass.dll，失败原因：%s",
            m_pBase.LastError().toStdString().c_str());
        return false;
    }

    //加载共享内存类
    if(m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory") != 0){
        Log(ThisModule, __LINE__, "加载库失败: WFMShareMenory.dll，失败原因：%s",
            m_pIWFM.LastError().toStdString().c_str());
        return false;
    }

    //注册接口类
    m_pBase->RegisterICmdRun(this);
    return m_pBase->StartRun();
}

void CSPBaseVDM::GetSPBaseData(SPBASEDATA &stData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pBase){
        Log(ThisModule, __LINE__, "加载SPBaseClass类失败！");
        return;
    }

    return m_pBase->GetSPBaseData(stData);
}

bool CSPBaseVDM::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pBase){
        Log(ThisModule, __LINE__, "加载SPBaseClass类失败！");
        return false;
    }

    return m_pBase->FireEvent(uMsgID, dwEventID, lpData);
}

bool CSPBaseVDM::FireStatusChanged(DWORD dwStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pBase){
        Log(ThisModule, __LINE__, "加载SPBaseClass类失败！");
        return false;
    }

    WFSDEVSTATUS stDevStatus;
    memset(&stDevStatus, 0, sizeof(stDevStatus));
    stDevStatus.dwState = dwStatus;
    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_DEVICE_STATUS, &stDevStatus);
}

bool CSPBaseVDM::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pBase){
        Log(ThisModule, __LINE__, "加载SPBaseClass类失败！");
        return false;
    }

    WFSHWERROR stHwError;
    memset(&stHwError, 0, sizeof(stHwError));
    stHwError.dwAction = dwAction;
    if(lpDescription != nullptr){
        stHwError.dwSize = strlen(lpDescription);
        stHwError.lpbDescription = (LPBYTE)lpDescription;
    }

    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_HARDWARE_ERROR, &stHwError);
}

HRESULT CSPBaseVDM::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pCmdFunc){
        Log(ThisModule, __LINE__, "没有注册回掉类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return m_pCmdFunc->OnOpen(lpLogicalName);
}

HRESULT CSPBaseVDM::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pCmdFunc){
        Log(ThisModule, __LINE__, "没有注册回掉类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return m_pCmdFunc->OnClose();
}

HRESULT CSPBaseVDM::OnStatus()
{
    THISMODULE(__FUNCTION__);

    if(nullptr == m_pCmdFunc){
        Log(ThisModule, __LINE__, "没有注册回掉类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return m_pCmdFunc->OnStatus();
}

HRESULT CSPBaseVDM::OnWaitTaken()
{
    //因为没有要等待取走什么的，直接返回取消
    return WFS_ERR_CANCELED;
}

HRESULT CSPBaseVDM::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pCmdFunc){
        Log(ThisModule, __LINE__, "没有注册回掉类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return m_pCmdFunc->OnCancelAsyncRequest();
}

HRESULT CSPBaseVDM::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pCmdFunc){
        Log(ThisModule, __LINE__, "没有注册回掉类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return m_pCmdFunc->OnUpdateDevPDL();
}

HRESULT CSPBaseVDM::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pCmdFunc){
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch(dwCommand){
    case WFS_CMD_VDM_ENTER_MODE_REQ:
    {
        //hRet = m_pCmdFunc->EnterRequest();
        hRet = m_pCmdFunc->EnterModeREQ(dwTimeOut);
    }
        break;
    case WFS_CMD_VDM_ENTER_MODE_ACK:
    {
        //hRet = m_pCmdFunc->EnterAck();
        hRet = m_pCmdFunc->EnterModeACK();
    }
        break;
    case WFS_CMD_VDM_EXIT_MODE_REQ:
    {
        //hRet = m_pCmdFunc->ExitRequest();
        hRet = m_pCmdFunc->ExitModeREQ(dwTimeOut);
    }
        break;
    case WFS_CMD_VDM_EXIT_MODE_ACK:
    {
        //hRet = m_pCmdFunc->ExitAck();
        hRet = m_pCmdFunc->ExitModeACK();
    }
        break;
    default:
        break;
    }

    return hRet;
}

HRESULT CSPBaseVDM::Fmt_WFSVDMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do{
        auto lpStatus = static_cast<LPWFSVDMSTATUS>(lpData);
        if(lpStatus == nullptr){
            return hRet;
        }

        LPWFSVDMSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSVDMSTATUS), lpResult, (LPVOID *)&lpNewData);
        if(hRet != WFS_SUCCESS){
            break;
        }

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSVDMSTATUS));
        lpNewData->lppAppStatus = nullptr;
        if(lpStatus->lppAppStatus != nullptr){
            int iCount = 0;
            while(lpStatus->lppAppStatus[iCount] != nullptr){
                iCount++;
            }

            LPWFSVDMAPPSTATUS *lppAppStatus = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSVDMAPPSTATUS) * (iCount + 1), lpResult, (LPVOID *)&lppAppStatus);
            if(hRet != WFS_SUCCESS){
                break;
            }

            memset(lppAppStatus, 0, (iCount + 1) * sizeof(LPWFSVDMAPPSTATUS));
            _auto.push_back(lppAppStatus);
            for(int i = 0; i < iCount; i++){
                LPWFSVDMAPPSTATUS lpAppStatus = nullptr;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSVDMAPPSTATUS), lpResult, (LPVOID *)&lpAppStatus);
                if(hRet != WFS_SUCCESS){
                    break;
                }

                _auto.push_back(lpAppStatus);
                memset(lpAppStatus, 0, sizeof(WFSVDMAPPSTATUS));
                lpAppStatus->wAppStatus = lpStatus->lppAppStatus[i]->wAppStatus;

                if(lpStatus->lppAppStatus[i]->lpszAppID != nullptr){
                    int iLen = strlen(lpStatus->lppAppStatus[i]->lpszAppID) + 1;
                    LPSTR lpszAppID = nullptr;
                    hRet = m_pIWFM->WFMAllocateMore(iLen, lpResult, (LPVOID *)&lpszAppID);
                    if(hRet != WFS_SUCCESS){\
                        break;
                    }

                    _auto.push_back(lpszAppID);
                    memcpy(lpszAppID, lpStatus->lppAppStatus[i]->lpszAppID, iLen -1);
                    lpAppStatus->lpszAppID = lpszAppID;
                }
                lppAppStatus[i] = lpAppStatus;
            }

            if(hRet != WFS_SUCCESS){
                break;
            }

        }
        lpNewData->lpszExtra = nullptr;
        if(lpStatus->lpszExtra != nullptr){
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpStatus->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
        }

        lpResult->lpBuffer = lpNewData;
    }while(false);

    return hRet;
}

HRESULT CSPBaseVDM::Fmt_WFSVDMCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do{
        auto lpCaps = static_cast<LPWFSVDMCAPS>(lpData);
        if(lpCaps == nullptr){
            break;
        }

        LPWFSVDMCAPS LpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSVDMCAPS), lpResult, (LPVOID *)&LpNewData);
        if(hRet != WFS_SUCCESS){
            break;
        }

        _auto.push_back(LpNewData);
        memcpy(LpNewData, lpCaps, sizeof(WFSVDMCAPS));
        LpNewData->lpszExtra = nullptr;
        if(lpCaps->lpszExtra != nullptr){
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, LpNewData->lpszExtra, lpCaps->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
        }

        lpResult->lpBuffer = LpNewData;
    }while(false);

    return hRet;
}

HRESULT CSPBaseVDM::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    Q_UNUSED(lpQueryDetails);
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pCmdFunc){
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch(dwCategory){
    case WFS_INF_VDM_STATUS:
    {
        LPWFSVDMSTATUS lpstStatus = nullptr;
        hRet = m_pCmdFunc->GetStatus(lpstStatus);
        if(hRet != WFS_SUCCESS){
            break;
        }
        hRet = Fmt_WFSVDMSTATUS(lpstStatus, lpResult);
    }
        break;
    case WFS_INF_VDM_CAPABILITIES:
    {
        LPWFSVDMCAPS lpstCaps = nullptr;
        hRet = m_pCmdFunc->GetCapabilities(lpstCaps);
        if(hRet != WFS_SUCCESS){
            break;
        }

        hRet = Fmt_WFSVDMCAPS(lpstCaps, lpResult);
    }
        break;
    default:
        break;
    }

    return hRet;
}

// @function:事件数据结构体转换为WFSRESULT.lpBuffer
// @param:   uMsgID:事件类别ID dwEventID:事件ID lpData：事件数据结构体
// @return:  WFS_SUCCESS：成功
HRESULT CSPBaseVDM::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch(uMsgID){
    case WFS_EXECUTE_EVENT:
        break;
    case WFS_SERVICE_EVENT:
    {
        switch(dwEventID){
        case WFS_SRVE_VDM_ENTER_MODE_REQ:
        case WFS_SRVE_VDM_EXIT_MODE_REQ:
            lpResult->lpBuffer = nullptr;
            hRet = WFS_SUCCESS;
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
        switch(dwEventID){
        case WFS_SYSE_HARDWARE_ERROR:
            hRet = m_pBase->Fmt_WFSHWERROR(lpResult, lpData);
            break;
        case WFS_SYSE_DEVICE_STATUS:
            hRet = m_pBase->Fmt_WFSDEVSTATUS(lpResult, lpData);
            break;
        case WFS_SYSE_VDM_MODEENTERED:
        case WFS_SYSE_VDM_MODEEXITED:
            lpResult->lpBuffer = nullptr;
            hRet = WFS_SUCCESS;
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
