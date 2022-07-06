#include "FMTResData.h"
#include <assert.h>

static const char *ThisFile = "CFMTResData.cpp";
//////////////////////////////////////////////////////////////////////////
CFMTResData::CFMTResData(LPCSTR lpLogType)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
}

CFMTResData::~CFMTResData() {}

bool CFMTResData::LoadWFMDll()
{
    THISMODULE(__FUNCTION__);
    // 加载共享内存类
    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
    {
        Log(ThisModule, __LINE__, "加载库失败: WFMShareMenory.dll");
        return false;
    }
    return true;
}

HRESULT CFMTResData::Fmt_WFSCIMMEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto lpItemPos = static_cast<LPWFSCIMITEMPOSITION>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);
    LPWFSCIMITEMPOSITION lpNewPos = nullptr;
    do
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMITEMPOSITION), lpResult, (LPVOID *)&lpNewPos);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpNewPos);
        memcpy(lpNewPos, lpItemPos, sizeof(WFSCIMITEMPOSITION));

        lpNewPos->lpRetractArea = nullptr;
        if (lpItemPos->lpRetractArea != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMRETRACT), lpResult, (LPVOID *)&lpNewPos->lpRetractArea);
            if (hRet != WFS_SUCCESS)
                return hRet;
            autofree.push_back(lpNewPos->lpRetractArea);
            memcpy(lpNewPos->lpRetractArea, lpItemPos->lpRetractArea, sizeof(WFSCDMRETRACT));
        }

        lpResult->lpBuffer = lpNewPos;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMINPUTREFUSE(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto pstats = static_cast<LPUSHORT>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);
    do
    {
        LPUSHORT lpPos = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(USHORT), lpResult, (LPVOID *)&lpPos);
        if (hRet != WFS_SUCCESS)
            return hRet;
        *lpPos = *pstats;
        autofree.push_back(lpPos);

        // 赋值
        lpResult->lpBuffer = lpPos;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMITEMTAKEN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    lpResult->lpBuffer = nullptr;
    return WFS_SUCCESS;
}

HRESULT CFMTResData::Fmt_WFSCIMCASHIN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpCastte = static_cast<LPWFSCIMCASHIN>(lpData);
    if (lpCastte == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        LPWFSCIMCASHIN lpNewCass = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCASHIN), lpResult, (LPVOID *)&lpNewCass);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewCass);

        memcpy(lpNewCass, lpCastte, sizeof(WFSCIMCASHIN));

        // notenumberlist
        LPWFSCIMNOTENUMBERLIST lpNewNoteNumList = nullptr;
        if (lpCastte->lpNoteNumberList != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewNoteNumList);
            memcpy(lpNewNoteNumList, lpCastte->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));
            USHORT usNoteNum = lpNewNoteNumList->usNumOfNoteNumbers;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usNoteNum, lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewNoteNumList->lppNoteNumber);

            for (int m = 0; m < usNoteNum; m++)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber[m]);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewNoteNumList->lppNoteNumber[m]);
                memcpy(lpNewNoteNumList->lppNoteNumber[m], lpCastte->lpNoteNumberList->lppNoteNumber[m], sizeof(WFSCIMNOTENUMBER));
            }

            lpNewCass->lpNoteNumberList = lpNewNoteNumList;
            lpNewNoteNumList = nullptr;
        }

        // phyCU
        USHORT usPhyCUCount = lpNewCass->usNumPhysicalCUs;
        if (usPhyCUCount < 1)
        {
            Log(ThisModule, __LINE__, "物理钞箱数小于1 usPhyCUCount=%d", usPhyCUCount);
            break;
        }

        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewCass->lppPhysical);

        for (int j = 0; j < usPhyCUCount; j++)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMPHCU), lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lppPhysical[j]);

            memcpy(lpNewCass->lppPhysical[j], lpCastte->lppPhysical[j], sizeof(WFSCIMPHCU));

            lpNewCass->lppPhysical[j]->lpPhysicalPositionName = nullptr;
            char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
            if (lpPhyPosName != nullptr)
            {
                hRet = m_pIWFM->WFMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                memcpy(lpNewCass->lppPhysical[j]->lpPhysicalPositionName, lpCastte->lppPhysical[j]->lpPhysicalPositionName,
                       sizeof(char) * (strlen(lpPhyPosName) + 1));
            }

            lpNewCass->lppPhysical[j]->lpszExtra = nullptr;
            char *lpExtra = lpCastte->lppPhysical[j]->lpszExtra;
            if (lpExtra != nullptr)
            {
                UINT uLen = GetLenOfSZZ(lpExtra);
                if (uLen == -1)
                {
                    Log(ThisModule, __LINE__, "lppPhysical[%d]->lpszExtra 格式错误", j);
                    return WFS_ERR_INVALID_DATA;
                }
                hRet = m_pIWFM->WFMAllocateMore(uLen, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpszExtra);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lppPhysical[j]->lpszExtra);

                memcpy(lpNewCass->lppPhysical[j]->lpszExtra, lpCastte->lppPhysical[j]->lpszExtra, sizeof(char) * uLen);
            }
        }

        lpNewCass->lpszExtra = nullptr;
        char *lpExt = lpCastte->lpszExtra;
        if (lpExt != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpExt);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpCastte->lpszExtra 格式错误");
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(uLen, lpResult, (LPVOID *)&lpNewCass->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lpszExtra);

            memcpy(lpNewCass->lpszExtra, lpExt, sizeof(char) * uLen);
        }

        lpResult->lpBuffer = lpNewCass;
        lpNewCass = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMITEMTAKEN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto pstats = static_cast<LPWORD>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);
    do
    {
        LPWORD lpPos = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpPos);
        if (hRet != WFS_SUCCESS)
            return hRet;
        *lpPos = *pstats;
        autofree.push_back(lpPos);

        // 赋值
        lpResult->lpBuffer = lpPos;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::FmtCDMGetResultBuffer(DWORD dwCategory, LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCategory)
    {
    case WFS_INF_CDM_STATUS:
    {
        hRet = Fmt_WFSCDMSTATUS(lpData, lpResult);
        break;
    }
    case WFS_INF_CDM_CAPABILITIES:
    {
        hRet = Fmt_WFSCDMCAPS(lpData, lpResult);
        break;
    }
    case WFS_INF_CDM_CASH_UNIT_INFO:
    {
        hRet = Fmt_WFSCDMCUINFO(lpData, lpResult);
        break;
    }
    case WFS_INF_CDM_MIX_TYPES:
    {
        hRet = Fmt_WFSCDMMIXTYPE(lpData, lpResult);
        break;
    }
    case WFS_INF_CDM_PRESENT_STATUS:
    {
        hRet = Fmt_WFSCDMPRESENTSTATUS(lpData, lpResult);
        break;
    }
    case WFS_INF_CDM_CURRENCY_EXP:
    {
        hRet = Fmt_WFSCDMCURRENCYEXP(lpData, lpResult);
        break;
    }
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMCURRENCYEXP(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    auto lppCurrencyExp = static_cast<LPWFSCDMCURRENCYEXP *>(lpData);
    if (lppCurrencyExp == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCDMCURRENCYEXP *lppNewData = nullptr;
        auto lppPos = lppCurrencyExp;
        while (*lppPos != nullptr)
        {
            lppPos++;
        }
        USHORT usCount = lppPos - lppCurrencyExp;

        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMCURRENCYEXP) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0, sizeof(LPWFSCDMCURRENCYEXP) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCURRENCYEXP), lpResult, (LPVOID *)&lppNewData[i]);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lppNewData[i]);

            memcpy(lppNewData[i], lppCurrencyExp[i], sizeof(WFSCDMCURRENCYEXP));
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMPRESENTSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpPreStats = static_cast<LPWFSCDMPRESENTSTATUS>(lpData);
    if (lpPreStats == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        LPWFSCDMPRESENTSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMPRESENTSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPreStats, sizeof(WFSCDMPRESENTSTATUS));

        LPWFSCDMDENOMINATION lpDeno = nullptr;
        if (lpPreStats->lpDenomination != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMDENOMINATION), lpResult, (LPVOID *)&lpDeno);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpDeno);
            memcpy(lpDeno, lpPreStats->lpDenomination, sizeof(WFSCDMDENOMINATION));
            USHORT usCount = lpDeno->usCount;
            if (usCount == 0)
            {
                lpDeno->lpulValues = nullptr;
            }
            else
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(ULONG) * usCount, lpResult, (LPVOID *)&lpDeno->lpulValues);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpDeno->lpulValues);
                memcpy(lpDeno->lpulValues, lpPreStats->lpDenomination->lpulValues, sizeof(ULONG) * usCount);
            }
        }
        lpNewData->lpDenomination = lpDeno;

        lpNewData->lpszExtra = nullptr;
        if (lpPreStats->lpszExtra != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpPreStats->lpszExtra);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpPreStats->lpszExtra 格式错误");
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewData->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
            memcpy(lpNewData->lpszExtra, lpPreStats->lpszExtra, sizeof(char) * uLen);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMMIXTYPE(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lppMixType = static_cast<LPWFSCDMMIXTYPE *>(lpData);
    if (lppMixType == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSCDMMIXTYPE *lppPos = lppMixType;
    while (*lppPos != nullptr)
    {
        lppPos++;
    }
    USHORT usCount = lppPos - lppMixType;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCDMMIXTYPE *lpNewData = nullptr;
    do
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMMIXTYPE) * (usCount + 1), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memset(lpNewData, 0, sizeof(LPWFSCDMMIXTYPE) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            LPWFSCDMMIXTYPE lpMixType = nullptr;
            HRESULT hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMMIXTYPE), lpResult, (LPVOID *)&lpMixType);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpMixType);
            memcpy(lpMixType, lppMixType[i], sizeof(WFSCDMMIXTYPE));

            UINT uLen = strlen(lppMixType[i]->lpszName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpMixType->lpszName);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpMixType->lpszName);
            memcpy(lpMixType->lpszName, lppMixType[i]->lpszName, sizeof(char) * uLen);

            lpNewData[i] = lpMixType;
            lpMixType = nullptr;
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpCaps = static_cast<LPWFSCDMCAPS>(lpData);
    if (lpCaps == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCDMCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSCDMCAPS));

        lpNewData->lpszExtra = nullptr;
        if (lpCaps->lpszExtra != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpCaps->lpszExtra);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpCaps->lpszExtra 格式错误");
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewData->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
            memcpy(lpNewData->lpszExtra, lpCaps->lpszExtra, sizeof(char) * uLen);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpStatus = static_cast<LPWFSCDMSTATUS>(lpData);
    if (lpStatus == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCDMSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSCDMSTATUS));

        LPWFSCDMOUTPOS *lppOutPos = lpStatus->lppPositions;
        while (*lppOutPos != nullptr)
        {
            lppOutPos++;
        }
        USHORT usCount = lppOutPos - lpStatus->lppPositions;
        lppOutPos = lpStatus->lppPositions;

        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMOUTPOS) * (usCount + 1), lpResult, (LPVOID *)&lpNewData->lppPositions);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lppPositions);

        memset(lpNewData->lppPositions, 0, sizeof(LPWFSCDMOUTPOS) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            LPWFSCDMOUTPOS lpOutPos = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMOUTPOS), lpResult, (LPVOID *)&lpOutPos);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpOutPos);

            memcpy(lpOutPos, lppOutPos[i], sizeof(WFSCDMOUTPOS));
            lpNewData->lppPositions[i] = lpOutPos;
        }

        lpNewData->lpszExtra = nullptr;
        if (lpStatus->lpszExtra != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpStatus->lpszExtra);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpStatus->lpszExtra 格式错误");
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewData->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
            memcpy(lpNewData->lpszExtra, lpStatus->lpszExtra, sizeof(char) * uLen);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMDENOMINATION(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    auto lpDenoData = static_cast<LPWFSCDMDENOMINATION>(lpData);
    if (lpDenoData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCDMDENOMINATION lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMDENOMINATION), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);

        memcpy(lpNewData, lpDenoData, sizeof(WFSCDMDENOMINATION));

        USHORT usCount = lpNewData->usCount;
        if (usCount == 0)
        {
            lpNewData->lpulValues = nullptr;
        }
        else
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(ULONG) * usCount, lpResult, (LPVOID *)&lpNewData->lpulValues);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpulValues);

            memcpy(lpNewData->lpulValues, lpDenoData->lpulValues, sizeof(ULONG) * usCount);
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMMEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto lpItemPos = static_cast<LPWFSCDMITEMPOSITION>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        LPWFSCDMITEMPOSITION lpNewPos = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMITEMPOSITION), lpResult, (LPVOID *)&lpNewPos);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpNewPos);
        memcpy(lpNewPos, lpItemPos, sizeof(WFSCDMITEMPOSITION));

        lpNewPos->lpRetractArea = nullptr;
        if (lpItemPos->lpRetractArea != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMRETRACT), lpResult, (LPVOID *)&lpNewPos->lpRetractArea);
            if (hRet != WFS_SUCCESS)
                return hRet;
            autofree.push_back(lpNewPos->lpRetractArea);
            memcpy(lpNewPos->lpRetractArea, lpItemPos->lpRetractArea, sizeof(WFSCDMRETRACT));
        }

        lpResult->lpBuffer = lpNewPos;
        lpNewPos = nullptr;
    } while (false);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMCIMCOUNTSCHANGED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto lpChangeData = static_cast<LPWFSCDMCOUNTSCHANGED>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);
    do
    {
        LPWFSCDMCOUNTSCHANGED lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCOUNTSCHANGED), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpChangeData, sizeof(WFSCDMCOUNTSCHANGED));
        autofree.push_back(lpNewData);

        USHORT usCount = lpNewData->usCount;
        LPUSHORT lpCUNumList = nullptr;
        if (lpChangeData->lpusCUNumList != nullptr && (usCount > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(usCount * sizeof(USHORT), lpResult, (LPVOID *)&lpCUNumList);
            if (hRet != WFS_SUCCESS)
                return hRet;
            autofree.push_back(lpCUNumList);
            memcpy(lpCUNumList, lpChangeData->lpusCUNumList, usCount * sizeof(USHORT));
        }
        lpNewData->lpusCUNumList = lpCUNumList;
        lpCUNumList = nullptr;

        lpResult->lpBuffer = lpNewData;
    } while (FALSE);
    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMCASHUNIT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpCastte = static_cast<LPWFSCDMCASHUNIT>(lpData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        LPWFSCDMCASHUNIT lpNewCass;
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCASHUNIT), lpResult, (LPVOID *)&lpNewCass);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewCass);

        memcpy(lpNewCass, lpCastte, sizeof(WFSCDMCASHUNIT));

        lpNewCass->lpszCashUnitName = nullptr;
        char *pszCashName = lpCastte->lpszCashUnitName;
        if (pszCashName != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(strlen(pszCashName) + 1, lpResult, (LPVOID *)&lpNewCass->lpszCashUnitName);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lpszCashUnitName);

            memcpy(lpNewCass->lpszCashUnitName, lpCastte->lpszCashUnitName, sizeof(char) * (strlen(pszCashName) + 1));
        }

        USHORT usPhyCUCount = lpCastte->usNumPhysicalCUs;
        if (usPhyCUCount < 1)
            break;

        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewCass->lppPhysical);

        for (int j = 0; j < usPhyCUCount; j++)
        {
            LPWFSCDMPHCU lpNewPCU = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMPHCU), lpResult, (LPVOID *)&lpNewPCU);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewPCU);

            memcpy(lpNewPCU, lpCastte->lppPhysical[j], sizeof(WFSCDMPHCU));

            lpNewPCU->lpPhysicalPositionName = nullptr;
            char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
            if (lpPhyPosName != nullptr)
            {
                hRet = m_pIWFM->WFMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewPCU->lpPhysicalPositionName);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewPCU->lpPhysicalPositionName);

                memcpy(lpNewPCU->lpPhysicalPositionName, lpCastte->lppPhysical[j]->lpPhysicalPositionName, sizeof(char) * (strlen(lpPhyPosName) + 1));
            }
            lpNewCass->lppPhysical[j] = lpNewPCU;
        }

        lpResult->lpBuffer = lpNewCass;
        lpNewCass = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMCUERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto lpError = static_cast<LPWFSCIMCUERROR>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);
    do
    {
        LPWFSCIMCUERROR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCUERROR), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpError, sizeof(WFSCIMCUERROR));
        autofree.push_back(lpNewData);

        hRet = Fmt_WFSCIMCASHIN(lpError->lpCashUnit, lpResult);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpResult->lpBuffer);

        lpNewData->lpCashUnit = (LPWFSCIMCASHIN)lpResult->lpBuffer;
        lpResult->lpBuffer = lpNewData;
    } while (FALSE);
    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMSUBCASHIN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto lpError = static_cast<LPWFSCIMNOTENUMBERLIST>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);
    do
    {
        LPWFSCIMNOTENUMBERLIST lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpError, sizeof(WFSCIMNOTENUMBERLIST));
        autofree.push_back(lpNewData);
        if (lpNewData->usNumOfNoteNumbers > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * lpNewData->usNumOfNoteNumbers, lpResult, (LPVOID *)&(lpNewData->lppNoteNumber));
            if (hRet != WFS_SUCCESS)
                return hRet;
            autofree.push_back(lpNewData->lppNoteNumber);
            LPWFSCIMNOTENUMBER pData = nullptr;
            memset(lpNewData->lppNoteNumber, 0x00, sizeof(LPWFSCIMNOTENUMBER) * lpNewData->usNumOfNoteNumbers);
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBER) * lpNewData->usNumOfNoteNumbers, lpResult, (LPVOID *)&(pData));
            if (hRet != WFS_SUCCESS)
                return hRet;
            autofree.push_back(pData);
            for (int i = 0; i < lpNewData->usNumOfNoteNumbers; i++)
            {
                memcpy(&pData[i], lpError->lppNoteNumber[i], sizeof(WFSCIMNOTENUMBER));
                lpNewData->lppNoteNumber[i] = &pData[i];
            }
        }

        lpResult->lpBuffer = lpNewData;
    } while (FALSE);
    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMCUERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    auto lpError = static_cast<LPWFSCDMCUERROR>(lpData);
    CAutoWFMFreeBuffer autofree(m_pIWFM, &hRet);
    do
    {
        LPWFSCDMCUERROR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCUERROR), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpError, sizeof(WFSCDMCUERROR));
        autofree.push_back(lpNewData);

        hRet = Fmt_WFSCDMCASHUNIT(lpError->lpCashUnit, lpResult);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpResult->lpBuffer);

        lpNewData->lpCashUnit = (LPWFSCDMCASHUNIT)lpResult->lpBuffer;
        lpResult->lpBuffer = lpNewData;
    } while (FALSE);
    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCDMCUINFO(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpCUData = static_cast<LPWFSCDMCUINFO>(lpData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        LPWFSCDMCUINFO lpNewInfo;
        LPWFSCDMCASHUNIT *lppCassAry;
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCUINFO), lpResult, (LPVOID *)&lpNewInfo);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewInfo);
        memcpy(lpNewInfo, lpCUData, sizeof(WFSCDMCUINFO));

        USHORT usCount = lpNewInfo->usCount;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMCASHUNIT) * usCount, lpResult, (LPVOID *)&lppCassAry);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lppCassAry);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCDMCASHUNIT lpNewCass;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCASHUNIT), lpResult, (LPVOID *)&lpNewCass);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass);

            LPWFSCDMCASHUNIT lpCastte = lpCUData->lppList[i];
            memcpy(lpNewCass, lpCastte, sizeof(WFSCDMCASHUNIT));

            char *pszCashName = lpCastte->lpszCashUnitName;
            lpNewCass->lpszCashUnitName = nullptr;
            if (pszCashName != nullptr)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * (strlen(pszCashName) + 1), lpResult, (LPVOID *)&lpNewCass->lpszCashUnitName);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lpszCashUnitName);

                memcpy(lpNewCass->lpszCashUnitName, lpCastte->lpszCashUnitName, sizeof(char) * (strlen(pszCashName) + 1));
            }

            USHORT usPhyCUCount = lpCastte->usNumPhysicalCUs;
            if (usPhyCUCount < 1)
                break;

            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lppPhysical);

            for (int j = 0; j < usPhyCUCount; j++)
            {
                LPWFSCDMPHCU lpNewPCU = nullptr;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMPHCU), lpResult, (LPVOID *)&lpNewPCU);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewPCU);

                memcpy(lpNewPCU, lpCastte->lppPhysical[j], sizeof(WFSCDMPHCU));

                lpNewPCU->lpPhysicalPositionName = nullptr;
                char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
                if (lpPhyPosName)
                {
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * (strlen(lpPhyPosName) + 1), lpResult, (LPVOID *)&lpNewPCU->lpPhysicalPositionName);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewPCU->lpPhysicalPositionName);

                    memcpy(lpNewPCU->lpPhysicalPositionName, lpCastte->lppPhysical[j]->lpPhysicalPositionName, sizeof(char) * (strlen(lpPhyPosName) + 1));
                }

                lpNewCass->lppPhysical[j] = lpNewPCU;
            }

            lppCassAry[i] = lpNewCass;
        }

        lpNewInfo->lppList = lppCassAry;
        lpResult->lpBuffer = lpNewInfo;
        lpNewInfo = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::FmtCDMExeResultBuffer(DWORD dwCommand, LPVOID lpData, LPWFSRESULT &lpResult, DWORD dwSize)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCommand)
    {
    case WFS_CMD_CDM_START_EXCHANGE:
    case WFS_CMD_CDM_TEST_CASH_UNITS:                           //30-00-00-00(FS#0007)
    {
        hRet = Fmt_WFSCDMCUINFO(lpData, lpResult);
        break;
    }
    case WFS_CMD_CDM_DISPENSE:
    {
        hRet = Fmt_WFSCDMDENOMINATION(lpData, lpResult);
        break;
    }
    case WFS_CMD_CDM_DENOMINATE:
    {
        hRet = Fmt_WFSCDMDENOMINATION(lpData, lpResult);
        break;
    }
    default:
        hRet = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRet;
}

HRESULT CFMTResData::FmtCIMGetResultBuffer(DWORD dwCategory, LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCategory)
    {
    case WFS_INF_CIM_STATUS:
    {
        hRet = Fmt_WFSCIMSTATUS(lpData, lpResult);
        break;
    }
    case WFS_INF_CIM_CAPABILITIES:
    {
        hRet = Fmt_WFSCIMCAPS(lpData, lpResult);
        break;
    }
    case WFS_INF_CIM_CASH_UNIT_INFO:
    {
        hRet = Fmt_WFSCIMCASHINFO(lpData, lpResult);
        break;
    }
    case WFS_INF_CIM_BANKNOTE_TYPES:
    {
        hRet = Fmt_WFSCIMBANKNOTETYPES(lpData, lpResult);
        break;
    }
    case WFS_INF_CIM_CASH_IN_STATUS:
    {
        hRet = Fmt_WFSCIMCASHINSTATUS(lpData, lpResult);
        break;
    }
    case WFS_INF_CIM_CURRENCY_EXP:
    {
        hRet = Fmt_WFSCIMCURRENCYEXP(lpData, lpResult);
        break;
    }
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpStatus = static_cast<LPWFSCIMSTATUS>(lpData);
    if (lpStatus == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCIMSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSCIMSTATUS));

        LPWFSCIMINPOS *lppOutPos = lpStatus->lppPositions;
        while (*lppOutPos != nullptr)
        {
            lppOutPos++;
        }
        USHORT usCount = lppOutPos - lpStatus->lppPositions;
        lppOutPos = lpStatus->lppPositions;

        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMINPOS) * (usCount + 1), lpResult, (LPVOID *)&lpNewData->lppPositions);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lppPositions);

        memset(lpNewData->lppPositions, 0, sizeof(LPWFSCIMINPOS) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            LPWFSCIMINPOS lpOutPos = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMINPOS), lpResult, (LPVOID *)&lpOutPos);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpOutPos);

            memcpy(lpOutPos, lppOutPos[i], sizeof(WFSCIMINPOS));
            lpNewData->lppPositions[i] = lpOutPos;
        }

        LPCSTR lpExtra = lpStatus->lpszExtra;
        if (lpExtra != nullptr)
        {
            usCount = GetLenOfSZZ(lpStatus->lpszExtra);
            if (usCount <= 0)
            {
                Log(ThisModule, __LINE__, "lpStatus->lpszExtra 格式错误");
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * usCount, lpResult, (LPVOID *)&lpNewData->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
            memcpy(lpNewData->lpszExtra, lpStatus->lpszExtra, sizeof(char) * usCount);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpCaps = static_cast<LPWFSCIMCAPS>(lpData);
    if (lpCaps == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCIMCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSCIMCAPS));

        if (lpCaps->lpszExtra != nullptr)
        {
            UINT uCount = GetLenOfSZZ(lpCaps->lpszExtra);
            if (uCount <= 0)
            {
                Log(ThisModule, __LINE__, "lpCaps->lpszExtra 格式错误");
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uCount, lpResult, (LPVOID *)&lpNewData->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
            memcpy(lpNewData->lpszExtra, lpCaps->lpszExtra, sizeof(char) * uCount);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMBANKNOTETYPES(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    auto lpNoteTypeList = static_cast<LPWFSCIMNOTETYPELIST>(lpData);
    if (lpNoteTypeList == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCIMNOTETYPELIST lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTETYPELIST), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);

        memcpy(lpNewData, lpNoteTypeList, sizeof(WFSCIMNOTETYPELIST));

        USHORT usCount = lpNewData->usNumOfNoteTypes;
        if (usCount > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTETYPE) * usCount, lpResult, (LPVOID *)&lpNewData->lppNoteTypes);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lppNoteTypes);
            for (int i = 0; i < usCount; i++)
            {
                LPWFSCIMNOTETYPE lpNoteType = nullptr;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTETYPE), lpResult, (LPVOID *)&lpNoteType);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNoteType);

                memcpy(lpNoteType, lpNoteTypeList->lppNoteTypes[i], sizeof(WFSCIMNOTETYPE));
                lpNewData->lppNoteTypes[i] = lpNoteType;
            }
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMCURRENCYEXP(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    auto lppCurrencyExp = static_cast<LPWFSCIMCURRENCYEXP *>(lpData);
    if (lppCurrencyExp == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCIMCURRENCYEXP *lppNewData = nullptr;
        auto lppPos = lppCurrencyExp;
        while (*lppPos != nullptr)
        {
            lppPos++;
        }
        USHORT usCount = lppPos - lppCurrencyExp;

        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMCURRENCYEXP) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0, sizeof(LPWFSCIMCURRENCYEXP) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCURRENCYEXP), lpResult, (LPVOID *)&lppNewData[i]);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lppNewData[i]);

            memcpy(lppNewData[i], lppCurrencyExp[i], sizeof(WFSCIMCURRENCYEXP));
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMCASHINSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEndType(m_szLogType);
    auto lpCashInStatus = static_cast<LPWFSCIMCASHINSTATUS>(lpData);
    if (lpCashInStatus == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCIMCASHINSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCASHINSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCashInStatus, sizeof(WFSCIMCASHINSTATUS));

        LPWFSCIMNOTENUMBERLIST lpNewNoteNumList = nullptr;
        if (lpCashInStatus->lpNoteNumberList != nullptr)
        {
//30-00-00-00(FT#0061)            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);     //30-00-00-00(FT#0061)
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewNoteNumList);

            memcpy(lpNewNoteNumList, lpCashInStatus->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));
            lpNewNoteNumList->lppNoteNumber = nullptr;              //30-00-00-00(FT#0061)

            USHORT usCount = lpNewNoteNumList->usNumOfNoteNumbers;
            if(usCount > 0){                                        //30-00-00-00(FT#0061)
                hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usCount, lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewNoteNumList->lppNoteNumber);

                for (int i = 0; i < usCount; i++)
                {
                    LPWFSCIMNOTENUMBER lpNoteNum = nullptr;
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNoteNum);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNoteNum);

                    memcpy(lpNoteNum, lpCashInStatus->lpNoteNumberList->lppNoteNumber[i], sizeof(WFSCIMNOTENUMBER));
                    lpNewNoteNumList->lppNoteNumber[i] = lpNoteNum;
                }
            }                                                     //30-00-00-00(FT#0061)
        }

        lpNewData->lpNoteNumberList = lpNewNoteNumList;
        lpNewNoteNumList = nullptr;

        LPCSTR lpExt = lpCashInStatus->lpszExtra;
        if (lpExt != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpCashInStatus->lpszExtra);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpCashInStatus->lpszExtra 格式错误");
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewData->lpszExtra);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpszExtra);
            memcpy(lpNewData->lpszExtra, lpCashInStatus->lpszExtra, sizeof(char) * uLen);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMNOTENUMBERLIST(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    auto lpNListData = static_cast<LPWFSCIMNOTENUMBERLIST>(lpData);
    if (lpNListData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        LPWFSCIMNOTENUMBERLIST lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);

        memcpy(lpNewData, lpNListData, sizeof(WFSCIMNOTENUMBERLIST));

        USHORT usCount = lpNewData->usNumOfNoteNumbers;
        if (usCount > 0)
        {
            //这里增加一个NULL结束，主要是为了兼容（长沙银行CashInStatus获取出错的问题，该行使用NULL作为判断长度的方式）
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * (usCount + 1), lpResult, (LPVOID *)&lpNewData->lppNoteNumber);
            if (hRet != WFS_SUCCESS)
                return hRet;

            _auto.push_back(lpNewData->lppNoteNumber);
            memset(lpNewData->lppNoteNumber, 0, sizeof(LPWFSCIMNOTENUMBER) * (usCount + 1));

            for (int i = 0; i < usCount; i++)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNewData->lppNoteNumber[i]);
                if (hRet != WFS_SUCCESS)
                    return hRet;

                _auto.push_back(lpNewData->lppNoteNumber[i]);
                memcpy(lpNewData->lppNoteNumber[i], lpNListData->lppNoteNumber[i], sizeof(WFSCIMNOTENUMBER));
            }
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CFMTResData::Fmt_WFSCIMCASHINFO(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    auto lpCashInfo = static_cast<LPWFSCIMCASHINFO>(lpData);
    if (lpCashInfo == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCIMCASHINFO lpNewData = nullptr;
    do
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCASHINFO), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCashInfo, sizeof(WFSCIMCASHINFO));

        LPWFSCIMCASHIN *lppCassAry;
        USHORT usCount = lpNewData->usCount;
        if (usCount <= 0)
            break;

        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMCASHIN) * usCount, lpResult, (LPVOID *)&lppCassAry);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lppCassAry);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCIMCASHIN lpNewCass;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCASHIN), lpResult, (LPVOID *)&lpNewCass);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass);

            LPWFSCIMCASHIN lpCastte = lpCashInfo->lppCashIn[i];
            memcpy(lpNewCass, lpCastte, sizeof(WFSCIMCASHIN));

            // notenumberlist
            lpNewCass->lpNoteNumberList = nullptr;
            if (lpCastte->lpNoteNumberList != nullptr)
            {
                LPWFSCIMNOTENUMBERLIST lpNewNoteNumList = nullptr;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewNoteNumList);
                memcpy(lpNewNoteNumList, lpCastte->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));

                USHORT usNoteNum = lpNewNoteNumList->usNumOfNoteNumbers;
                if (usNoteNum > 0)
                {
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usNoteNum, lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewNoteNumList->lppNoteNumber);

                    for (int m = 0; m < usNoteNum; m++)
                    {
                        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber[m]);
                        if (hRet != WFS_SUCCESS)
                            return hRet;
                        _auto.push_back(lpNewNoteNumList->lppNoteNumber[m]);
                        memcpy(lpNewNoteNumList->lppNoteNumber[m], lpCastte->lpNoteNumberList->lppNoteNumber[m], sizeof(WFSCIMNOTENUMBER));
                    }
                }

                lpNewCass->lpNoteNumberList = lpNewNoteNumList;
                lpNewNoteNumList = nullptr;
            }

            // phyCU
            USHORT usPhyCUCount = lpNewCass->usNumPhysicalCUs;
            if (usPhyCUCount < 1)
                break;

            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lppPhysical);

            for (int j = 0; j < usPhyCUCount; j++)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMPHCU), lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lppPhysical[j]);

                memcpy(lpNewCass->lppPhysical[j], lpCastte->lppPhysical[j], sizeof(WFSCIMPHCU));

                char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
                lpNewCass->lppPhysical[j]->lpPhysicalPositionName = nullptr;
                hRet = m_pIWFM->WFMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                memcpy(lpNewCass->lppPhysical[j]->lpPhysicalPositionName, lpCastte->lppPhysical[j]->lpPhysicalPositionName,
                       sizeof(char) * (strlen(lpPhyPosName) + 1));

                lpNewCass->lppPhysical[j]->lpszExtra = nullptr;
                char *lpExtra = lpCastte->lppPhysical[j]->lpszExtra;
                if (lpExtra != nullptr)
                {
                    UINT uLen = GetLenOfSZZ(lpExtra);
                    if (uLen <= 0)
                    {
                        Log(ThisModule, __LINE__, "lppPhysical[j]->lpszExtra 格式错误", j);
                        return WFS_ERR_INVALID_DATA;
                    }
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpszExtra);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewCass->lppPhysical[j]->lpszExtra);

                    memcpy(lpNewCass->lppPhysical[j]->lpszExtra, lpCastte->lppPhysical[j]->lpszExtra, sizeof(char) * uLen);
                }
            }

            lpNewCass->lpszExtra = nullptr;
            char *lpExt = lpCastte->lpszExtra;
            if (lpExt != nullptr)
            {
                UINT uLen = GetLenOfSZZ(lpExt);
                if (uLen <= 0)
                {
                    Log(ThisModule, __LINE__, "lpCastte->lpszExtra 格式错误");
                    return WFS_ERR_INVALID_DATA;
                }
                hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewCass->lpszExtra);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lpszExtra);

                memcpy(lpNewCass->lpszExtra, lpCastte->lpszExtra, sizeof(char) * uLen);
            }

            lppCassAry[i] = lpNewCass;
        }

        lpNewData->lppCashIn = lppCassAry;
    } while (FALSE);

    lpResult->lpBuffer = lpNewData;
    lpNewData = nullptr;
    return hRet;
}

HRESULT CFMTResData::Fmt_NODATA(LPWFSRESULT &lpResult)
{
    lpResult->lpBuffer = nullptr;
    return WFS_SUCCESS;
}

HRESULT CFMTResData::FmtCIMExeResultBuffer(DWORD dwCommand, LPVOID lpData, LPWFSRESULT &lpResult, DWORD dwSize)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCommand)
    {
    case WFS_CMD_CIM_CASH_IN:
        hRet = Fmt_WFSCIMNOTENUMBERLIST(lpData, lpResult);
        break;
    case WFS_CMD_CIM_CASH_IN_END:
        hRet = Fmt_WFSCIMCASHINFO(lpData, lpResult);
        break;
    case WFS_CMD_CIM_START_EXCHANGE:
        hRet = Fmt_WFSCIMCASHINFO(lpData, lpResult);
        break;
    case WFS_CMD_CIM_CASH_IN_ROLLBACK:
        hRet = Fmt_WFSCIMCASHINFO(lpData, lpResult);
        break;
    default:
        hRet = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRet;
}
