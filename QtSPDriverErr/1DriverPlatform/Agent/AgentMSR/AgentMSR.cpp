#include "AgentMSR.h"

static const char *DEVTYPE = "MSR";
static const char *ThisFile = "AgentMSR.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentMSR;
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
bool CAgentMSR::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////
CAgentMSR::CAgentMSR()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentMSR::~CAgentMSR() {}

void CAgentMSR::Release() {}

HRESULT CAgentMSR::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
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

HRESULT CAgentMSR::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
        case WFS_CMD_IDC_RESET:
            hRet = Exe_WFS_CMD_IDC_RESET(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_READ_RAW_DATA:
            hRet = Exe_WFS_CMD_IDC_RAW_DATA(lpCmdData, lpCopyCmdData);
            break;
        default:
            break;
    }
    return hRet;
}

HRESULT CAgentMSR::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
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
        hRet = Fmt_WFSIDCSTATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_IDC_CAPABILITIES:
        hRet = Fmt_WFSIDCCAPS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_IDC_FORM_LIST:
        hRet = Fmt_WFSIDCFORMLIST(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_IDC_QUERY_FORM:
        hRet = Fmt_WFSIDCFORM(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentMSR::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
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
    case WFS_CMD_IDC_RESET_COUNT:
    case WFS_CMD_IDC_RESET:
        hRet = WFS_SUCCESS;
        break;
    case WFS_CMD_IDC_RETAIN_CARD:
        hRet = Fmt_WFSIDCRETAINCARD(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_READ_RAW_DATA:
        hRet = Fmt_WFSIDCCARDDATAARY(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CHIP_IO:
        hRet = Fmt_WFSIDCCHIPIO(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_IDC_CHIP_POWER:
        hRet = Fmt_WFSIDCCHIPPOWEROUT(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentMSR::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
    {
        switch (dwEventID)
        {
        case WFS_EXEE_IDC_INVALIDTRACKDATA:
            hRet = Fmt_WFSIDCTRACKEVENT(lpData, lpResult);
            break;
        case WFS_EXEE_IDC_MEDIARETAINED:
            hRet = WFS_SUCCESS;
            break;
        case WFS_EXEE_IDC_MEDIAINSERTED:
            hRet = WFS_SUCCESS;
            break;
        case WFS_EXEE_IDC_INVALIDMEDIA:
            hRet = WFS_SUCCESS;
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
        case WFS_SRVE_IDC_MEDIADETECTED:
            hRet = Fmt_MEDIADETECTED(lpData, lpResult);
            break;
        case WFS_SRVE_IDC_MEDIAREMOVED:
            hRet = WFS_SUCCESS;
            break;
        case WFS_SRVE_IDC_CARDACTION:
            hRet = Fmt_WFSIDCCARDACT(lpData, lpResult);
            break;
        default:
            break;
        }
    }
    break;
    case WFS_USER_EVENT:
    {
        switch (dwEventID)
        {
        case WFS_USRE_IDC_RETAINBINTHRESHOLD:
            hRet = Fmt_RETAINBINTHRESHOLD(lpData, lpResult);
            break;
        default:
            break;
        }
    }
    break;
    case WFS_SYSTEM_EVENT:
    {
        switch (dwEventID)
        {
        case WFS_SYSE_HARDWARE_ERROR:
            hRet = Fmt_WFSHWERROR(lpResult, lpData);
            break;
        case WFS_SYSE_DEVICE_STATUS:
            hRet = Fmt_WFSDEVSTATUS(lpResult, lpData);
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

//////////////////////////////////////////////////////////////////////////

HRESULT CAgentMSR::Get_WFS_IDC_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentMSR::Exe_WFS_CMD_IDC_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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

HRESULT CAgentMSR::Exe_WFS_CMD_IDC_RAW_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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

//--------------------------------------------出参内存拷贝---------------------------------
HRESULT CAgentMSR::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
{
    THISMODULE(__FUNCTION__);
    UINT uLen = GetLenOfSZZ(lpszOldExtra);
    if (uLen == 0)
    {
        Log(ThisModule, __LINE__, "lpszOldExtra格式错误");
        return WFS_ERR_INVALID_DATA;
    }

    HRESULT hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpszNewExtra);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
        return hRet;
    }

    memcpy(lpszNewExtra, lpszOldExtra, sizeof(char) * uLen);
    return WFS_SUCCESS;
}
HRESULT CAgentMSR::Fmt_WFSIDCSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSIDCSTATUS>(lpData);
        if (lpStatus == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_Status = (LPWFSIDCSTATUS)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpStatus, sizeof(WFSIDCSTATUS));
        lpGRG_Status->lpszExtra = nullptr;
        if (lpStatus->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_Status->lpszExtra, lpStatus->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpGRG_Status->lpszExtra);
        }

        lpResult->lpBuffer = lpGRG_Status;
        lpGRG_Status = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSIDCCAPS>(lpData);
        if (lpCaps == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCAPS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_Caps = (LPWFSIDCCAPS)lpResult->lpBuffer;
        if (lpGRG_Caps == nullptr)
            break;

        memcpy(lpGRG_Caps, lpCaps, sizeof(WFSIDCCAPS));
        lpGRG_Caps->lpszExtra = nullptr;
        // 有扩展状态
        if (lpCaps->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_Caps->lpszExtra, lpCaps->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpGRG_Caps->lpszExtra);
        }

        lpResult->lpBuffer = lpGRG_Caps;
        lpGRG_Caps = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCFORM(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpFormList = static_cast<LPWFSIDCFORM>(lpData);
        if (lpFormList == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCFORM), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_FormList = (LPWFSIDCFORM)lpResult->lpBuffer;
        if (lpGRG_FormList == nullptr)
            break;

        memcpy(lpGRG_FormList, lpFormList, sizeof(WFSIDCFORM));
        lpGRG_FormList->lpszFormName = nullptr;
        if (lpFormList->lpszFormName != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpFormList->lpszFormName) + 1, lpResult, (LPVOID *)&lpGRG_FormList->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpGRG_FormList->lpszFormName);
            memcpy(lpGRG_FormList->lpszFormName, lpFormList->lpszFormName, strlen(lpFormList->lpszFormName) + 1);
        }

        if (lpFormList->lpszTracks != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpFormList->lpszTracks) + 1, lpResult, (LPVOID *)&lpGRG_FormList->lpszTracks);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpGRG_FormList->lpszTracks);
            memcpy(lpGRG_FormList->lpszFormName, lpFormList->lpszTracks, strlen(lpFormList->lpszTracks) + 1);
        }

        if (lpFormList->lpszTrack1Fields != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_FormList->lpszTrack1Fields, lpFormList->lpszTrack1Fields);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpGRG_FormList->lpszTrack1Fields);
        }

        if (lpFormList->lpszTrack2Fields != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_FormList->lpszTrack2Fields, lpFormList->lpszTrack2Fields);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpGRG_FormList->lpszTrack2Fields);
        }

        if (lpFormList->lpszTrack3Fields != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_FormList->lpszTrack3Fields, lpFormList->lpszTrack3Fields);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpGRG_FormList->lpszTrack3Fields);
        }

        lpResult->lpBuffer = lpGRG_FormList;
        lpGRG_FormList = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCRETAINCARD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCardRetain = static_cast<LPWFSIDCRETAINCARD>(lpData);
        if (lpCardRetain == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCRETAINCARD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        memcpy(lpResult->lpBuffer, lpCardRetain, sizeof(WFSIDCRETAINCARD));
    } while (false);
    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCCARDDATAARY(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    if (lpData == nullptr)
        return hRet;

    LPWFSIDCCARDDATA *lppPos = (LPWFSIDCCARDDATA *)lpData;
    while (*lppPos != nullptr)
    {
        lppPos++;
    }
    USHORT usCount = lppPos - (LPWFSIDCCARDDATA *)lpData;
    if (usCount == 0)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    do
    {
        auto lppCardData = static_cast<LPWFSIDCCARDDATA *>(lpData);
        if (lppCardData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSIDCCARDDATA) * (usCount + 1), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lppGRG_CardData = (LPWFSIDCCARDDATA *)lpResult->lpBuffer;
        if (lppGRG_CardData == nullptr)
            break;

        memcpy(lppGRG_CardData, lppCardData, sizeof(LPWFSIDCCARDDATA) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCARDDATA), lpResult, (LPVOID *)&lppGRG_CardData[i]);
            if (hRet != WFS_SUCCESS)
                return hRet;

            _auto.push_back(lppGRG_CardData[i]);
            memcpy(lppGRG_CardData[i], lppCardData[i], sizeof(WFSIDCCARDDATA));

            if (lppCardData[i]->ulDataLength > 0)
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(BYTE) * lppCardData[i]->ulDataLength, lpResult, (LPVOID *)&lppGRG_CardData[i]->lpbData);
                if (hRet != WFS_SUCCESS)
                    return hRet;

                _auto.push_back(lppGRG_CardData[i]->lpbData);
                memcpy(lppGRG_CardData[i]->lpbData, lppCardData[i]->lpbData, sizeof(BYTE) * lppCardData[i]->ulDataLength);
            }
        }

        lpResult->lpBuffer = lppGRG_CardData;
        lppGRG_CardData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCCHIPIO(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpChipIO = static_cast<LPWFSIDCCHIPIO>(lpData);
        if (lpChipIO == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCHIPIO), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ChipIO = (LPWFSIDCCHIPIO)lpResult->lpBuffer;
        if (lpGRG_ChipIO == nullptr)
            break;

        memcpy(lpGRG_ChipIO, lpChipIO, sizeof(WFSIDCCHIPIO));
        if (lpChipIO->ulChipDataLength > 0)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(BYTE) * lpChipIO->ulChipDataLength, lpResult, (LPVOID *)&lpGRG_ChipIO->lpbChipData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ChipIO->lpbChipData);
            memcpy(lpGRG_ChipIO->lpbChipData, lpChipIO->lpbChipData, sizeof(BYTE) * lpChipIO->ulChipDataLength);
        }

        lpResult->lpBuffer = lpGRG_ChipIO;
        lpGRG_ChipIO = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCCHIPPOWEROUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpChipPowerOut = static_cast<LPWFSIDCCHIPPOWEROUT>(lpData);
        if (lpChipPowerOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCHIPPOWEROUT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ChipPowerOut = (LPWFSIDCCHIPPOWEROUT)lpResult->lpBuffer;
        if (lpGRG_ChipPowerOut == nullptr)
            break;

        memcpy(lpGRG_ChipPowerOut, lpChipPowerOut, sizeof(WFSIDCCHIPPOWEROUT));
        if ((lpChipPowerOut->ulChipDataLength > 0) && (lpChipPowerOut->lpbChipData != nullptr))
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(BYTE) * lpChipPowerOut->ulChipDataLength, lpResult, (LPVOID *)&lpGRG_ChipPowerOut->lpbChipData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ChipPowerOut->lpbChipData);
            memcpy(lpGRG_ChipPowerOut->lpbChipData, lpChipPowerOut->lpbChipData, sizeof(BYTE) * lpChipPowerOut->ulChipDataLength);
        }

        lpResult->lpBuffer = lpGRG_ChipPowerOut;
        lpGRG_ChipPowerOut = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCFORMLIST(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpFormList = static_cast<LPSTR>(lpData);
        if (lpFormList == nullptr)
            break;

        size_t usSize = strlen(lpFormList) + 1;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        memcpy(lpResult->lpBuffer, lpFormList, usSize);
    } while (false);

    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpEvent = static_cast<LPWFSIDCTRACKEVENT>(lpData);
        if (lpEvent == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCTRACKEVENT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_Event = (LPWFSIDCTRACKEVENT)lpResult->lpBuffer;
        if (lpGRG_Event == nullptr)
            break;

        memcpy(lpGRG_Event, lpEvent, sizeof(WFSIDCTRACKEVENT));
        if (lpEvent->lpstrTrack != nullptr)
        {
            size_t uLen = strlen(lpEvent->lpstrTrack);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpGRG_Event->lpstrTrack);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_Event->lpstrTrack);
            memcpy(lpGRG_Event->lpstrTrack, lpEvent->lpstrTrack, sizeof(char) * uLen);

            uLen = strlen(lpEvent->lpstrData);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpGRG_Event->lpstrData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_Event->lpstrData);
            memcpy(lpGRG_Event->lpstrData, lpEvent->lpstrData, sizeof(char) * uLen);
        }

        lpResult->lpBuffer = lpGRG_Event;
        lpGRG_Event = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentMSR::Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpwResetOut = static_cast<LPWORD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ResetOut = (LPWORD)lpResult->lpBuffer;
        if (lpGRG_ResetOut == nullptr)
            break;

        *lpGRG_ResetOut = *lpwResetOut;
        lpResult->lpBuffer = lpGRG_ResetOut;
        lpGRG_ResetOut = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentMSR::Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpfwRetainBin = static_cast<LPWORD>(lpData);
        if (lpfwRetainBin == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_RetainBin = (LPWORD)lpResult->lpBuffer;
        if (lpGRG_RetainBin == nullptr)
            break;

        *lpGRG_RetainBin = *lpfwRetainBin;
        lpResult->lpBuffer = lpGRG_RetainBin;
        lpGRG_RetainBin = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpAct = static_cast<LPWFSIDCCARDACT>(lpData);
        if (lpAct == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCARDACT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_Act = (LPWFSIDCCARDACT)lpResult->lpBuffer;
        if (lpGRG_Act == nullptr)
            break;

        memcpy(lpGRG_Act, lpAct, sizeof(WFSIDCCARDACT));
        lpResult->lpBuffer = lpGRG_Act;
        lpGRG_Act = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSHWERROR>(lpData);
        if (lpStatus == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSHWERROR), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_Status = (LPWFSHWERROR)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;
        memcpy(lpGRG_Status, lpStatus, sizeof(WFSHWERROR));

        LPSTR lpBuff = nullptr;
        ULONG ulSize = sizeof(char) * 256;
        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpGRG_Status->lpszLogicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpGRG_Status->lpszPhysicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpGRG_Status->lpszWorkstationName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpGRG_Status->lpszAppID = lpBuff;
        lpBuff = nullptr;

        if (lpStatus->dwSize > 0)
        {
            // lpbDescription是“ErrorDetail = 00XXXXXXX\x0\x0” (修正了14个字符+错误代码7个字符+空结束2个字符)
            ulSize = lpStatus->dwSize + 2;  // 此特殊处理：多加两位
            hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
                break;
            }
            _auto.push_back(lpBuff);
            memset(lpBuff, 0x00, ulSize);
            memcpy(lpBuff, lpStatus->lpbDescription, lpStatus->dwSize);
            lpGRG_Status->dwSize = ulSize;
            lpGRG_Status->lpbDescription = (LPBYTE)lpBuff;
            lpBuff = nullptr;
        }

        // 判断是否有数据
        if (lpStatus->lpszLogicalName != nullptr)
            strcpy(lpGRG_Status->lpszLogicalName, lpStatus->lpszLogicalName);
        if (lpStatus->lpszPhysicalName != nullptr)
            strcpy(lpGRG_Status->lpszPhysicalName, lpStatus->lpszPhysicalName);
        if (lpStatus->lpszWorkstationName != nullptr)
            strcpy(lpGRG_Status->lpszWorkstationName, lpStatus->lpszWorkstationName);
        if (lpStatus->lpszAppID != nullptr)
            strcpy(lpGRG_Status->lpszAppID, lpStatus->lpszAppID);

        // 赋值
        lpResult->lpBuffer = lpGRG_Status;
        lpGRG_Status = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgentMSR::Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSDEVSTATUS>(lpData);
        if (lpStatus == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSDEVSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_Status = (LPWFSDEVSTATUS)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpStatus, sizeof(WFSDEVSTATUS));

        LPSTR lpBuff = nullptr;
        ULONG ulSize = sizeof(char) * 256;
        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpGRG_Status->lpszPhysicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpGRG_Status->lpszWorkstationName = lpBuff;
        lpBuff = nullptr;

        // 判断是否有数据
        if (lpStatus->lpszPhysicalName != nullptr)
            strcpy(lpGRG_Status->lpszPhysicalName, lpStatus->lpszPhysicalName);
        if (lpStatus->lpszWorkstationName != nullptr)
            strcpy(lpGRG_Status->lpszWorkstationName, lpStatus->lpszWorkstationName);

        // 赋值
        lpResult->lpBuffer = lpGRG_Status;
        lpGRG_Status = nullptr;
    } while (false);
    return hRet;
}
