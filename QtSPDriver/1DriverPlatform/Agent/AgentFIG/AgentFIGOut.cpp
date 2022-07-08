#include "AgentFIG.h"

static const char *DEVTYPE  = "FIG";
static const char *ThisFile = "AgentFIGOut.cpp";
//////////////////////////////////////////////////////////////////////////
HRESULT CAgentFIG::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
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
        hRet = Get_WFS_FIG_STATUS_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_CAPABILITIES:
        hRet = Get_WFS_FIG_CAPABILITIES_Out(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentFIG::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
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
            hRet = Exe_WFS_CMD_PTR_READ_IMAGE_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_PTR_RESET:
            hRet = WFS_SUCCESS;
            break;
        default:
            break;
    }
    return hRet;
}


HRESULT CAgentFIG::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        switch (dwEventID)
        {
        case WFS_EXEE_PTR_MEDIAINSERTED:
            hRet = Fmt_NODATA(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_FIELDERROR:
            hRet = Fmt_WFS_EXEE_PTR_FIELDERROR(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_FIELDWARNING:
            hRet = Fmt_WFS_EXEE_PTR_FIELDERROR(lpData, lpResult);
            break;
        case WFS_EXEE_PTR_MEDIAPRESENTED:
            hRet = Fmt_WFSMEDIAPRESENTED(lpData, lpResult);
            break;
        default:
            break;
        }
        break;
    case WFS_SERVICE_EVENT:
        switch (dwEventID)
        {
        case WFS_SRVE_PTR_MEDIATAKEN:
        case WFS_SRVE_PTR_MEDIAINSERTED:
            hRet = Fmt_NODATA(lpData, lpResult);
            break;
        case WFS_SRVE_PTR_MEDIADETECTED:
            hRet = Fmt_WFS_SRVE_PTR_MEDIADETECTED(lpData, lpResult);
            break;
        default:
            break;
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

HRESULT CAgentFIG::Get_WFS_FIG_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSPTRSTATUS>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSPTRSTATUS lpGRG_Status = (LPWFSPTRSTATUS)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpData, sizeof(WFSPTRSTATUS));
        lpGRG_Status->lpszExtra = nullptr;
        lpGRG_Status->lppRetractBins = nullptr;
        if (lpData->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_Status->lpszExtra, lpData->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpGRG_Status->lpszExtra);
        }

        if (lpData->lppRetractBins != nullptr)
        {
            INT nBinCnt = 0;
            for (nBinCnt = 0; ; nBinCnt ++)
            {
                if (lpData->lppRetractBins[nBinCnt] == nullptr)
                {
                    break;
                }
            }
            nBinCnt ++;

            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSPTRRETRACTBINS) * nBinCnt, lpResult, (LPVOID *)&lpGRG_Status->lppRetractBins);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Status->lppRetractBins);

            for (INT i = 0; i < nBinCnt; i ++)
            {
                lpGRG_Status->lppRetractBins[i] = nullptr;
                if(lpData->lppRetractBins[i] != nullptr){
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRRETRACTBINS), lpResult, (LPVOID *)&lpGRG_Status->lppRetractBins[i]);
                    if (hRet != WFS_SUCCESS)
                    {
                        break;
                    }
                    _auto.push_back(lpGRG_Status->lppRetractBins[i]);
                    memcpy(lpGRG_Status->lppRetractBins[i], lpData->lppRetractBins[i], sizeof(WFSPTRRETRACTBINS));
                }

            }
        } else
        {
            lpGRG_Status->lppRetractBins = nullptr;
        }


        lpResult->lpBuffer = lpGRG_Status;
        lpGRG_Status = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentFIG::Get_WFS_FIG_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSPTRCAPS>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRCAPS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSPTRCAPS lpGRG_Caps = (LPWFSPTRCAPS)lpResult->lpBuffer;
        if (lpGRG_Caps == nullptr)
            break;

        memcpy(lpGRG_Caps, lpData, sizeof(WFSPTRCAPS));
        lpGRG_Caps->lpszExtra = nullptr;
        lpGRG_Caps->lpusMaxRetract = nullptr;

        if (lpData->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_Caps->lpszExtra, lpData->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpGRG_Caps->lpszExtra);
        }

//        if (lpData->lpszWindowsPrinter != nullptr)
//        {
//            UINT uLen = strlen(lpData->lpszWindowsPrinter);
//            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Caps->lpszWindowsPrinter);
//            if (hRet != WFS_SUCCESS)
//            {
//                break;
//            }
//            _auto.push_back(lpGRG_Caps->lpszWindowsPrinter);
//            memset(lpGRG_Caps->lpszWindowsPrinter, 0x00, uLen + 1);
//            memcpy(lpGRG_Caps->lpszWindowsPrinter, lpData->lpszWindowsPrinter, uLen);
//        } else
//        {
//            lpGRG_Caps->lpszWindowsPrinter = nullptr;
//        }

        if (lpData->usRetractBins > 0)
        {
            if (lpData->lpusMaxRetract != nullptr)
            {
                lpGRG_Caps->lpusMaxRetract = nullptr;
            }

            hRet = m_pIWFM->SIMAllocateMore(sizeof(USHORT) * lpData->usRetractBins, lpResult, (LPVOID *)&lpGRG_Caps->lpusMaxRetract);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Caps->lpusMaxRetract);
            memcpy(lpGRG_Caps->lpusMaxRetract, lpData->lpusMaxRetract, sizeof(USHORT) * lpData->usRetractBins);
        } else
        {
            lpGRG_Caps->lpusMaxRetract = nullptr;
        }

        lpResult->lpBuffer = lpGRG_Caps;
        lpGRG_Caps = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentFIG::Exe_WFS_CMD_PTR_READ_IMAGE_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lppImageData = static_cast<LPWFSPTRIMAGE *>(lpQueryDetails);
        if (lppImageData == nullptr)
            break;

        USHORT usCount = 0;
        while (true)
        {
            if (lppImageData[usCount] == nullptr)
                break;
            usCount++;
        }

        LPWFSPTRIMAGE *lppNewData = nullptr;
        hRet                      = m_pIWFM->SIMAllocateMore(sizeof(LPWFSPTRIMAGE) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0x00, sizeof(LPWFSPTRIMAGE) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            LPWFSPTRIMAGE lpImage = nullptr;
            hRet                  = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRIMAGE), lpResult, (LPVOID *)&lpImage);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpImage);
            memcpy(lpImage, lppImageData[i], sizeof(WFSPTRIMAGE));
            lpImage->lpbData = nullptr;
            if ((lppImageData[i]->lpbData != nullptr) && (lppImageData[i]->ulDataLength > 0))
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(BYTE) * lppImageData[i]->ulDataLength, lpResult, (LPVOID *)&lpImage->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpImage->lpbData);
                memcpy(lpImage->lpbData, lppImageData[i]->lpbData, sizeof(BYTE) * lppImageData[i]->ulDataLength);
            }
            lppNewData[i] = lpImage;
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData         = nullptr;
    } while (false);

    return hRet;
    return hRet;
}

HRESULT CAgentFIG::Fmt_NODATA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    lpResult->lpBuffer = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentFIG::Fmt_WFS_EXEE_PTR_FIELDERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRFIELDFAIL>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRFIELDFAIL), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ResetOut = (LPWFSPTRFIELDFAIL)lpResult->lpBuffer;
        if (lpGRG_ResetOut == nullptr)
            break;

        memcpy(lpGRG_ResetOut, lpwResetOut, sizeof(WFSPTRFIELDFAIL));
        if (lpwResetOut->lpszFieldName != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut->lpszFieldName) + 1, lpResult, (LPVOID *)&lpGRG_ResetOut->lpszFieldName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ResetOut->lpszFieldName);
            memcpy(lpGRG_ResetOut->lpszFieldName, lpwResetOut->lpszFieldName, strlen(lpwResetOut->lpszFieldName) + 1);
        }

        if (lpwResetOut->lpszFormName != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut->lpszFormName) + 1, lpResult, (LPVOID *)&lpGRG_ResetOut->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ResetOut->lpszFormName);
            memcpy(lpGRG_ResetOut->lpszFormName, lpwResetOut->lpszFormName, strlen(lpwResetOut->lpszFormName) + 1);
        }
        lpResult->lpBuffer = lpGRG_ResetOut;
        lpGRG_ResetOut          = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentFIG::Fmt_WFS_SRVE_PTR_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRMEDIADETECTED>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRMEDIADETECTED), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSPTRMEDIADETECTED));
    } while (false);

    return hRet;
}

HRESULT CAgentFIG::Fmt_WFSMEDIAPRESENTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpWfsPtrMediaPresented = static_cast<LPWFSPTRMEDIAPRESENTED>(lpData);
        if (lpWfsPtrMediaPresented == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRMEDIAPRESENTED), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer,lpWfsPtrMediaPresented,sizeof(WFSPTRMEDIAPRESENTED));
    } while (false);
    return hRet;
}

HRESULT CAgentFIG::Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData)
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

HRESULT CAgentFIG::Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData)
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


HRESULT CAgentFIG::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
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


