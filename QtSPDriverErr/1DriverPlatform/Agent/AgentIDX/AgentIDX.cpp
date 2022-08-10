#include "AgentIDX.h"

static const char *DEVTYPE = "IDX";
static const char *ThisFile = "AgentIDX.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentIDX;
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
bool CAgentIDX::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////
CAgentIDX::CAgentIDX()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentIDX::~CAgentIDX() {}

void CAgentIDX::Release() {}

HRESULT CAgentIDX::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
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
        case WFS_INF_IDC_STATUS:
        case WFS_INF_IDC_CAPABILITIES:
            hRet = Get_WFS_IDC_NODATA(lpQueryDetails, lpCopyCmdData);
            break;
        default:
            break;
    }
    return hRet;
}

HRESULT CAgentIDX::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
        case WFS_CMD_IDC_EJECT_CARD:
        case WFS_CMD_IDC_RETAIN_CARD:
            hRet = Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_RESET:
            hRet = Exe_WFS_CMD_IDC_RESET(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_READ_RAW_DATA:
            hRet = Exe_WFS_CMD_IDC_RAW_DATA(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_CHIP_IO:
            hRet = Exe_WFS_CMD_IDC_CHIP_IO(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_CHIP_POWER:
            break;
        default:
            break;
    }
    return hRet;
}



//////////////////////////////////////////////////////////////////////////

HRESULT CAgentIDX::Get_WFS_IDC_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentIDX::Exe_WFS_CMD_IDC_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWORD>(lpCmdData);
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpData != nullptr)
    {
        LPWORD lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        *lpNewData = *lpData;
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return WFS_SUCCESS;
}

HRESULT CAgentIDX::Exe_WFS_CMD_IDC_RAW_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPWORD lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    *lpNewData = *lpData;
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentIDX::Exe_WFS_CMD_IDC_CHIP_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpChipIO = static_cast<LPWFSIDCCHIPIO>(lpCmdData);
        if (lpChipIO == nullptr)
            break;

        LPWFSIDCCHIPIO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSIDCCHIPIO), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpChipIO, sizeof(WFSIDCCHIPIO));
        lpNewData->lpbChipData = nullptr;
        if (lpChipIO->ulChipDataLength > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpChipIO->ulChipDataLength, lpNewData, (LPVOID *)&lpNewData->lpbChipData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbChipData);
            memcpy(lpNewData->lpbChipData, lpChipIO->lpbChipData, sizeof(BYTE) * lpChipIO->ulChipDataLength);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

