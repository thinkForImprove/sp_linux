#include "AgentCRS.h"

static const char *ThisFile = "Agent_CRS.cpp";
static const char *DEVTYPE = "CRS";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgent_CRS;
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
bool CAgent_CRS::LoadWFMDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////
inline UINT GetLenOfSZZ(const char *lpszz)
{
    const char *p = lpszz;
    while (TRUE)
    {
        if (p == nullptr || (p + 1) == nullptr)
        {
            return -1;
        }
        if ((*p == NULL) && (*(p + 1) == NULL))
        {
            break;
        }
        p++;
    }
    return (p - lpszz) + 2;
}
//////////////////////////////////////////////////////////////////////////
CAgent_CRS::CAgent_CRS()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgent_CRS::~CAgent_CRS() {}

void CAgent_CRS::Release() {}

HRESULT CAgent_CRS::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    if (!LoadWFMDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    DWORD dwCatOffset = (dwCategory / 100) * 100;
    if (dwCatOffset == CDM_SERVICE_OFFSET)
        return CDMGetInfo(dwCategory, lpQueryDetails, lpCopyCmdData);
    else
        return CIMGetInfo(dwCategory, lpQueryDetails, lpCopyCmdData);
}

HRESULT CAgent_CRS::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (!LoadWFMDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }

    DWORD dwCmdOffset = (dwCommand / 100) * 100;
    if (dwCmdOffset == CDM_SERVICE_OFFSET)
        return CDMExecute(dwCommand, lpCmdData, lpCopyCmdData);
    else
        return CIMExecute(dwCommand, lpCmdData, lpCopyCmdData);
}

HRESULT CAgent_CRS::CDMGetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch (dwCategory)
    {
    case WFS_INF_CDM_STATUS:
        hRet = Get_WFS_INF_CDM_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CDM_CAPABILITIES:
        hRet = Get_WFS_INF_CDM_CAPABILITIES(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CDM_CASH_UNIT_INFO:
        hRet = Get_WFS_INF_CDM_CASH_UNIT_INFO(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CDM_MIX_TYPES:
        hRet = Get_WFS_INF_CDM_MIX_TYPES(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CDM_PRESENT_STATUS:
        hRet = Get_WFS_INF_CDM_PRESENT_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CDM_CURRENCY_EXP:
        hRet = Get_WFS_INF_CDM_CURRENCY_EXP(lpQueryDetails, lpCopyCmdData);
    default:
        break;
    }
    return hRet;
}

HRESULT CAgent_CRS::CDMExecute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch (dwCommand)
    {
    case WFS_CMD_CDM_DENOMINATE:
        hRet = Exe_WFS_CMD_CDM_DENOMINATE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_DISPENSE:
        hRet = Exe_WFS_CMD_CDM_DISPENSE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_PRESENT:
        hRet = Exe_WFS_CMD_CDM_PRESENT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_REJECT:
        hRet = Exe_WFS_CMD_CDM_REJECT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_RETRACT:
        hRet = Exe_WFS_CMD_CDM_RETRACT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_OPEN_SHUTTER:
        hRet = Exe_WFS_CMD_CDM_OPEN_SHUTTER(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_CLOSE_SHUTTER:
        hRet = Exe_WFS_CMD_CDM_CLOSE_SHUTTER(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_START_EXCHANGE:
        hRet = ExeWFS_CMD_CDM_START_EXCHANGE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_END_EXCHANGE:
        hRet = Exe_WFS_CMD_CDM_END_EXCHANGE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_TEST_CASH_UNITS:                   //30-00-00-00(FS#0007)
    case WFS_CMD_CDM_RESET:
        hRet = Exe_WFS_CMD_CDM_RESET(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CDM_SET_CASH_UNIT_INFO:
        hRet = Exe_WFS_CMD_CDM_END_EXCHANGE(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgent_CRS::CIMGetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch (dwCategory)
    {
    case WFS_INF_CIM_STATUS:
        hRet = Get_WFS_INF_CIM_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CIM_CAPABILITIES:
        hRet = Get_WFS_INF_CIM_CAPABILITIES(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CIM_CASH_UNIT_INFO:
        hRet = Get_WFS_INF_CIM_CASH_UNIT_INFO(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CIM_BANKNOTE_TYPES:
        hRet = Get_WFS_INF_CIM_BANKNOTE_TYPES(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CIM_CASH_IN_STATUS:
        hRet = Get_WFS_INF_CIM_CASH_IN_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CIM_CURRENCY_EXP:
        hRet = Get_WFS_INF_CIM_CURRENCY_EXP(lpQueryDetails, lpCopyCmdData);
        break;
    }
    return hRet;
}

HRESULT CAgent_CRS::CIMExecute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch (dwCommand)
    {
    case WFS_CMD_CIM_CASH_IN_START:
        hRet = Exe_WFS_CMD_CIM_CASH_IN_START(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_CASH_IN:
        hRet = Exe_WFS_CMD_CIM_CASH_IN(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_CASH_IN_END:
        hRet = Exe_WFS_CMD_CIM_CASH_IN_END(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_CASH_IN_ROLLBACK:
        hRet = Exe_WFS_CMD_CIM_CASH_IN_ROLLBACK(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_RETRACT:
        hRet = Exe_WFS_CMD_CIM_RETRACT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_OPEN_SHUTTER:
        hRet = Exe_WFS_CMD_CIM_OPEN_SHUTTER(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_CLOSE_SHUTTER:
        hRet = Exe_WFS_CMD_CIM_CLOSE_SHUTTER(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_START_EXCHANGE:
        hRet = Exe_WFS_CMD_CIM_START_EXCHANGE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_END_EXCHANGE:
        hRet = Exe_WFS_CMD_CIM_END_EXCHANGE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_RESET:
        hRet = Exe_WFS_CMD_CIM_RESET(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_CONFIGURE_NOTETYPES:
        hRet = Exe_WFS_CMD_CIM_CONFIGURE_NOTETYPES(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_SET_CASH_UNIT_INFO:
        hRet = Exe_WFS_CMD_CIM_END_EXCHANGE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CIM_SET_CASH_IN_LIMIT:
        hRet = Exe_WFS_CMD_CIM_SET_CASH_IN_LIMIT(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
HRESULT CAgent_CRS::Get_WFS_INF_CDM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CDM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CDM_CASH_UNIT_INFO(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CDM_MIX_TYPES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CDM_CURRENCY_EXP(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CDM_PRESENT_STATUS(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;

    LPWFSCIMCASHINSTART lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WORD));

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_DENOMINATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCDMDENOMINATE>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCDMDENOMINATE lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCDMDENOMINATE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    _auto.push_back(lpNewData);

    lpNewData->usMixNumber = lpData->usMixNumber;
    lpNewData->usTellerID = lpData->usTellerID;

    lpNewData->lpDenomination = nullptr;
    if (lpData->lpDenomination != nullptr)
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMDENOMINATION), lpNewData, (LPVOID *)&lpNewData->lpDenomination);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpDenomination);

        memcpy(lpNewData->lpDenomination, lpData->lpDenomination, sizeof(WFSCDMDENOMINATION));

        USHORT usCount = lpNewData->lpDenomination->usCount;
        if (0 == usCount)
        {
            lpNewData->lpDenomination->lpulValues = nullptr;
        }
        else
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(ULONG) * usCount, lpNewData, (LPVOID *)&lpNewData->lpDenomination->lpulValues);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpDenomination->lpulValues);
            memcpy(lpNewData->lpDenomination->lpulValues, lpData->lpDenomination->lpulValues, usCount * sizeof(ULONG));
        }
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_DISPENSE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCDMDISPENSE>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCDMDISPENSE lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCDMDISPENSE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    _auto.push_back(lpNewData);
    memcpy(lpNewData, lpData, sizeof(WFSCDMDISPENSE));

    lpNewData->lpDenomination = nullptr;
    if (lpData->lpDenomination != nullptr)
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMDENOMINATION), lpNewData, (LPVOID *)&lpNewData->lpDenomination);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpDenomination);

        memcpy(lpNewData->lpDenomination, lpData->lpDenomination, sizeof(WFSCDMDENOMINATION));

        USHORT usCount = lpNewData->lpDenomination->usCount;
        lpNewData->lpDenomination->lpulValues = nullptr;

        if ((usCount != 0) && (lpData->lpDenomination->lpulValues != nullptr))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(ULONG) * usCount, lpNewData, (LPVOID *)&lpNewData->lpDenomination->lpulValues);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lpDenomination->lpulValues);
            memcpy(lpNewData->lpDenomination->lpulValues, lpData->lpDenomination->lpulValues, sizeof(ULONG) * usCount);
        }
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_PRESENT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWORD lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WORD));

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_REJECT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpCmdData);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_RETRACT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCDMRETRACT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSCDMRETRACT lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCDMRETRACT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSCDMRETRACT));

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_OPEN_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData == nullptr)
        return WFS_SUCCESS;

    LPWORD lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WORD));

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_CLOSE_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData == nullptr)
        return WFS_SUCCESS;

    LPWORD lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WORD));

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::ExeWFS_CMD_CDM_START_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCDMSTARTEX>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCDMSTARTEX lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCDMSTARTEX), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    _auto.push_back(lpNewData);
    memcpy(lpNewData, lpData, sizeof(WFSCDMSTARTEX));

    lpNewData->lpusCUNumList = nullptr;
    if (lpData->lpusCUNumList != nullptr && lpData->usCount > 0)            //30-00-00-00(FT#0027)
    {
        USHORT usCount = lpNewData->usCount;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(USHORT) * usCount, lpNewData, (LPVOID *)&lpNewData->lpusCUNumList);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpusCUNumList);
        memcpy(lpNewData->lpusCUNumList, lpData->lpusCUNumList, sizeof(USHORT) * usCount);
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_END_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    auto lpData = static_cast<LPWFSCDMCUINFO>(lpCmdData);
    if (lpData != nullptr)
    {
        LPWFSCDMCUINFO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCDMCUINFO), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpData, sizeof(WFSCDMCUINFO));

        lpNewData->lppList = nullptr;
        if (lpData->lppList == nullptr)
            return WFS_ERR_INVALID_DATA;

        USHORT usCount = lpNewData->usCount;
        if (usCount > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMCASHUNIT) * usCount, lpNewData, (LPVOID *)&lpNewData->lppList);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lppList);

            for (int i = 0; i < usCount; i++)
            {
                if (lpData->lppList[i] == nullptr)
                    return WFS_ERR_INVALID_DATA;

                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMCASHUNIT), lpNewData, (LPVOID *)&lpNewData->lppList[i]);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewData->lppList[i]);

                LPWFSCDMCASHUNIT lpNewCastte = lpNewData->lppList[i];

                memcpy(lpNewCastte, lpData->lppList[i], sizeof(WFSCDMCASHUNIT));

                lpNewCastte->lpszCashUnitName = nullptr;
                char *pszCashName = lpData->lppList[i]->lpszCashUnitName;
                if (pszCashName != nullptr)
                {
                    hRet = m_pIWFM->WFMAllocateMore(strlen(pszCashName) + 1, lpNewData, (LPVOID *)&lpNewCastte->lpszCashUnitName);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewCastte->lpszCashUnitName);

                    memcpy(lpNewCastte->lpszCashUnitName, lpData->lppList[i]->lpszCashUnitName, sizeof(char) * (strlen(pszCashName) + 1));
                }

                USHORT usPhyCUCount = lpData->lppList[i]->usNumPhysicalCUs;
                if (usPhyCUCount < 1)
                {
                    return WFS_ERR_INVALID_DATA;
                }
                hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCDMPHCU) * usPhyCUCount, lpNewData, (LPVOID *)&lpNewCastte->lppPhysical);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCastte->lppPhysical);

                for (int j = 0; j < usPhyCUCount; j++)
                {
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMPHCU), lpNewData, (LPVOID *)&lpNewCastte->lppPhysical[j]);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewCastte->lppPhysical[j]);

                    memcpy(lpNewCastte->lppPhysical[j], lpData->lppList[i]->lppPhysical[j], sizeof(WFSCDMPHCU));

                    char *lpPhyPosName = lpData->lppList[i]->lppPhysical[j]->lpPhysicalPositionName;
                    if (lpPhyPosName != nullptr)
                    {
                        lpNewCastte->lppPhysical[j]->lpPhysicalPositionName = nullptr;
                        hRet = m_pIWFM->WFMAllocateMore(strlen(lpPhyPosName) + 1, lpNewData, (LPVOID *)&lpNewCastte->lppPhysical[j]->lpPhysicalPositionName);
                        if (hRet != WFS_SUCCESS)
                            return hRet;
                        _auto.push_back(lpNewCastte->lppPhysical[j]->lpPhysicalPositionName);

                        memcpy(lpNewCastte->lppPhysical[j]->lpPhysicalPositionName, lpData->lppList[i]->lppPhysical[j]->lpPhysicalPositionName,
                               sizeof(char) * (strlen(lpPhyPosName) + 1));
                    }
                }
            }
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CDM_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCDMITEMPOSITION>(lpCmdData);
    if (lpData == nullptr)
        return WFS_SUCCESS;  // WFS_ERR_INVALID_POINTER;// 支持空指针

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCDMITEMPOSITION lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCDMITEMPOSITION), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    _auto.push_back(lpNewData);
    memcpy(lpNewData, lpData, sizeof(WFSCDMITEMPOSITION));

    lpNewData->lpRetractArea = nullptr;
    if (lpData->lpRetractArea != nullptr)
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCDMRETRACT), lpNewData, (LPVOID *)&lpNewData->lpRetractArea);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpRetractArea);

        memcpy(lpNewData->lpRetractArea, lpData->lpRetractArea, sizeof(WFSCDMRETRACT));
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_CASH_UNIT_INFO(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_BANKNOTE_TYPES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_CASH_IN_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    Q_UNUSED(lpQueryDetails);
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Get_WFS_INF_CIM_CURRENCY_EXP(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CASH_IN_START(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCIMCASHINSTART>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSCIMCASHINSTART lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCIMCASHINSTART), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSCIMCASHINSTART));

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CASH_IN(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    Q_UNUSED(lpCmdData);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CASH_IN_END(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    Q_UNUSED(lpCmdData);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CASH_IN_ROLLBACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    Q_UNUSED(lpCmdData);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_RETRACT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCIMRETRACT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPWFSCIMRETRACT lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCIMRETRACT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSCIMRETRACT));

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_OPEN_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData != nullptr)
    {
        LPWORD lpNewData = nullptr;
        HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        memcpy(lpNewData, lpData, sizeof(WORD));

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CLOSE_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData != nullptr)
    {
        LPWORD lpNewData = nullptr;
        HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        memcpy(lpNewData, lpData, sizeof(WORD));

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_START_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCIMSTARTEX>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCIMSTARTEX lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCIMSTARTEX), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    _auto.push_back(lpNewData);
    memcpy(lpNewData, lpData, sizeof(WFSCIMSTARTEX));

    lpNewData->lpusCUNumList = nullptr;
    if (lpData->lpusCUNumList != nullptr && (lpNewData->usCount > 0))
    {
        USHORT usCount = lpNewData->usCount;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(USHORT) * usCount, lpNewData, (LPVOID *)&lpNewData->lpusCUNumList);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpusCUNumList);

        memcpy(lpNewData->lpusCUNumList, lpData->lpusCUNumList, sizeof(USHORT) * usCount);
    }

    lpNewData->lpOutput = nullptr;
    if (lpData->lpOutput != nullptr)
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMOUTPUT), lpNewData, (LPVOID *)&lpNewData->lpOutput);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpOutput);
        memcpy(lpNewData->lpOutput, lpData->lpOutput, sizeof(WFSCIMOUTPUT));
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_END_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    auto lpData = static_cast<LPWFSCIMCASHINFO>(lpCmdData);
    if (lpData != nullptr)
    {
        LPWFSCIMCASHINFO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCIMCASHINFO), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData);

        memcpy(lpNewData, lpData, sizeof(WFSCIMCASHINFO));

        lpNewData->lppCashIn = nullptr;
        if (lpData->lppCashIn == nullptr)
            return WFS_ERR_INVALID_DATA;

        USHORT usCount = lpNewData->usCount;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMCASHIN) * usCount, lpNewData, (LPVOID *)&lpNewData->lppCashIn);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lppCashIn);

        for (int i = 0; i < usCount; i++)
        {
            if (lpData->lppCashIn[i] == nullptr)
            {
                return WFS_ERR_INVALID_DATA;
            }

            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMCASHIN), lpNewData, (LPVOID *)&lpNewData->lppCashIn[i]);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewData->lppCashIn[i]);

            LPWFSCIMCASHIN lpNewCastte = lpNewData->lppCashIn[i];
            memcpy(lpNewCastte, lpData->lppCashIn[i], sizeof(WFSCIMCASHIN));

            // notenumberlist
            lpNewCastte->lpNoteNumberList = nullptr;
            if (lpData->lppCashIn[i]->lpNoteNumberList != nullptr)
            {
                LPWFSCIMNOTENUMBERLIST lpNewNoteNumList = nullptr;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBERLIST), lpNewData, (LPVOID *)&lpNewNoteNumList);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewNoteNumList);

                memcpy(lpNewNoteNumList, lpData->lppCashIn[i]->lpNoteNumberList, sizeof(WFSCIMNOTENUMBERLIST));
                USHORT usNoteNum = lpData->lppCashIn[i]->lpNoteNumberList->usNumOfNoteNumbers;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMNOTENUMBER) * usNoteNum, lpNewData, (LPVOID *)&lpNewNoteNumList->lppNoteNumber);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewNoteNumList->lppNoteNumber);
                for (int m = 0; m < usNoteNum; m++)
                {
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMNOTENUMBER), lpNewData, (LPVOID *)&lpNewNoteNumList->lppNoteNumber[m]);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewNoteNumList->lppNoteNumber[m]);
                    memcpy(lpNewNoteNumList->lppNoteNumber[m], lpData->lppCashIn[i]->lpNoteNumberList->lppNoteNumber[m], sizeof(WFSCIMNOTENUMBER));
                }
                lpNewCastte->lpNoteNumberList = lpNewNoteNumList;
                lpNewNoteNumList = nullptr;
            }

            // phyCU
            USHORT usPhyCUCount = lpNewCastte->usNumPhysicalCUs;
            if (usPhyCUCount < 1)
            {
                return WFS_ERR_INVALID_DATA;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSCIMPHCU) * usPhyCUCount, lpNewData, (LPVOID *)&lpNewCastte->lppPhysical);
            if (hRet != WFS_SUCCESS)
                return hRet;
            _auto.push_back(lpNewCastte->lppPhysical);

            for (int j = 0; j < usPhyCUCount; j++)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMPHCU), lpNewData, (LPVOID *)&lpNewCastte->lppPhysical[j]);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCastte->lppPhysical[j]);

                memcpy(lpNewCastte->lppPhysical[j], lpData->lppCashIn[i]->lppPhysical[j], sizeof(WFSCIMPHCU));

                lpNewCastte->lppPhysical[j]->lpPhysicalPositionName = nullptr;
                char *lpPhyPosName = lpData->lppCashIn[i]->lppPhysical[j]->lpPhysicalPositionName;
                if (lpPhyPosName != nullptr)
                {
                    lpNewCastte->lppPhysical[j]->lpPhysicalPositionName = nullptr;
                    hRet = m_pIWFM->WFMAllocateMore(strlen(lpPhyPosName) + 1, lpNewData, (LPVOID *)&lpNewCastte->lppPhysical[j]->lpPhysicalPositionName);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewCastte->lppPhysical[j]->lpPhysicalPositionName);
                    memcpy(lpNewCastte->lppPhysical[j]->lpPhysicalPositionName, lpData->lppCashIn[i]->lppPhysical[j]->lpPhysicalPositionName,
                           sizeof(char) * (strlen(lpPhyPosName) + 1));
                }

                char *lpExtra = lpData->lppCashIn[i]->lppPhysical[j]->lpszExtra;
                lpNewCastte->lppPhysical[j]->lpszExtra = nullptr;
                if (lpExtra != nullptr)
                {
                    UINT uLen = GetLenOfSZZ(lpExtra);
                    if (uLen == -1)
                    {
                        Log(ThisModule, WFS_ERR_INVALID_DATA, "lppPhysical[%d]->lpszExtra 格式错误", j);
                        return WFS_ERR_INVALID_DATA;
                    }
                    hRet = m_pIWFM->WFMAllocateMore(uLen, lpNewData, (LPVOID *)&lpNewCastte->lppPhysical[j]->lpszExtra);
                    if (hRet != WFS_SUCCESS)
                        return hRet;
                    _auto.push_back(lpNewCastte->lppPhysical[j]->lpszExtra);
                    memcpy(lpNewCastte->lppPhysical[j]->lpszExtra, lpData->lppCashIn[i]->lppPhysical[j]->lpszExtra, sizeof(char) * uLen);
                }
            }

            // szExtra
            lpNewCastte->lpszExtra = nullptr;
            char *szExtra = lpData->lppCashIn[i]->lpszExtra;
            if (szExtra != nullptr)
            {
                UINT uLen = GetLenOfSZZ(szExtra);
                if (uLen == -1)
                {
                    Log(ThisModule, WFS_ERR_INVALID_DATA, "lppCashIn[%d]->lpszExtra格式错误", i);
                    return WFS_ERR_INVALID_DATA;
                }
                hRet = m_pIWFM->WFMAllocateMore(uLen, lpNewData, (LPVOID *)&lpNewCastte->lpszExtra);
                if (hRet != WFS_SUCCESS)
                    return hRet;
                _auto.push_back(lpNewCastte->lpszExtra);
                memcpy(lpNewCastte->lpszExtra, lpData->lppCashIn[i]->lpszExtra, sizeof(char) * uLen);
            }
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }

    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCIMITEMPOSITION>(lpCmdData);
    if (lpData == nullptr)
//30-00-00-00(FT#0028)        return WFS_ERR_INVALID_POINTER;
        return WFS_SUCCESS;                     //30-00-00-00(FT#0028)

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCIMITEMPOSITION lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCIMITEMPOSITION), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    _auto.push_back(lpNewData);
    memcpy(lpNewData, lpData, sizeof(WFSCIMITEMPOSITION));

    lpNewData->lpRetractArea = nullptr;
    if (lpData->lpRetractArea != nullptr)
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMRETRACT), lpNewData, (LPVOID *)&lpNewData->lpRetractArea);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpRetractArea);
        memcpy(lpNewData->lpRetractArea, lpData->lpRetractArea, sizeof(WFSCIMRETRACT));
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_CONFIGURE_NOTETYPES(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPUSHORT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    LPUSHORT lpNoteIDs = lpData;
    UINT uLen = 0;
    if (lpNoteIDs != nullptr)
    {
        while (*lpNoteIDs != 0)
        {
            lpNoteIDs++;
            uLen++;
        }
    }
    uLen++;
    LPUSHORT lpNewData = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(USHORT) * uLen, WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    memcpy(lpNewData, lpData, sizeof(USHORT) * uLen);

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgent_CRS::Exe_WFS_CMD_CIM_SET_CASH_IN_LIMIT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;

    auto lpData = static_cast<LPWFSCIMCASHINLIMIT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCIMCASHINLIMIT lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCIMCASHINLIMIT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;
    _auto.push_back(lpNewData);
    memcpy(lpNewData, lpData, sizeof(WFSCIMCASHINLIMIT));

    lpNewData->lpAmountLimit = nullptr;
    if (lpData->lpAmountLimit != nullptr)
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSCIMAMOUNTLIMIT), lpNewData, (LPVOID *)&lpNewData->lpAmountLimit);
        if (hRet != WFS_SUCCESS)
            return hRet;
        _auto.push_back(lpNewData->lpAmountLimit);
        memcpy(lpNewData->lpAmountLimit, lpData->lpAmountLimit, sizeof(WFSCIMAMOUNTLIMIT));
    }

    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}
