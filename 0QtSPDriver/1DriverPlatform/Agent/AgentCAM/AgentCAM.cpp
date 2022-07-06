#include "AgentCAM.h"

static const char *DEVTYPE  = "CAM";
static const char *ThisFile = "AgentCAM.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p)
{
    p = new CAgentCAM;
    return (p != nullptr) ? 0 : -1;
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

CAgentCAM::CAgentCAM()
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
}

CAgentCAM::~CAgentCAM()
{

}

void CAgentCAM::Release()
{

}

HRESULT CAgentCAM::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Fail");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_CATEGORY;
    switch (dwCategory){
    case WFS_INF_CAM_STATUS:
        hRet = Get_WFS_INF_CAM_STATUS(lpQueryDetails, lpCopyCmdData);
        break;
    case WFS_INF_CAM_CAPABILITIES:
        hRet = Get_WFS_INF_CAM_CAPABILITIES(lpQueryDetails, lpCopyCmdData);
        break;
    default:
        break;
    }

    return hRet;
}

HRESULT CAgentCAM::Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!LoadDll())
    {
        Log(ThisModule, __LINE__, "Load WFMShareMenory Fail");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    switch (dwCommand){
    case WFS_CMD_CAM_TAKE_PICTURE:
        hRet = Exe_WFS_CMD_CAM_TAKE_PICTURE(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CAM_RESET:
        hRet = Exe_WFS_CMD_CAM_RESET(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CAM_TAKE_PICTURE_EX:
        hRet = Exe_WFS_CMD_CAM_TAKE_PICTURE_EX(lpCmdData, lpCopyCmdData);
        break;
    case WFS_CMD_CAM_DISPLAY:
        hRet = Exe_WFS_CMD_CAM_DISPLAY(lpCmdData, lpCopyCmdData);
        break;
    default:
        break;
    }

    return hRet;
}

HRESULT CAgentCAM::Get_WFS_INF_CAM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpQueryDetails = nullptr;
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentCAM::Get_WFS_INF_CAM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpQueryDetails = nullptr;
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentCAM::Exe_WFS_CMD_CAM_TAKE_PICTURE(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpCamTakePic = static_cast<LPWFSCAMTAKEPICT>(lpCmdData);
    if (lpCamTakePic == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCAMTAKEPICT lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(LPWFSCAMTAKEPICT), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCamTakePic, sizeof(LPWFSCAMTAKEPICT));

        if (lpCamTakePic->lpszCamData!= nullptr)
        {
            DWORD dwSize = strlen(lpCamTakePic->lpszCamData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData,
                                            (LPVOID *)&lpNewData->lpszCamData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszCamData);
            memcpy(lpNewData->lpszCamData, lpCamTakePic->lpszCamData, sizeof(char)*dwSize);
        }

        if (lpCamTakePic->lpszUNICODECamData != nullptr)
        {
            LPSTR lpUnChar = QString::fromStdWString(
                        lpCamTakePic->lpszUNICODECamData).toLocal8Bit().data();
            UINT uLen = GetLenOfSZZ(lpUnChar);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpPtrForm->lpszUNICODECamData 格式错误");
                hRet = WFS_ERR_INVALID_DATA;
                break;
            }

            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpNewData,
                                            (LPVOID *)&lpNewData->lpszUNICODECamData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszUNICODECamData);
            memcpy(lpNewData->lpszUNICODECamData, lpCamTakePic->lpszUNICODECamData,
                   sizeof(char)*uLen);
        }

        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CAgentCAM::Exe_WFS_CMD_CAM_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCmdData = nullptr;
    lpCopyCmdData = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentCAM::Exe_WFS_CMD_CAM_TAKE_PICTURE_EX(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    auto lpCamTakePicEx = static_cast<LPWFSCAMTAKEPICTEX>(lpCmdData);
    if (lpCamTakePicEx == nullptr)
        return WFS_ERR_INVALID_POINTER;

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    LPWFSCAMTAKEPICTEX lpNewData = nullptr;

    do
    {
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCAMTAKEPICTEX), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCamTakePicEx, sizeof(WFSCAMTAKEPICTEX));

        if (lpCamTakePicEx->lpszCamData!= nullptr)
        {
            DWORD dwSize = strlen(lpCamTakePicEx->lpszCamData) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData,
                                            (LPVOID *)&lpNewData->lpszCamData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszCamData);
            memcpy(lpNewData->lpszCamData, lpCamTakePicEx->lpszCamData, sizeof(char)*dwSize);
        }

        if (lpCamTakePicEx->lpszUNICODECamData != nullptr)
        {
            LPSTR lpUnChar = QString::fromStdWString(
                        lpCamTakePicEx->lpszUNICODECamData).toLocal8Bit().data();
            UINT uLen = GetLenOfSZZ(lpUnChar);
            if (uLen <= 0)
            {
                Log(ThisModule, __LINE__, "lpPtrForm->lpszUNICODECamData 格式错误");
                hRet = WFS_ERR_INVALID_DATA;
                break;
            }

            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpNewData,
                                            (LPVOID *)&lpNewData->lpszUNICODECamData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszUNICODECamData);
            memcpy(lpNewData->lpszUNICODECamData, lpCamTakePicEx->lpszUNICODECamData,
                   sizeof(char)*uLen);
        }

        if (lpCamTakePicEx->lpszPictureFile!= nullptr)
        {
            DWORD dwSize = strlen(lpCamTakePicEx->lpszPictureFile) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * dwSize, lpNewData,
                                            (LPVOID *)&lpNewData->lpszPictureFile);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpszPictureFile);
            memcpy(lpNewData->lpszPictureFile, lpCamTakePicEx->lpszPictureFile, sizeof(char)*dwSize);
        }


        lpCopyCmdData = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;

}

HRESULT CAgentCAM::Exe_WFS_CMD_CAM_DISPLAY(LPVOID lpCmdData, LPVOID &lpCopyCmdData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    lpCopyCmdData = nullptr;
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    if (lpCmdData != nullptr)
    {
        auto lpData = static_cast<LPWFSCAMDISP>(lpCmdData);
        LPWFSCAMDISP lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSCAMDISP), WFS_MEM_FLAG, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            return hRet;
        memcpy(lpNewData, lpData, sizeof(WFSCAMDISP));
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

bool CAgentCAM::LoadDll()
{
    if(m_pIWFM != nullptr){
        return true;
    }

    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        return false;
    return true;
}
