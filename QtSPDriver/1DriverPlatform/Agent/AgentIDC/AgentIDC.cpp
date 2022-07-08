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
    case WFS_INF_CRD_STATUS:        // 发卡模块状态
    case WFS_INF_CRD_CAPABILITIES:  // 发卡模块能力值
    case WFS_INF_CRD_CARD_UNIT_INFO:// 打卡模块取卡箱状态/箱容量
        Log(ThisModule, __LINE__,"1case WFS_INF_IDC_STATUS:%d",hRet);
        hRet = Get_WFS_IDC_NODATA(lpQueryDetails, lpCopyCmdData);
        Log(ThisModule, __LINE__,"2case WFS_INF_IDC_STATUS:%d",hRet);
        break;
    case WFS_INF_IDC_QUERY_FORM:
        hRet = Get_WFS_INF_IDC_QUERYFORM(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        Log(ThisModule, __LINE__,"3case WFS_INF_IDC_STATUS:%d",hRet);
        break;
    }
    Log(ThisModule, __LINE__,"4case WFS_INF_IDC_STATUS:%d",hRet);
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
    case WFS_CMD_IDC_CMEMPTYALL_CARD:
        hRet = Exe_WFS_CMD_CMEMPTYALL_CARD(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CMCLEARSLOT:
        hRet = Exe_WFS_CMD_CMCLEARSLOT(lpCmdData, lpCopyCmdData);

    case WFS_CMD_CRD_DISPENSE_CARD:
        hRet = Exe_WFS_CMD_CRD_DISPENSE_CARD(lpCmdData, lpCopyCmdData); // 发卡
        break;
    case WFS_CMD_CRD_EJECT_CARD:
        hRet = Exe_WFS_CMD_CRD_EJECT_CARD(lpCmdData, lpCopyCmdData);    // 退卡
        break;
    case WFS_CMD_CRD_RETAIN_CARD:
        hRet = Exe_WFS_CMD_CRD_RETAIN_CARD(lpCmdData, lpCopyCmdData);   // 回收卡
        break;
    case WFS_CMD_CRD_RESET:
        hRet = Exe_WFS_CMD_CRD_RESET(lpCmdData, lpCopyCmdData);         // 复位
        break;
    case WFS_CMD_CRD_SET_GUIDANCE_LIGHT:
        hRet = Exe_WFS_CMD_CRD_SET_GUIDANCE_LIGHT(lpCmdData, lpCopyCmdData);// 设置灯状态
        break;
    case WFS_CMD_CRD_SET_CARD_UNIT_INFO:
        hRet = Exe_WFS_CMD_CRD_SET_CARD_UNIT_INFO(lpCmdData, lpCopyCmdData);// 设置卡箱信息
        break;
    case WFS_CMD_CRD_POWER_SAVE_CONTROL:
        hRet = Exe_WFS_CMD_CRD_POWER_SAVE_CONTROL(lpCmdData, lpCopyCmdData);// 发卡
        break;
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


HRESULT CAgentIDC::Exe_WFS_CMD_CMEMPTYALL_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
}

HRESULT CAgentIDC::Exe_WFS_CMD_CMCLEARSLOT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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

//////////////////////////////////////////////////////////////////////////
// CRD系列命令(Card Dispenser)
// 发卡
HRESULT CAgentIDC::Exe_WFS_CMD_CRD_DISPENSE_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCrdDispense = static_cast<LPWFSCRDDISPENSE>(lpCmdData);
        if (lpCrdDispense == nullptr)
        {
            break;
        }

        LPWFSCRDDISPENSE lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCRDDISPENSE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCrdDispense, sizeof(WFSCRDDISPENSE));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

// 退卡
HRESULT CAgentIDC::Exe_WFS_CMD_CRD_EJECT_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return Get_WFS_IDC_NODATA(lpCmdData, lpCopyCmdData);
}

// 回收卡
HRESULT CAgentIDC::Exe_WFS_CMD_CRD_RETAIN_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCrdRetain = static_cast<LPWFSCRDRETAINCARD>(lpCmdData);
        if (lpCrdRetain == nullptr)
        {
            break;
        }

        LPWFSCRDRETAINCARD lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCRDRETAINCARD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCrdRetain, sizeof(WFSCRDRETAINCARD));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

// 复位
HRESULT CAgentIDC::Exe_WFS_CMD_CRD_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCrdReset = static_cast<LPWFSCRDRESET>(lpCmdData);
        if (lpCrdReset == nullptr)
        {
            break;
        }

        LPWFSCRDRESET lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCRDRESET), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCrdReset, sizeof(WFSCRDRESET));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

// 设置灯状态
HRESULT CAgentIDC::Exe_WFS_CMD_CRD_SET_GUIDANCE_LIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCrdSetGuidanceLight = static_cast<LPWFSCRDSETGUIDLIGHT>(lpCmdData);
        if (lpCrdSetGuidanceLight == nullptr)
        {
            break;
        }

        LPWFSCRDSETGUIDLIGHT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCRDSETGUIDLIGHT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCrdSetGuidanceLight, sizeof(WFSCRDSETGUIDLIGHT));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

// 设置卡箱信息
HRESULT CAgentIDC::Exe_WFS_CMD_CRD_SET_CARD_UNIT_INFO(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCrdCuInfo = static_cast<LPWFSCRDCUINFO>(lpCmdData);
        if (lpCrdCuInfo == nullptr)
        {
            break;
        }

        LPWFSCRDCUINFO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCRDCUINFO), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpNewData);

        lpNewData->usCount = lpCrdCuInfo->usCount;

        USHORT usCount = lpCrdCuInfo->usCount;
        if (usCount <= 0)
        {
            lpNewData->lppList = nullptr;
            lpCopyCmdData = lpNewData;
            lpNewData = nullptr;
            break;
        }

        LPWFSCRDCARDUNIT *lppCardUnitList;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCRDCARDUNIT) * usCount, lpNewData, (LPVOID *)&lppCardUnitList);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lppCardUnitList);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCRDCARDUNIT lpNewCardUnit;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCRDCARDUNIT), lpNewData, (LPVOID *)&lpNewCardUnit);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCardUnit);

            LPWFSCRDCARDUNIT lpCardUnit = lpCrdCuInfo->lppList[i];
            memcpy(lpNewCardUnit, lpCardUnit, sizeof(WFSCRDCARDUNIT));

            if (lpCardUnit->lpszCardName == nullptr)
            {
                lpNewCardUnit->lpszCardName = nullptr;
            } else
            {
                INT nStrLen = strlen(lpCardUnit->lpszCardName);
                LPSTR lpCardName;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(CHAR) * (nStrLen + 1), lpNewData, (LPVOID *)&lpCardName);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpCardName);
                memcpy(lpCardName, lpCardUnit->lpszCardName, nStrLen);
                lpNewCardUnit->lpszCardName = lpCardName;
            }
            lppCardUnitList[i] = lpNewCardUnit;
        }
        lpNewData->lppList = lppCardUnitList;
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

// 激活/停用节能功能
HRESULT CAgentIDC::Exe_WFS_CMD_CRD_POWER_SAVE_CONTROL(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCrdPowerSaveControl = static_cast<LPWFSCRDPOWERSAVECONTROL>(lpCmdData);
        if (lpCrdPowerSaveControl == nullptr)
        {
            break;
        }

        LPWFSCRDPOWERSAVECONTROL lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCRDPOWERSAVECONTROL), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCrdPowerSaveControl, sizeof(WFSCRDPOWERSAVECONTROL));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}
