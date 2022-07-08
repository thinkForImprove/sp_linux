#include "AgentIDX.h"

static const char *DEVTYPE = "IDX";
static const char *ThisFile = "AgentIDX.cpp";
//////////////////////////////////////////////////////////////////////////
HRESULT CAgentIDX::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
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
            hRet = Get_WFS_IDC_STATUS_Out(lpQueryDetails, lpCopyCmdData);
            break;
        case WFS_INF_IDC_CAPABILITIES:
            hRet = Get_WFS_IDC_CAPABILITIES_Out(lpQueryDetails, lpCopyCmdData);
            break;
        default:
            break;
    }
    return hRet;
}



HRESULT CAgentIDX::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
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
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_RETAIN_CARD:
            hRet = Exe_WFS_CMD_IDC_RETAINCARD_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_READ_RAW_DATA:
            hRet = Exe_WFS_CMD_IDC_RAW_DATA_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_RESET:
            hRet = WFS_SUCCESS;
            break;
        default:
            break;
    }

    return hRet;
}

HRESULT CAgentIDX::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
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

HRESULT CAgentIDX::Get_WFS_IDC_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSIDCSTATUS>(lpQueryDetails);
        if (lpData == nullptr)
            break;
         hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
         if (hRet != WFS_SUCCESS)
             break;
         _auto.push_back(lpResult->lpBuffer);
         LPWFSIDCSTATUS lpGRG_Status = (LPWFSIDCSTATUS)lpResult->lpBuffer;
         if (lpGRG_Status == nullptr)
             break;
         memcpy(lpGRG_Status, lpData, sizeof(WFSIDCSTATUS));
         lpGRG_Status->lpszExtra = nullptr;
         if (lpData->lpszExtra != nullptr)
         {
             hRet = Fmt_ExtraStatus(lpResult, lpGRG_Status->lpszExtra, lpData->lpszExtra);
             if(hRet != WFS_SUCCESS){
                 break;
             }
             _auto.push_back(lpGRG_Status->lpszExtra);
         }
         lpResult->lpBuffer = lpGRG_Status;
         lpGRG_Status = nullptr;
    }while(false);
    return hRet;
}
HRESULT CAgentIDX::Get_WFS_IDC_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSIDCCAPS>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCAPS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCCAPS lpGRG_Caps = (LPWFSIDCCAPS)lpResult->lpBuffer;
        if (lpGRG_Caps == nullptr)
            break;
        memcpy(lpGRG_Caps, lpData, sizeof(WFSIDCCAPS));
        lpGRG_Caps->lpszExtra = nullptr;

        if (lpData->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_Caps->lpszExtra, lpData->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpGRG_Caps->lpszExtra);
        }

        lpResult->lpBuffer = lpGRG_Caps;
        lpGRG_Caps = nullptr;
    }while (false);
    return hRet;
}


HRESULT CAgentIDX::Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    do
    {
        auto lpwResetOut = static_cast<LPWORD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        lpResult->lpBuffer = lpwResetOut;
    } while (false);
    return hRet;
}


HRESULT CAgentIDX::Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSIDCCARDACT>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCARDACT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSIDCCARDACT));
    } while (false);
    return hRet;
}
HRESULT CAgentIDX::Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    do
    {
        auto lpwResetOut = static_cast<LPWORD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        lpResult->lpBuffer = lpwResetOut;
    } while (false);
    return hRet;


    return hRet;
}

HRESULT CAgentIDX::Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSIDCTRACKEVENT>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCTRACKEVENT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ResetOut = (LPWFSIDCTRACKEVENT)lpResult->lpBuffer;
        if (lpGRG_ResetOut == nullptr)
            break;

        memcpy(lpGRG_ResetOut, lpwResetOut, sizeof(WFSIDCTRACKEVENT));
        if (lpwResetOut->lpstrTrack != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut->lpstrTrack) + 1, lpResult, (LPVOID *)&lpGRG_ResetOut->lpstrTrack);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ResetOut->lpstrTrack);
            memcpy(lpGRG_ResetOut->lpstrTrack, lpwResetOut->lpstrTrack, strlen(lpwResetOut->lpstrTrack) + 1);
        }

        if (lpwResetOut->lpstrData != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut->lpstrData) + 1, lpResult, (LPVOID *)&lpGRG_ResetOut->lpstrData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ResetOut->lpstrData);
            memcpy(lpGRG_ResetOut->lpstrData, lpwResetOut->lpstrData, strlen(lpwResetOut->lpstrData) + 1);
        }
        lpResult->lpBuffer = lpGRG_ResetOut;
        lpGRG_ResetOut          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentIDX::Exe_WFS_CMD_IDC_RETAINCARD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSIDCRETAINCARD>(lpQueryDetails);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCRETAINCARD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSIDCRETAINCARD));
    } while (false);
    return hRet;
}

HRESULT CAgentIDX::Exe_WFS_CMD_IDC_RAW_DATA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);


    do
    {
        auto lpData = static_cast<LPWFSIDCCARDDATA *>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        USHORT usCount = 0;
        for (usCount = 0; ; usCount ++)
        {
            if (lpData[usCount] == nullptr)
            {
                usCount = usCount + 1;
                break;
            }
        }
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCARDDATA) * usCount, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCCARDDATA *lpGRG_Data = (LPWFSIDCCARDDATA *)lpResult->lpBuffer;
        if (lpGRG_Data == nullptr)
            break;
        memcpy(lpGRG_Data, lpData, sizeof(sizeof(WFSIDCCARDDATA) * usCount));
        for (INT i = 0; i < usCount; i ++)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCARDDATA), lpResult, (LPVOID *)&lpGRG_Data[i]);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Data[i]);
            memcpy(lpGRG_Data[i], lpData[i], sizeof(WFSIDCCARDDATA));

            if (lpData[i]->ulDataLength > 0)
            {
                if (lpData[i]->lpbData == nullptr)
                {
                    lpGRG_Data[i]->lpbData = nullptr;
                } else
                {
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (lpData[i]->ulDataLength), lpResult, (LPVOID *)&lpGRG_Data[i]->lpbData);
                    if (hRet != WFS_SUCCESS)
                    {
                        break;
                    }
                    _auto.push_back(lpGRG_Data[i]->lpbData);
                    memcpy(lpGRG_Data[i]->lpbData, lpData[i]->lpbData, lpData[i]->ulDataLength);
                }
            } else
            {
                lpGRG_Data[i]->lpbData = nullptr;
            }
        }
        lpResult->lpBuffer = lpGRG_Data;
        lpGRG_Data = nullptr;
    } while (false);
    return hRet;
}



HRESULT CAgentIDX::Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData)
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

HRESULT CAgentIDX::Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData)
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

HRESULT CAgentIDX::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
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








































































