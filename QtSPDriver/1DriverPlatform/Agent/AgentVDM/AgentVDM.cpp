#include "AgentVDM.h"
static const char *DEVTYPE  = "VDM";
static const char *ThisFile = "AgentVDM.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentVDM;
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////

CAgentVDM::CAgentVDM()
{
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentVDM::~CAgentVDM()
{

}

void CAgentVDM::Release()
{

}

HRESULT CAgentVDM::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if(!LoadDll()){
        Log(ThisModule, __LINE__, "Load WFMShareMenory fail");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch(dwCategory){
    case WFS_INF_VDM_STATUS:
        hRet = Get_WFS_INF_VDM_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_VDM_CAPABILITIES:
        hRet = Get_WFS_INF_VDM_CAPABILITIES(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentVDM::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if(!LoadDll()){
        Log(ThisModule, __LINE__, "Load WFMShareMenory fail");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch(dwCommand){
    case WFS_CMD_VDM_ENTER_MODE_REQ:
        hRet = Exe_WFS_CMD_VDM_ENTER_MODE_REQ(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_VDM_ENTER_MODE_ACK:
        hRet = Exe_WFS_CMD_VDM_ENTER_MODE_ACK(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_VDM_EXIT_MODE_REQ:
        hRet = Exe_WFS_CMD_VDM_EXIT_MODE_REQ(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_VDM_EXIT_MODE_ACK:
        hRet = Exe_WFS_CMD_VDM_EXIT_MODE_ACK(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }

    return hRet;
}

HRESULT CAgentVDM::GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if(!LoadDll()){
        Log(ThisModule, __LINE__, "Load WFMShareMenory fail");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch(dwCategory){
    case WFS_INF_VDM_STATUS:
        hRet = Fmt_WFSVDMSTATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_VDM_CAPABILITIES:
        hRet = Fmt_WFSVDMCAPS(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentVDM::ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if(!LoadDll()){
        Log(ThisModule, __LINE__, "Load WFMShareMenory fail");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch(dwCommand){
    case WFS_CMD_VDM_ENTER_MODE_REQ:
    case WFS_CMD_VDM_ENTER_MODE_ACK:
    case WFS_CMD_VDM_EXIT_MODE_REQ:
    case WFS_CMD_VDM_EXIT_MODE_ACK:
        hRet = WFS_SUCCESS;
        break;
    default:
        break;
    }

    return hRet;
}

HRESULT CAgentVDM::CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Faile");
        return WFS_ERR_INTERNAL_ERROR;
    }
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch(uMsgID){
    case WFS_SERVICE_EVENT:
    {
        switch(dwEventID){
        case WFS_SRVE_VDM_ENTER_MODE_REQ:
        case WFS_SRVE_VDM_EXIT_MODE_REQ:
            lpResult->lpBuffer = nullptr;
            hRet = WFS_SUCCESS;
            break;
        default:
            break;
        }
    }
        break;
    case WFS_SYSTEM_EVENT:
    {
        switch(dwEventID){
        case WFS_SYSE_HARDWARE_ERROR:
            hRet = Fmt_WFSHWERROR(lpResult, lpData);
            break;
        case WFS_SYSE_DEVICE_STATUS:
            hRet = Fmt_WFSDEVSTATUS(lpResult, lpData);
            break;
        case WFS_SYSE_VDM_MODEENTERED:
        case WFS_SYSE_VDM_MODEEXITED:
            lpResult->lpBuffer = nullptr;
            hRet = WFS_SUCCESS;
            break;
        default:
            break;
        }
    }
        break;
    case WFS_EXECUTE_EVENT:
    case WFS_USER_EVENT:
    default:
        break;
    }
    return hRet;
}

HRESULT CAgentVDM::Get_WFS_INF_VDM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    return WFS_SUCCESS;
}

HRESULT CAgentVDM::Get_WFS_INF_VDM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    return WFS_SUCCESS;
}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_ENTER_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return WFS_SUCCESS;
}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_ENTER_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return WFS_SUCCESS;
}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_EXIT_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return WFS_SUCCESS;
}

HRESULT CAgentVDM::Exe_WFS_CMD_VDM_EXIT_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    return WFS_SUCCESS;
}

bool CAgentVDM::LoadDll()
{
    if(m_pIWFM != nullptr){
       return true;
    }

    if(0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory")){
        return false;
    }
    return true;
}

//--------------------------------------------出参内存拷贝---------------------------------
HRESULT CAgentVDM::Fmt_WFSVDMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do{
        auto lpStatus = static_cast<LPWFSVDMSTATUS>(lpData);
        if(lpStatus == nullptr){
            return hRet;
        }

        LPWFSVDMSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSVDMSTATUS), lpResult, (LPVOID *)&lpNewData);
        if(hRet != WFS_SUCCESS){
            break;
        }

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSVDMSTATUS));
        lpNewData->lppAppStatus = nullptr;
        if(lpStatus->lppAppStatus != nullptr){
            int iCount = 0;
            while(lpStatus->lppAppStatus[iCount] != nullptr){
                iCount++;
            }

            LPWFSVDMAPPSTATUS *lppAppStatus = nullptr;
            hRet = m_pIWFM->SIMAllocateMore(sizeof(LPWFSVDMAPPSTATUS) * (iCount + 1), lpResult, (LPVOID *)&lppAppStatus);
            if(hRet != WFS_SUCCESS){
                break;
            }

            memset(lppAppStatus, 0, (iCount + 1) * sizeof(LPWFSVDMAPPSTATUS));
            _auto.push_back(lppAppStatus);
            for(int i = 0; i < iCount; i++){
                LPWFSVDMAPPSTATUS lpAppStatus = nullptr;
                hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSVDMAPPSTATUS), lpResult, (LPVOID *)&lpAppStatus);
                if(hRet != WFS_SUCCESS){
                    break;
                }

                _auto.push_back(lpAppStatus);
                memset(lpAppStatus, 0, sizeof(WFSVDMAPPSTATUS));
                lpAppStatus->wAppStatus = lpStatus->lppAppStatus[i]->wAppStatus;

                if(lpStatus->lppAppStatus[i]->lpszAppID != nullptr){
                    int iLen = strlen(lpStatus->lppAppStatus[i]->lpszAppID) + 1;
                    LPSTR lpszAppID = nullptr;
                    hRet = m_pIWFM->SIMAllocateMore(iLen, lpResult, (LPVOID *)&lpszAppID);
                    if(hRet != WFS_SUCCESS){
                        break;
                    }

                    _auto.push_back(lpszAppID);
                    memcpy(lpszAppID, lpStatus->lppAppStatus[i]->lpszAppID, iLen -1);
                    lpAppStatus->lpszAppID = lpszAppID;
                }
                lppAppStatus[i] = lpAppStatus;
            }

            if(hRet != WFS_SUCCESS){
                break;
            }

        }
        lpNewData->lpszExtra = nullptr;
        if(lpStatus->lpszExtra != nullptr){
            hRet = Fmt_ExtraStatus(lpResult, lpNewData->lpszExtra, lpStatus->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
        }

        lpResult->lpBuffer = lpNewData;
    }while(false);

    return hRet;
}

HRESULT CAgentVDM::Fmt_WFSVDMCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_SUCCESS;
    CAutoSIMFreeBuffer _auto(m_pIWFM, &hRet);

    do{
        auto lpCaps = static_cast<LPWFSVDMCAPS>(lpData);
        if(lpCaps == nullptr){
            break;
        }

        LPWFSVDMCAPS LpNewData = nullptr;
        hRet = m_pIWFM->SIMAllocateMore(sizeof(WFSVDMCAPS), lpResult, (LPVOID *)&LpNewData);
        if(hRet != WFS_SUCCESS){
            break;
        }

        _auto.push_back(LpNewData);
        memcpy(LpNewData, lpCaps, sizeof(WFSVDMCAPS));
        LpNewData->lpszExtra = nullptr;
        if(lpCaps->lpszExtra != nullptr){
            hRet = Fmt_ExtraStatus(lpResult, LpNewData->lpszExtra, lpCaps->lpszExtra);
            if(hRet != WFS_SUCCESS){
                break;
            }
        }

        lpResult->lpBuffer = LpNewData;
    }while(false);

    return hRet;
}

HRESULT CAgentVDM::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
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

HRESULT CAgentVDM::Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData)
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

HRESULT CAgentVDM::Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData)
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
