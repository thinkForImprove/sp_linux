#include "AgentIDC.h"

static const char *DEVTYPE  = "IDC";
static const char *ThisFile = "AgentIDCOut.cpp";
//////////////////////////////////////////////////////////////////////////
HRESULT CAgentIDC::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }
    Log(ThisModule, __LINE__,"0case WFS_INF_IDC_STATUS");
    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch (dwCategory)
    {
        case WFS_INF_IDC_QUERY_FORM:
            hRet = Get_WFS_INF_IDC_QUERYFORM_Out(lpQueryDetails, lpCopyCmdData);
            break;
        case WFS_INF_IDC_STATUS:
            hRet = Get_WFS_IDC_STATUS_Out(lpQueryDetails, lpCopyCmdData);
             break;
        case WFS_INF_IDC_CAPABILITIES:
            hRet = Get_WFS_IDC_CAPABILITIES_Out(lpQueryDetails, lpCopyCmdData);
             break;
        case WFS_INF_IDC_FORM_LIST:
            hRet = Get_WFS_IDC_FORMLIST_Out(lpQueryDetails, lpCopyCmdData);
            break;
        case WFS_INF_CRD_STATUS:
            hRet = Get_WFS_IDC_CRDSTATUS_Out(lpQueryDetails, lpCopyCmdData);
            break;
        case WFS_INF_CRD_CAPABILITIES:
            hRet = Get_WFS_IDC_CRDCAPS_Out(lpQueryDetails, lpCopyCmdData);
            break;
        case WFS_INF_CRD_CARD_UNIT_INFO:
            hRet = Get_WFS_IDC_CARDUNITINFO_Out(lpQueryDetails, lpCopyCmdData);
            break;
        default:
            break;
    }
    return hRet;
}

HRESULT CAgentIDC::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
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
        case WFS_CMD_IDC_READ_RAW_DATA:
            hRet = Exe_WFS_CMD_IDC_RAW_DATA_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_CHIP_IO:
            hRet = Exe_WFS_CMD_IDC_CHIP_IO_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_CHIP_POWER:
            hRet = Exe_WFS_CMD_IDC_CHIP_POWER_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_CMSTATUS:
            hRet = Exe_WFS_CMD_IDC_CMSTATUS_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_RETAIN_CARD:
            hRet = Exe_WFS_CMD_IDC_RETAIN_CARD_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_GETCARDINFO:
            hRet = Exe_WFS_CMD_IDC_CMSTATUS_Out(lpCmdData, lpCopyCmdData);
            break;
        case WFS_CMD_IDC_EJECT_CARD:
        hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_RESET_COUNT:
        hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_RESET:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_CMEJECT_CARD:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_SETCARDDATA:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_CMRETAIN_CARD:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_CMREDUCE_COUNT:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_CMSET_COUNT:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CMEMPTY_CARD:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_CMRESET:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_REDUCE_COUNT:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_SET_COUNT:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_IDC_INTAKE_CARD_BACK:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CRD_DISPENSE_CARD:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CRD_EJECT_CARD:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CRD_RETAIN_CARD:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CRD_RESET:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CRD_SET_CARD_UNIT_INFO:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CRD_SET_GUIDANCE_LIGHT:
            hRet = WFS_SUCCESS;
            break;
        case WFS_CMD_CRD_POWER_SAVE_CONTROL:
            hRet = WFS_SUCCESS;
            break;
        default:
            break;
    }
    return hRet;
}

HRESULT CAgentIDC::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
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
        case WFS_EXEE_CRD_CARDUNITERROR:
            hRet = Fmt_EXEE_CRD_CARDUNITERROR(lpData, lpResult);
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
        case WFS_SRVE_CRD_MEDIAREMOVED:
            hRet = WFS_SUCCESS;
            break;
        case WFS_SRVE_CRD_MEDIADETECTED:
            hRet = Fmt_SRVE_CRD_MEDIADETECTED(lpData, lpResult);
            break;
        case WFS_SRVE_CRD_CARDUNITINFOCHANGED:
            hRet = Fmt_SRVE_CRD_CARDUNITINFOCHANGED(lpData, lpResult);
            break;
        case WFS_SRVE_CRD_DEVICEPOSITION:
            hRet = Fmt_SRVE_CRD_DEVICEPOSITION(lpData, lpResult);
            break;
        case WFS_SRVE_CRD_POWER_SAVE_CHANGE:
            hRet = Fmt_SRVE_CRD_POWER_SAVE_CHANGE(lpData, lpResult);
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
        case WFS_USRE_CRD_CARDUNITTHRESHOLD:
            hRet = Fmt_USRE_CRD_CARDUNITTHRESHOLD(lpData, lpResult);
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

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_RETAINCARD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_RAW_DATA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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
        for (INT i = 0; i < usCount - 1; i ++)
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


HRESULT CAgentIDC::Get_WFS_INF_IDC_QUERYFORM_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {

        auto lpData = static_cast<LPWFSIDCFORM>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCFORM), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCFORM lpGRG_Form = (LPWFSIDCFORM)lpResult->lpBuffer;
        if (lpGRG_Form == nullptr)
            break;
        memcpy(lpGRG_Form, lpData, sizeof(WFSIDCFORM));

        lpGRG_Form->lpszFormName = nullptr;
        lpGRG_Form->lpszTracks = nullptr;
        lpGRG_Form->lpszTrack1Fields = nullptr;
        lpGRG_Form->lpszTrack2Fields = nullptr;
        lpGRG_Form->lpszTrack3Fields = nullptr;
        if(lpGRG_Form->lpszFormName != nullptr)
        {
            UINT uLen = strlen(lpData->lpszFormName);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Form->lpszFormName);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Form->lpszFormName);
            memcpy(lpGRG_Form->lpszFormName, lpData->lpszFormName, uLen + 1);
        } else
        {
            lpGRG_Form->lpszFormName = nullptr;
        }

        if(lpGRG_Form->lpszTracks != nullptr)
        {
            UINT uLen = strlen(lpData->lpszTracks);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Form->lpszTracks);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Form->lpszTracks);
            memcpy(lpGRG_Form->lpszTracks, lpData->lpszTracks, uLen + 1);
        } else
        {
            lpGRG_Form->lpszTracks = nullptr;
        }

        if(lpGRG_Form->lpszTrack1Fields != nullptr)
        {
            UINT uLen = strlen(lpData->lpszTrack1Fields);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Form->lpszTrack1Fields);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Form->lpszTrack1Fields);
            memcpy(lpGRG_Form->lpszTrack1Fields, lpData->lpszTrack1Fields, uLen + 1);
        } else
        {
            lpGRG_Form->lpszTrack1Fields = nullptr;
        }

        if(lpGRG_Form->lpszTrack2Fields != nullptr)
        {
            UINT uLen = strlen(lpData->lpszTrack2Fields);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Form->lpszTrack2Fields);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Form->lpszTrack2Fields);
            memcpy(lpGRG_Form->lpszTrack2Fields, lpData->lpszTrack2Fields, uLen + 1);
        } else
        {
            lpGRG_Form->lpszTrack2Fields = nullptr;
        }

        if(lpGRG_Form->lpszTrack3Fields != nullptr)
        {
            UINT uLen = strlen(lpData->lpszTrack3Fields);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (uLen + 1), lpResult, (LPVOID *)&lpGRG_Form->lpszTrack3Fields);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpGRG_Form->lpszTrack3Fields);
            memcpy(lpGRG_Form->lpszTrack3Fields, lpData->lpszTrack3Fields, uLen + 1);
        } else
        {
            lpGRG_Form->lpszTrack3Fields = nullptr;
        }
        lpResult->lpBuffer = lpGRG_Form;
        lpGRG_Form = nullptr;
    }while(false);

    return hRet;
}

HRESULT CAgentIDC::Get_WFS_IDC_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Get_WFS_IDC_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Get_WFS_IDC_FORMLIST_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Get_WFS_IDC_CRDSTATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
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
         hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
         if (hRet != WFS_SUCCESS)
             break;
         _auto.push_back(lpResult->lpBuffer);
         LPWFSCRDSTATUS lpGRG_Status = (LPWFSCRDSTATUS)lpResult->lpBuffer;
         if (lpGRG_Status == nullptr)
             break;
         memcpy(lpGRG_Status, lpData, sizeof(WFSCRDSTATUS));
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

HRESULT CAgentIDC::Get_WFS_IDC_CRDCAPS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSCRDCAPS>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDCAPS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCRDCAPS lpGRG_Caps = (LPWFSCRDCAPS)lpResult->lpBuffer;
        if (lpGRG_Caps == nullptr)
            break;
        memcpy(lpGRG_Caps, lpData, sizeof(WFSCRDCAPS));
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

HRESULT CAgentIDC::Get_WFS_IDC_CARDUNITINFO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSCRDCUINFO>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDCUINFO), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCRDCUINFO lpGRG_CardInfo = (LPWFSCRDCUINFO)lpResult->lpBuffer;
        if (lpGRG_CardInfo == nullptr)
            break;
        memcpy(lpGRG_CardInfo, lpData, sizeof(WFSCRDCUINFO));

        LPWFSCRDCARDUNIT *lppCardUnitList;
        USHORT usCount = lpGRG_CardInfo->usCount;
        if (usCount <= 0)
        {
            break;
        }

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCRDCARDUNIT) * usCount, lpResult, (LPVOID *)&lppCardUnitList);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lppCardUnitList);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCRDCARDUNIT lpNewCardUnit;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDCARDUNIT), lpResult, (LPVOID *)&lpNewCardUnit);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCardUnit);

            LPWFSCRDCARDUNIT lpCardUnit = lpData->lppList[i];
            memcpy(lpNewCardUnit, lpCardUnit, sizeof(WFSCRDCARDUNIT));

            if (lpData->lppList[i]->lpszCardName == nullptr)
            {
                lpNewCardUnit->lpszCardName = nullptr;
            } else
            {
                INT nStrLen = strlen(lpData->lppList[i]->lpszCardName);
                LPSTR lpCardName;
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * (nStrLen + 1), lpResult, (LPVOID *)&lpCardName);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpCardName);
                memcpy(lpCardName, lpData->lppList[i]->lpszCardName, nStrLen);
                lpNewCardUnit->lpszCardName = lpCardName;
            }
            lppCardUnitList[i] = lpNewCardUnit;
        }

        lpGRG_CardInfo->lppList = lppCardUnitList;
        lpResult->lpBuffer = lpGRG_CardInfo;
        lpGRG_CardInfo = nullptr;
    } while (false);


}
HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CHIP_IO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSIDCCHIPIO>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCHIPIO), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCCHIPIO lpGRG_CIO = (LPWFSIDCCHIPIO)lpResult->lpBuffer;
        if (lpGRG_CIO == nullptr)
            break;
        memcpy(lpGRG_CIO, lpData, sizeof(WFSIDCCHIPIO));
        lpGRG_CIO->lpbChipData = nullptr;
        if (lpData->ulChipDataLength > 0)
        {
            if (lpData->lpbChipData == nullptr)
            {
                lpGRG_CIO->lpbChipData = nullptr;
            } else {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * lpData->ulChipDataLength, lpResult, (LPVOID *)&lpGRG_CIO->lpbChipData);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lpGRG_CIO->lpbChipData);
                memset(lpGRG_CIO->lpbChipData, 0x00, lpData->ulChipDataLength);
                memcpy(lpGRG_CIO->lpbChipData, lpData->lpbChipData, lpData->ulChipDataLength);
            }
        } else {
            lpGRG_CIO->lpbChipData = nullptr;
        }

        lpResult->lpBuffer = lpGRG_CIO;
        lpGRG_CIO= nullptr;

    }while(false);

    return hRet;
}


HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CHIP_POWER_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSIDCCHIPPOWEROUT>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCCHIPPOWEROUT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCCHIPPOWEROUT lpGRG_CPOW = (LPWFSIDCCHIPPOWEROUT)lpResult->lpBuffer;
        if (lpGRG_CPOW == nullptr)
            break;
        memcpy(lpGRG_CPOW, lpData, sizeof(WFSIDCCHIPPOWEROUT));
        lpGRG_CPOW->lpbChipData = nullptr;
        if (lpData->ulChipDataLength < 0)
        {
            if (lpData->lpbChipData == nullptr)
            {
                lpGRG_CPOW->lpbChipData = nullptr;
            } else
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(CHAR) * lpData->ulChipDataLength, lpResult, (LPVOID *)&lpGRG_CPOW->lpbChipData);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lpGRG_CPOW->lpbChipData);
                memset(lpGRG_CPOW->lpbChipData, 0x00, lpData->ulChipDataLength);
                memcpy(lpGRG_CPOW->lpbChipData, lpData->lpbChipData, lpData->ulChipDataLength);
            }
        } else
        {
            lpGRG_CPOW->lpbChipData = nullptr;
        }
        lpResult->lpBuffer = lpGRG_CPOW;
        lpGRG_CPOW= nullptr;
    }while(false);
    return hRet;
}

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_RETAIN_CARD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSIDCRETAINCARD>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSIDCRETAINCARD), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCRETAINCARD lpGRG_ReCard = (LPWFSIDCRETAINCARD)lpResult->lpBuffer;
        if (lpGRG_ReCard == nullptr)
            break;
        memcpy(lpGRG_ReCard, lpData, sizeof(WFSIDCRETAINCARD));

        lpResult->lpBuffer = lpGRG_ReCard;
        lpGRG_ReCard = nullptr;
    }while(false);
    return hRet;
}

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_CMSTATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPBYTE>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(118, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCSTATUS lpNewData = (LPWFSIDCSTATUS)lpResult->lpBuffer;
        if (lpNewData == nullptr)
            break;
        memcpy(lpNewData, lpData, sizeof(WFSIDCSTATUS));

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgentIDC::Exe_WFS_CMD_IDC_GETCARDINFO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPBYTE>(lpQueryDetails);
        if (lpData == nullptr)
            break;
        DWORD dwLen = sizeof(WFSIDCSTATUS);
        hRet = m_pIWFM->SIMAllocateMore(dwLen, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpResult->lpBuffer);
        LPWFSIDCSTATUS lpNewData = (LPWFSIDCSTATUS)lpResult->lpBuffer;
        if (lpNewData == nullptr)
            break;
        memcpy(lpNewData, lpData, sizeof(WFSIDCSTATUS));

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}



HRESULT CAgentIDC::Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Fmt_EXEE_CRD_CARDUNITERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSCRDCUERROR>(lpData);
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDCUERROR), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ResetOut = (LPWFSCRDCUERROR)lpResult->lpBuffer;
        if (lpGRG_ResetOut == nullptr)
            break;

        memcpy(lpGRG_ResetOut, lpwResetOut, sizeof(WFSCRDCUERROR));
        if (lpwResetOut->lpCardUnit != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDCARDUNIT), lpResult, (LPVOID *)&lpGRG_ResetOut->lpCardUnit);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpGRG_ResetOut->lpCardUnit);
            memcpy(lpGRG_ResetOut->lpCardUnit, lpwResetOut->lpCardUnit, sizeof(WFSCRDCARDUNIT));

            if(lpwResetOut->lpCardUnit->lpszCardName != nullptr)
            {
                hRet = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut->lpCardUnit->lpszCardName) + 1, lpResult, (LPVOID *)&lpGRG_ResetOut->lpCardUnit->lpszCardName);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpGRG_ResetOut->lpCardUnit->lpszCardName);
                memcpy(lpGRG_ResetOut->lpCardUnit->lpszCardName, lpwResetOut->lpCardUnit->lpszCardName, strlen(lpwResetOut->lpCardUnit->lpszCardName) + 1);
            }
        }
        lpResult->lpBuffer = lpGRG_ResetOut;
        lpGRG_ResetOut          = nullptr;
    } while (false);

    return hRet;
}


HRESULT CAgentIDC::Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Fmt_SRVE_CRD_CARDUNITINFOCHANGED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSCRDCARDUNIT>(lpData);
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDCARDUNIT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ResetOut = (LPWFSCRDCARDUNIT)lpResult->lpBuffer;
        if (lpGRG_ResetOut == nullptr)
            break;

        memcpy(lpGRG_ResetOut, lpwResetOut, sizeof(WFSCRDCARDUNIT));
        if(lpwResetOut->lpszCardName != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut->lpszCardName) + 1, lpResult, (LPVOID *)&lpGRG_ResetOut->lpszCardName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ResetOut->lpszCardName);
            memcpy(lpGRG_ResetOut->lpszCardName, lpwResetOut->lpszCardName, strlen(lpwResetOut->lpszCardName) + 1);
        }
        lpResult->lpBuffer = lpGRG_ResetOut;
        lpGRG_ResetOut          = nullptr;
    }while(false);
    return hRet;
}

HRESULT CAgentIDC::Fmt_SRVE_CRD_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Fmt_SRVE_CRD_DEVICEPOSITION(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSCRDDEVICEPOSITION>(lpData);
        if (lpwResetOut == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDDEVICEPOSITION), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSCRDDEVICEPOSITION));
    }while(false);
    return hRet;
}

HRESULT CAgentIDC::Fmt_SRVE_CRD_POWER_SAVE_CHANGE(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSCRDPOWERSAVECHANGE>(lpData);
        if (lpwResetOut == nullptr)
            break;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDPOWERSAVECHANGE), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSCRDPOWERSAVECHANGE));
    }while(false);
    return hRet;
}

HRESULT CAgentIDC::Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
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

HRESULT CAgentIDC::Fmt_USRE_CRD_CARDUNITTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpwResetOut = static_cast<LPWFSCRDCARDUNIT>(lpData);
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCRDCARDUNIT), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        auto lpGRG_ResetOut = (LPWFSCRDCARDUNIT)lpResult->lpBuffer;
        if (lpGRG_ResetOut == nullptr)
            break;

        memcpy(lpGRG_ResetOut, lpwResetOut, sizeof(WFSCRDCARDUNIT));
        if(lpwResetOut->lpszCardName != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(lpwResetOut->lpszCardName) + 1, lpResult, (LPVOID *)&lpGRG_ResetOut->lpszCardName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpGRG_ResetOut->lpszCardName);
            memcpy(lpGRG_ResetOut->lpszCardName, lpwResetOut->lpszCardName, strlen(lpwResetOut->lpszCardName) + 1);
        }
        lpResult->lpBuffer = lpGRG_ResetOut;
        lpGRG_ResetOut          = nullptr;
    }while(false);
    return hRet;
}

HRESULT CAgentIDC::Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData)
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

HRESULT CAgentIDC::Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData)
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

HRESULT CAgentIDC::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
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




























