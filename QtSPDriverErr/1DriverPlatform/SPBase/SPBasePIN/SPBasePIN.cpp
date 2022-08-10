#include "SPBasePIN.h"

static const char *ThisFile = "SPBasePIN.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" SPBASEPINSHARED_EXPORT long CreateISPBasePIN(LPCSTR lpDevType, ISPBasePIN *&p)
{
    p = new CSPBasePIN(lpDevType);
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CSPBasePIN::CSPBasePIN(LPCSTR lpLogType) : m_pCmdFunc(nullptr)
{
    strcpy(m_szLogType, lpLogType);
    SetLogFile(LOGFILE, ThisFile, lpLogType);
    m_pCmdFunc = nullptr;
}

CSPBasePIN::~CSPBasePIN()
{
}

void CSPBasePIN::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

void CSPBasePIN::RegisterICmdFunc(ICmdFunc *pCmdFunc)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdFunc = pCmdFunc;
    return;
}

bool CSPBasePIN::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载Base类
    if (0 != m_pBase.Load("SPBaseClass.dll", "CreateISPBaseClass", m_szLogType))
    {
        Log(ThisModule, __LINE__, "加载SPBaseClass类失败！");
        return false;
    }

    // 加载共享内存类
    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
    {
        Log(ThisModule, __LINE__, "加载库失败: WFMShareMenory.dll");
        return false;
    }

    // 注册回调
    m_pBase->RegisterICmdRun(this);
    return m_pBase->StartRun();
}

void CSPBasePIN::GetSPBaseData(SPBASEDATA &stData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return;
    }
    return m_pBase->GetSPBaseData(stData);
}

bool CSPBasePIN::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }
    return m_pBase->FireEvent(uMsgID, dwEventID, lpData);
}

bool CSPBasePIN::FireStatusChanged(DWORD dwStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }

    WFSDEVSTATUS stStatus;
    memset(&stStatus, 0x00, sizeof(stStatus));
    stStatus.dwState = dwStatus;
    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_DEVICE_STATUS, &stStatus);
}

bool CSPBasePIN::FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription /*= nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_pBase)
    {
        Log(ThisModule, __LINE__, "没有加载SPBaseClass类失败！");
        return false;
    }

    WFSHWERROR stStatus;
    memset(&stStatus, 0x00, sizeof(stStatus));
    stStatus.dwAction = dwAction;
    if (lpDescription != nullptr)
    {
        stStatus.dwSize = strlen((char *)lpDescription);
        stStatus.lpbDescription = (LPBYTE)lpDescription;
    }
    return m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_HARDWARE_ERROR, &stStatus);
}

HRESULT CSPBasePIN::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    return m_pCmdFunc->OnOpen(lpLogicalName);
}

HRESULT CSPBasePIN::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnClose();
}

HRESULT CSPBasePIN::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnStatus();
}

HRESULT CSPBasePIN::OnWaitTaken()
{
    //因为没有要等待取走什么的，直接返回取消
    return WFS_ERR_CANCELED;
}

HRESULT CSPBasePIN::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnCancelAsyncRequest();
}

HRESULT CSPBasePIN::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return m_pCmdFunc->OnUpdateDevPDL();
}


HRESULT CSPBasePIN::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_UNSUPP_DATA;
    switch (uMsgID)
    {
    case WFS_EXECUTE_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_EXEE_PIN_KEY:
                hRet = Fmt_WFSPINKEY(lpData, lpResult);
                break;
            default:
                break;
            }
        }
        break;
    case WFS_SERVICE_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_SRVE_PIN_INITIALIZED:
                hRet = Fmt_WFSPININIT(lpData, lpResult);
                break;
            default:
                break;
            }
        }
        break;
    case WFS_USER_EVENT:
        break;
    case WFS_SYSTEM_EVENT:
        {
            switch (dwEventID)
            {
            case WFS_SYSE_HARDWARE_ERROR:
                hRet = m_pBase->Fmt_WFSHWERROR(lpResult, lpData);
                break;
            case WFS_SYSE_DEVICE_STATUS:
                hRet = m_pBase->Fmt_WFSDEVSTATUS(lpResult, lpData);
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
    return hRet;
}


HRESULT CSPBasePIN::GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    switch (dwCategory)
    {
    case WFS_INF_PIN_STATUS:
        {
            LPWFSPINSTATUS lpstStatus = nullptr;
            hRet = m_pCmdFunc->GetStatus(lpstStatus);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPINSTATUS(lpstStatus, lpResult);
        }
        break;
    case WFS_INF_PIN_CAPABILITIES:
        {
            LPWFSPINCAPS lpstCaps = nullptr;
            hRet = m_pCmdFunc->GetCapabilities(lpstCaps);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPINCAPS(lpstCaps, lpResult);
        }
        break;
    case WFS_INF_PIN_KEY_DETAIL:
        {
            LPSTR lpszKeyName = static_cast<LPSTR>(lpQueryDetails);
            //if (lpszKeyName == nullptr)// 支持空指针
            //    break;
#ifdef QT_WIN32
            LPWFSPINKEYDETAIL *lppKeyDetail = nullptr;
            hRet = m_pCmdFunc->GetKeyDetail(lpszKeyName, lppKeyDetail);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPINKEYDETAIL(lppKeyDetail, lpResult);
#else
/* 深圳原版代码处理
            LPWFSPINKEYDETAILEX *lppKeyDetail = nullptr;
            hRet = m_pCmdFunc->GetKeyDetailEx(lpszKeyName, lppKeyDetail);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPINKEYDETAILEX(lppKeyDetail, lpResult);
 */
            LPWFSPINKEYDETAIL *lppKeyDetail = nullptr;
            hRet = m_pCmdFunc->GetKeyDetail(lpszKeyName, lppKeyDetail);
            if(hRet != WFS_SUCCESS){
                break;
            }
            hRet = Fmt_WFSPINKEYDETAIL(lppKeyDetail, lpResult);
#endif
        }
        break;
    case WFS_INF_PIN_FUNCKEY_DETAIL:
        {
            LPULONG lpFDKMask = static_cast<LPULONG>(lpQueryDetails);
            if (lpFDKMask == nullptr)
                break;

            LPWFSPINFUNCKEYDETAIL lpFunKeyDetail = nullptr;
            hRet = m_pCmdFunc->GetFuncKeyDetail(lpFDKMask, lpFunKeyDetail);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPINFUNCKEYDETAIL(lpFunKeyDetail, lpResult);
        }
        break;
    case WFS_INF_PIN_KEY_DETAIL_EX:
        {
            LPSTR lpszKeyName = static_cast<LPSTR>(lpQueryDetails);
            //if (lpszKeyName == nullptr)// 支持空指针
            //    break;

            LPWFSPINKEYDETAILEX *lppKeyDetail = nullptr;
            hRet = m_pCmdFunc->GetKeyDetailEx(lpszKeyName, lppKeyDetail);
            if (hRet != WFS_SUCCESS)
                break;

            hRet = Fmt_WFSPINKEYDETAILEX(lppKeyDetail, lpResult);
        }
        break;
    default:
        hRet = WFS_ERR_UNSUPP_CATEGORY;
        break;
    }
    return hRet;
}

HRESULT CSPBasePIN::Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pCmdFunc == nullptr)
    {
        Log(ThisModule, __LINE__, "没有注册回调类ICmdFunc");
        return WFS_ERR_INTERNAL_ERROR;
    }

    HRESULT hRes = WFS_ERR_INTERNAL_ERROR;
    switch (dwCommand)
    {
    case WFS_CMD_PIN_CRYPT:
        {
            LPWFSPINCRYPT  lpCrypt = static_cast<LPWFSPINCRYPT>(lpCmdData);
            if (lpCrypt == nullptr)
                break;

            LPWFSXDATA lpxCryptData = nullptr;
            hRes = m_pCmdFunc->Crypt(lpCrypt, lpxCryptData);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = Fmt_WFSXDATA(lpxCryptData, lpResult);
        }
        break;
    case WFS_CMD_PIN_IMPORT_KEY:
        {
            LPWFSPINIMPORT  lpImportData = static_cast<LPWFSPINIMPORT>(lpCmdData);
            if (lpImportData == nullptr)
                break;
            LPWFSXDATA lpxKVC = nullptr;
            hRes = m_pCmdFunc->ImportKey(lpImportData, lpxKVC);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = Fmt_WFSXDATA(lpxKVC, lpResult);
        }
        break;
    case WFS_CMD_PIN_ENC_IO:                                                            //30-00-00-00(FS#0003)
       {
        LPWFSPINENCIO  lpEncIO = static_cast<LPWFSPINENCIO>(lpCmdData);                 //30-00-00-00(FS#0003)
        if (lpEncIO == nullptr)                                                         //30-00-00-00(FS#0003)
            break;                                                                      //30-00-00-00(FS#0003)

//        LPWFSXDATA lpxEncIOData = nullptr;                                            //30-00-00-00(FS#0003)
        LPWFSPINENCIO lpxEncIOData = nullptr;                                           //30-00-00-00(FS#0003)
        hRes = m_pCmdFunc->EncIo(lpEncIO, lpxEncIOData);                                //30-00-00-00(FS#0003)
        if (hRes != WFS_SUCCESS)                                                        //30-00-00-00(FS#0003)
            break;                                                                      //30-00-00-00(FS#0003)
        hRes = Fmt_WFSPINENCIO(lpxEncIOData, lpResult);                                 //30-00-00-00(FS#0003)
        break;                                                                          //30-00-00-00(FS#0003)
       }
    case WFS_CMD_PIN_GET_PIN:
        {
            LPWFSPINGETPIN lpGetPin = static_cast<LPWFSPINGETPIN>(lpCmdData);
            if (lpGetPin == nullptr)
                break;

            LPWFSPINENTRY  lpEntry = nullptr;
            hRes = m_pCmdFunc->GetPIN(lpGetPin, lpEntry, dwTimeOut);
            if (hRes != WFS_SUCCESS)
                break;
            hRes = Fmt_WFSPINENTRY(lpEntry, lpResult);
        }
        break;
    case WFS_CMD_PIN_GET_PINBLOCK:
        {
            LPWFSPINBLOCK  lpPinBlock = static_cast<LPWFSPINBLOCK>(lpCmdData);
            if (lpPinBlock == nullptr)
                break;

            LPWFSXDATA lpxPinBlock = nullptr;
            hRes = m_pCmdFunc->GetPinBlock(lpPinBlock, lpxPinBlock);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSXDATA(lpxPinBlock, lpResult);
        }
        break;
    case WFS_CMD_PIN_GET_DATA:
        {
            LPWFSPINGETDATA lpPinGetData = static_cast<LPWFSPINGETDATA>(lpCmdData);
            if (lpPinGetData == nullptr)
                break;

            LPWFSPINDATA lpPinData = nullptr;
            hRes = m_pCmdFunc->GetData(lpPinGetData, lpPinData, dwTimeOut);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPINDATA(lpPinData, lpResult);
        }
        break;
    case WFS_CMD_PIN_INITIALIZATION:
        {
            LPWFSPININIT lpInit = static_cast<LPWFSPININIT>(lpCmdData);
            if (lpInit == nullptr)
                break;

            LPWFSXDATA lpxIdentification = nullptr;
            hRes = m_pCmdFunc->Initialization(lpInit, lpxIdentification);
            if (hRes != WFS_SUCCESS)
                break;

            // 此时不支持返回值
            //hRes = Fmt_WFSXDATA(lpxIdentification, lpResult);
        }
        break;
    case WFS_CMD_PIN_RESET:
        hRes = m_pCmdFunc->Reset();
        break;
    case WFS_CMD_PIN_GENERATE_KCV:
        {
            LPWFSPINGENERATEKCV  lpGenerateKCV = static_cast<LPWFSPINGENERATEKCV>(lpCmdData);
            if (lpGenerateKCV == nullptr)
                break;

            LPWFSPINKCV lpKCV = nullptr;
            hRes = m_pCmdFunc->GenerateKCV(lpGenerateKCV, lpKCV);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPINKCV(lpKCV, lpResult);
        }
        break;
    case WFS_CMD_PIN_GET_PINBLOCK_EX:
        {
            LPWFSPINBLOCKEX lpPinBlockEx = static_cast<LPWFSPINBLOCKEX>(lpCmdData);
            if (lpPinBlockEx == nullptr)
                break;

            LPWFSXDATA lpxPinBlock = nullptr;
            hRes = m_pCmdFunc->GetPinBlockEx(lpPinBlockEx, lpxPinBlock);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSXDATA(lpxPinBlock, lpResult);
        }
        break;
    case WFS_CMD_PIN_IMPORT_KEY_EX:
        {
            LPWFSPINIMPORTKEYEX  lpImportEx = static_cast<LPWFSPINIMPORTKEYEX>(lpCmdData);
            if (lpImportEx == nullptr)
                break;

            hRes = m_pCmdFunc->ImportKeyEx(lpImportEx);
            if (hRes != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_PIN_START_KEY_EXCHANGE:
        {
            LPWFSPINSTARTKEYEXCHANGE lpStartKeyExchange = nullptr;          //30-00-00-00(FS#0006)
            hRes = m_pCmdFunc->StarKeyExchange(lpStartKeyExchange);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPINSTARTKEYEXCHANGE(lpStartKeyExchange, lpResult);
        }
        break;
    case WFS_CMD_PIN_IMPORT_RSA_PUBLIC_KEY:
        {
            auto lpPublicKey = static_cast<LPWFSPINIMPORTRSAPUBLICKEY>(lpCmdData);
            if (lpPublicKey == nullptr)
                break;

            LPWFSPINIMPORTRSAPUBLICKEYOUTPUT lpPublicKeyOutput = nullptr;
            hRes = m_pCmdFunc->ImportRSAPublicKey(lpPublicKey, lpPublicKeyOutput);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPINIMPORTRSAPUBLICKEYOUTPUT(lpPublicKeyOutput, lpResult);
        }
        break;
    case WFS_CMD_PIN_EXPORT_RSA_ISSUER_SIGNED_ITEM:
        {
            auto lpSignedItem = static_cast<LPWFSPINEXPORTRSAISSUERSIGNEDITEM>(lpCmdData);
            if (lpSignedItem == nullptr)
                break;

            LPWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT lpSignedItemOutput = nullptr;
            hRes = m_pCmdFunc->ExportRSAIssuerSignedItem(lpSignedItem, lpSignedItemOutput);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT(lpSignedItemOutput, lpResult);
        }
        break;
    case WFS_CMD_PIN_IMPORT_RSA_SIGNED_DES_KEY:
        {
            auto  lpSignedDESKey = static_cast<LPWFSPINIMPORTRSASIGNEDDESKEY>(lpCmdData);
            if (lpSignedDESKey == nullptr)
                break;

            LPWFSPINIMPORTRSASIGNEDDESKEYOUTPUT lpSignedDESKeyOutput = nullptr;
            hRes = m_pCmdFunc->ImportRSASignedDesKey(lpSignedDESKey, lpSignedDESKeyOutput);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPINIMPORTRSASIGNEDDESKEYOUTPUT(lpSignedDESKeyOutput, lpResult);
        }
        break;
    case WFS_CMD_PIN_GENERATE_RSA_KEY_PAIR:
        {
            auto lpGenerateRSAKeyPair = static_cast<LPWFSPINGENERATERSAKEYPAIR>(lpCmdData);
            if (lpGenerateRSAKeyPair == nullptr)
                break;

            hRes = m_pCmdFunc->GenerateRSAKeyPair(lpGenerateRSAKeyPair);
            if (hRes != WFS_SUCCESS)
                break;
        }
        break;
    case WFS_CMD_PIN_EXPORT_RSA_EPP_SIGNED_ITEM:
        {
            auto lpEppSignedItem = static_cast<LPWFSPINEXPORTRSAEPPSIGNEDITEM>(lpCmdData);
            if (lpEppSignedItem == nullptr)
                break;

            LPWFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT lpEppSignedItemOutput = nullptr;
            hRes = m_pCmdFunc->ExportRSAEppSignedItem(lpEppSignedItem, lpEppSignedItemOutput);
            if (hRes != WFS_SUCCESS)
                break;

            hRes = Fmt_WFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT(lpEppSignedItemOutput, lpResult);
        }
        break;
    default:
        hRes = WFS_ERR_UNSUPP_COMMAND;
        break;
    }
    return hRes;
}

HRESULT CSPBasePIN::Fmt_WFSXDATA(CAutoWFMFreeBuffer &_auto, const LPWFSXDATA lpXData, LPWFSXDATA &lpNewXData, LPWFSRESULT &lpResult)
{
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
        if (hRet != WFS_SUCCESS)
            break;
        _auto.push_back(lpNewXData);
        memset(lpNewXData, 0x00, sizeof(WFSXDATA));
        if (lpXData->lpbData != nullptr && lpXData->usLength > 0)
        {
            LPBYTE lpbData = nullptr;
            USHORT usLength = lpXData->usLength;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * usLength, lpResult, (LPVOID *)&lpbData);
            if (hRet != WFS_SUCCESS)
                break;
            _auto.push_back(lpbData);
            memcpy(lpbData, lpXData->lpbData, usLength);
            lpNewXData->lpbData = lpbData;
            lpNewXData->usLength = usLength;
        }

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSXDATA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpXData = (LPWFSXDATA)lpData;
        if (lpXData == nullptr)
            break;

        LPWFSXDATA lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memset(lpNewData, 0x00, sizeof(WFSXDATA));
        if ((lpXData->lpbData != nullptr) && (lpXData->usLength > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpXData->usLength, lpResult, (LPVOID *)&lpNewData->lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbData);
            lpNewData->usLength = lpXData->usLength;
            memcpy(lpNewData->lpbData, lpXData->lpbData, lpXData->usLength);

        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINSTATUS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSPINSTATUS>(lpData);
        if (lpStatus == nullptr)
            break;

        LPWFSPINSTATUS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINSTATUS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpStatus, sizeof(WFSPINSTATUS));
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

HRESULT CSPBasePIN::Fmt_WFSPINCAPS(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpCaps = static_cast<LPWFSPINCAPS>(lpData);
        if (lpCaps == nullptr)
            break;

        LPWFSPINCAPS lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINCAPS), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpCaps, sizeof(WFSPINCAPS));
        lpNewData->lpszExtra = nullptr;

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

HRESULT CSPBasePIN::Fmt_WFSPINKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
#ifdef QT_WIN32
        auto lppKeyDetails = (LPWFSPINKEYDETAIL *)lpData;
        if (lppKeyDetails == nullptr)
            break;

        USHORT usCount = 0;
        while (lppKeyDetails[usCount] != nullptr) usCount++;
        if (usCount == 0)
            break;

        LPWFSPINKEYDETAIL *lppNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPINKEYDETAIL) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0x00, sizeof(LPWFSPINKEYDETAIL) * (usCount + 1));

        for (USHORT i = 0; i < usCount; i++)
        {
            LPWFSPINKEYDETAIL lpNewKey = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINKEYDETAIL), lpResult, (LPVOID *)&lpNewKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewKey);
            memcpy(lpNewKey, lppKeyDetails[i], sizeof(WFSPINKEYDETAIL));

            UINT usSize = strlen(lppKeyDetails[i]->lpsKeyName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewKey->lpsKeyName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewKey->lpsKeyName);
            memcpy(lpNewKey->lpsKeyName, lppKeyDetails[i]->lpsKeyName, sizeof(char)*usSize);
            lpNewKey->lpsKeyName[usSize - 1] = 0x00;
            lppNewData[i] = lpNewKey;
        }

        if (hRet == WFS_SUCCESS)
        {
            lpResult->lpBuffer = lppNewData;
            lppNewData = nullptr;
        }
#else
        auto lppKeyDetail = (LPWFSPINKEYDETAIL *)lpData;
        if (lppKeyDetail == nullptr)
            break;

        USHORT usCount = 0;
        while (lppKeyDetail[usCount] != nullptr) usCount++;
        if (usCount == 0)
            break;

        LPWFSPINKEYDETAIL *lppNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPINKEYDETAIL) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0, sizeof(LPWFSPINKEYDETAIL) * (usCount + 1));

        for (USHORT i = 0; i < usCount; i++)
        {
            LPWFSPINKEYDETAIL lpNewKey = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINKEYDETAIL), lpResult, (LPVOID *)&lpNewKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewKey);
            memcpy(lpNewKey, lppKeyDetail[i], sizeof(WFSPINKEYDETAIL));

            UINT usSize = strlen(lppKeyDetail[i]->lpsKeyName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewKey->lpsKeyName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewKey->lpsKeyName);
            memcpy(lpNewKey->lpsKeyName, lppKeyDetail[i]->lpsKeyName, sizeof(char)*usSize);
            lpNewKey->lpsKeyName[usSize - 1] = 0x00;
            lppNewData[i] = lpNewKey;
        }

        if (hRet == WFS_SUCCESS)
        {
            lpResult->lpBuffer = lppNewData;
            lppNewData = nullptr;
        }
#endif
    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINFUNCKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpFuncKeyDetail = (LPWFSPINFUNCKEYDETAIL)lpData;
        if (lpFuncKeyDetail == nullptr)
            break;

        LPWFSPINFUNCKEYDETAIL lppNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINFUNCKEYDETAIL), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memcpy(lppNewData, lpFuncKeyDetail, sizeof(WFSPINFUNCKEYDETAIL));

        if ((lpFuncKeyDetail->lppFDKs != nullptr) && (lpFuncKeyDetail->usNumberFDKs > 0))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPINFDK) * lpFuncKeyDetail->usNumberFDKs, lpResult, (LPVOID *)&lppNewData->lppFDKs);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lppNewData->lppFDKs);
            for (USHORT i = 0; i < lpFuncKeyDetail->usNumberFDKs; i++)
            {
                LPWFSPINFDK lpNewFDK = nullptr;
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINFDK), lpResult, (LPVOID *)&lpNewFDK);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewFDK);
                memcpy(lpNewFDK, lpFuncKeyDetail->lppFDKs[i], sizeof(WFSPINFDK));
                lppNewData->lppFDKs[i] = lpNewFDK;
            }
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINKEYDETAILEX(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lppKeyDetailEx = (LPWFSPINKEYDETAILEX *)lpData;
        if (lppKeyDetailEx == nullptr)
            break;

        USHORT usCount = 0;
        while (lppKeyDetailEx[usCount] != nullptr) usCount++;
        if (usCount == 0)
            break;

        LPWFSPINKEYDETAILEX *lppNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPINKEYDETAILEX) * (usCount + 1), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memset(lppNewData, 0, sizeof(LPWFSPINKEYDETAILEX) * (usCount + 1));

        for (USHORT i = 0; i < usCount; i++)
        {
            LPWFSPINKEYDETAILEX lpNewKey = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINKEYDETAILEX), lpResult, (LPVOID *)&lpNewKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewKey);
            memcpy(lpNewKey, lppKeyDetailEx[i], sizeof(WFSPINKEYDETAILEX));

            UINT usSize = strlen(lppKeyDetailEx[i]->lpsKeyName) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewKey->lpsKeyName);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewKey->lpsKeyName);
            memcpy(lpNewKey->lpsKeyName, lppKeyDetailEx[i]->lpsKeyName, sizeof(char)*usSize);
            lpNewKey->lpsKeyName[usSize - 1] = 0x00;
            lppNewData[i] = lpNewKey;
        }

        if (hRet == WFS_SUCCESS)
        {
            lpResult->lpBuffer = lppNewData;
            lppNewData = nullptr;
        }

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINSECUREKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {

        auto lpSecurekeyDetail = (LPWFSPINSECUREKEYDETAIL)lpData;
        if (lpSecurekeyDetail == nullptr)
            break;

        LPWFSPINSECUREKEYDETAIL lppNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINSECUREKEYDETAIL), lpResult, (LPVOID *)&lppNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lppNewData);
        memcpy(lppNewData, lpSecurekeyDetail, sizeof(WFSPINSECUREKEYDETAIL));

        if (lpSecurekeyDetail->lpFuncKeyDetail != nullptr)
        {
            lppNewData->lpFuncKeyDetail = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINFUNCKEYDETAIL), lpResult, (LPVOID *)&lppNewData->lpFuncKeyDetail);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lppNewData->lpFuncKeyDetail);
            memcpy(lppNewData->lpFuncKeyDetail, lpSecurekeyDetail->lpFuncKeyDetail, sizeof(WFSPINFUNCKEYDETAIL));

            if ((lpSecurekeyDetail->lpFuncKeyDetail->lppFDKs != nullptr) &&
                (lpSecurekeyDetail->lpFuncKeyDetail->usNumberFDKs > 0))
            {
                for (int i = 0; i < lpSecurekeyDetail->lpFuncKeyDetail->usNumberFDKs; i++)
                {
                    LPWFSPINFDK lpNewFDK = nullptr;
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINFDK), lpResult, (LPVOID *)&lpNewFDK);
                    if (hRet != WFS_SUCCESS)
                        break;

                    _auto.push_back(lpNewFDK);
                    memcpy(lpNewFDK, lpSecurekeyDetail->lpFuncKeyDetail->lppFDKs[i], sizeof(WFSPINFDK));
                    lppNewData->lpFuncKeyDetail->lppFDKs[i] = lpNewFDK;
                }
            }
        }

        if (lpSecurekeyDetail->lppHexKeys != nullptr)
        {
            USHORT usCount = 0;
            while (lpSecurekeyDetail->lppHexKeys[usCount] != nullptr) usCount++;
            if (usCount == 0)
                break;

            LPWFSPINHEXKEYS *lppNewHEXKEYS = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPINHEXKEYS) * (usCount + 1), lpResult, (LPVOID *)&lppNewHEXKEYS);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lppNewHEXKEYS);
            memcpy(lppNewHEXKEYS, lpSecurekeyDetail->lppHexKeys, sizeof(LPWFSPINHEXKEYS) * (usCount + 1));
            for (int i = 0; i < usCount; i++)
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINHEXKEYS), lpResult, (LPVOID *)&lppNewHEXKEYS[i]);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lppNewHEXKEYS[i]);
                memcpy(lppNewHEXKEYS[i], lpSecurekeyDetail->lppHexKeys[i], sizeof(WFSPINHEXKEYS));
            }
            lppNewData->lppHexKeys = lppNewHEXKEYS;
            lppNewHEXKEYS = nullptr;
        }

        lpResult->lpBuffer = lppNewData;
        lppNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINENTRY(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    do
    {
        auto lpEntry = (LPWFSPINENTRY)lpData;
        if (lpEntry == nullptr)
            break;

        LPWFSPINENTRY lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINENTRY), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        memcpy(lpNewData, lpEntry, sizeof(WFSPINENTRY));
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINLOCALDESRESULT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpbResult = (LPBOOL)lpData;
        if (lpbResult == nullptr)
            break;

        LPBOOL lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(BOOL), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpbResult, sizeof(BOOL));

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINCREATEOFFSETRESULT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpsOffset = (LPSTR)lpData;
        if (lpsOffset == nullptr)
            break;

        UINT usSize = strlen(lpsOffset) + 1;
        LPSTR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpsOffset, sizeof(char)*usSize);

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;
    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINLOCALEUROCHEQUERESULT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpbResult = (LPBOOL)lpData;
        if (lpbResult == nullptr)
            break;

        LPBOOL lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(BOOL), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpbResult, sizeof(BOOL));

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINPRESENTRESULT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {

        auto lpPresentResult = (LPWFSPINPRESENTRESULT)lpData;
        if (lpPresentResult == nullptr)
            break;

        LPWFSPINPRESENTRESULT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINPRESENTRESULT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPresentResult, sizeof(WFSPINPRESENTRESULT));
        if ((lpPresentResult->lpbChipData != nullptr) && (lpPresentResult->ulChipDataLength))
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpPresentResult->ulChipDataLength, lpResult, (LPVOID *)&lpNewData->lpbChipData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbChipData);
            memcpy(lpNewData->lpbChipData, lpPresentResult->lpbChipData, sizeof(BYTE)*lpPresentResult->ulChipDataLength);
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINDATA(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpPinData = (LPWFSPINDATA)lpData;
        if (lpPinData == nullptr)
            break;

        LPWFSPINDATA lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINDATA), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpPinData, sizeof(WFSPINDATA));

        if (lpPinData->usKeys > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(LPWFSPINKEY) * lpPinData->usKeys, lpResult, (LPVOID *)&lpNewData->lpPinKeys);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpPinKeys);
            for (USHORT i = 0; i < lpPinData->usKeys; i++)
            {
                LPWFSPINKEY lpNewKey = nullptr;
                if (lpPinData->lpPinKeys[i] != nullptr)
                {
                    hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINKEY), lpResult, (LPVOID *)&lpNewKey);
                    if (hRet != WFS_SUCCESS)
                        break;

                    _auto.push_back(lpNewKey);
                    memcpy(lpNewKey, lpPinData->lpPinKeys[i], sizeof(WFSPINKEY));
                }
                lpNewData->lpPinKeys[i] = lpNewKey;
                lpNewKey = nullptr;
            }
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINLOCALBANKSYSRESULT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpbResult = (LPBOOL)lpData;
        if (lpbResult == nullptr)
            break;

        LPBOOL lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(BOOL), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpbResult, sizeof(BOOL));

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINBANKSYSIO(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpBankSYSIO = (LPWFSPINBANKSYSIO)lpData;
        if (lpBankSYSIO == nullptr)
            break;

        LPWFSPINBANKSYSIO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINBANKSYSIO), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpBankSYSIO, sizeof(WFSPINBANKSYSIO));
        if (lpBankSYSIO->ulLength > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpBankSYSIO->ulLength, lpResult, (LPVOID *)&lpNewData->lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbData);
            memcpy(lpNewData->lpbData, lpBankSYSIO->lpbData, sizeof(BYTE)*lpBankSYSIO->ulLength);
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINSECMSG(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpSecMSG = (LPWFSPINSECMSG)lpData;
        if (lpSecMSG == nullptr)
            break;

        LPWFSPINSECMSG lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINSECMSG), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpSecMSG, sizeof(WFSPINSECMSG));
        if (lpSecMSG->ulLength > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpSecMSG->ulLength, lpResult, (LPVOID *)&lpNewData->lpbMsg);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpbMsg);
            memcpy(lpNewData->lpbMsg, lpSecMSG->lpbMsg, sizeof(BYTE)*lpSecMSG->ulLength);
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINENCIO(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpENCIO = (LPWFSPINENCIO)lpData;
        if (lpENCIO == nullptr)
            break;

        LPWFSPINENCIO lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINENCIO), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpENCIO, sizeof(WFSPINENCIO));
        lpNewData->lpvData = nullptr;								//30-00-00-00(FS#0003)
        if (lpENCIO->ulDataLength > 0)
        {
            hRet = m_pIWFM->WFMAllocateMore(lpENCIO->ulDataLength, lpResult, (LPVOID *)&lpNewData->lpvData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpvData);
            memcpy(lpNewData->lpvData, lpENCIO->lpvData, lpENCIO->ulDataLength);
        }

		//30-00-00-00(FS#0003) add start
        //判断是什么命令
        WORD wCommandOut = 0;
        if(lpNewData->lpvData != NULL){
            LPPROTCHNGENERATESM2KEYPAIROUT lpCommandIn = (LPPROTCHNGENERATESM2KEYPAIROUT)lpNewData->lpvData;
            wCommandOut =  lpCommandIn->wCommand;
        }else{
           return  WFS_ERR_INVALID_DATA;
        }
        DWORD dwSize = 0;
        if(wCommandOut == WFS_CMD_ENC_IO_CHN_GENERATE_SM2_KEY_PAIR){

               LPPROTCHNGENERATESM2KEYPAIROUT lpPairKeyOut= (LPPROTCHNGENERATESM2KEYPAIROUT)lpENCIO->lpvData;
               memcpy(lpNewData->lpvData, lpENCIO->lpvData, sizeof(PROTCHNGENERATESM2KEYPAIROUT));

        }
        if(wCommandOut == WFS_CMD_ENC_IO_CHN_EXPORT_SM2_EPP_SIGNED_ITEM){

       LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT lpPairKeyOut= (LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpENCIO->lpvData;
   //    if((lpPairKeyOut->lpxKeyCheckValue != NULL) && (lpPairKeyOut->lpxKeyCheckValue->usLength != 0)){
       LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT out = (LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpNewData->lpvData;
       out->lpxSignature = nullptr;
       out->lpxSelfSignature = nullptr;
           hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpNewData->lpvData)->lpxValue);
           if (hRet != WFS_SUCCESS)
               break;

           _auto.push_back(((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpNewData->lpvData)->lpxValue);
           memcpy(((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpNewData->lpvData)->lpxValue,
                  ((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpENCIO->lpvData)->lpxValue, sizeof(WFSXDATA));

           out->lpxValue->lpbData = nullptr;
           hRet = m_pIWFM->WFMAllocateMore(((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpENCIO->lpvData)->lpxValue->usLength, lpResult,
                 (LPVOID *)&((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpNewData->lpvData)->lpxValue->lpbData);
           if (hRet != WFS_SUCCESS)
               break;

          //if((((LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT)lpNewData->lpvData)->lpxKeyCheckValue->lpbData != nullptr) ||
         //      (((LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT)lpNewData->lpvData)->lpxKeyCheckValue->usLength != 0)){
             //  hRet = m_pIWFM->WFMAllocateMore(((LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT)lpENCIO->lpvData)->lpxKeyCheckValue->usLength, lpResult,
             //        (LPVOID *)&((LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT)lpNewData->lpvData)->lpxKeyCheckValue->lpbData);
             //  if (hRet != WFS_SUCCESS)
             //      break;

               _auto.push_back(((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpNewData->lpvData)->lpxValue->lpbData);
               memcpy(((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpNewData->lpvData)->lpxValue->lpbData,
                      ((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpENCIO->lpvData)->lpxValue->lpbData,
                      (((LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT)lpENCIO->lpvData)->lpxValue->usLength));
      //     }
   //      }
        }

        if(wCommandOut == WFS_CMD_ENC_IO_CHN_IMPORT_SM2_SIGNED_SM4_KEY){
            LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT lpOut = (LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT)lpNewData->lpvData;
            LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT lpIn = (LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT)lpENCIO->lpvData;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpOut->lpxKeyCheckValue);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpOut->lpxKeyCheckValue);
            memcpy(lpOut->lpxKeyCheckValue,lpIn->lpxKeyCheckValue, sizeof(WFSXDATA));

            hRet = m_pIWFM->WFMAllocateMore(lpIn->lpxKeyCheckValue->usLength, lpResult, (LPVOID *)&lpOut->lpxKeyCheckValue->lpbData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpOut->lpxKeyCheckValue->lpbData);
            memcpy(lpOut->lpxKeyCheckValue->lpbData,lpIn->lpxKeyCheckValue->lpbData, lpIn->lpxKeyCheckValue->usLength);

        }
		//30-00-00-00(FS#0003) add end
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINSECUREKEYENTRYOUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpSecurekeyEntryout = (LPWFSPINSECUREKEYENTRYOUT)lpData;
        if (lpSecurekeyEntryout == nullptr)
            break;

        LPWFSPINSECUREKEYENTRYOUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINSECUREKEYENTRYOUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpSecurekeyEntryout, sizeof(WFSPINSECUREKEYENTRYOUT));
        if (lpSecurekeyEntryout->lpxKCV != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpSecurekeyEntryout->lpxKCV, sizeof(WFSXDATA));

            if ((lpSecurekeyEntryout->lpxKCV->lpbData != nullptr) && (lpSecurekeyEntryout->lpxKCV->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpSecurekeyEntryout->lpxKCV->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpSecurekeyEntryout->lpxKCV->lpbData, sizeof(BYTE)*lpSecurekeyEntryout->lpxKCV->usLength);
            }
            lpNewData->lpxKCV = lpNewXData;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINKCV(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpKCV = (LPWFSPINKCV)lpData;
        if (lpKCV == nullptr)
            break;

        LPWFSPINKCV lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINKCV), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpKCV, sizeof(WFSPINKCV));
        if (lpKCV->lpxKCV != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpKCV->lpxKCV, sizeof(WFSXDATA));

            if ((lpKCV->lpxKCV->lpbData != nullptr) && (lpKCV->lpxKCV->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpKCV->lpxKCV->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpKCV->lpxKCV->lpbData, sizeof(BYTE)*lpKCV->lpxKCV->usLength);
            }
            lpNewData->lpxKCV = lpNewXData;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINSTARTKEYEXCHANGE(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpStartkeyExg = (LPWFSPINSTARTKEYEXCHANGE)lpData;
        if (lpStartkeyExg == nullptr)
            break;

        LPWFSPINSTARTKEYEXCHANGE lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINSTARTKEYEXCHANGE), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
//30-00-00-00(FS#0006)        memcpy(lpNewData, lpStartkeyExg, sizeof(WFSPINSTARTKEYEXCHANGE));
        memset(lpNewData, 0, sizeof(WFSPINSTARTKEYEXCHANGE));       //30-00-00-00(FS#0006)
        if (lpStartkeyExg->lpxRandomItem != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpStartkeyExg->lpxRandomItem, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;                  //30-00-00-00(FS#0006)

            if ((lpStartkeyExg->lpxRandomItem->lpbData != nullptr) && (lpStartkeyExg->lpxRandomItem->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpStartkeyExg->lpxRandomItem->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpStartkeyExg->lpxRandomItem->lpbData, sizeof(BYTE)*lpStartkeyExg->lpxRandomItem->usLength);
            }
            lpNewData->lpxRandomItem = lpNewXData;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINIMPORTRSAPUBLICKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINIMPORTRSAPUBLICKEYOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINIMPORTRSAPUBLICKEYOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINIMPORTRSAPUBLICKEYOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINIMPORTRSAPUBLICKEYOUTPUT));
        lpNewData->lpxKeyCheckValue = nullptr;                             //30-00-00-00(FS#0005)
        if (lpOut->lpxKeyCheckValue != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOut->lpxKeyCheckValue, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;                                  //30-00-00-00(FS#0005)

            if ((lpOut->lpxKeyCheckValue->lpbData != nullptr) && (lpOut->lpxKeyCheckValue->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOut->lpxKeyCheckValue->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOut->lpxKeyCheckValue->lpbData, sizeof(BYTE)*lpOut->lpxKeyCheckValue->usLength);
            }
            lpNewData->lpxKeyCheckValue = lpNewXData;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT));
        lpNewData->lpxValue = nullptr;                              //30-00-00-00(FS#0005)
        if (lpOut->lpxValue != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOut->lpxValue, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;                       //30-00-00-00(FS#0005)

            if ((lpOut->lpxValue->lpbData != nullptr) && (lpOut->lpxValue->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOut->lpxValue->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOut->lpxValue->lpbData, sizeof(BYTE)*lpOut->lpxValue->usLength);
            }
            lpNewData->lpxValue = lpNewXData;
        }

        lpNewData->lpxSignature = nullptr;              //30-00-00-00(FS#0005)
        if (lpOut->lpxSignature != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOut->lpxSignature, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;              //30-00-00-00(FS#0005)

            if ((lpOut->lpxSignature->lpbData != nullptr) && (lpOut->lpxSignature->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOut->lpxSignature->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOut->lpxSignature->lpbData, sizeof(BYTE)*lpOut->lpxSignature->usLength);
            }
            lpNewData->lpxSignature = lpNewXData;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINIMPORTRSASIGNEDDESKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINIMPORTRSASIGNEDDESKEYOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINIMPORTRSASIGNEDDESKEYOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINIMPORTRSASIGNEDDESKEYOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINIMPORTRSASIGNEDDESKEYOUTPUT));
        lpNewData->lpxKeyCheckValue = nullptr;          //30-00-00-00(FS#0005)
        if (lpOut->lpxKeyCheckValue != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOut->lpxKeyCheckValue, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;             //30-00-00-00(FS#0005)

            if ((lpOut->lpxKeyCheckValue->lpbData != nullptr) && (lpOut->lpxKeyCheckValue->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOut->lpxKeyCheckValue->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOut->lpxKeyCheckValue->lpbData, sizeof(BYTE)*lpOut->lpxKeyCheckValue->usLength);
            }
            lpNewData->lpxKeyCheckValue = lpNewXData;
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINGENERATERSAKEYPAIR(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINGENERATERSAKEYPAIR)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINGENERATERSAKEYPAIR lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINGENERATERSAKEYPAIR), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINGENERATERSAKEYPAIR));
        if (lpOut->lpsKey != nullptr)
        {
            UINT uSize = strlen(lpOut->lpsKey) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uSize, lpResult, (LPVOID *)&lpNewData->lpsKey);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsKey);
            memcpy(lpNewData->lpsKey, lpOut->lpsKey, sizeof(char)*uSize);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    HRESULT hRet = WFS_SUCCESS;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memset(lpNewData, 0, sizeof(WFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT));           //30-00-00-00(FS#0006)
        if (lpOut->lpxValue != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxValue;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;                                          //30-00-00-00(FS#0006)

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxValue = lpNewXData;
        }

        if (lpOut->lpxSelfSignature != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxSelfSignature;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;                                          //30-00-00-00(FS#0006)

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxSelfSignature = lpNewXData;
        }

        if (lpOut->lpxSignature != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxSignature;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));
            lpNewXData->lpbData = nullptr;                                          //30-00-00-00(FS#0006)

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxSignature = lpNewXData;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);
    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINLOADCERTIFICATEOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINLOADCERTIFICATEOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINLOADCERTIFICATEOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINLOADCERTIFICATEOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINLOADCERTIFICATEOUTPUT));
        if (lpOut->lpxCertificateData != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxCertificateData;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxCertificateData = lpNewXData;
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);
    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINKEY(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    do
    {
        auto lpEntry = (LPWFSPINKEY)lpData;
        if (lpEntry == nullptr)
            break;

        LPWFSPINKEY lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINKEY), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        memcpy(lpNewData, lpEntry, sizeof(WFSPINKEY));
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPININIT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpInit = (LPWFSPININIT)lpData;
        if (lpInit == nullptr)
            break;

        LPWFSPININIT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPININIT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memset(lpNewData, 0x00, sizeof(WFSPININIT));
        if (lpInit->lpxIdent != nullptr)
        {
            hRet = Fmt_WFSXDATA(_auto, lpInit->lpxIdent, lpNewData->lpxIdent, lpResult);
            if (hRet != WFS_SUCCESS)
                break;
        }
        if (lpInit->lpxKey != nullptr)
        {
            hRet = Fmt_WFSXDATA(_auto, lpInit->lpxKey, lpNewData->lpxKey, lpResult);
            if (hRet != WFS_SUCCESS)
                break;
        }
        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);

    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINGETCERTIFICATEOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINGETCERTIFICATEOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINGETCERTIFICATEOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINGETCERTIFICATEOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINGETCERTIFICATEOUTPUT));
        if (lpOut->lpxCertificate != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxCertificate;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxCertificate = lpNewXData;
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);
    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINREPLACECERTIFICATEOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINREPLACECERTIFICATEOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINREPLACECERTIFICATEOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINREPLACECERTIFICATEOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINREPLACECERTIFICATEOUTPUT));
        if (lpOut->lpxNewCertificateData != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxNewCertificateData;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxNewCertificateData = lpNewXData;
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);
    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT));
        if (lpOut->lpxRSAData != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxRSAData;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxRSAData = lpNewXData;
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);
    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINEMVIMPORTPUBLICKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINEMVIMPORTPUBLICKEYOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINEMVIMPORTPUBLICKEYOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINEMVIMPORTPUBLICKEYOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINEMVIMPORTPUBLICKEYOUTPUT));
        if (lpOut->lpsExpiryDate != nullptr)
        {
            UINT usSize = strlen(lpOut->lpsExpiryDate) + 1;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * usSize, lpResult, (LPVOID *)&lpNewData->lpsExpiryDate);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewData->lpsExpiryDate);
            memcpy(lpNewData->lpsExpiryDate, lpOut->lpsExpiryDate, sizeof(char)*usSize);
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);
    return hRet;
}

HRESULT CSPBasePIN::Fmt_WFSPINDIGESTOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);

    do
    {
        auto lpOut = (LPWFSPINDIGESTOUTPUT)lpData;
        if (lpOut == nullptr)
            break;

        LPWFSPINDIGESTOUTPUT lpNewData = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSPINDIGESTOUTPUT), lpResult, (LPVOID *)&lpNewData);
        if (hRet != WFS_SUCCESS)
            break;

        _auto.push_back(lpNewData);
        memcpy(lpNewData, lpOut, sizeof(WFSPINDIGESTOUTPUT));
        if (lpOut->lpxDigestOutput != nullptr)
        {
            LPWFSXDATA lpNewXData = nullptr;
            LPWFSXDATA lpOutXData = lpOut->lpxDigestOutput;
            hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSXDATA), lpResult, (LPVOID *)&lpNewXData);
            if (hRet != WFS_SUCCESS)
                break;

            _auto.push_back(lpNewXData);
            memcpy(lpNewXData, lpOutXData, sizeof(WFSXDATA));

            if ((lpOutXData->lpbData != nullptr) && (lpOutXData->usLength > 0))
            {
                hRet = m_pIWFM->WFMAllocateMore(sizeof(BYTE) * lpOutXData->usLength, lpResult, (LPVOID *)&lpNewXData->lpbData);
                if (hRet != WFS_SUCCESS)
                    break;

                _auto.push_back(lpNewXData->lpbData);
                memcpy(lpNewXData->lpbData, lpOutXData->lpbData, sizeof(BYTE)*lpOutXData->usLength);
            }
            lpNewData->lpxDigestOutput = lpNewXData;
        }

        lpResult->lpBuffer = lpNewData;
        lpNewData = nullptr;

    } while (false);
    return hRet;
}

//30-00-00-00(FT#0070)
HRESULT CSPBasePIN::OnClearCancelSemphoreCount()
{
    return m_pCmdFunc->OnClearCancelSemphoreCount();
}
