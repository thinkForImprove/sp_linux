#include "AgentFIG.h"

static const char *DEVTYPE = "FIG";
static const char *ThisFile = "AgentFIG.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentFIG;
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
bool CAgentFIG::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////
CAgentFIG::CAgentFIG()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentFIG::~CAgentFIG() {}

void CAgentFIG::Release() {}

HRESULT CAgentFIG::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
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
        case WFS_INF_PTR_STATUS:
        case WFS_INF_PTR_CAPABILITIES:
            hRet = Get_WFS_PTR_NODATA(lpQueryDetails, lpCopyCmdData);
            break;
        default:
            break;
    }
    return hRet;
}

HRESULT CAgentFIG::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
        case WFS_CMD_PTR_READ_IMAGE:
            hRet = Exe_WFS_CMD_PTR_READ_IMAGE(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_PTR_RESET:
            hRet = Exe_WFS_CMD_PTR_RESET(lpCmdData, lpCopyCmdData);
            break;
        default:
            break;
    }
    return hRet;
}

//////////////////////////////////////////////////////////////////////////

HRESULT CAgentFIG::Get_WFS_PTR_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentFIG::Exe_WFS_CMD_PTR_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpCmdData != nullptr)
    {
        auto lpData = static_cast<LPWFSPTRRESET>(lpCmdData);
        LPWFSPTRRESET lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRRESET), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpData, sizeof(WFSPTRRESET));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    else
    {
        hRet = WFS_SUCCESS;
        lpCopyCmdData = nullptr;
    }

    return hRet;
}

HRESULT CAgentFIG::Exe_WFS_CMD_PTR_READ_IMAGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpPtrForm = static_cast<LPWFSPTRIMAGEREQUEST>(lpCmdData);
    if (lpPtrForm == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSPTRIMAGEREQUEST lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRIMAGEREQUEST), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPtrForm, sizeof(WFSPTRIMAGEREQUEST));

        if (lpPtrForm->lpszFrontImageFile != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszFrontImageFile) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszFrontImageFile);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFrontImageFile);
            memcpy(lpNewData->lpszFrontImageFile, lpPtrForm->lpszFrontImageFile, sizeof(char)*dwSize);
        }

        if (lpPtrForm->lpszBackImageFile != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszBackImageFile) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszBackImageFile);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszBackImageFile);
            memcpy(lpNewData->lpszBackImageFile, lpPtrForm->lpszBackImageFile, sizeof(char)*dwSize);
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}
