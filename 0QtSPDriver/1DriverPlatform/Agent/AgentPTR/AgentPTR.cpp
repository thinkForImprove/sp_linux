#include "AgentPTR.h"

static const char *DEVTYPE  = "PTR";
static const char *ThisFile = "AgentPTR.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentPTR;
    return (p != nullptr) ? 0 : -1;
}

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
CAgentPTR::CAgentPTR()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentPTR::~CAgentPTR()
{

}

void CAgentPTR::Release()
{

}
HRESULT CAgentPTR::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
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
    case WFS_INF_PTR_CAPABILITIES:
    case WFS_INF_PTR_FORM_LIST:
    case WFS_INF_PTR_MEDIA_LIST:
        hRet = Get_WFS_PTR_NODATA(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_QUERY_FORM:
        hRet = Get_WFS_INF_PTR_QUERY_FORM(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_QUERY_MEDIA:
        hRet = Get_WFS_INF_PTR_QUERY_MEDIA(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_PTR_QUERY_FIELD:
        hRet = Get_WFS_INF_PTR_QUERY_FIELD(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentPTR::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
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
        hRet = Exe_WFS_CMD_PTR_CONTROL_MEDIA(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_PRINT_FORM:
        hRet = Exe_WFS_CMD_PTR_PRINT_FORM(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_READ_FORM:
        hRet = Exe_WFS_CMD_PTR_READ_FORM(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_RAW_DATA:
        hRet = Exe_WFS_CMD_PTR_RAW_DATA(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_MEDIA_EXTENTS:
        hRet = Exe_WFS_CMD_PTR_MEDIA_EXTENTS(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_RESET_COUNT:
        hRet = Exe_WFS_CMD_PTR_RESET_COUNT(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_READ_IMAGE:
        hRet = Exe_WFS_CMD_PTR_READ_IMAGE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_RESET:
        hRet = Exe_WFS_CMD_PTR_RESET(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_RETRACT_MEDIA:
        hRet = Exe_WFS_CMD_PTR_RETRACT_MEDIA(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_PTR_DISPENSE_PAPER:
        hRet = Exe_WFS_CMD_PTR_DISPENSE_PAPER(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAgentPTR::Get_WFS_PTR_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentPTR::Get_WFS_INF_PTR_QUERY_FORM(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpQueryDetails);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPSTR lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(char) * (strlen(lpData) + 1), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(char) * (strlen(lpData) + 1));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentPTR::Get_WFS_INF_PTR_QUERY_MEDIA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPSTR>(lpQueryDetails);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPSTR lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(char) * (strlen(lpData) + 1), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(char) * (strlen(lpData) + 1));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentPTR::Get_WFS_INF_PTR_QUERY_FIELD(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSPTRQUERYFIELD>(lpQueryDetails);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSPTRQUERYFIELD lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRQUERYFIELD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpData, sizeof(WFSPTRQUERYFIELD));
        if (lpData->lpszFormName != nullptr)
        {
            DWORD dwSize = strlen(lpData->lpszFormName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFormName);
            memcpy(lpNewData->lpszFormName, lpData->lpszFormName, sizeof(char)*dwSize);
        }

        if (lpData->lpszFieldName != nullptr)
        {
            DWORD dwSize = strlen(lpData->lpszFieldName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszFieldName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFieldName);
            memcpy(lpNewData->lpszFieldName, lpData->lpszFieldName, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}


HRESULT CAgentPTR::Exe_WFS_CMD_PTR_CONTROL_MEDIA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPDWORD>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPWORD lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(DWORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    *lpNewData = *lpData;
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_PRINT_FORM(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpPtrForm = static_cast<LPWFSPTRPRINTFORM>(lpCmdData);
    if (lpPtrForm == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSPTRPRINTFORM lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRPRINTFORM), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPtrForm, sizeof(WFSPTRPRINTFORM));
        if (lpPtrForm->lpszFormName != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszFormName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFormName);
            memcpy(lpNewData->lpszFormName, lpPtrForm->lpszFormName, sizeof(char)*dwSize);
        }

        if (lpPtrForm->lpszMediaName != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszMediaName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszMediaName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszMediaName);
            memcpy(lpNewData->lpszMediaName, lpPtrForm->lpszMediaName, sizeof(char)*dwSize);
        }

        if (lpPtrForm->lpszFields != nullptr)
        {
            UINT uLen = GetLenOfSZZ(lpPtrForm->lpszFields);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpPtrForm->lpszFields 格式错误");
                hRet = WFS_ERR_INVALID_DATA;
                break;
            }

            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpNewData, (LPVOID *)&lpNewData->lpszFields);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFields);
            memcpy(lpNewData->lpszFields, lpPtrForm->lpszFields, sizeof(char)*uLen);
        }

        if (lpPtrForm->lpszUNICODEFields != nullptr)
        {
            LPSTR lpUnChar = QString::fromStdWString(lpPtrForm->lpszUNICODEFields).toLocal8Bit().data();
            UINT uLen = GetLenOfSZZ(lpUnChar);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpPtrForm->lpszUNICODEFields 格式错误");
                hRet = WFS_ERR_INVALID_DATA;
                break;
            }

            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpNewData, (LPVOID *)&lpNewData->lpszUNICODEFields);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszUNICODEFields);
            memcpy(lpNewData->lpszUNICODEFields, lpPtrForm->lpszUNICODEFields, sizeof(char)*uLen);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_READ_FORM(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpPtrForm = static_cast<LPWFSPTRREADFORM>(lpCmdData);
    if (lpPtrForm == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSPTRREADFORM lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRREADFORM), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPtrForm, sizeof(WFSPTRREADFORM));
        if (lpPtrForm->lpszFormName != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszFormName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszFormName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFormName);
            memcpy(lpNewData->lpszFormName, lpPtrForm->lpszFormName, sizeof(char)*dwSize);
        }

        if (lpPtrForm->lpszFieldNames != nullptr)
        {
            char *p = lpPtrForm->lpszFieldNames;
            DWORD dwSize = 0;
            while ((*p != 0x00) || (*(p + 1) != 0x00 || *(p + 2) != 0x00))
            {
                p++;
                dwSize++;
            }
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize + 3, lpNewData, (LPVOID *)&lpNewData->lpszFieldNames);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFieldNames);
            memcpy(lpNewData->lpszFieldNames, lpPtrForm->lpszFieldNames, sizeof(char)*dwSize + 3);
        }

        if (lpPtrForm->lpszMediaName != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszMediaName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszMediaName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszMediaName);
            memcpy(lpNewData->lpszMediaName, lpPtrForm->lpszMediaName, sizeof(char)*dwSize);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_RAW_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSPTRRAWDATA>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSPTRRAWDATA lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRRAWDATA), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpData, sizeof(WFSPTRRAWDATA));
        if (lpData->lpbData != nullptr && lpData->ulSize > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpData->ulSize, lpNewData, (LPVOID *)&lpNewData->lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbData);
            memcpy(lpNewData->lpbData, lpData->lpbData, sizeof(BYTE)*lpData->ulSize);
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_MEDIA_EXTENTS(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWFSPTRMEDIAUNIT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPWFSPTRMEDIAUNIT lpNewData = nullptr;


    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRMEDIAUNIT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    memcpy(lpNewData, lpData, sizeof(WFSPTRMEDIAUNIT));
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_RESET_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPUSHORT>(lpCmdData);
    HRESULT hRet = WFS_SUCCESS;
    if (lpData != nullptr)
    {
        LPUSHORT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(USHORT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        memcpy(lpNewData, lpData, sizeof(USHORT));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_READ_IMAGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpPtrForm = static_cast<LPWFSPTRIMAGEREQUEST>(lpCmdData);
    if (lpPtrForm == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSPTRIMAGEREQUEST lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRIMAGEREQUEST), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPtrForm, sizeof(WFSPTRIMAGEREQUEST));

        if (lpPtrForm->lpszFrontImageFile != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszFrontImageFile) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszFrontImageFile);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszFrontImageFile);
            memcpy(lpNewData->lpszFrontImageFile, lpPtrForm->lpszFrontImageFile, sizeof(char)*dwSize);
        }

        if (lpPtrForm->lpszBackImageFile != nullptr)
        {
            DWORD dwSize = strlen(lpPtrForm->lpszBackImageFile) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData, (LPVOID *)&lpNewData->lpszBackImageFile);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszBackImageFile);
            memcpy(lpNewData->lpszBackImageFile, lpPtrForm->lpszBackImageFile, sizeof(char)*dwSize);
        }
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpCmdData != nullptr)
    {
        auto lpData = static_cast<LPWFSPTRRESET>(lpCmdData);
        LPWFSPTRRESET lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSPTRRESET), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpData, sizeof(WFSPTRRESET));
        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    }
    else
    {
        hRet = WFS_SUCCESS;
        lpCopyCmdData = nullptr;
    }

    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_RETRACT_MEDIA(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPUSHORT>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPUSHORT lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(USHORT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    *lpNewData = *lpData;
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return hRet;
}

HRESULT CAgentPTR::Exe_WFS_CMD_PTR_DISPENSE_PAPER(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpData = static_cast<LPWORD>(lpCmdData);
    if (lpData == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    LPWORD lpNewData = nullptr;
    hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WORD), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
    if (hRet != WFS_SUCCESS)
        return hRet;

    *lpNewData = *lpData;
    lpCopyCmdData = lpNewData;
    lpNewData = nullptr;
    return hRet;
}
//////////////////////////////////////////////////////////////////////////
bool CAgentPTR::LoadDll()
{
    if (m_pIWFM != nullptr)
        return true;

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;

    return true;
}
