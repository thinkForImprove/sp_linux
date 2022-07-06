#include "AgentIDC.h"

static const char *DEVTYPE = "IDC";
static const char *ThisFile = "AgentIDC.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentIDC;
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
bool CAgentIDC::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////
CAgentIDC::CAgentIDC()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentIDC::~CAgentIDC() {}

void CAgentIDC::Release() {}

HRESULT CAgentIDC::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
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
    case WFS_INF_IDC_FORM_LIST:
        hRet = Get_WFS_IDC_NODATA(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_IDC_QUERY_FORM:
        hRet = Get_WFS_INF_IDC_QUERYFORM(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentIDC::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
    case WFS_CMD_IDC_RESET_COUNT:
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
        hRet = Exe_WFS_CMD_IDC_CHIP_POWER(lpCmdData, lpCopyCmdData);
        break;
#ifdef CARD_REJECT_GD_MODE
    case WFS_CMD_IDC_CMEJECT_CARD:
        hRet = Exe_WFS_CMD_IDC_CMEJECT_CARD(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CMSTATUS:
        hRet = Exe_WFS_CMD_IDC_CMSTATUS(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_SETCARDDATA:
        hRet = Exe_WFS_CMD_IDC_SETCARDDATA(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CMRETAIN_CARD:
        hRet = Exe_WFS_CMD_IDC_CMRETAIN_CARD(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CMREDUCE_COUNT:
        hRet = Exe_WFS_CMD_IDC_CMREDUCE_COUNT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CMSET_COUNT:
        hRet = Exe_WFS_CMD_IDC_CMSET_COUNT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CMEMPTY_CARD:
        hRet = Exe_WFS_CMD_CMEMPTY_CARD(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_GETCARDINFO:
        hRet = Exe_WFS_CMD_IDC_GETCARDINFO(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CMRESET:
        hRet = Exe_WFS_CMD_IDC_CMRESET(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_REDUCE_COUNT:
        hRet = Exe_WFS_CMD_IDC_REDUCE_COUNT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_SET_COUNT:
        hRet = Exe_WFS_CMD_IDC_SET_COUNT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_INTAKE_CARD_BACK:
        hRet = Exe_WFS_CMD_IDC_INTAKE_CARD_BACK(lpCmdData, lpCopyCmdData);
        break;
#endif
    default:
        break;
    }
    return hRet;
}

//////////////////////////////////////////////////////////////////////////

HRESULT CAgentIDC::Get_WFS_IDC_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentIDC::Get_WFS_INF_IDC_QUERYFORM(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPSTR lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(strlen((LPSTR)lpData) + 1, WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, strlen((LPSTR)lpData) + 1);
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_RAW_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CHIP_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CHIP_POWER(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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

#ifdef CARD_REJECT_GD_MODE
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CMEJECT_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpCmdData);
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpData != nullptr)
    {
        LPSTR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(strlen(lpData) + 1, WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        strcpy(lpNewData, lpData);
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return WFS_SUCCESS;
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CMSTATUS(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpCmdData);
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpData != nullptr)
    {
        LPSTR lpNewData = nullptr;
        WORD dwLen = strlen(lpData);
        hRet = m_pIWFM->WFMAllocateBuffer((dwLen > 119 ? dwLen : 119) + 1, WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memset(lpNewData, 0x0, (dwLen > 119 ? dwLen : 119) + 1);
        strcpy(lpNewData, lpData);
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return WFS_SUCCESS;
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_SETCARDDATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpCmdData);
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpData != nullptr)
    {
        LPSTR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(strlen(lpData) + 1, WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        strcpy(lpNewData, lpData);
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return WFS_SUCCESS;
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CMRETAIN_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CMREDUCE_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CMSET_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
HRESULT CAgentIDC::Exe_WFS_CMD_CMEMPTY_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_GETCARDINFO(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpCmdData);
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpData != nullptr)
    {
        LPSTR lpNewData = nullptr;
        WORD dwLen = strlen(lpData);
        hRet = m_pIWFM->WFMAllocateBuffer(dwLen + 1, WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memset(lpNewData, 0x0, dwLen + 1);
        strcpy(lpNewData, lpData);
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return WFS_SUCCESS;
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CMRESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_REDUCE_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_SET_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_INTAKE_CARD_BACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
}
#endif
