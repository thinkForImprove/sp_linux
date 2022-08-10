#include "AgentCAM.h"

static const char *DEVTYPE  = "CAM";
static const char *ThisFile = "AgentCAMOut.cpp";
//////////////////////////////////////////////////////////////////////////
HRESULT CAgentCAM::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch(dwCategory)
    {
        case WFS_INF_CAM_STATUS:
            hRet = Get_WFS_CAM_STATUS_Out(lpQueryDetails,lpCopyCmdData);
            break;
        case WFS_INF_CAM_CAPABILITIES:
            hRet = Get_WFS_CAM_CAPABILITIES_Out(lpQueryDetails,lpCopyCmdData);
            break;
        default:
            break;
    }
    return hRet;
}

HRESULT CAgentCAM::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch(dwCommand)
    {
        case WFS_CMD_CAM_TAKE_PICTURE:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CAM_RESET:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CAM_TAKE_PICTURE_EX:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CAM_DISPLAY:
            hRet = Exe_WFS_CMD_CAM_TAKE_DISPLAY_Out(lpCmdData,lpCopyCmdData);
            break;
        case WFS_CMD_CAM_GET_SIGNATURE:
            hRet = Exe_WFS_CMD_CAM_GET_SIGNATURE_Out(lpCmdData,lpCopyCmdData);
            break;
        case WFS_CMD_CAM_DISPLAYEX:
            hRet = WFS_SUCCESS;
            break;
        default:
            break;
    }
    return hRet;
}



HRESULT CAgentCAM::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
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
                case WFS_EXEE_CAM_LIVEERROR:
                    hRet = Fmt_WFS_EXEE_CAM_LIVEERROR(lpData, lpResult);
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
            default:
                break;
            }
        }
        break;
    case WFS_USER_EVENT:
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

HRESULT CAgentCAM::Get_WFS_CAM_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSCAMSTATUS>(lpQueryDetails);
        if(lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCAMSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCAMSTATUS lpGRG_Status = (LPWFSCAMSTATUS)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpData, sizeof(WFSCAMSTATUS));
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
    } while (false);

    return hRet;
}


HRESULT CAgentCAM::Get_WFS_CAM_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSCAMCAPS>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCAMCAPS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCAMCAPS lpGRG_Caps = (LPWFSCAMCAPS)lpResult->lpBuffer;
        if (lpGRG_Caps == nullptr)
            break;
        memcpy(lpGRG_Caps, lpData, sizeof(WFSCAMCAPS));
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

HRESULT CAgentCAM::Exe_WFS_CMD_CAM_GET_SIGNATURE_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSCAMSIGNDATA>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCAMSIGNDATA), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCAMSIGNDATA lpGRG_Data = (LPWFSCAMSIGNDATA)lpResult->lpBuffer;
        if (lpGRG_Data == nullptr)
            break;
        memcpy(lpGRG_Data, lpData, sizeof(WFSCAMSIGNDATA));
        lpGRG_Data->data = nullptr;

        if (lpData->data_length < 0)
        {
            if (lpData->data == nullptr)
            {
                lpGRG_Data->data = nullptr;
            } else
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * lpData->data_length, lpResult, (LPVOID *)&lpGRG_Data->data);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lpGRG_Data->data);
                memset(lpGRG_Data->data, 0x00, lpData->data_length);
                memcpy(lpGRG_Data->data, lpData->data, lpData->data_length);
            }
        } else
        {
            lpGRG_Data->data = nullptr;
        }
        lpResult->lpBuffer = lpGRG_Data;
        lpGRG_Data = nullptr;

    }while (false);

    return hRet;
}

HRESULT CAgentCAM::Exe_WFS_CMD_CAM_TAKE_DISPLAY_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSCAMDISP>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCAMDISP), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);

        memcpy(lpResult->lpBuffer, lpData, sizeof(WFSCAMDISP));

    }while (false);
    return hRet;
}

HRESULT CAgentCAM::Fmt_WFS_EXEE_CAM_LIVEERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSCAMLIVEERROR>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCAMLIVEERROR), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);

        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSCAMLIVEERROR));
    } while (false);

    return hRet;
}

HRESULT CAgentCAM::Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData)
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

HRESULT CAgentCAM::Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData)
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
HRESULT CAgentCAM::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
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


































