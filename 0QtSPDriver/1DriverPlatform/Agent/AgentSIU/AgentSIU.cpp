#include "AgentSIU.h"

static const char *DEVTYPE  = "SIU";
static const char *ThisFile = "AgentSIU.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentSIU;
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CAgentSIU::CAgentSIU()
{
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentSIU::~CAgentSIU()
{

}

void CAgentSIU::Release()
{

}

HRESULT CAgentSIU::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
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
    case WFS_INF_SIU_STATUS:
        hRet = Get_WFS_INF_SIU_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_SIU_CAPABILITIES:
        hRet = Get_WFS_INF_SIU_CAPABILITIES(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentSIU::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
    case WFS_CMD_SIU_ENABLE_EVENTS:
        hRet = Exe_WFS_CMD_SIU_ENABLE_EVENTS(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_SIU_SET_DOOR:
        hRet = Exe_WFS_CMD_SIU_SET_DOOR(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_SIU_SET_INDICATOR:
        hRet = Exe_WFS_CMD_SIU_SET_INDICATOR(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_SIU_SET_GUIDLIGHT:
        hRet = Exe_WFS_CMD_SIU_SET_GUIDLIGHT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_SIU_RESET:
        hRet = Exe_WFS_CMD_SIU_RESET(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAgentSIU::Get_WFS_INF_SIU_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentSIU::Get_WFS_INF_SIU_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentSIU::Exe_WFS_CMD_SIU_ENABLE_EVENTS(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSSIUENABLE>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPWFSSIUENABLE lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSSIUENABLE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSSIUENABLE));
    lpNewData->lpszExtra = nullptr;// 暂时不支持扩展入参
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentSIU::Exe_WFS_CMD_SIU_SET_DOOR(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSSIUSETDOOR>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSSIUSETDOOR lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSSIUSETDOOR), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSSIUSETDOOR));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentSIU::Exe_WFS_CMD_SIU_SET_INDICATOR(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSSIUSETINDICATOR>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSSIUSETINDICATOR lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSSIUSETINDICATOR), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSSIUSETINDICATOR));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentSIU::Exe_WFS_CMD_SIU_SET_GUIDLIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSSIUSETGUIDLIGHT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSSIUSETGUIDLIGHT lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSSIUSETGUIDLIGHT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSSIUSETGUIDLIGHT));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentSIU::Exe_WFS_CMD_SIU_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
bool CAgentSIU::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}
