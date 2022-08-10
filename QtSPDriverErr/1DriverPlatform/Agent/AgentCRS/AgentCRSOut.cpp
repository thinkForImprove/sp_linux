#include "AgentCRS.h"

static const char *DEVTYPE = "CRS";
static const char *ThisFile = "AgentCRSOut.cpp";

////////////////////////////////////////////
HRESULT CAgent_CRS::Get_WFS_INF_CDM_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSCDMSTATUS>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMSTATUS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCDMSTATUS lpGRG_Status = (LPWFSCDMSTATUS)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpData, sizeof(WFSCDMSTATUS));
        lpGRG_Status->lpszExtra = nullptr;
        lpGRG_Status->lppPositions = nullptr;

        if (lpData->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpGRG_Status->lpszExtra, lpData->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpGRG_Status->lpszExtra);
        }

        if (lpData->lppPositions != nullptr)
        {
            LPWFSCDMOUTPOS *lppOutPos = lpData->lppPositions;
            while (*lppOutPos != nullptr)
            {
                lppOutPos++;
            }
            USHORT usCount = lppOutPos - lpData->lppPositions;

            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMOUTPOS) * (usCount + 1), lpResult, (LPVOID *)&lpGRG_Status->lppPositions);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpGRG_Status->lppPositions);

            memset(lpGRG_Status->lppPositions, 0, sizeof(LPWFSCDMOUTPOS) * (usCount + 1));

            for (int i = 0; i < usCount; i++)
            {
                if(lpData->lppPositions[i] != nullptr){
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMOUTPOS), lpResult, (LPVOID *)&lpGRG_Status->lppPositions[i]);
                    if (hRet != WFS_SUCCESS)
                    {
                        break;
                    }
                    _auto.push_back(lpGRG_Status->lppPositions[i]);
                    memcpy(lpGRG_Status->lppPositions[i], lpData->lppPositions[i], sizeof(WFSCDMOUTPOS));
                }

            }
        }

        lpResult->lpBuffer = lpGRG_Status;
        lpGRG_Status = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgent_CRS::Get_WFS_INF_CDM_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSCDMCAPS>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCAPS), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCDMCAPS lpGRG_Status = (LPWFSCDMCAPS)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpData, sizeof(WFSCDMCAPS));
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

HRESULT CAgent_CRS::Get_WFS_INF_CDM_CASH_UNIT_INFO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpData = static_cast<LPWFSCDMCUINFO>(lpQueryDetails);
        if (lpData == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCUINFO), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCDMCUINFO lpGRG_Status = (LPWFSCDMCUINFO)lpResult->lpBuffer;
        if (lpGRG_Status == nullptr)
            break;

        memcpy(lpGRG_Status, lpData, sizeof(WFSCDMCUINFO));
        if (lpGRG_Status->lppList != nullptr)
        {
            USHORT usCount = lpGRG_Status->usCount;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMCUINFO) * (usCount + 1), lpResult, (LPVOID *)&lpGRG_Status->lppList);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpGRG_Status->lppList);

            for (int i = 0; i < usCount; i++)
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCASHUNIT), lpResult, (LPVOID *)&lpGRG_Status->lppList[i]);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpGRG_Status->lppList[i]);
                memcpy(lpGRG_Status->lppList[i], lpData->lppList[i], sizeof(WFSCDMCASHUNIT));

                LPWFSCDMCASHUNIT lpCastte = lpGRG_Status->lppList[i];
                char *pszCashName = lpCastte->lpszCashUnitName;
                if (pszCashName != nullptr)
                {
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * (strlen(pszCashName) + 1), lpResult, (LPVOID *)&lpCastte->lpszCashUnitName);
                    if (hRet != WFS_SUCCESS)
                        break;
                    _auto.push_back(lpCastte->lpszCashUnitName);

                    memcpy(lpCastte->lpszCashUnitName, lpData->lppList[i]->lpszCashUnitName, sizeof(char) * (strlen(pszCashName) + 1));
                }

                USHORT usPhyCUCount = lpCastte->usNumPhysicalCUs;
                if (usPhyCUCount < 1)
                    break;

                hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpCastte->lppPhysical);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpCastte->lppPhysical);

                for (int j = 0; j < usPhyCUCount; j++)
                {
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMPHCU), lpResult, (LPVOID *)&lpCastte->lppPhysical[j]);
                    if (hRet != WFS_SUCCESS)
                        break;
                    _auto.push_back(lpCastte->lppPhysical[j]);

                    memcpy(lpCastte->lppPhysical[j], lpData->lppList[i]->lppPhysical[j], sizeof(WFSCDMPHCU));
                    char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
                    if (lpPhyPosName)
                    {
                        hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * (strlen(lpPhyPosName) + 1), lpResult, (LPVOID *)&lpCastte->lppPhysical[j]->lpPhysicalPositionName);
                        if (hRet != WFS_SUCCESS)
                            break;
                        _auto.push_back(lpCastte->lppPhysical[j]->lpPhysicalPositionName);

                        memcpy(lpCastte->lppPhysical[j]->lpPhysicalPositionName, lpData->lppList[i]->lppPhysical[j]->lpPhysicalPositionName, sizeof(char) * (strlen(lpPhyPosName) + 1));
                    }
                }
            }
        }

        lpResult->lpBuffer = lpGRG_Status;
        lpGRG_Status = nullptr;
    } while (false);

    return hRet;
}

// ok , up interface need recheck
HRESULT CAgent_CRS::Get_WFS_INF_CDM_MIX_TYPES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lppMixType = static_cast<LPWFSCDMMIXTYPE *>(lpQueryDetails);
        if (lppMixType == nullptr)
            break;

        LPWFSCDMMIXTYPE *lppPos = lppMixType;
        while (*lppPos != nullptr)
        {
            lppPos++;
        }
        USHORT usCount = lppPos - lppMixType;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMMIXTYPE) * (usCount + 1), lpResult, (LPVOID *)&lpResult->lpBuffer);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpResult->lpBuffer);
        LPWFSCDMMIXTYPE *lpNewData = (LPWFSCDMMIXTYPE *)lpResult->lpBuffer;
        memcpy(lpNewData, lppMixType, sizeof(LPWFSCDMMIXTYPE) * (usCount + 1));

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCDMMIXTYPE lpMixType = nullptr;
            HRESULT hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMMIXTYPE), lpResult, (LPVOID *)&lpMixType);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpMixType);
            memcpy(lpMixType, lppMixType[i], sizeof(WFSCDMMIXTYPE));

            UINT uLen = strlen(lppMixType[i]->lpszName) + 1;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpMixType->lpszName);
            if (hRet != WFS_SUCCESS)
                break;
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

HRESULT CAgent_CRS::Get_WFS_INF_CDM_PRESENT_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpPreStats = static_cast<LPWFSCDMPRESENTSTATUS>(lpQueryDetails);
        if (lpPreStats == nullptr)
            break;

        LPWFSCDMPRESENTSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMPRESENTSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPreStats, sizeof(WFSCDMPRESENTSTATUS));

        LPWFSCDMDENOMINATION lpDeno = nullptr;
        if (lpPreStats->lpDenomination != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMDENOMINATION), lpResult, (LPVOID *)&lpDeno);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpDeno);
            memcpy(lpDeno, lpPreStats->lpDenomination, sizeof(WFSCDMDENOMINATION));
            USHORT usCount = lpDeno->usCount;
            if (usCount == 0)
            {
                lpDeno->lpulValues = nullptr;
            }
            else
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(ULONG) * usCount, lpResult, (LPVOID *)&lpDeno->lpulValues);
                if (hRet != WFS_SUCCESS)
                    break;
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
            hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewData->lpszExtra);
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

HRESULT CAgent_CRS::Get_WFS_INF_CDM_CURRENCY_EXP_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lppCurrencyExp = static_cast<LPWFSCDMCURRENCYEXP *>(lpQueryDetails);
        if (lppCurrencyExp == nullptr)
            break;
        LPWFSCDMCURRENCYEXP *lppNewData = nullptr;
        auto lppPos = lppCurrencyExp;
        while (*lppPos != nullptr)
        {
            lppPos++;
        }
        USHORT usCount = lppPos - lppCurrencyExp;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMCURRENCYEXP) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0, sizeof(LPWFSCDMCURRENCYEXP) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCURRENCYEXP), lpResult, (LPVOID *)&lppNewData[i]);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lppNewData[i]);

            memcpy(lppNewData[i], lppCurrencyExp[i], sizeof(WFSCDMCURRENCYEXP));
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData = nullptr;
    } while (FALSE);

    return hRet;
}


HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_DENOMINATE_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpDenoData = static_cast<LPWFSCDMDENOMINATION>(lpQueryDetails);
        if (lpDenoData == nullptr)
            break;
        LPWFSCDMDENOMINATION lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMDENOMINATION), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);

        memcpy(lpNewData, lpDenoData, sizeof(WFSCDMDENOMINATION));

        USHORT usCount = lpNewData->usCount;
        if (usCount == 0)
        {
            lpNewData->lpulValues = nullptr;
        }
        else
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(ULONG) * usCount, lpResult, (LPVOID *)&lpNewData->lpulValues);
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


HRESULT CAgent_CRS::ExeWFS_CMD_CDM_START_EXCHANGE_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCUData = static_cast<LPWFSCDMCUINFO>(lpQueryDetails);
        if (lpCUData == nullptr)
            break;

        LPWFSCDMCUINFO lpNewInfo;
        LPWFSCDMCASHUNIT *lppCassAry;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCUINFO), lpResult, (LPVOID *)&lpNewInfo);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewInfo);
        memcpy(lpNewInfo, lpCUData, sizeof(WFSCDMCUINFO));

        USHORT usCount = lpNewInfo->usCount;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMCASHUNIT) * usCount, lpResult, (LPVOID *)&lppCassAry);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lppCassAry);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCDMCASHUNIT lpNewCass;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCASHUNIT), lpResult, (LPVOID *)&lpNewCass);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewCass);

            LPWFSCDMCASHUNIT lpCastte = lpCUData->lppList[i];
            memcpy(lpNewCass, lpCastte, sizeof(WFSCDMCASHUNIT));

            char *pszCashName = lpCastte->lpszCashUnitName;
            lpNewCass->lpszCashUnitName = nullptr;
            if (pszCashName != nullptr)
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * (strlen(pszCashName) + 1), lpResult, (LPVOID *)&lpNewCass->lpszCashUnitName);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpNewCass->lpszCashUnitName);

                memcpy(lpNewCass->lpszCashUnitName, lpCastte->lpszCashUnitName, sizeof(char) * (strlen(pszCashName) + 1));
            }

            USHORT usPhyCUCount = lpCastte->usNumPhysicalCUs;
            if (usPhyCUCount < 1)
                break;

            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewCass->lppPhysical);

            for (int j = 0; j < usPhyCUCount; j++)
            {
                LPWFSCDMPHCU lpNewPCU = nullptr;
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMPHCU), lpResult, (LPVOID *)&lpNewPCU);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpNewPCU);

                memcpy(lpNewPCU, lpCastte->lppPhysical[j], sizeof(WFSCDMPHCU));

                lpNewPCU->lpPhysicalPositionName = nullptr;
                char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
                if (lpPhyPosName)
                {
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * (strlen(lpPhyPosName) + 1), lpResult, (LPVOID *)&lpNewPCU->lpPhysicalPositionName);
                    if (hRet != WFS_SUCCESS)
                        break;
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

HRESULT CAgent_CRS::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
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



HRESULT CAgent_CRS::Get_WFS_INF_CIM_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpStatus = static_cast<LPWFSCIMSTATUS>(lpQueryDetails);
        if (lpStatus == nullptr)
            break;
        LPWFSCIMSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSCIMSTATUS));

        LPWFSCIMINPOS *lppOutPos = lpStatus->lppPositions;
        while (*lppOutPos != nullptr)
        {
            lppOutPos++;
        }
        USHORT usCount = lppOutPos - lpStatus->lppPositions;
        lppOutPos = lpStatus->lppPositions;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMINPOS) * (usCount + 1), lpResult, (LPVOID *)&lpNewData->lppPositions);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData->lppPositions);

        memset(lpNewData->lppPositions, 0, sizeof(LPWFSCIMINPOS) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            LPWFSCIMINPOS lpOutPos = nullptr;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMINPOS), lpResult, (LPVOID *)&lpOutPos);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpOutPos);

            memcpy(lpOutPos, lppOutPos[i], sizeof(WFSCIMINPOS));
            lpNewData->lppPositions[i] = lpOutPos;
        }

        if (lpStatus->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpStatus->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpNewData->lpszExtra);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}


HRESULT CAgent_CRS::Get_WFS_INF_CIM_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCaps = static_cast<LPWFSCIMCAPS>(lpQueryDetails);
        if (lpCaps == nullptr)
            break;

        LPWFSCIMCAPS lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSCIMCAPS));

        if (lpCaps->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpCaps->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpNewData->lpszExtra);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_CASH_UNIT_INFO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCIMCASHINFO lpNewData = nullptr;
    do
    {
        auto lpCashInfo = static_cast<LPWFSCIMCASHINFO>(lpQueryDetails);
        if (lpCashInfo == nullptr)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCASHINFO), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCashInfo, sizeof(WFSCIMCASHINFO));

        LPWFSCIMCASHIN *lppCassAry;
        USHORT usCount = lpNewData->usCount;
        if (usCount <= 0)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMCASHIN) * usCount, lpResult, (LPVOID *)&lppCassAry);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lppCassAry);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCIMCASHIN lpNewCass;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCASHIN), lpResult, (LPVOID *)&lpNewCass);
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
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewNoteNumList);
                memcpy(lpNewNoteNumList, lpCastte->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));

                USHORT usNoteNum = lpNewNoteNumList->usNumOfNoteNumbers;
                if (usNoteNum > 0)
                {
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usNoteNum, lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewNoteNumList->lppNoteNumber);

                    for (int m = 0; m < usNoteNum; m++)
                    {
                        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber[m]);
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

            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lppPhysical);

            for (int j = 0; j < usPhyCUCount; j++)
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMPHCU), lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lppPhysical[j]);

                memcpy(lpNewCass->lppPhysical[j], lpCastte->lppPhysical[j], sizeof(WFSCIMPHCU));

                char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
                lpNewCass->lppPhysical[j]->lpPhysicalPositionName = nullptr;
                hRet = m_pIWFM->SIMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
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
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpszExtra);
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
                hRet = m_pIWFM->SIMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpNewCass->lpszExtra);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCass->lpszExtra);

                memcpy(lpNewCass->lpszExtra, lpCastte->lpszExtra, sizeof(char) * uLen);
            }

            lppCassAry[i] = lpNewCass;
        }

        lpNewData->lppCashIn = lppCassAry;
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_BANKNOTE_TYPES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpNoteTypeList = static_cast<LPWFSCIMNOTETYPELIST>(lpQueryDetails);
        if (lpNoteTypeList == nullptr)
            break;

        LPWFSCIMNOTETYPELIST lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTETYPELIST), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);

        memcpy(lpNewData, lpNoteTypeList, sizeof(WFSCIMNOTETYPELIST));

        USHORT usCount = lpNewData->usNumOfNoteTypes;
        if (usCount > 0)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMNOTETYPE) * usCount, lpResult, (LPVOID *)&lpNewData->lppNoteTypes);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lppNoteTypes);
            for (int i = 0; i < usCount; i++)
            {
                LPWFSCIMNOTETYPE lpNoteType = nullptr;
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTETYPE), lpResult, (LPVOID *)&lpNoteType);
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


HRESULT CAgent_CRS::Get_WFS_INF_CIM_CASH_IN_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCashInStatus = static_cast<LPWFSCIMCASHINSTATUS>(lpQueryDetails);
        if (lpCashInStatus == nullptr)
            break;

        LPWFSCIMCASHINSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCASHINSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCashInStatus, sizeof(WFSCIMCASHINSTATUS));

        LPWFSCIMNOTENUMBERLIST lpNewNoteNumList = nullptr;
        if (lpCashInStatus->lpNoteNumberList != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);       //30-00-00-00(FT#0061)
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewNoteNumList);

            memcpy(lpNewNoteNumList, lpCashInStatus->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));
            lpNewNoteNumList->lppNoteNumber = nullptr;                  //30-00-00-00(FT#0061)

            USHORT usCount = lpNewNoteNumList->usNumOfNoteNumbers;
            if(usCount > 0){                                            //30-00-00-00(FT#0061)
                hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usCount, lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpNewNoteNumList->lppNoteNumber);

                for (int i = 0; i < usCount; i++)
                {
                    LPWFSCIMNOTENUMBER lpNoteNum = nullptr;
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNoteNum);
                    if (hRet != WFS_SUCCESS)
                        break;
                    _auto.push_back(lpNoteNum);

                    memcpy(lpNoteNum, lpCashInStatus->lpNoteNumberList->lppNoteNumber[i], sizeof(WFSCIMNOTENUMBER));
                    lpNewNoteNumList->lppNoteNumber[i] = lpNoteNum;
                }
            }
        }                                                               //30-00-00-00(FT#0061)

        lpNewData->lpNoteNumberList = lpNewNoteNumList;
        lpNewNoteNumList = nullptr;

        if (lpCashInStatus->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpCashInStatus->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpNewData->lpszExtra);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}


HRESULT CAgent_CRS::Get_WFS_INF_CIM_CURRENCY_EXP_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lppCurrencyExp = static_cast<LPWFSCIMCURRENCYEXP *>(lpQueryDetails);
        if (lppCurrencyExp == nullptr)
            break;

        LPWFSCIMCURRENCYEXP *lppNewData = nullptr;
        auto lppPos = lppCurrencyExp;
        while (*lppPos != nullptr)
        {
            lppPos++;
        }
        USHORT usCount = lppPos - lppCurrencyExp;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMCURRENCYEXP) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0, sizeof(LPWFSCIMCURRENCYEXP) * (usCount + 1));
        for (int i = 0; i < usCount; i++)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCURRENCYEXP), lpResult, (LPVOID *)&lppNewData[i]);
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


HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CASH_IN_END_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCashInfo = static_cast<LPWFSCIMCASHINFO>(lpQueryDetails);
        if (lpCashInfo == nullptr)
            break;
        LPWFSCIMCASHINFO lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCASHINFO), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCashInfo, sizeof(WFSCIMCASHINFO));

        LPWFSCIMCASHIN *lppCassAry;
        USHORT usCount = lpNewData->usCount;
        if (usCount <= 0)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMCASHIN) * usCount, lpResult, (LPVOID *)&lppCassAry);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lppCassAry);

        for (int i = 0; i < usCount; i++)
        {
            LPWFSCIMCASHIN lpNewCass;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCASHIN), lpResult, (LPVOID *)&lpNewCass);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewCass);

            LPWFSCIMCASHIN lpCastte = lpCashInfo->lppCashIn[i];
            memcpy(lpNewCass, lpCastte, sizeof(WFSCIMCASHIN));

            // notenumberlist
            lpNewCass->lpNoteNumberList = nullptr;
            if (lpCastte->lpNoteNumberList != nullptr)
            {
                LPWFSCIMNOTENUMBERLIST lpNewNoteNumList = nullptr;
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpNewNoteNumList);
                memcpy(lpNewNoteNumList, lpCastte->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));

                USHORT usNoteNum = lpNewNoteNumList->usNumOfNoteNumbers;
                if (usNoteNum > 0)
                {
                    hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usNoteNum, lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
                    if (hRet != WFS_SUCCESS)
                        break;
                    _auto.push_back(lpNewNoteNumList->lppNoteNumber);

                    for (int m = 0; m < usNoteNum; m++)
                    {
                        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber[m]);
                        if (hRet != WFS_SUCCESS)
                            break;
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

            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewCass->lppPhysical);

            for (int j = 0; j < usPhyCUCount; j++)
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMPHCU), lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpNewCass->lppPhysical[j]);

                memcpy(lpNewCass->lppPhysical[j], lpCastte->lppPhysical[j], sizeof(WFSCIMPHCU));

                char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
                lpNewCass->lppPhysical[j]->lpPhysicalPositionName = nullptr;
                hRet = m_pIWFM->SIMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                memcpy(lpNewCass->lppPhysical[j]->lpPhysicalPositionName, lpCastte->lppPhysical[j]->lpPhysicalPositionName,
                       sizeof(char) * (strlen(lpPhyPosName) + 1));

                if (lpCastte->lppPhysical[j]->lpszExtra != nullptr)
                {
                    hRet = Fmt_ExtraStatus(lpResult, lpNewCass->lppPhysical[j]->lpszExtra, lpCastte->lppPhysical[j]->lpszExtra);
                    if(hRet != WFS_SUCCESS){
                        break;
                    }
                    _auto.push_back(lpNewCass->lppPhysical[j]->lpszExtra);
                }
            }

            if (lpCastte->lpszExtra != nullptr)
            {
                hRet = Fmt_ExtraStatus(lpResult, lpNewCass->lpszExtra, lpCastte->lpszExtra);
                if(hRet != WFS_SUCCESS){
                    break;
                }
                _auto.push_back(lpNewCass->lpszExtra);
            }

            lppCassAry[i] = lpNewCass;
        }

        lpNewData->lppCashIn = lppCassAry;
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}


HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CASH_IN_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpNListData = static_cast<LPWFSCIMNOTENUMBERLIST>(lpQueryDetails);
        if (lpNListData == nullptr)
            break;

        LPWFSCIMNOTENUMBERLIST lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewData);

        memcpy(lpNewData, lpNListData, sizeof(WFSCIMNOTENUMBERLIST));

        USHORT usCount = lpNewData->usNumOfNoteNumbers;
        if (usCount > 0)
        {
            //这里增加一个NULL结束，主要是为了兼容（长沙银行CashInStatus获取出错的问题，该行使用NULL作为判断长度的方式）
            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * (usCount + 1), lpResult, (LPVOID *)&lpNewData->lppNoteNumber);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lppNoteNumber);
            memset(lpNewData->lppNoteNumber, 0, sizeof(LPWFSCIMNOTENUMBER) * (usCount + 1));

            for (int i = 0; i < usCount; i++)
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNewData->lppNoteNumber[i]);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewData->lppNoteNumber[i]);
                memcpy(lpNewData->lppNoteNumber[i], lpNListData->lppNoteNumber[i], sizeof(WFSCIMNOTENUMBER));
            }
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (FALSE);

    return hRet;
}

HRESULT CAgent_CRS::Fmt_WFS_CDM_CU_INFO(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCastte = static_cast<LPWFSCDMCASHUNIT>(lpData);
        if (lpData == nullptr)
            break;

        LPWFSCDMCASHUNIT lpNewCass;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCASHUNIT), lpResult, (LPVOID *)&lpNewCass);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewCass);

        memcpy(lpNewCass, lpCastte, sizeof(WFSCDMCASHUNIT));

        lpNewCass->lpszCashUnitName = nullptr;
        char *pszCashName = lpCastte->lpszCashUnitName;
        if (pszCashName != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(pszCashName) + 1, lpResult, (LPVOID *)&lpNewCass->lpszCashUnitName);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lpszCashUnitName);

            memcpy(lpNewCass->lpszCashUnitName, lpCastte->lpszCashUnitName, sizeof(char) * (strlen(pszCashName) + 1));
        }

        USHORT usPhyCUCount = lpCastte->usNumPhysicalCUs;
        if (usPhyCUCount < 1)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewCass->lppPhysical);

        for (int j = 0; j < usPhyCUCount; j++)
        {
            LPWFSCDMPHCU lpNewPCU = nullptr;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMPHCU), lpResult, (LPVOID *)&lpNewPCU);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewPCU);

            memcpy(lpNewPCU, lpCastte->lppPhysical[j], sizeof(WFSCDMPHCU));

            lpNewPCU->lpPhysicalPositionName = nullptr;
            char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
            if (lpPhyPosName != nullptr)
            {
                hRet = m_pIWFM->SIMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewPCU->lpPhysicalPositionName);
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

HRESULT CAgent_CRS::Fmt_WFS_CIM_CASH_IN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpCastte = static_cast<LPWFSCIMCASHIN>(lpData);
        if (lpCastte == nullptr)
            break;

        LPWFSCIMCASHIN lpNewCass = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCASHIN), lpResult, (LPVOID *)&lpNewCass);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewCass);

        memcpy(lpNewCass, lpCastte, sizeof(WFSCIMCASHIN));

        LPWFSCIMNOTENUMBERLIST lpNewNoteNumList = nullptr;
        if (lpCastte->lpNoteNumberList != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewNoteNumList);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewNoteNumList);
            memcpy(lpNewNoteNumList, lpCastte->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));
            USHORT usNoteNum = lpNewNoteNumList->usNumOfNoteNumbers;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usNoteNum, lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewNoteNumList->lppNoteNumber);

            for (int m = 0; m < usNoteNum; m++)
            {
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpResult, (LPVOID *)&lpNewNoteNumList->lppNoteNumber[m]);
                if (hRet != WFS_SUCCESS)
                    break;
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

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewCass->lppPhysical);

        for (int j = 0; j < usPhyCUCount; j++)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMPHCU), lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewCass->lppPhysical[j]);

            memcpy(lpNewCass->lppPhysical[j], lpCastte->lppPhysical[j], sizeof(WFSCIMPHCU));

            lpNewCass->lppPhysical[j]->lpPhysicalPositionName = nullptr;
            char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
            if (lpPhyPosName != nullptr)
            {
                hRet = m_pIWFM->SIMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                if (hRet != WFS_SUCCESS)
                    break;
                _auto.push_back(lpNewCass->lppPhysical[j]->lpPhysicalPositionName);
                memcpy(lpNewCass->lppPhysical[j]->lpPhysicalPositionName, lpCastte->lppPhysical[j]->lpPhysicalPositionName,
                       sizeof(char) * (strlen(lpPhyPosName) + 1));
            }

            if (lpCastte->lppPhysical[j]->lpszExtra != nullptr)
            {
                hRet = Fmt_ExtraStatus(lpResult, lpNewCass->lppPhysical[j]->lpszExtra, lpCastte->lppPhysical[j]->lpszExtra);
                if(hRet != WFS_SUCCESS){
                    break;
                }
                _auto.push_back(lpNewCass->lppPhysical[j]->lpszExtra);
            }
        }

        if (lpCastte->lpszExtra != nullptr)
        {
            hRet = Fmt_ExtraStatus(lpResult, lpNewCass->lpszExtra, lpCastte->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
            _auto.push_back(lpNewCass->lpszExtra);
        }

        lpResult->lpBuffer = lpNewCass;
        lpNewCass = nullptr;
    } while (FALSE);

    return hRet;
}



HRESULT CAgent_CRS::Fmt_WFS_EXEE_CDM_CASHUNITERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        auto lpError = static_cast<LPWFSCDMCUERROR>(lpData);
        if (lpError == nullptr)
            break;
        LPWFSCDMCUERROR lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCUERROR), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpError, sizeof(WFSCDMCUERROR));
        autofree.push_back(lpNewData);

        hRet = Fmt_WFS_CDM_CU_INFO(lpError->lpCashUnit, lpResult);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpResult->lpBuffer);

        lpNewData->lpCashUnit = (LPWFSCDMCASHUNIT)lpResult->lpBuffer;
        lpResult->lpBuffer = lpNewData;
    } while (FALSE);
    return hRet;
}

HRESULT CAgent_CRS::Fmt_WFS_EXEE_CIM_CASHUNITERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        auto lpError = static_cast<LPWFSCIMCUERROR>(lpData);
        if (lpError == nullptr)
            break;
        LPWFSCIMCUERROR lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMCUERROR), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpError, sizeof(WFSCIMCUERROR));
        autofree.push_back(lpNewData);

        hRet = Fmt_WFS_CIM_CASH_IN(lpError->lpCashUnit, lpResult);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpResult->lpBuffer);

        lpNewData->lpCashUnit = (LPWFSCIMCASHIN)lpResult->lpBuffer;
        lpResult->lpBuffer = lpNewData;
    } while (FALSE);
    return hRet;
}

HRESULT CAgent_CRS::Fmt_WFS_EXEE_CIM_INPUTREFUSE(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
#ifdef _PISA_STD_
        auto pstats = static_cast<LPUSHORT>(lpData);
        if (pstats == nullptr)
            break;

        LPWFSCIMINPUTREFUSE lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMINPUTREFUSE), lpResult, (LPVOID *)&lpNewData);
        if(hRet != WFS_SUCCESS){
            return hRet;
        }
        autofree.push_back(lpNewData);
        memset(lpNewData, 0, sizeof(WFSCIMINPUTREFUSE));

        LPUSHORT lpPos = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(USHORT), lpResult, (LPVOID *)&lpPos);
        if (hRet != WFS_SUCCESS)
            return hRet;
        *lpPos = *pstats;
        lpNewData->lpusReason = lpPos;
        autofree.push_back(lpPos);

        // 赋值
        lpResult->lpBuffer = lpNewData;
#else
        auto pstats = static_cast<LPUSHORT>(lpData);
        LPUSHORT lpPos = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(USHORT), lpResult, (LPVOID *)&lpPos);
        if (hRet != WFS_SUCCESS)
            break;
        *lpPos = *pstats;
        autofree.push_back(lpPos);

        lpResult->lpBuffer = lpPos;
#endif
    } while (FALSE);

    return hRet;
}

HRESULT CAgent_CRS::Fmt_WFS_EXEE_CIM_SUBCASHIN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        auto lpError = static_cast<LPWFSCIMNOTENUMBERLIST>(lpData);
        LPWFSCIMNOTENUMBERLIST lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;
        memcpy(lpNewData, lpError, sizeof(WFSCIMNOTENUMBERLIST));
        autofree.push_back(lpNewData);
        if (lpNewData->usNumOfNoteNumbers > 0)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * lpNewData->usNumOfNoteNumbers, lpResult, (LPVOID *)&(lpNewData->lppNoteNumber));
            if (hRet != WFS_SUCCESS)
                break;
            autofree.push_back(lpNewData->lppNoteNumber);
            LPWFSCIMNOTENUMBER pData = nullptr;
            memset(lpNewData->lppNoteNumber, 0x00, sizeof(LPWFSCIMNOTENUMBER) * lpNewData->usNumOfNoteNumbers);
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMNOTENUMBER) * lpNewData->usNumOfNoteNumbers, lpResult, (LPVOID *)&(pData));
            if (hRet != WFS_SUCCESS)
                break;
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


HRESULT CAgent_CRS::Fmt_WFS_SRVE_CDM_ITEMSTAKEN(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    auto pstats = static_cast<LPWORD>(lpData);
    do
    {
        LPWORD lpPos = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WORD), lpResult, (LPVOID *)&lpPos);
        if (hRet != WFS_SUCCESS)
            break;
        *lpPos = *pstats;
        autofree.push_back(lpPos);

        lpResult->lpBuffer = lpPos;
    } while (FALSE);

    return hRet;
}

HRESULT CAgent_CRS::Fmt_WFS_SRVE_CDM_CASHUNITINFOCHANGED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        auto lpCastte = static_cast<LPWFSCDMCASHUNIT>(lpData);
        if (lpData == nullptr)
            break;

        LPWFSCDMCASHUNIT lpNewCass;
        CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCASHUNIT), lpResult, (LPVOID *)&lpNewCass);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewCass);

        memcpy(lpNewCass, lpCastte, sizeof(WFSCDMCASHUNIT));

        lpNewCass->lpszCashUnitName = nullptr;
        char *pszCashName = lpCastte->lpszCashUnitName;
        if (pszCashName != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(strlen(pszCashName) + 1, lpResult, (LPVOID *)&lpNewCass->lpszCashUnitName);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCass->lpszCashUnitName);

            memcpy(lpNewCass->lpszCashUnitName, lpCastte->lpszCashUnitName, sizeof(char) * (strlen(pszCashName) + 1));
        }

        USHORT usPhyCUCount = lpCastte->usNumPhysicalCUs;
        if (usPhyCUCount < 1)
            break;

        hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSCDMPHCU) * usPhyCUCount, lpResult, (LPVOID *)&lpNewCass->lppPhysical);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewCass->lppPhysical);

        for (int j = 0; j < usPhyCUCount; j++)
        {
            LPWFSCDMPHCU lpNewPCU = nullptr;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMPHCU), lpResult, (LPVOID *)&lpNewPCU);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpNewPCU);

            memcpy(lpNewPCU, lpCastte->lppPhysical[j], sizeof(WFSCDMPHCU));

            lpNewPCU->lpPhysicalPositionName = nullptr;
            char *lpPhyPosName = lpCastte->lppPhysical[j]->lpPhysicalPositionName;
            if (lpPhyPosName != nullptr)
            {
                hRet = m_pIWFM->SIMAllocateMore(strlen(lpPhyPosName) + 1, lpResult, (LPVOID *)&lpNewPCU->lpPhysicalPositionName);
                if (hRet != WFS_SUCCESS)
                    break;
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


HRESULT CAgent_CRS::Fmt_WFS_CDM_CIM_COUNTS_CHANGED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        auto lpChangeData = static_cast<LPWFSCDMCOUNTSCHANGED>(lpData);
        LPWFSCDMCOUNTSCHANGED lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMCOUNTSCHANGED), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpChangeData, sizeof(WFSCDMCOUNTSCHANGED));
        autofree.push_back(lpNewData);

        USHORT usCount = lpNewData->usCount;
        LPUSHORT lpCUNumList = nullptr;
        if (lpChangeData->lpusCUNumList != nullptr && (usCount > 0))
        {
            hRet = m_pIWFM->SIMAllocateMore(usCount * sizeof(USHORT), lpResult, (LPVOID *)&lpCUNumList);
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

HRESULT CAgent_CRS::Fmt_WFS_SRVE_CDM_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        auto lpItemPos = static_cast<LPWFSCDMITEMPOSITION>(lpData);

        LPWFSCDMITEMPOSITION lpNewPos = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMITEMPOSITION), lpResult, (LPVOID *)&lpNewPos);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpNewPos);
        memcpy(lpNewPos, lpItemPos, sizeof(WFSCDMITEMPOSITION));

        lpNewPos->lpRetractArea = nullptr;
        if (lpItemPos->lpRetractArea != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMRETRACT), lpResult, (LPVOID *)&lpNewPos->lpRetractArea);
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

HRESULT CAgent_CRS::Fmt_WFS_SRVE_CIM_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer autofree(m_pIWFM, &hRet);

    do
    {
        auto lpItemPos = static_cast<LPWFSCIMITEMPOSITION>(lpData);
        LPWFSCIMITEMPOSITION lpNewPos = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCIMITEMPOSITION), lpResult, (LPVOID *)&lpNewPos);
        if (hRet != WFS_SUCCESS)
            return hRet;
        autofree.push_back(lpNewPos);
        memcpy(lpNewPos, lpItemPos, sizeof(WFSCIMITEMPOSITION));

        lpNewPos->lpRetractArea = nullptr;
        if (lpItemPos->lpRetractArea != nullptr)
        {
            hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSCDMRETRACT), lpResult, (LPVOID *)&lpNewPos->lpRetractArea);
            if (hRet != WFS_SUCCESS)
                return hRet;
            autofree.push_back(lpNewPos->lpRetractArea);
            memcpy(lpNewPos->lpRetractArea, lpItemPos->lpRetractArea, sizeof(WFSCDMRETRACT));
        }

        lpResult->lpBuffer = lpNewPos;
    } while (FALSE);

    return hRet;
}

HRESULT CAgent_CRS::Fmt_WFS_SYSE_HARDWARE_ERROR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpStatus = static_cast<LPWFSHWERROR>(lpData);
        if (lpStatus == nullptr)
        {
            break;
        }
        LPWFSHWERROR lpNewStatus = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSHWERROR), lpResult, (LPVOID *)&lpNewStatus);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpNewStatus);
        memset(lpNewStatus, 0x00, sizeof(WFSHWERROR));
        lpNewStatus->dwAction = lpStatus->dwAction;

        LPSTR lpBuff = nullptr;
        ULONG ulSize = sizeof(char) * 256;
        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszLogicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszPhysicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszWorkstationName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszAppID = lpBuff;
        lpBuff = nullptr;

        if (lpStatus->dwSize > 0)
        {
            // lpbDescription是“ErrorDetail = 00XXXXXXX\x0\x0” (修正了14个字符+错误代码7个字符+空结束2个字符)
            ulSize = lpStatus->dwSize + 2;  // 此特殊处理：多加两位
            hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
            if (hRet != WFS_SUCCESS)
            {
                break;
            }
            _auto.push_back(lpBuff);
            memset(lpBuff, 0x00, ulSize);
            memcpy(lpBuff, lpStatus->lpbDescription, lpStatus->dwSize);
            lpNewStatus->dwSize = ulSize;
            lpNewStatus->lpbDescription = (LPBYTE)lpBuff;
            lpBuff = nullptr;
        }

        // 判断是否有数据
        if (lpStatus->lpszLogicalName != nullptr)
            strcpy(lpNewStatus->lpszLogicalName, lpStatus->lpszLogicalName);
        if (lpStatus->lpszPhysicalName != nullptr)
            strcpy(lpNewStatus->lpszPhysicalName, lpStatus->lpszPhysicalName);
        if (lpStatus->lpszWorkstationName != nullptr)
            strcpy(lpNewStatus->lpszWorkstationName, lpStatus->lpszWorkstationName);
        if (lpStatus->lpszAppID != nullptr)
            strcpy(lpNewStatus->lpszAppID, lpStatus->lpszAppID);

        // 赋值
        lpResult->lpBuffer = lpNewStatus;
        lpNewStatus = nullptr;
    } while (false);
    return hRet;
}

HRESULT CAgent_CRS::Fmt_WFS_SYSE_DEVICE_STATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT            hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpStatus = static_cast<LPWFSDEVSTATUS>(lpData);
        if (lpStatus == nullptr)
        {
            break;
        }
        LPWFSDEVSTATUS lpNewStatus = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSDEVSTATUS), lpResult, (LPVOID *)&lpNewStatus);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpNewStatus);
        memset(lpNewStatus, 0x00, sizeof(WFSDEVSTATUS));
        lpNewStatus->dwState = lpStatus->dwState;

        LPSTR lpBuff = nullptr;
        ULONG ulSize = sizeof(char) * 256;
        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszPhysicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->SIMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszWorkstationName = lpBuff;
        lpBuff = nullptr;

        // 判断是否有数据
        if (lpStatus->lpszPhysicalName != nullptr)
            strcpy(lpNewStatus->lpszPhysicalName, lpStatus->lpszPhysicalName);
        if (lpStatus->lpszWorkstationName != nullptr)
            strcpy(lpNewStatus->lpszWorkstationName, lpStatus->lpszWorkstationName);

        // 赋值
        lpResult->lpBuffer = lpNewStatus;
        lpNewStatus = nullptr;
    } while (false);
    return hRet;
}
