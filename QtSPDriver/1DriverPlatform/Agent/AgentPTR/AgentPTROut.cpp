#include "AgentPTR.h"

static const char *DEVTYPE  = "PTR";
static const char *ThisFile = "AgentPTROut.cpp";
//////////////////////////////////////////////////////////////////////////
HRESULT CAgentPTR::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
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
        hRet = Get_WFS_PTR_STATUS_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_CAPABILITIES:
        hRet = Get_WFS_PTR_CAPABILITIES_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_FORM_LIST:
        hRet = Get_WFS_PTR_FORM_LIST_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_MEDIA_LIST:
        hRet = Get_WFS_PTR_MEDIA_LIST_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_QUERY_FORM:
        hRet = Get_WFS_INF_PTR_QUERY_FORM_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_QUERY_MEDIA:
        hRet = Get_WFS_INF_PTR_QUERY_MEDIA_Out(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_QUERY_FIELD:
        hRet = Get_WFS_INF_PTR_QUERY_FIELD_Out(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentPTR::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
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
    case WFS_CMD_PTR_CONTROL_MEDIA:
        hRet = WFS_SUCCESS;
        break;
    case WFS_CMD_PTR_PRINT_FORM:
        hRet = WFS_SUCCESS;
        break;
    case WFS_CMD_PTR_READ_FORM:
        hRet = Exe_WFS_CMD_PTR_READ_FORM_Out(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_RAW_DATA:
        hRet = Exe_WFS_CMD_PTR_RAW_DATA_Out(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_MEDIA_EXTENTS:
        hRet = Exe_WFS_CMD_PTR_MEDIA_EXTENTS_Out(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_RESET_COUNT:
        hRet = WFS_SUCCESS;
        break;
    case WFS_CMD_PTR_READ_IMAGE:
        hRet = Exe_WFS_CMD_PTR_READ_IMAGE_Out(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_RESET:
        hRet = WFS_SUCCESS;
        break;
    case WFS_CMD_PTR_RETRACT_MEDIA:
        hRet = Exe_WFS_CMD_PTR_RETRACT_MEDIA_Out(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_DISPENSE_PAPER:
        hRet = WFS_SUCCESS;
        break;
    default:
        Log(ThisModule, __LINE__, "PTROUT--ExecuteOut: hRet = %d, dwCommand = %d", hRet, dwCommand);
        break;
    }
    return hRet;
}

HRESULT CAgentPTR::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Fail");
        return WFS_ERR_INTERNAL_ERROR;
    }
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        switch (dwEventID)
        {
        case WFS_EXEE_PTR_NOMEDIA:
            hRet = Fmt_WFS_PTR_NOMEDIA(lpData, lpResult);
            break;
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
            hRet = Exe_WFSMEDIAPRESENTED_Out(lpData, lpResult);
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
    case WFS_USER_EVENT:
        switch (dwEventID)
        {
        case WFS_USRE_PTR_RETRACTBINTHRESHOLD:
            hRet = Fmt_WFS_USRE_PTR_RETRACTBINTHRESHOLD(lpData, lpResult);
            break;
        case WFS_USRE_PTR_PAPERTHRESHOLD:
            hRet = Fmt_WFS_USRE_PTR_PAPERTHRESHOLD(lpData, lpResult);
            break;
        case WFS_USRE_PTR_TONERTHRESHOLD:
        case WFS_USRE_PTR_INKTHRESHOLD:
        case WFS_USRE_PTR_LAMPTHRESHOLD:
            hRet = Fmt_DATA_WORD(lpData, lpResult);
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

//////////////////////////////////////////////////////////////////////////
HRESULT CAgentPTR::Get_WFS_PTR_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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

HRESULT CAgentPTR::Get_WFS_PTR_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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

HRESULT CAgentPTR::Get_WFS_PTR_FORM_LIST_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPSTR>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        UINT uLen = GetLenOfSZZ(lpData);
        hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * uLen, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);

        memcpy(lpResult->lpBuffer, lpData, uLen);
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Get_WFS_PTR_MEDIA_LIST_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPSTR>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        UINT uLen = GetLenOfSZZ(lpData);
        hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * uLen, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        memcpy(lpResult->lpBuffer, lpData, uLen);
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Get_WFS_INF_PTR_QUERY_FORM_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {

        auto lpData = static_cast<LPWFSFRMHEADER>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSFRMHEADER), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSFRMHEADER lpGRG_Frm = (LPWFSFRMHEADER)lpResult->lpBuffer;
        if (lpGRG_Frm == nullptr)
            break;

        memcpy(lpGRG_Frm, lpData, sizeof(WFSFRMHEADER));
        lpGRG_Frm->lpszFields = nullptr;
        lpGRG_Frm->lpszFormName = nullptr;
        lpGRG_Frm->lpszUserPrompt = nullptr;

        if (lpData->lpszFormName != nullptr)
        {
            UINT uLen = strlen(lpData->lpszFormName);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Frm->lpszFormName);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Frm->lpszFormName);
            memcpy(lpGRG_Frm->lpszFormName, lpData->lpszFormName, uLen + 1);
        } else
        {
            lpGRG_Frm->lpszFormName = nullptr;
        }

        if (lpData->lpszUserPrompt != nullptr)
        {
            UINT uLen = strlen(lpData->lpszUserPrompt);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Frm->lpszUserPrompt);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Frm->lpszUserPrompt);
            memcpy(lpGRG_Frm->lpszUserPrompt, lpData->lpszUserPrompt, uLen + 1);
        } else
        {
            lpGRG_Frm->lpszUserPrompt = nullptr;
        }

        if (lpData->lpszFields != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpData->lpszFields);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * uLen, lpResult, (LPVOID *)&lpGRG_Frm->lpszFields);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Frm->lpszFields);
            memset(lpGRG_Frm->lpszFields, 0x00, uLen);
            memcpy(lpGRG_Frm->lpszFields, lpData->lpszFields, uLen);
        } else
        {
            lpGRG_Frm->lpszFields = nullptr;
        }

        lpResult->lpBuffer = lpGRG_Frm;
        lpGRG_Frm = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Get_WFS_INF_PTR_QUERY_MEDIA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSFRMMEDIA>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSFRMMEDIA), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSFRMMEDIA lpGRG_Media = (LPWFSFRMMEDIA)lpResult->lpBuffer;
        if (lpGRG_Media == nullptr)
            break;
        memcpy(lpGRG_Media, lpData, sizeof(WFSFRMMEDIA));

        lpResult->lpBuffer = lpGRG_Media;
        lpGRG_Media = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Get_WFS_INF_PTR_QUERY_FIELD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lppData = static_cast<LPWFSFRMFIELD*>(lpQueryDetails);
        if (lppData == nullptr)
            break;

        USHORT usCount = 0;
        for (usCount = 0; ; usCount ++)
        {
            if (lppData[usCount] == nullptr)
            {
                usCount = usCount + 1;
                break;
            }
        }

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSFRMFIELD) * usCount, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSFRMFIELD *lppGRG_Field = (LPWFSFRMFIELD *)lpResult->lpBuffer;
        if (lppGRG_Field == nullptr)
            break;

        for (INT i = 0; i < usCount - 1; i ++)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSFRMFIELD), lpResult, (LPVOID *)&lppGRG_Field[i]);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lppGRG_Field[i]);
            memcpy(lppGRG_Field[i], lppData[i], sizeof(WFSFRMFIELD));
            lppGRG_Field[i]->lpszFieldName = nullptr;
            lppGRG_Field[i]->lpszFormat = nullptr;
            lppGRG_Field[i]->lpszInitialValue = nullptr;
            lppGRG_Field[i]->lpszUNICODEFormat = nullptr;
            lppGRG_Field[i]->lpszUNICODEInitialValue = nullptr;

            if (lppData[i]->lpszFieldName != nullptr)
            {
                UINT uLen = strlen(lppData[i]->lpszFieldName);
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lppGRG_Field[i]->lpszFieldName);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Field[i]->lpszFieldName);
                memset(lppGRG_Field[i]->lpszFieldName, 0x00, uLen + 1);
                memcpy(lppGRG_Field[i]->lpszFieldName, lppData[i]->lpszFieldName, uLen);
            } else
            {
                lppGRG_Field[i]->lpszFieldName = nullptr;
            }

            if (lppData[i]->lpszFormat != nullptr)
            {
                UINT uLen = strlen(lppData[i]->lpszFormat);
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lppGRG_Field[i]->lpszFormat);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Field[i]->lpszFormat);
                memset(lppGRG_Field[i]->lpszFormat, 0x00, uLen);
                memcpy(lppGRG_Field[i]->lpszFormat, lppData[i]->lpszFormat, uLen);
            } else
            {
                lppGRG_Field[i]->lpszFormat = nullptr;
            }

            if (lppData[i]->lpszInitialValue != nullptr)
            {
                UINT uLen = strlen(lppData[i]->lpszInitialValue);
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lppGRG_Field[i]->lpszInitialValue);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Field[i]->lpszInitialValue);
                memset(lppGRG_Field[i]->lpszInitialValue, 0x00, uLen);
                memcpy(lppGRG_Field[i]->lpszInitialValue, lppData[i]->lpszInitialValue, uLen);
            } else
            {
                lppGRG_Field[i]->lpszInitialValue = nullptr;
            }

            if (lppData[i]->lpszUNICODEFormat != nullptr)
            {
                LPSTR  lpUnChar = QString::fromStdWString(lppData[i]->lpszUNICODEFormat).toLocal8Bit().data();
                UINT uLen = strlen(lpUnChar);
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lppGRG_Field[i]->lpszUNICODEFormat);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Field[i]->lpszUNICODEFormat);
                memset(lppGRG_Field[i]->lpszUNICODEFormat, 0x00, uLen);
                memcpy(lppGRG_Field[i]->lpszUNICODEFormat, lppData[i]->lpszUNICODEFormat, uLen);
            } else
            {
                lppGRG_Field[i]->lpszUNICODEFormat = nullptr;
            }

            if (lppData[i]->lpszUNICODEInitialValue != nullptr)
            {
                LPSTR  lpUnChar = QString::fromStdWString(lppData[i]->lpszUNICODEInitialValue).toLocal8Bit().data();
                UINT uLen = strlen(lpUnChar);
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lppGRG_Field[i]->lpszUNICODEInitialValue);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Field[i]->lpszUNICODEInitialValue);
                memset(lppGRG_Field[i]->lpszUNICODEInitialValue, 0x00, uLen);
                memcpy(lppGRG_Field[i]->lpszUNICODEInitialValue, lppData[i]->lpszUNICODEInitialValue, uLen);
            } else
            {
                lppGRG_Field[i]->lpszUNICODEFormat = nullptr;
            }
        }

        lppGRG_Field[usCount] = nullptr;

        lpResult->lpBuffer = lppGRG_Field;
        lppGRG_Field = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_READ_FORM_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSPTRREADFORMOUT>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRREADFORMOUT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSPTRREADFORMOUT lpGRG_Read = (LPWFSPTRREADFORMOUT)lpResult->lpBuffer;
        if (lpGRG_Read == nullptr)
            break;

        memcpy(lpGRG_Read, lpData, sizeof(WFSPTRREADFORMOUT));
        lpGRG_Read->lpszFields = nullptr;
        lpGRG_Read->lpszUNICODEFields = nullptr;

        if (lpData->lpszFields != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpData->lpszFields);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Read->lpszFields);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Read->lpszFields);
            memset(lpGRG_Read->lpszFields, 0x00, uLen);
            memcpy(lpGRG_Read->lpszFields, lpData->lpszFields, uLen);
        } else
        {
            lpGRG_Read->lpszFields = nullptr;
        }

        if (lpData->lpszUNICODEFields != nullptr)
        {
            LPSTR  lpUnChar = QString::fromStdWString(lpData->lpszUNICODEFields).toLocal8Bit().data();
            UINT uLen = GetLenOfSZZ(lpUnChar);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Read->lpszUNICODEFields);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Read->lpszUNICODEFields);
            memset(lpGRG_Read->lpszUNICODEFields, 0x00, uLen);
            memcpy(lpGRG_Read->lpszUNICODEFields, lpData->lpszUNICODEFields, uLen);
        } else
        {
            lpGRG_Read->lpszUNICODEFields = nullptr;
        }

        lpResult->lpBuffer = lpGRG_Read;
        lpGRG_Read = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_RAW_DATA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSPTRRAWDATAIN>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRRAWDATAIN), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSPTRRAWDATAIN lpGRG_Raw = (LPWFSPTRRAWDATAIN)lpResult->lpBuffer;
        if (lpGRG_Raw == nullptr)
            break;
        memcpy(lpGRG_Raw, lpData, sizeof(WFSPTRRAWDATAIN));
        lpGRG_Raw->lpbData = nullptr;

        if (lpData->ulSize < 0)
        {
            if (lpData->lpbData == nullptr)
            {
                lpGRG_Raw->lpbData = nullptr;
            } else
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * lpData->ulSize, lpResult, (LPVOID *)&lpGRG_Raw->lpbData);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lpGRG_Raw->lpbData);
                memset(lpGRG_Raw->lpbData, 0x00, lpData->ulSize);
                memcpy(lpGRG_Raw->lpbData, lpData->lpbData, lpData->ulSize);
            }
        } else
        {
            lpGRG_Raw->lpbData = nullptr;
        }

        lpResult->lpBuffer = lpGRG_Raw;
        lpGRG_Raw = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_MEDIA_EXTENTS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSPTRMEDIAEXT>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRMEDIAEXT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSPTRMEDIAEXT lpGRG_Ext = (LPWFSPTRMEDIAEXT)lpResult->lpBuffer;
        if (lpGRG_Ext == nullptr)
            break;
        memcpy(lpGRG_Ext, lpData, sizeof(WFSPTRRAWDATAIN));

        lpResult->lpBuffer = lpGRG_Ext;
        lpGRG_Ext = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_READ_IMAGE_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);    

    do
    {
        auto lppImageData = static_cast<LPWFSPTRIMAGE *>(lpQueryDetails);
        if (lppImageData == nullptr){
        Log(ThisModule, __LINE__, "PTR-Exe_WFS_CMD_PTR_READ_IMAGE_Out: 111");
            break;
        }

        USHORT usCount = 0;
        while (true)
        {
            if (lppImageData[usCount] == nullptr)
                break;
            usCount++;
        }

        LPWFSPTRIMAGE *lppNewData = nullptr;
        hRet                      = m_pIWFM->SIMAllocateMore(sizeof(LPWFSPTRIMAGE) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS){
         Log(ThisModule, __LINE__, "PTR-Exe_WFS_CMD_PTR_READ_IMAGE_Out: 222 ret = %d", hRet);
            break;
        }

        _auto.push_back(lppNewData);
        memset(lppNewData, 0x00, sizeof(LPWFSPTRIMAGE) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            LPWFSPTRIMAGE lpImage = nullptr;
            hRet                  = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRIMAGE), lpResult, (LPVOID *)&lpImage);
            if (hRet != WFS_SUCCESS){
             Log(ThisModule, __LINE__, "PTR-Exe_WFS_CMD_PTR_READ_IMAGE_Out: 333 ret = %d, i = %d", hRet, i);
                break;
            }
            _auto.push_back(lpImage);
            memcpy(lpImage, lppImageData[i], sizeof(WFSPTRIMAGE));
            lpImage->lpbData = nullptr;
            if ((lppImageData[i]->lpbData != nullptr) && (lppImageData[i]->ulDataLength > 0))
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(BYTE) * lppImageData[i]->ulDataLength, lpResult, (LPVOID *)&lpImage->lpbData);
                if (hRet != WFS_SUCCESS){
                 Log(ThisModule, __LINE__, "PTR-Exe_WFS_CMD_PTR_READ_IMAGE_Out: 444 ret = %d, i = %d", hRet, i);
                    break;
                }
                _auto.push_back(lpImage->lpbData);
                memcpy(lpImage->lpbData, lppImageData[i]->lpbData, sizeof(BYTE) * lppImageData[i]->ulDataLength);
            }
            lppNewData[i] = lpImage;
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData         = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_RETRACT_MEDIA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {

        auto lpData = static_cast<LPUSHORT>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(USHORT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        memcpy(lpResult->lpBuffer, lpData, sizeof(USHORT));
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFSMEDIAPRESENTED_Out(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpWfsPtrMediaPresented = static_cast<LPWFSPTRMEDIAPRESENTED>(lpData);
        if (lpWfsPtrMediaPresented == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRMEDIAPRESENTED), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        lpResult->lpBuffer     = lpWfsPtrMediaPresented;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Fmt_WFS_PTR_NOMEDIA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    do
    {
        auto lpwResetOut = static_cast<LPSTR>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet            = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut) + 1, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, strlen(lpwResetOut) + 1);
    } while (false);
    return hRet;
}

HRESULT CAgentPTR::Fmt_WFS_EXEE_PTR_FIELDERROR(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentPTR::Fmt_WFS_USRE_PTR_RETRACTBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRBINTHRESHOLD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRBINTHRESHOLD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSPTRBINTHRESHOLD));
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Fmt_WFS_USRE_PTR_PAPERTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSPTRPAPERTHRESHOLD>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSPTRPAPERTHRESHOLD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSPTRPAPERTHRESHOLD));
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Fmt_DATA_WORD(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentPTR::Fmt_WFS_SRVE_PTR_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentPTR::Fmt_NODATA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    lpResult->lpBuffer = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentPTR::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
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
