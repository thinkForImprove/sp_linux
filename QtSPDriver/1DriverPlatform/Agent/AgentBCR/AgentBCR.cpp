#include "AgentBCR.h"

static const char *DEVTYPE  = "BCR";
static const char *ThisFile = "AgentBCR.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentBCR;
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CAgentBCR::CAgentBCR()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentBCR::~CAgentBCR()
{

}

void CAgentBCR::Release()
{

}

HRESULT CAgentBCR::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch (dwCategory)
    {
    case WFS_INF_BCR_STATUS:
        hRet = Get_WFS_INF_BCR_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_BCR_CAPABILITIES:
        hRet = Get_WFS_INF_BCR_CAPABILITIES(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentBCR::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch (dwCommand)
    {
    case WFS_CMD_BCR_READ:
        hRet = Exe_WFS_CMD_BCR_READ(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_BCR_RESET:
        hRet = Exe_WFS_CMD_BCR_RESET(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_BCR_SET_GUIDANCE_LIGHT:
        hRet = Exe_WFS_CMD_BCR_SET_GUIDANCE_LIGHT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_BCR_POWER_SAVE_CONTROL:
        hRet = Exe_WFS_CMD_BCR_POWER_SAVE_CONTROL(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentBCR::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch (dwCategory)
    {
    case WFS_INF_BCR_STATUS:
        hRet = Get_WFS_INF_BCR_STATUS_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_BCR_CAPABILITIES:
        hRet = Get_WFS_INF_BCR_CAPABILITIES_Out(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentBCR::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch (dwCommand)
    {
    case WFS_CMD_BCR_RESET:
    case WFS_CMD_BCR_SET_GUIDANCE_LIGHT:
    case WFS_CMD_BCR_POWER_SAVE_CONTROL:
        hRet = WFS_SUCCESS;
        break;
    case WFS_CMD_BCR_READ:
        hRet = Exe_WFS_CMD_BCR_READ_Out(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentBCR::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Fail");
        return WFS_ERR_INTERNAL_ERROR;
    }
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        switch (dwEventID)
        {
        default:
            break;
        }
        break;
    case WFS_SERVICE_EVENT:
        switch (dwEventID)
        {
        case WFS_SRVE_BCR_DEVICEPOSITION:
            hRet = Fmt_WFS_SRVE_BCR_DEVICEPOSITION(lpData, lpResult);
            break;
        case WFS_SRVE_BCR_POWER_SAVE_CHANGE:
            hRet = Fmt_WFS_SRVE_BCR_POWER_SAVE_CHANGE(lpData, lpResult);
            break;
        default:
            break;
        }
        break;
    case WFS_USER_EVENT:
        switch (dwEventID)
        {
        default:
            break;
        }
        break;
    case WFS_SYSTEM_EVENT:
        {
            switch (dwEventID)
            {
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

//////////////////////////////////////////////////////////////////////////

HRESULT CAgentBCR::Get_WFS_INF_BCR_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpQueryDetails = nullptr;
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentBCR::Get_WFS_INF_BCR_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpQueryDetails = nullptr;
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentBCR::Exe_WFS_CMD_BCR_READ(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSBCRREADINPUT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSBCRREADINPUT lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSBCRREADINPUT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    _auto.push_back(lpNewData);
    lpNewData->lpwSymbologies = nullptr;
    if (lpData->lpwSymbologies != nullptr)
    {
        UINT uCount = 0;
        while (uCount < 100)
        {
            // The array is terminated with a zero value
            if (lpData->lpwSymbologies[uCount++] == 0)
            {
                break;
            }
        }
        hRet = m_pIWFM->WFMAllocateMore(uCount * sizeof(WORD), lpNewData, (LPVOID *)&lpNewData->lpwSymbologies);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData->lpwSymbologies, lpData->lpwSymbologies, uCount * sizeof(WORD));
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentBCR::Exe_WFS_CMD_BCR_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCmdData = nullptr;
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentBCR::Exe_WFS_CMD_BCR_SET_GUIDANCE_LIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSBCRSETGUIDLIGHT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSBCRSETGUIDLIGHT lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSBCRSETGUIDLIGHT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSBCRSETGUIDLIGHT));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentBCR::Exe_WFS_CMD_BCR_POWER_SAVE_CONTROL(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSBCRPOWERSAVECONTROL>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSBCRPOWERSAVECONTROL lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSBCRPOWERSAVECONTROL), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSBCRPOWERSAVECONTROL));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
bool CAgentBCR::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}



