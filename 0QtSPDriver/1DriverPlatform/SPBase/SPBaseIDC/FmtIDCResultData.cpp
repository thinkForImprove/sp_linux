#include "FmtIDCResultData.h"

CFmtIDCResultData::CFmtIDCResultData()
{

}

CFmtIDCResultData::~CFmtIDCResultData()
{

}

HRESULT CFmtIDCResultData::Fmt_WFSIDCSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM.GetDll(), &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSBCRSTATUS>(lpData);
        if (lpStatus == nullptr)
            break;

        LPWFSBCRSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSBCRSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSBCRSTATUS));
        lpNewData->lpszExtra = nullptr;
        if (lpStatus->lpszExtra != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpStatus->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}

HRESULT CFmtIDCResultData::Fmt_WFSIDCCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM.GetDll(), &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSBCRCAPS>(lpData);
        if (lpCaps == nullptr)
            break;

        LPWFSBCRCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSBCRCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSBCRCAPS));
        lpNewData->lpszExtra = nullptr;
        if (lpCaps->lpwSymbologies != nullptr)
        {
            WORD wCount = 0;
            while (true)
            {
                if (lpCaps->lpwSymbologies[wCount++] == 0)
                {
                    LPWORD lpSymb = nullptr;
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(WORD) * wCount, lpResult, (LPVOID *)&lpSymb);
                    if (hRet != WFS_SUCCESS)
                    {
                        lpNewData->lpwSymbologies = nullptr;
                        break;
                    }
                    _auto.push_back(lpSymb);
                    memcpy(lpSymb, lpCaps->lpwSymbologies, sizeof(WORD) * wCount);
                    lpNewData->lpwSymbologies = lpSymb;
                    lpSymb = nullptr;
                    break;
                }
            }
        }
        // 有扩展状态
        if (lpCaps->lpszExtra != nullptr)
        {
            hRet = m_pBase->Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpCaps->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);
    return hRet;
}
