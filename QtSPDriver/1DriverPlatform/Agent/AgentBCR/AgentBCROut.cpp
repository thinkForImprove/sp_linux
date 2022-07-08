#include "AgentBCR.h"

static const char *DEVTYPE  = "BCR";
static const char *ThisFile = "AgentBCROut.cpp";

//////////////////////////////////////////////////////////////////////////
HRESULT CAgentBCR::Get_WFS_INF_BCR_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSBCRSTATUS>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSBCRSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSBCRSTATUS lpGRG_Status = (LPWFSBCRSTATUS)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpData, sizeof(WFSBCRSTATUS));
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

HRESULT CAgentBCR::Get_WFS_INF_BCR_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpData = static_cast<LPWFSBCRCAPS>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSBCRCAPS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSBCRCAPS lpGRG_Caps = (LPWFSBCRCAPS)lpResult->lpBuffer;
        if (lpGRG_Caps == nullptr)
            break;

        memcpy(lpGRG_Caps, lpData, sizeof(WFSBCRCAPS));
        lpGRG_Caps->lpszExtra = nullptr;
        lpGRG_Caps->lpwSymbologies = nullptr;

        if (lpData->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_Caps->lpszExtra, lpData->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpGRG_Caps->lpszExtra);
        }

        if (lpData->lpwSymbologies != nullptr)
        {
            WORD wCount = 0;
            while (true)
            {
                if (lpData->lpwSymbologies[wCount++] == 0)
                {
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(WORD) * wCount, lpResult, (LPVOID *)&lpGRG_Caps->lpwSymbologies);
                    if (hRet != WFS_SUCCESS)
                    {
                        break;
                    }
                    _auto.push_back(lpGRG_Caps->lpwSymbologies);
                    memcpy(lpGRG_Caps->lpwSymbologies, lpData->lpwSymbologies, sizeof(WORD) * wCount);
                    break;
                }
            }
        }

        lpResult->lpBuffer = lpGRG_Caps;
        lpGRG_Caps = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentBCR::Exe_WFS_CMD_BCR_READ_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lppData = static_cast<LPWFSBCRREADOUTPUT*>(lpQueryDetails);
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

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSBCRREADOUTPUT) * usCount, lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSBCRREADOUTPUT *lppGRG_Image = (LPWFSBCRREADOUTPUT *)lpResult->lpBuffer;
        if (lppGRG_Image == nullptr)
            break;
        memcpy(lppGRG_Image, lppData, sizeof(WFSBCRREADOUTPUT));

        for (INT i = 0; i < usCount; i ++)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSBCRREADOUTPUT), lpResult, (LPVOID *)&lppGRG_Image[i]);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lppGRG_Image[i]);
            memcpy(lppGRG_Image[i], lppData[i], sizeof(WFSBCRREADOUTPUT));

            if (lppData[i]->lpszSymbologyName == nullptr)
            {
                lppGRG_Image[i]->lpszSymbologyName = nullptr;
            } else
            {
                USHORT uLen = strlen(lppData[i]->lpszSymbologyName) + 1;
                hRet = m_pIWFM->SIMAllocateMore(uLen, lpResult, (LPVOID *)&lppGRG_Image[i]->lpszSymbologyName);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Image[i]->lpszSymbologyName);
                memcpy(lppGRG_Image[i]->lpszSymbologyName, lppData[i]->lpszSymbologyName, uLen);
            }

            if (lppData[i]->lpxBarcodeData == nullptr)
            {
                lppGRG_Image[i]->lpxBarcodeData = nullptr;
            } else
            {
                USHORT uLen = strlen(lppData[i]->lpszSymbologyName) + 1;
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSBCRXDATA), lpResult, (LPVOID *)&lppGRG_Image[i]->lpxBarcodeData);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Image[i]->lpxBarcodeData);
                memcpy(lppGRG_Image[i]->lpxBarcodeData, lppData[i]->lpxBarcodeData, sizeof(WFSBCRXDATA));

                DWORD dwNewSize = lppData[i]->lpxBarcodeData->usLength * sizeof(BYTE);
                hRet = m_pIWFM->SIMAllocateMore(dwNewSize, lpResult, (LPVOID *)&lppGRG_Image[i]->lpxBarcodeData->lpbData);
                if (hRet != WFS_SUCCESS)
                {
                    break;
                }
                _auto.push_back(lppGRG_Image[i]->lpxBarcodeData->lpbData);
                memcpy(lppGRG_Image[i]->lpxBarcodeData->lpbData, lppData[i]->lpxBarcodeData->lpbData, dwNewSize);
            }
        }

        lppGRG_Image[usCount] = nullptr;

        lpResult->lpBuffer = lppGRG_Image;
        lppGRG_Image = nullptr;
    } while (false);


    return hRet;
}

HRESULT CAgentBCR::Fmt_WFS_SRVE_BCR_DEVICEPOSITION(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSBCRDEVICEPOSITION>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSBCRDEVICEPOSITION), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSBCRDEVICEPOSITION));
    } while (false);

    return hRet;
}

HRESULT CAgentBCR::Fmt_WFS_SRVE_BCR_POWER_SAVE_CHANGE(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;

    do
    {
        auto lpwResetOut = static_cast<LPWFSBCRPOWERSAVECHANGE>(lpData);
        if (lpwResetOut == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSBCRPOWERSAVECHANGE), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpResult->lpBuffer, lpwResetOut, sizeof(WFSBCRPOWERSAVECHANGE));
    } while (false);

    return hRet;
}

HRESULT CAgentBCR::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
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
