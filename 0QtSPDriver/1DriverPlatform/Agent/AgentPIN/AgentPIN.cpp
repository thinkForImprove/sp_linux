#include "AgentPIN.h"

static const char *DEVTYPE  = "PIN";
static const char *ThisFile = "AgentPIN.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentPIN;
    return (p != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CAgentPIN::CAgentPIN()
{
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentPIN::~CAgentPIN()
{

}

void CAgentPIN::Release()
{

}

HRESULT CAgentPIN::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
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
    case WFS_INF_PIN_STATUS:
    case WFS_INF_PIN_CAPABILITIES:
        hRet = CMD_WFS_PIN_NODATA(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PIN_KEY_DETAIL:
        hRet = Get_WFS_INF_PIN_KEY_DETAIL(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PIN_FUNCKEY_DETAIL:
        hRet = Get_WFS_INF_PIN_FUNCKEY_DETAIL(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PIN_KEY_DETAIL_EX:
        hRet = Get_WFS_INF_PIN_KEY_DETAILEX(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentPIN::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
    case WFS_CMD_PIN_CRYPT:
        hRet = Exe_WFS_CMD_PIN_CRYPT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_IMPORT_KEY:
        hRet = Exe_WFS_CMD_PIN_IMPORT_KEY(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_GET_PIN:
        hRet = Exe_WFS_CMD_PIN_GET_PIN(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_GET_PINBLOCK:
        hRet = Exe_WFS_CMD_PIN_GET_PINBLOCK(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_GET_DATA:
        hRet = Exe_WFS_CMD_PIN_GET_DATA(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_INITIALIZATION:
        hRet = Exe_WFS_CMD_PIN_INITIALIZATION(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_RESET:
        hRet = CMD_WFS_PIN_NODATA(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_GENERATE_KCV:
        hRet = Exe_WFS_CMD_PIN_GENERATE_KCV(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_IMPORT_KEY_EX:
        hRet = Exe_WFS_CMD_PIN_IMPORT_KEY_EX(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_GET_PINBLOCK_EX:
        hRet = Exe_WFS_CMD_PIN_GET_PINBLOCK_EX(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_START_KEY_EXCHANGE:
        hRet = CMD_WFS_PIN_NODATA(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_IMPORT_RSA_PUBLIC_KEY:
        hRet = Exe_WFS_CMD_PIN_IMPORT_RSA_PUBLIC_KEY(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_EXPORT_RSA_ISSUER_SIGNED_ITEM:
        hRet = Exe_WFS_CMD_PIN_EXPORT_RSA_ISSUER_SIGNED_ITEM(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_IMPORT_RSA_SIGNED_DES_KEY:
        hRet = Exe_WFS_CMD_PIN_IMPORT_RSA_SIGNED_DES_KEY(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_GENERATE_RSA_KEY_PAIR:
        hRet = Exe_WFS_CMD_PIN_GENERATE_RSA_KEY_PAIR(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PIN_EXPORT_RSA_EPP_SIGNED_ITEM:
        hRet = Exe_WFS_CMD_PIN_EXPORT_RSA_EPP_SIGNED_ITEM(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAgentPIN::CMD_WFS_PIN_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentPIN::Get_WFS_INF_PIN_KEY_DETAIL(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpQueryDetails);
    if (lpData != nullptr)
    {
        LPSTR lpNewData = nullptr;
        HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(char) * (strlen(lpData) + 1), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        memcpy(lpNewData, lpData, sizeof(char) * (strlen(lpData) + 1));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return WFS_SUCCESS;
}

HRESULT CAgentPIN::Get_WFS_INF_PIN_FUNCKEY_DETAIL(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPULONG>(lpQueryDetails);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPULONG lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(ULONG), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(ULONG));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}
HRESULT CAgentPIN::Get_WFS_INF_PIN_KEY_DETAILEX(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpQueryDetails);
    if (lpData != nullptr)
    {
        LPSTR lpNewData = nullptr;
        HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(char) * (strlen(lpData) + 1), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        memcpy(lpNewData, lpData, sizeof(char) * (strlen(lpData) + 1));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return WFS_SUCCESS;
}

HRESULT CAgentPIN::Get_WFSXDATA(LPWFSXDATA lpXData, CAutoWFMFreeBuffer *autofree, LPVOID lpSourceData, LPWFSXDATA &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto lpData = static_cast<LPWFSXDATA>(lpXData);
        if (lpData == nullptr)
            break;

        LPWFSXDATA lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpSourceData, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        autofree->push_back(lpNewData);
        memset(lpNewData, 0x00, sizeof(WFSXDATA));
        if ((lpData->lpbData != nullptr) && (lpData->usLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpData->usLength, lpSourceData, (LPVOID *)&lpNewData->lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            autofree->push_back(lpNewData->lpbData);
            lpNewData->usLength = lpData->usLength;
            memcpy(lpNewData->lpbData, lpData->lpbData, lpData->usLength);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_CRYPT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpPINCrypt = static_cast<LPWFSPINCRYPT>(lpCmdData);
        if (lpPINCrypt == nullptr)
            break;

        LPWFSPINCRYPT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINCRYPT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPINCrypt, sizeof(WFSPINCRYPT));

        lpNewData->lpsKey = nullptr;
        if (lpPINCrypt->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpPINCrypt->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpPINCrypt->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsStartValueKey = nullptr;
        if (lpPINCrypt->lpsStartValueKey != nullptr)
        {
            DWORD dwSize = strlen(lpPINCrypt->lpsStartValueKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsStartValueKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsStartValueKey);
            memcpy(lpNewData->lpsStartValueKey, lpPINCrypt->lpsStartValueKey, sizeof(char)*dwSize);
        }

        lpNewData->lpxCryptData = nullptr;
        if (lpPINCrypt->lpxCryptData != nullptr)
        {
            hRet = Get_WFSXDATA(lpPINCrypt->lpxCryptData, &_auto, (LPVOID)lpNewData, lpNewData->lpxCryptData);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxKeyEncKey = nullptr;
        if (lpPINCrypt->lpxKeyEncKey != nullptr)
        {
            hRet = Get_WFSXDATA(lpPINCrypt->lpxKeyEncKey, &_auto, (LPVOID)lpNewData, lpNewData->lpxKeyEncKey);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxStartValue = nullptr;
        if (lpPINCrypt->lpxStartValue != nullptr)
        {
            hRet = Get_WFSXDATA(lpPINCrypt->lpxStartValue, &_auto, (LPVOID)lpNewData, lpNewData->lpxStartValue);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);


    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_IMPORT_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpPINImport = static_cast<LPWFSPINIMPORT>(lpCmdData);
        if (lpPINImport == nullptr)
            break;

        LPWFSPINIMPORT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINIMPORT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPINImport, sizeof(WFSPINIMPORT));

        lpNewData->lpsKey = nullptr;
        if (lpPINImport->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpPINImport->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpPINImport->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsEncKey = nullptr;
        if (lpPINImport->lpsEncKey != nullptr)
        {
            DWORD dwSize = strlen(lpPINImport->lpsEncKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsEncKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsEncKey);
            memcpy(lpNewData->lpsEncKey, lpPINImport->lpsEncKey, sizeof(char)*dwSize);
        }

        lpNewData->lpxValue = nullptr;
        if (lpPINImport->lpxValue != nullptr)
        {
            hRet = Get_WFSXDATA(lpPINImport->lpxValue, &_auto, (LPVOID)lpNewData, lpNewData->lpxValue);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxIdent = nullptr;
        if (lpPINImport->lpxIdent != nullptr)
        {
            hRet = Get_WFSXDATA(lpPINImport->lpxIdent, &_auto, (LPVOID)lpNewData, lpNewData->lpxIdent);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);


    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_DERIVE_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpDeriveData = static_cast<LPWFSPINDERIVE>(lpCmdData);
        if (lpDeriveData == nullptr)
            break;

        LPWFSPINDERIVE lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINDERIVE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpDeriveData, sizeof(WFSPINDERIVE));

        lpNewData->lpsKey = nullptr;
        if (lpDeriveData->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpDeriveData->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpDeriveData->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsKeyGenKey = nullptr;
        if (lpDeriveData->lpsKeyGenKey != nullptr)
        {
            DWORD dwSize = strlen(lpDeriveData->lpsKeyGenKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKeyGenKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKeyGenKey);
            memcpy(lpNewData->lpsKeyGenKey, lpDeriveData->lpsKeyGenKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsStartValueKey = nullptr;
        if (lpDeriveData->lpsStartValueKey != nullptr)
        {
            DWORD dwSize = strlen(lpDeriveData->lpsStartValueKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsStartValueKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsStartValueKey);
            memcpy(lpNewData->lpsStartValueKey, lpDeriveData->lpsStartValueKey, sizeof(char)*dwSize);
        }

        lpNewData->lpxInputData = nullptr;
        if (lpDeriveData->lpxInputData != nullptr)
        {
            hRet = Get_WFSXDATA(lpDeriveData->lpxInputData, &_auto, (LPVOID)lpNewData, lpNewData->lpxInputData);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxIdent = nullptr;
        if (lpDeriveData->lpxIdent != nullptr)
        {
            hRet = Get_WFSXDATA(lpDeriveData->lpxIdent, &_auto, (LPVOID)lpNewData, lpNewData->lpxIdent);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxStartValue = nullptr;
        if (lpDeriveData->lpxStartValue != nullptr)
        {
            hRet = Get_WFSXDATA(lpDeriveData->lpxStartValue, &_auto, (LPVOID)lpNewData, lpNewData->lpxStartValue);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GET_PIN(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpGetPin = static_cast<LPWFSPINGETPIN>(lpCmdData);
    if (lpGetPin == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPWFSPINGETPIN lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINGETPIN), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpGetPin, sizeof(WFSPINGETPIN));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_LOCAL_DES(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpLocalDES = static_cast<LPWFSPINLOCALDES>(lpCmdData);
        if (lpLocalDES == nullptr)
            break;

        LPWFSPINLOCALDES lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINLOCALDES), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpLocalDES, sizeof(WFSPINLOCALDES));

        lpNewData->lpsKey = nullptr;
        if (lpLocalDES->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpLocalDES->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpLocalDES->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsOffset = nullptr;
        if (lpLocalDES->lpsOffset != nullptr)
        {
            DWORD dwSize = strlen(lpLocalDES->lpsOffset) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsOffset);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsOffset);
            memcpy(lpNewData->lpsOffset, lpLocalDES->lpsOffset, sizeof(char)*dwSize);
        }

        lpNewData->lpsValidationData = nullptr;
        if (lpLocalDES->lpsValidationData != nullptr)
        {
            DWORD dwSize = strlen(lpLocalDES->lpsValidationData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsValidationData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsValidationData);
            memcpy(lpNewData->lpsValidationData, lpLocalDES->lpsValidationData, sizeof(char)*dwSize);
        }

        lpNewData->lpsDecTable = nullptr;
        if (lpLocalDES->lpsDecTable != nullptr)
        {
            DWORD dwSize = strlen(lpLocalDES->lpsDecTable) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsDecTable);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsDecTable);
            memcpy(lpNewData->lpsDecTable, lpLocalDES->lpsDecTable, sizeof(char)*dwSize);
        }

        lpNewData->lpxKeyEncKey = nullptr;
        if (lpLocalDES->lpxKeyEncKey != nullptr)
        {
            hRet = Get_WFSXDATA(lpLocalDES->lpxKeyEncKey, &_auto, (LPVOID)lpNewData, lpNewData->lpxKeyEncKey);
            if (hRet != WFS_SUCCESS)
                break;
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_CREATE_OFFSET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpCreateOffset = static_cast<LPWFSPINCREATEOFFSET>(lpCmdData);
        if (lpCreateOffset == nullptr)
            break;

        LPWFSPINCREATEOFFSET lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINCREATEOFFSET), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCreateOffset, sizeof(LPWFSPINLOCALDES));

        lpNewData->lpsValidationData = nullptr;
        if (lpCreateOffset->lpsValidationData != nullptr)
        {
            DWORD dwSize = strlen(lpCreateOffset->lpsValidationData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsValidationData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsValidationData);
            memcpy(lpNewData->lpsValidationData, lpCreateOffset->lpsValidationData, sizeof(char)*dwSize);
        }

        lpNewData->lpsKey = nullptr;
        if (lpCreateOffset->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpCreateOffset->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpCreateOffset->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsDecTable = nullptr;
        if (lpCreateOffset->lpsDecTable != nullptr)
        {
            DWORD dwSize = strlen(lpCreateOffset->lpsDecTable) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsDecTable);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsDecTable);
            memcpy(lpNewData->lpsDecTable, lpCreateOffset->lpsDecTable, sizeof(char)*dwSize);
        }

        lpNewData->lpxKeyEncKey = nullptr;
        if (lpCreateOffset->lpxKeyEncKey != nullptr)
        {
            hRet = Get_WFSXDATA(lpCreateOffset->lpxKeyEncKey, &_auto, (LPVOID)lpNewData, lpNewData->lpxKeyEncKey);
            if (hRet != WFS_SUCCESS)
                break;
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_LOCAL_EUROCHEQUE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpLocalEurocheque = static_cast<LPWFSPINLOCALEUROCHEQUE>(lpCmdData);
        if (lpLocalEurocheque == nullptr)
            break;

        LPWFSPINLOCALEUROCHEQUE lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINLOCALEUROCHEQUE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpLocalEurocheque, sizeof(WFSPINLOCALEUROCHEQUE));

        lpNewData->lpsEurochequeData = nullptr;
        if (lpLocalEurocheque->lpsEurochequeData != nullptr)
        {
            DWORD dwSize = strlen(lpLocalEurocheque->lpsEurochequeData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsEurochequeData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsEurochequeData);
            memcpy(lpNewData->lpsEurochequeData, lpLocalEurocheque->lpsEurochequeData, sizeof(char)*dwSize);
        }

        lpNewData->lpsPVV = nullptr;
        if (lpLocalEurocheque->lpsPVV != nullptr)
        {
            DWORD dwSize = strlen(lpLocalEurocheque->lpsPVV) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsPVV);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsPVV);
            memcpy(lpNewData->lpsPVV, lpLocalEurocheque->lpsPVV, sizeof(char)*dwSize);
        }

        lpNewData->lpsKey = nullptr;
        if (lpLocalEurocheque->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpLocalEurocheque->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpLocalEurocheque->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsDecTable = nullptr;
        if (lpLocalEurocheque->lpsDecTable != nullptr)
        {
            DWORD dwSize = strlen(lpLocalEurocheque->lpsDecTable) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsDecTable);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsDecTable);
            memcpy(lpNewData->lpsDecTable, lpLocalEurocheque->lpsDecTable, sizeof(char)*dwSize);
        }

        lpNewData->lpxKeyEncKey = nullptr;
        if (lpLocalEurocheque->lpxKeyEncKey != nullptr)
        {
            hRet = Get_WFSXDATA(lpLocalEurocheque->lpxKeyEncKey, &_auto, (LPVOID)lpNewData, lpNewData->lpxKeyEncKey);
            if (hRet != WFS_SUCCESS)
                break;
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_LOCAL_VISA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpLocalVisa = static_cast<LPWFSPINLOCALVISA>(lpCmdData);
        if (lpLocalVisa == nullptr)
            break;

        LPWFSPINLOCALVISA lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINLOCALVISA), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpLocalVisa, sizeof(WFSPINLOCALVISA));

        lpNewData->lpsKey = nullptr;
        if (lpLocalVisa->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpLocalVisa->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpLocalVisa->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsPAN = nullptr;
        if (lpLocalVisa->lpsPAN != nullptr)
        {
            DWORD dwSize = strlen(lpLocalVisa->lpsPAN) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsPAN);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsPAN);
            memcpy(lpNewData->lpsPAN, lpLocalVisa->lpsPAN, sizeof(char)*dwSize);
        }

        lpNewData->lpsPVV = nullptr;
        if (lpLocalVisa->lpsPVV != nullptr)
        {
            DWORD dwSize = strlen(lpLocalVisa->lpsPVV) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsPVV);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsPVV);
            memcpy(lpNewData->lpsPVV, lpLocalVisa->lpsPVV, sizeof(char)*dwSize);
        }

        lpNewData->lpxKeyEncKey = nullptr;
        if (lpLocalVisa->lpxKeyEncKey != nullptr)
        {
            hRet = Get_WFSXDATA(lpLocalVisa->lpxKeyEncKey, &_auto, (LPVOID)lpNewData, lpNewData->lpxKeyEncKey);
            if (hRet != WFS_SUCCESS)
                break;
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_PRESENT_IDC(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpPresentIDC = static_cast<LPWFSPINPRESENTIDC>(lpCmdData);
        if (lpPresentIDC == nullptr)
            break;

        LPWFSPINPRESENTIDC lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINPRESENTIDC), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPresentIDC, sizeof(WFSPINPRESENTIDC));

        lpNewData->lpbChipData = nullptr;
        if ((lpPresentIDC->lpbChipData != nullptr) && (lpPresentIDC->ulChipDataLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpPresentIDC->ulChipDataLength, lpNewData, (LPVOID *)&lpNewData->lpbChipData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbChipData);
            memcpy(lpNewData->lpbChipData, lpPresentIDC->lpbChipData, sizeof(BYTE)*lpPresentIDC->ulChipDataLength);
        }

        lpNewData->lpAlgorithmData = nullptr;
        if (lpPresentIDC->lpAlgorithmData != nullptr)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINPRESENTCLEAR), lpNewData, (LPVOID *)&lpNewData->lpAlgorithmData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpAlgorithmData);
            memcpy(lpNewData->lpAlgorithmData, lpPresentIDC->lpAlgorithmData, sizeof(WFSPINPRESENTCLEAR));
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GET_PINBLOCK(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpPinBlock = static_cast<LPWFSPINBLOCK>(lpCmdData);
        if (lpPinBlock == nullptr)
            break;

        LPWFSPINBLOCK lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINBLOCK), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPinBlock, sizeof(WFSPINBLOCK));

        lpNewData->lpsKey = nullptr;
        if (lpPinBlock->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlock->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpPinBlock->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsCustomerData = nullptr;
        if (lpPinBlock->lpsCustomerData != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlock->lpsCustomerData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsCustomerData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsCustomerData);
            memcpy(lpNewData->lpsCustomerData, lpPinBlock->lpsCustomerData, sizeof(char)*dwSize);
        }

        lpNewData->lpsXORData = nullptr;
        if (lpPinBlock->lpsXORData != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlock->lpsXORData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsXORData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsXORData);
            memcpy(lpNewData->lpsXORData, lpPinBlock->lpsXORData, sizeof(char)*dwSize);
        }

        lpNewData->lpsKeyEncKey = nullptr;
        if (lpPinBlock->lpsKeyEncKey != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlock->lpsKeyEncKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKeyEncKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKeyEncKey);
            memcpy(lpNewData->lpsKeyEncKey, lpPinBlock->lpsKeyEncKey, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GET_PINBLOCK_EX(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpPinBlockEx = static_cast<LPWFSPINBLOCKEX>(lpCmdData);
        if (lpPinBlockEx == nullptr)
            break;

        LPWFSPINBLOCKEX lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINBLOCKEX), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPinBlockEx, sizeof(WFSPINBLOCKEX));

        lpNewData->lpsKey = nullptr;
        if (lpPinBlockEx->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlockEx->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpPinBlockEx->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsCustomerData = nullptr;
        if (lpPinBlockEx->lpsCustomerData != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlockEx->lpsCustomerData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsCustomerData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsCustomerData);
            memcpy(lpNewData->lpsCustomerData, lpPinBlockEx->lpsCustomerData, sizeof(char)*dwSize);
        }

        lpNewData->lpsXORData = nullptr;
        if (lpPinBlockEx->lpsXORData != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlockEx->lpsXORData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsXORData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsXORData);
            memcpy(lpNewData->lpsXORData, lpPinBlockEx->lpsXORData, sizeof(char)*dwSize);
        }

        lpNewData->lpsKeyEncKey = nullptr;
        if (lpPinBlockEx->lpsKeyEncKey != nullptr)
        {
            DWORD dwSize = strlen(lpPinBlockEx->lpsKeyEncKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKeyEncKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKeyEncKey);
            memcpy(lpNewData->lpsKeyEncKey, lpPinBlockEx->lpsKeyEncKey, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GET_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpGetData = static_cast<LPWFSPINGETDATA>(lpCmdData);
        if (lpGetData == nullptr)
            break;

        LPWFSPINGETDATA lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINGETDATA), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpGetData, sizeof(WFSPINGETDATA));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_INITIALIZATION(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpSPInit = static_cast<LPWFSPININIT>(lpCmdData);
        if (lpSPInit == nullptr)
            break;

        LPWFSPININIT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPININIT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpSPInit, sizeof(WFSPININIT));

        lpNewData->lpxIdent = nullptr;
        if (lpSPInit->lpxIdent != nullptr)
        {
            hRet = Get_WFSXDATA(lpSPInit->lpxIdent, &_auto, (LPVOID)lpNewData, lpNewData->lpxIdent);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxKey = nullptr;
        if (lpSPInit->lpxKey != nullptr)
        {
            hRet = Get_WFSXDATA(lpSPInit->lpxKey, &_auto, (LPVOID)lpNewData, lpNewData->lpxKey);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_LOCAL_BANKSYS(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpLocalBankSys = static_cast<LPWFSPINLOCALBANKSYS>(lpCmdData);
        if (lpLocalBankSys == nullptr)
            break;

        LPWFSPINLOCALBANKSYS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINLOCALBANKSYS), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpLocalBankSys, sizeof(WFSPINLOCALBANKSYS));

        lpNewData->lpxATMVAC = nullptr;
        if (lpLocalBankSys->lpxATMVAC != nullptr)
        {
            hRet = Get_WFSXDATA(lpLocalBankSys->lpxATMVAC, &_auto, (LPVOID)lpNewData, lpNewData->lpxATMVAC);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_BANKSYS_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpBankSysIO = static_cast<LPWFSPINBANKSYSIO>(lpCmdData);
        if (lpBankSysIO == nullptr)
            break;

        LPWFSPINBANKSYSIO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINLOCALBANKSYS), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpBankSysIO, sizeof(WFSPINLOCALBANKSYS));

        lpNewData->lpbData = nullptr;
        if ((lpBankSysIO->lpbData != nullptr) && (lpBankSysIO->ulLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpBankSysIO->ulLength, lpNewData, (LPVOID *)&lpNewData->lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbData);
            memcpy(lpNewData->lpbData, lpBankSysIO->lpbData, sizeof(BYTE)*lpBankSysIO->ulLength);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_HSM_SET_TDATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        lpCopyCmdData = nullptr;
        auto lpXTData = static_cast<LPWFSXDATA>(lpCmdData);
        if (lpXTData == nullptr)
            break;

        LPWFSXDATA lpNewData = nullptr;
        if ((lpXTData->lpbData != nullptr) && (lpXTData->usLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateBuffer(sizeof(BYTE) * lpXTData->usLength, WFS_MEM_FLAG, (LPVOID *)&lpNewData);
            if (hRet != WFS_SUCCESS)
                break;

            memcpy(lpNewData, lpXTData, sizeof(BYTE)*lpXTData->usLength);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_SECURE_MSG_SEND(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        lpCopyCmdData = nullptr;
        auto lpSecMsg = static_cast<LPWFSPINSECMSG>(lpCmdData);
        if (lpSecMsg == nullptr)
            break;

        LPWFSPINSECMSG lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINSECMSG), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpSecMsg, sizeof(WFSPINSECMSG));

        lpNewData->lpbMsg = nullptr;
        if ((lpSecMsg->lpbMsg != nullptr) && (lpSecMsg->ulLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpSecMsg->ulLength, lpNewData, (LPVOID *)&lpNewData->lpbMsg);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbMsg);
            memcpy(lpNewData->lpbMsg, lpSecMsg->lpbMsg, sizeof(BYTE)*lpSecMsg->ulLength);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_SECURE_MSG_RECEIVE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        lpCopyCmdData = nullptr;
        auto lpSecMsg = static_cast<LPWFSPINSECMSG>(lpCmdData);
        if (lpSecMsg == nullptr)
            break;

        LPWFSPINSECMSG lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINSECMSG), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpSecMsg, sizeof(WFSPINSECMSG));

        lpNewData->lpbMsg = nullptr;
        if ((lpSecMsg->lpbMsg != nullptr) && (lpSecMsg->ulLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpSecMsg->ulLength, lpNewData, (LPVOID *)&lpNewData->lpbMsg);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbMsg);
            memcpy(lpNewData->lpbMsg, lpSecMsg->lpbMsg, sizeof(BYTE)*lpSecMsg->ulLength);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GET_JOURNAL(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;

    do
    {
        lpCopyCmdData = nullptr;
        auto lpwProtocol = static_cast<LPWORD>(lpCmdData);
        if (lpwProtocol == nullptr)
            break;

        LPWORD lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        *lpNewData = *lpwProtocol;
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_IMPORT_KEY_EX(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpImportKeyEX = static_cast<LPWFSPINIMPORTKEYEX>(lpCmdData);
        if (lpImportKeyEX == nullptr)
            break;

        LPWFSPINIMPORTKEYEX lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINIMPORTKEYEX), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memset(lpNewData, 0x00, sizeof(WFSPINIMPORTKEYEX));
        lpNewData->dwUse = lpImportKeyEX->dwUse;
        lpNewData->wKeyCheckMode = lpImportKeyEX->wKeyCheckMode;

        if (lpImportKeyEX->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpImportKeyEX->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpImportKeyEX->lpsKey, sizeof(char)*dwSize);
        }

        if (lpImportKeyEX->lpsEncKey != nullptr)
        {
            DWORD dwSize = strlen(lpImportKeyEX->lpsEncKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsEncKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsEncKey);
            memcpy(lpNewData->lpsEncKey, lpImportKeyEX->lpsEncKey, sizeof(char)*dwSize);
        }

        if (lpImportKeyEX->lpxControlVector != nullptr)
        {
            hRet = Get_WFSXDATA(lpImportKeyEX->lpxControlVector, &_auto, (LPVOID)lpNewData, lpNewData->lpxControlVector);
            if (hRet != WFS_SUCCESS)
                break;
        }

        if (lpImportKeyEX->lpxKeyCheckValue != nullptr)
        {
            hRet = Get_WFSXDATA(lpImportKeyEX->lpxKeyCheckValue, &_auto, (LPVOID)lpNewData, lpNewData->lpxKeyCheckValue);
            if (hRet != WFS_SUCCESS)
                break;
        }

        if (lpImportKeyEX->lpxValue != nullptr)
        {
            hRet = Get_WFSXDATA(lpImportKeyEX->lpxValue, &_auto, (LPVOID)lpNewData, lpNewData->lpxValue);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_ENC_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpEncIO = static_cast<LPWFSPINENCIO>(lpCmdData);
        if (lpEncIO == nullptr)
            break;

        LPWFSPINENCIO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINENCIO), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpEncIO, sizeof(WFSPINENCIO));

        lpNewData->lpvData = nullptr;
        if ((lpEncIO->lpvData != nullptr) && (lpEncIO->ulDataLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpEncIO->ulDataLength, lpNewData, (LPVOID *)&lpNewData->lpvData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpvData);
            memcpy(lpNewData->lpvData, lpEncIO->lpvData, sizeof(BYTE)*lpEncIO->ulDataLength);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_HSM_INIT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpHSMInit = static_cast<LPWFSPINHSMINIT>(lpCmdData);
        if (lpHSMInit == nullptr)
            break;

        LPWFSPINHSMINIT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINHSMINIT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpHSMInit, sizeof(WFSPINHSMINIT));
        lpNewData->lpxOnlineTime = nullptr;
        if (lpHSMInit->lpxOnlineTime != nullptr)
        {
            hRet = Get_WFSXDATA(lpHSMInit->lpxOnlineTime, &_auto, (LPVOID)lpNewData, lpNewData->lpxOnlineTime);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_SECUREKEY_ENTRY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpSecurekeyEntry = static_cast<LPWFSPINSECUREKEYENTRY>(lpCmdData);
        if (lpSecurekeyEntry == nullptr)
            break;

        LPWFSPINSECUREKEYENTRY lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINSECUREKEYENTRY), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpSecurekeyEntry, sizeof(WFSPINSECUREKEYENTRY));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GENERATE_KCV(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpGenKCV = static_cast<LPWFSPINGENERATEKCV>(lpCmdData);
        if (lpGenKCV == nullptr)
            break;

        LPWFSPINGENERATEKCV lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINGENERATEKCV), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpGenKCV, sizeof(WFSPINGENERATEKCV));

        lpNewData->lpsKey = nullptr;
        if (lpGenKCV->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpGenKCV->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpGenKCV->lpsKey, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_IMPORT_RSA_PUBLIC_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpRSAPublicKey = static_cast<LPWFSPINIMPORTRSAPUBLICKEY>(lpCmdData);
        if (lpRSAPublicKey == nullptr)
            break;

        LPWFSPINIMPORTRSAPUBLICKEY lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINIMPORTRSAPUBLICKEY), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpRSAPublicKey, sizeof(WFSPINIMPORTRSAPUBLICKEY));

        lpNewData->lpsKey = nullptr;
        if (lpRSAPublicKey->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpRSAPublicKey->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpRSAPublicKey->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsSigKey = nullptr;
        if (lpRSAPublicKey->lpsSigKey != nullptr)
        {
            DWORD dwSize = strlen(lpRSAPublicKey->lpsSigKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsSigKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsSigKey);
            memcpy(lpNewData->lpsSigKey, lpRSAPublicKey->lpsSigKey, sizeof(char)*dwSize);
        }

        lpNewData->lpxSignature = nullptr;
        if (lpRSAPublicKey->lpxSignature != nullptr)
        {
            hRet = Get_WFSXDATA(lpRSAPublicKey->lpxSignature, &_auto, (LPVOID)lpNewData, lpNewData->lpxSignature);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxValue = nullptr;
        if (lpRSAPublicKey->lpxValue != nullptr)
        {
            hRet = Get_WFSXDATA(lpRSAPublicKey->lpxValue, &_auto, (LPVOID)lpNewData, lpNewData->lpxValue);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_EXPORT_RSA_ISSUER_SIGNED_ITEM(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpRSAExportSignedItem = static_cast<LPWFSPINEXPORTRSAISSUERSIGNEDITEM>(lpCmdData);
        if (lpRSAExportSignedItem == nullptr)
            break;

        LPWFSPINEXPORTRSAISSUERSIGNEDITEM lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINEXPORTRSAISSUERSIGNEDITEM), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpRSAExportSignedItem, sizeof(WFSPINEXPORTRSAISSUERSIGNEDITEM));

        lpNewData->lpsName = nullptr;
        if (lpRSAExportSignedItem->lpsName != nullptr)
        {
            DWORD dwSize = strlen(lpRSAExportSignedItem->lpsName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsName);
            memcpy(lpNewData->lpsName, lpRSAExportSignedItem->lpsName, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_IMPORT_RSA_SIGNED_DES_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpRSASignedDESKey = static_cast<LPWFSPINIMPORTRSASIGNEDDESKEY>(lpCmdData);
        if (lpRSASignedDESKey == nullptr)
            break;

        LPWFSPINIMPORTRSASIGNEDDESKEY lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINIMPORTRSASIGNEDDESKEY), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpRSASignedDESKey, sizeof(WFSPINIMPORTRSASIGNEDDESKEY));

        lpNewData->lpsKey = nullptr;
        if (lpRSASignedDESKey->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpRSASignedDESKey->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpRSASignedDESKey->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsDecryptKey = nullptr;
        if (lpRSASignedDESKey->lpsDecryptKey != nullptr)
        {
            DWORD dwSize = strlen(lpRSASignedDESKey->lpsDecryptKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsDecryptKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsDecryptKey);
            memcpy(lpNewData->lpsDecryptKey, lpRSASignedDESKey->lpsDecryptKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsSigKey = nullptr;
        if (lpRSASignedDESKey->lpsSigKey != nullptr)
        {
            DWORD dwSize = strlen(lpRSASignedDESKey->lpsSigKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsSigKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsSigKey);
            memcpy(lpNewData->lpsSigKey, lpRSASignedDESKey->lpsSigKey, sizeof(char)*dwSize);
        }


        lpNewData->lpxSignature = nullptr;
        if (lpRSASignedDESKey->lpxSignature != nullptr)
        {
            hRet = Get_WFSXDATA(lpRSASignedDESKey->lpxSignature, &_auto, (LPVOID)lpNewData, lpNewData->lpxSignature);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpNewData->lpxValue = nullptr;
        if (lpRSASignedDESKey->lpxValue != nullptr)
        {
            hRet = Get_WFSXDATA(lpRSASignedDESKey->lpxValue, &_auto, (LPVOID)lpNewData, lpNewData->lpxValue);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GENERATE_RSA_KEY_PAIR(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpGenerateRSAKeyPair = static_cast<LPWFSPINGENERATERSAKEYPAIR>(lpCmdData);
        if (lpGenerateRSAKeyPair == nullptr)
            break;

        LPWFSPINGENERATERSAKEYPAIR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINGENERATERSAKEYPAIR), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpGenerateRSAKeyPair, sizeof(WFSPINGENERATERSAKEYPAIR));

        lpNewData->lpsKey = nullptr;
        if (lpGenerateRSAKeyPair->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpGenerateRSAKeyPair->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpGenerateRSAKeyPair->lpsKey, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_EXPORT_RSA_EPP_SIGNED_ITEM(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpEppSignedItem = static_cast<LPWFSPINEXPORTRSAEPPSIGNEDITEM>(lpCmdData);
        if (lpEppSignedItem == nullptr)
            break;

        LPWFSPINEXPORTRSAEPPSIGNEDITEM lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINEXPORTRSAEPPSIGNEDITEM), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpEppSignedItem, sizeof(WFSPINEXPORTRSAEPPSIGNEDITEM));

        lpNewData->lpsName = nullptr;
        if (lpEppSignedItem->lpsName != nullptr)
        {
            DWORD dwSize = strlen(lpEppSignedItem->lpsName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsName);
            memcpy(lpNewData->lpsName, lpEppSignedItem->lpsName, sizeof(char)*dwSize);
        }

        lpNewData->lpsSigKey = nullptr;
        if (lpEppSignedItem->lpsSigKey != nullptr)
        {
            DWORD dwSize = strlen(lpEppSignedItem->lpsSigKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsSigKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsSigKey);
            memcpy(lpNewData->lpsSigKey, lpEppSignedItem->lpsSigKey, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_LOAD_CERTIFICATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpLoadCertificate = static_cast<LPWFSPINLOADCERTIFICATE>(lpCmdData);
        if (lpLoadCertificate == nullptr)
            break;

        LPWFSPINLOADCERTIFICATE lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINLOADCERTIFICATE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpLoadCertificate, sizeof(WFSPINLOADCERTIFICATE));

        lpNewData->lpxLoadCertificate = nullptr;
        if (lpLoadCertificate->lpxLoadCertificate != nullptr)
        {
            hRet = Get_WFSXDATA(lpLoadCertificate->lpxLoadCertificate, &_auto, (LPVOID)lpNewData, lpNewData->lpxLoadCertificate);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_GET_CERTIFICATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpGetCertificate = static_cast<LPWFSPINGETCERTIFICATE>(lpCmdData);
        if (lpGetCertificate == nullptr)
            break;

        LPWFSPINGETCERTIFICATE lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINGETCERTIFICATE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpGetCertificate, sizeof(WFSPINGETCERTIFICATE));

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_REPLACE_CERTIFICATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpReplaceCertificate = static_cast<LPWFSPINREPLACECERTIFICATE>(lpCmdData);
        if (lpReplaceCertificate == nullptr)
            break;

        LPWFSPINREPLACECERTIFICATE lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINREPLACECERTIFICATE), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpReplaceCertificate, sizeof(WFSPINREPLACECERTIFICATE));

        lpNewData->lpxReplaceCertificate = nullptr;
        if (lpReplaceCertificate->lpxReplaceCertificate != nullptr)
        {
            hRet = Get_WFSXDATA(lpReplaceCertificate->lpxReplaceCertificate, &_auto, (LPVOID)lpNewData, lpNewData->lpxReplaceCertificate);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_IMPORT_RSA_ENCIPHERED_PKCS7_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpData = static_cast<LPWFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT>(lpCmdData);
        if (lpData == nullptr)
            break;

        LPWFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpData, sizeof(WFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT));

        lpNewData->lpxRSAData = nullptr;
        if (lpData->lpxRSAData != nullptr)
        {
            hRet = Get_WFSXDATA(lpData->lpxRSAData, &_auto, (LPVOID)lpNewData, lpNewData->lpxRSAData);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_EMV_IMPORT_PUBLIC_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpEMVImportPubKey = static_cast<LPWFSPINEMVIMPORTPUBLICKEY>(lpCmdData);
        if (lpEMVImportPubKey == nullptr)
            break;

        LPWFSPINEMVIMPORTPUBLICKEY lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINEMVIMPORTPUBLICKEY), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpEMVImportPubKey, sizeof(WFSPINEMVIMPORTPUBLICKEY));

        lpNewData->lpsKey = nullptr;
        if (lpEMVImportPubKey->lpsKey != nullptr)
        {
            DWORD dwSize = strlen(lpEMVImportPubKey->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpEMVImportPubKey->lpsKey, sizeof(char)*dwSize);
        }

        lpNewData->lpsSigKey = nullptr;
        if (lpEMVImportPubKey->lpsSigKey != nullptr)
        {
            DWORD dwSize = strlen(lpEMVImportPubKey->lpsSigKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpsSigKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsSigKey);
            memcpy(lpNewData->lpsSigKey, lpEMVImportPubKey->lpsSigKey, sizeof(char)*dwSize);
        }

        lpNewData->lpxImportData = nullptr;
        if (lpEMVImportPubKey->lpxImportData != nullptr)
        {
            hRet = Get_WFSXDATA(lpEMVImportPubKey->lpxImportData, &_auto, (LPVOID)lpNewData, lpNewData->lpxImportData);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPIN::Exe_WFS_CMD_PIN_DIGEST(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        lpCopyCmdData = nullptr;
        auto lpDigest = static_cast<LPWFSPINDIGEST>(lpCmdData);
        if (lpDigest == nullptr)
            break;

        LPWFSPINDIGEST lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPINDIGEST), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpDigest, sizeof(WFSPINDIGEST));

        lpNewData->lpxDigestInput = nullptr;
        if (lpDigest->lpxDigestInput != nullptr)
        {
            hRet = Get_WFSXDATA(lpDigest->lpxDigestInput, &_auto, (LPVOID)lpNewData, lpNewData->lpxDigestInput);
            if (hRet != WFS_SUCCESS)
                break;
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

//////////////////////////////////////////////////////////////////////////
bool CAgentPIN::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}
