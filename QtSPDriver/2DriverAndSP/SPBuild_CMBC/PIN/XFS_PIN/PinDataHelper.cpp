#include "PinDataHelper.h"


//////////////////////////////////////////////////////////////////////////
CWFSPINKEYDETAIL::CWFSPINKEYDETAIL() : m_ppKeys(nullptr)
{

}

CWFSPINKEYDETAIL::~CWFSPINKEYDETAIL()
{
    Clear();
    if (m_ppKeys != nullptr)
    {
        delete[]m_ppKeys;
        m_ppKeys = nullptr;
    }
}

void CWFSPINKEYDETAIL::Clear()
{
    for (auto &it : m_mapKeys)
    {
        if (it.second != nullptr)
        {
            if (it.second->lpsKeyName != nullptr)
            {
                delete[]it.second->lpsKeyName;
                it.second->lpsKeyName = nullptr;
            }
            delete it.second;
            it.second = nullptr;
        }
    }
    m_mapKeys.clear();
}

void CWFSPINKEYDETAIL::Add(LPCSTR lpsKeyName, WORD fwUse, BOOL bLoaded /*= TRUE*/)
{
    if (m_ppKeys == nullptr)
    {
        m_ppKeys = new LPWFSPINKEYDETAIL[KEY_MAX_NUM + 1];
        if (m_ppKeys == nullptr)
            return;
        memset(m_ppKeys, 0x00, sizeof(LPWFSPINKEYDETAIL) * (KEY_MAX_NUM + 1));
    }

    auto it = m_mapKeys.find(lpsKeyName);
    if (it != m_mapKeys.end())
    {
        it->second->fwUse = fwUse;
        it->second->bLoaded = bLoaded;
        return;
    }
    if (m_mapKeys.size() >= KEY_MAX_NUM)
        return;

    LPWFSPINKEYDETAIL pKey = new WFSPINKEYDETAIL;
    if (pKey == nullptr)
        return;
    memset(pKey, 0, sizeof(WFSPINKEYDETAIL));
    pKey->lpsKeyName = new char[MAX_PATH];
    if (pKey->lpsKeyName == nullptr)
    {
        delete pKey;
        pKey = nullptr;
        return;
    }
    memset(pKey->lpsKeyName, 0x00, MAX_PATH);
    strcpy(pKey->lpsKeyName, lpsKeyName);
    pKey->fwUse = fwUse;
    pKey->bLoaded = bLoaded;
    m_mapKeys.insert(pair<std::string, LPWFSPINKEYDETAIL>(lpsKeyName, pKey));
    return;
}

LPWFSPINKEYDETAIL *CWFSPINKEYDETAIL::GetData()
{
    if (m_ppKeys != nullptr)
    {
        memset(m_ppKeys, 0x00, sizeof(LPWFSPINKEYDETAIL) * (KEY_MAX_NUM + 1));
        UINT i = 0;
        for (auto &it : m_mapKeys)
        {
            m_ppKeys[i++] = it.second;
        }
    }
    return m_ppKeys;
}
//////////////////////////////////////////////////////////////////////////

CWFSPINKEYDETAILEX::CWFSPINKEYDETAILEX()
{

}

CWFSPINKEYDETAILEX::~CWFSPINKEYDETAILEX()
{
    Clear();
    if (m_ppKeys != nullptr)
    {
        delete[]m_ppKeys;
        m_ppKeys = nullptr;
    }
}

void CWFSPINKEYDETAILEX::Clear()
{
    for (auto &it : m_mapKeys)
    {
        if (it.second != nullptr)
        {
            if (it.second->lpsKeyName != nullptr)
            {
                delete[]it.second->lpsKeyName;
                it.second->lpsKeyName = nullptr;
            }
            delete it.second;
            it.second = nullptr;
        }
    }
    m_mapKeys.clear();
}

void CWFSPINKEYDETAILEX::Add(LPCSTR lpsKeyName, DWORD fwUse, BYTE byVersion, BOOL bLoaded)
{
    if (m_ppKeys == nullptr)
    {
        m_ppKeys = new LPWFSPINKEYDETAILEX[KEY_MAX_NUM + 1];
        if (m_ppKeys == nullptr)
            return;
        memset(m_ppKeys, 0x00, sizeof(LPWFSPINKEYDETAILEX) * (KEY_MAX_NUM + 1));
    }

    auto it = m_mapKeys.find(lpsKeyName);
    if (it != m_mapKeys.end())
    {
        it->second->dwUse = fwUse;
        it->second->bVersion = byVersion;
        it->second->bLoaded = bLoaded;
        return;
    }
    if (m_mapKeys.size() >= KEY_MAX_NUM)
        return;

    LPWFSPINKEYDETAILEX pKey = new WFSPINKEYDETAILEX;
    if (pKey == nullptr)
        return;
    pKey->lpsKeyName = new char[MAX_PATH];
    if (pKey->lpsKeyName == nullptr)
    {
        delete pKey;
        pKey = nullptr;
        return;
    }
    memset(pKey->lpsKeyName, 0x00, MAX_PATH);
    strcpy(pKey->lpsKeyName, lpsKeyName);
    pKey->dwUse = fwUse;
    pKey->bVersion = byVersion;
    pKey->bLoaded = bLoaded;
    pKey->bGeneration = 0xFF;
    memset(pKey->bActivatingDate, 0xFF, sizeof(BYTE) * 4);
    memset(pKey->bExpiryDate, 0xFF, sizeof(BYTE) * 4);
    m_mapKeys.insert(pair<std::string, LPWFSPINKEYDETAILEX>(lpsKeyName, pKey));
    return;
}

LPWFSPINKEYDETAILEX *CWFSPINKEYDETAILEX::GetData()
{
    if (m_ppKeys != nullptr)
    {
        memset(m_ppKeys, 0x00, sizeof(LPWFSPINKEYDETAILEX) * (KEY_MAX_NUM + 1));
        UINT i = 0;
        for (auto &it : m_mapKeys)
        {
            m_ppKeys[i++] = it.second;
        }
    }
    return m_ppKeys;
}
//////////////////////////////////////////////////////////////////////////
CWFSXDATA::CWFSXDATA()
{
    memset(&m_stData, 0x00, sizeof(m_stData));
}

CWFSXDATA::~CWFSXDATA()
{
    Clear();
}

void CWFSXDATA::Clear()
{
    m_stData.usLength = 0;
    if (m_stData.lpbData != nullptr)
    {
        delete[]m_stData.lpbData;
        m_stData.lpbData = nullptr;
    }
}

void CWFSXDATA::Add(const LPBYTE lpbData, USHORT usLength)
{
    if (usLength > m_stData.usLength || m_stData.lpbData == nullptr)
    {
        Clear();
        m_stData.lpbData = new BYTE[usLength];
        if (m_stData.lpbData == nullptr)
            return;
    }
    else
    {
        memset(m_stData.lpbData, 0x00, m_stData.usLength);
    }

    m_stData.usLength = usLength;
    memcpy(m_stData.lpbData, lpbData, usLength);
    return;
}

LPWFSXDATA CWFSXDATA::GetData()
{
    return &m_stData;
}
//////////////////////////////////////////////////////////////////////////
CWFSPINFUNCKEYDETAIL::CWFSPINFUNCKEYDETAIL()
{
    Clear();
}

CWFSPINFUNCKEYDETAIL::~CWFSPINFUNCKEYDETAIL()
{

}

void CWFSPINFUNCKEYDETAIL::Clear()
{
    m_usNumberFDKs = 0;
    memset(&m_stKeys, 0x00, sizeof(m_stKeys));
    memset(&m_stFDKs, 0x00, sizeof(m_stFDKs));
    memset(&m_pszFDK, 0x00, sizeof(m_pszFDK));
}

void CWFSPINFUNCKEYDETAIL::SetMask(ULONG ulFuncMask)
{
    m_stKeys.ulFuncMask = ulFuncMask;
}

void CWFSPINFUNCKEYDETAIL::AddFDK(const WFSPINFDK &stFDK)
{
    if (m_usNumberFDKs >= FDK_MAX_NUM)
        return;
    memcpy(&m_stFDKs[m_usNumberFDKs++], &stFDK, sizeof(WFSPINFDK));
}

LPWFSPINFUNCKEYDETAIL CWFSPINFUNCKEYDETAIL::GetData()
{
    for (int i = 0; i < FDK_MAX_NUM; i++)
    {
        m_pszFDK[i] = &m_stFDKs[i];
    }
    m_stKeys.lppFDKs = m_pszFDK;
    m_stKeys.usNumberFDKs = m_usNumberFDKs;
    return &m_stKeys;
}
//////////////////////////////////////////////////////////////////////////
CWFSPINDATA::CWFSPINDATA()
{
    Clear();
}

CWFSPINDATA::~CWFSPINDATA()
{

}

void CWFSPINDATA::Clear()
{
    m_usKeys = 0;
    memset(&m_stPinData, 0x00, sizeof(m_stPinData));
    memset(&m_stPinKey, 0x00, sizeof(m_stPinKey));
}

void CWFSPINDATA::Add(const WFSPINKEY &stKey)
{
    if (m_usKeys >= KEY_MAX_INPUT)
        return;
    m_stPinKey[m_usKeys++] = stKey;
}

void CWFSPINDATA::Add(const vectorWFSPINKEY &vtKeys)
{
    for (auto &it : vtKeys)
    {
        if (m_usKeys >= KEY_MAX_INPUT)
            break;
        m_stPinKey[m_usKeys++] = it;
    }
}

LPWFSPINDATA CWFSPINDATA::GetData(WORD wCompletion)
{
    if (m_usKeys > 0)
    {
        for (USHORT i = 0; i < m_usKeys && i < KEY_MAX_INPUT; i++)
        {
            m_pszPinKey[i] = &m_stPinKey[i];
        }
        m_stPinData.lpPinKeys = m_pszPinKey;
    }

    m_stPinData.usKeys = m_usKeys;
    m_stPinData.wCompletion = wCompletion;
    return &m_stPinData;
}
//////////////////////////////////////////////////////////////////////////
CInputDataHelper::CInputDataHelper(const LPWFSPINGETDATA lpPinGetData)
{
    m_bNotCheckFDK = false;
    m_ulActiveFDKs = lpPinGetData->ulActiveFDKs;
    m_ulActiveKeys = lpPinGetData->ulActiveKeys;
    m_ulTerminateFDKs = lpPinGetData->ulTerminateFDKs;
    m_ulTerminateKeys = lpPinGetData->ulTerminateKeys;
    m_usMinLen = 0;
    m_usMaxLen = lpPinGetData->usMaxLen;
    m_bAutoEnd = lpPinGetData->bAutoEnd;
    m_bGetPin = FALSE;
    if (m_usMaxLen == 0)
    {
        /* 深圳原处理
        m_usMaxLen = KEY_MAX_INPUT;
        m_bAutoEnd = FALSE;
        */
    }
}

CInputDataHelper::CInputDataHelper(const LPWFSPINGETPIN lpGetPin)
{
    m_ulActiveFDKs = lpGetPin->ulActiveFDKs;
    m_ulActiveKeys = lpGetPin->ulActiveKeys;
    m_ulTerminateFDKs = lpGetPin->ulTerminateFDKs;
    m_ulTerminateKeys = lpGetPin->ulTerminateKeys;
    m_usMinLen = lpGetPin->usMinLen;
    m_usMaxLen = lpGetPin->usMaxLen;
    m_bAutoEnd = lpGetPin->bAutoEnd;
    m_cEcho = lpGetPin->cEcho;
    m_bGetPin = TRUE;

    if (m_usMinLen == 0 && m_usMaxLen == 0)
    {
        /* 深圳原处理
        m_usMinLen = 4;
        m_usMaxLen = 6;
        */
    }
}

CInputDataHelper::~CInputDataHelper()
{

}

bool CInputDataHelper::IsSupportKey(ULONG ulAllFuncKey, ULONG ulAllFDK)
{
    if (!m_bNotCheckFDK)
    {
        if ((m_ulActiveFDKs & ulAllFDK) != m_ulActiveFDKs)
            return false;
    }
    if ((m_ulActiveKeys & ulAllFuncKey) != m_ulActiveKeys)
        return false;
    if ((m_ulTerminateFDKs & m_ulActiveFDKs) != m_ulTerminateFDKs)
        return false;
    if ((m_ulTerminateKeys & m_ulActiveKeys) != m_ulTerminateKeys)
        return false;
    return true;
}

bool CInputDataHelper::IsSupportGetPinKey(ULONG ulAllFuncKey)
{
    if (!m_bNotCheckFDK)
    {
        if (m_ulActiveFDKs != 0 || m_ulTerminateFDKs != 0)
            return false;
    }
    if ((m_ulActiveKeys & ulAllFuncKey) != m_ulActiveKeys)
        return false;
    if ((m_ulTerminateKeys & m_ulActiveKeys) != m_ulTerminateKeys)
        return false;
    if (m_ulActiveKeys & WFS_PIN_FK_00)
        return false;
    if (m_ulActiveKeys & WFS_PIN_FK_DECPOINT)
        return false;
    if (m_ulActiveKeys & WFS_PIN_FK_HELP)
        return false;
//30-00-00-00(FS#0010)    if (m_ulActiveKeys & WFS_PIN_FK_BACKSPACE)
//30-00-00-00(FS#0010)        return false;
    //追加FDK按键的判断
    if((m_ulTerminateFDKs & m_ulActiveFDKs) != m_ulTerminateFDKs){
        return false;
    }
    if(m_ulTerminateKeys & (~(WFS_PIN_FK_ENTER | WFS_PIN_FK_CANCEL))){
        return false;
    }
    return true;
}

bool CInputDataHelper::IsHasActiveKey()
{
    return (m_ulActiveKeys > 0);
}

bool CInputDataHelper::IsActiveKey(ULONG ulFuncKey)
{
    if (ulFuncKey == 0)
        return false;
    if ((m_ulActiveKeys & ulFuncKey) != ulFuncKey)
        return false;
    return true;
}

bool CInputDataHelper::IsActiveFDK(ULONG ulFDK)
{
    if (ulFDK == 0)
        return false;
    if ((m_ulActiveFDKs & ulFDK) != ulFDK)
        return false;
    return true;
}

bool CInputDataHelper::IsTermKey(ULONG ulFuncKey)
{
    if (ulFuncKey == 0)
        return false;
    if ((m_ulTerminateKeys & ulFuncKey) != ulFuncKey)
        return false;
    return true;
}

bool CInputDataHelper::IsTermFDK(ULONG ulFDK)
{
    if (ulFDK == 0)
        return false;
    if ((m_ulTerminateFDKs & ulFDK) != ulFDK)
        return false;
    return true;
}

USHORT CInputDataHelper::MinLen()
{
    return m_usMinLen;
}

USHORT CInputDataHelper::MaxLen()
{
    return m_usMaxLen;
}

BOOL CInputDataHelper::IsAutoEnd()
{
    return m_bAutoEnd;
}

BOOL CInputDataHelper::IsGetPin()
{
    return m_bGetPin;
}

void CInputDataHelper::SetEndKey(WFSPINKEY stKey)
{
    m_stKey = stKey;
}

WFSPINKEY CInputDataHelper::GetEndKey()
{
    return m_stKey;
}

void CInputDataHelper::SetNotCheckFDK(bool bNotCheck)
{
    m_bNotCheckFDK = bNotCheck;
}

ULONG CInputDataHelper::GetActiveKey()
{
    return m_ulActiveKeys;
}

ULONG CInputDataHelper::GetActiveFDK()
{
    return m_ulActiveFDKs;
}

char CInputDataHelper::GetEchoChar()
{
    return m_cEcho;
}

//////////////////////////////////////////////////////////////////////////

