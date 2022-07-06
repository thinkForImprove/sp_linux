#include "AgentVDM.h"
static const char *DEVTYPE  = "VDM";
static const char *ThisFile = "AgentVDM.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentVDM;
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////

CAgentVDM::CAgentVDM()
{
}

CAgentVDM::~CAgentVDM()
{

}

void CAgentVDM::Release()
{

}

HRESULT CAgentVDM::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if(!LoadDll()){
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch(dwCategory){
    case WFS_INF_VDM_STATUS:
        hRet = Get_WFS_INF_VDM_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_VDM_CAPABILITIES:
        hRet = Get_WFS_INF_VDM_CAPABILITIES(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentVDM::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if(!LoadDll()){
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch(dwCommand){
    case WFS_CMD_VDM_ENTER_MODE_REQ:
        hRet = Exe_WFS_CMD_VDM_ENTER_MODE_REQ(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_VDM_ENTER_MODE_ACK:
        hRet = Exe_WFS_CMD_VDM_ENTER_MODE_ACK(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_VDM_EXIT_MODE_REQ:
        hRet = Exe_WFS_CMD_VDM_EXIT_MODE_REQ(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_VDM_EXIT_MODE_ACK:
        hRet = Exe_WFS_CMD_VDM_EXIT_MODE_ACK(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }

    return hRet;
}

HRESULT CAgentVDM::Get_WFS_INF_VDM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{

}

HRESULT CAgentVDM::Get_WFS_INF_VDM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{

}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_ENTER_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{

}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_ENTER_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{

}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_EXIT_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{

}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_EXIT_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{

}

bool CAgentVDM::LoadDll()
{
    if(m_pIWFM != nullptr){
       return true;
    }

    if(0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory")){
        return false;
    }
    return true;
}
