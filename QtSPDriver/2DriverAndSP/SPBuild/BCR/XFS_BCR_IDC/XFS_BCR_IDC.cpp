#include "XFS_BCR_IDC.h"

static const char *DEVTYPE = "BCR";
static const char *ThisFile = "XFS_BCR_IDC.cpp";
//////////////////////////////////////////////////////////////////////////
CXFS_BCR_IDC::CXFS_BCR_IDC() : m_bNeedRepair(FALSE),
                         m_pMutexGetStatus(nullptr)
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    m_nResetFailedTimes = 0;
    m_bJamm = FALSE;
    m_bChipPowerOff = FALSE;
}

CXFS_BCR_IDC::~CXFS_BCR_IDC()
{

}


long CXFS_BCR_IDC::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseIDC
    if (0 != m_pBase.Load("SPBaseIDC.dll", "CreateISPBaseIDC", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseIDC失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

void CXFS_BCR_IDC::ControlLED(BYTE byLedCtrl)
{
    return ;
}

long CXFS_BCR_IDC::AcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_bChipPowerOff = FALSE;
    //读卡时不选任何参数值返回DATAINVALID
    if ((dwReadOption & WFS_IDC_TRACK_WM) == 0)
    {
        Log(ThisModule, -1, "接收卡参数无效");
        return WFS_ERR_INVALID_DATA;
    }    

    if (m_Status.fwDevice == WFS_IDC_DEVHWERROR ||
        m_Status.fwDevice == WFS_IDC_DEVOFFLINE ||
        m_Status.fwDevice == WFS_IDC_DEVPOWEROFF)
        return WFS_ERR_HARDWARE_ERROR;

    int nRet = UpdateCardStatus();
    if (nRet < 0)
        return WFS_ERR_HARDWARE_ERROR;

    char szData[8192] = { 0 };
    DWORD dwLen = sizeof(szData);
    DWORD dwType = 0;
    // 开始等待扫描二维码
    long lRet = m_pDev->ReadBCR(dwType, szData, dwLen, dwTimeOut);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "扫描码失败：%d", lRet);
        if (lRet == -4)
            return WFS_ERR_CANCELED;
        else if (lRet = -9)
            return WFS_ERR_TIMEOUT;
        return WFS_ERR_HARDWARE_ERROR;
    }

    FireCardInserted();
    if(dwLen == 0){
        SetTrackInfo(WFS_IDC_TRACK_WM, WFS_IDC_DATAMISSING, 0, NULL);
        FireCardInvalidMedia();
        return WFS_ERR_IDC_INVALIDMEDIA;
    } else {
        SetTrackInfo(WFS_IDC_TRACK_WM, WFS_IDC_DATAOK, dwLen, (LPBYTE)szData);
    }

    return WFS_SUCCESS;
}

long CXFS_BCR_IDC::Convert2XFSErrCode(long lIDCErrCode)
{
    switch (lIDCErrCode)
    {
    case 0: return WFS_SUCCESS;
    default:          return WFS_ERR_HARDWARE_ERROR;
    }
}

const char *CXFS_BCR_IDC::ProcessReturnCode(int nCode)
{
    switch (nCode)
    {
    default:                    return "未定义错误码";
    }
}

long CXFS_BCR_IDC::WaitItemTaken()
{
    UpdateCardStatus();
    return 0;
}

long CXFS_BCR_IDC::UpdateCardStatus()
{
    THISMODULE(__FUNCTION__);

    DEVBCRSTATUS stStatus;
    int iRetStatus = m_pDev->GetStatus(stStatus);
    UpdateDevStatus(iRetStatus);

    return iRetStatus;
}

void CXFS_BCR_IDC::UpdateDevStatus(int iRet)
{
    THISMODULE(__FUNCTION__);

    WORD fwDevice = WFS_IDC_DEVHWERROR;
    DWORD dwHWAct = WFS_ERR_ACT_NOACTION;

    fwDevice = (iRet == 0) ? WFS_IDC_DEVONLINE : WFS_IDC_DEVOFFLINE;

//    switch (iRet)
//    {
//    case ERR_IDC_SUCCESS:
//        fwDevice = WFS_IDC_DEVONLINE;
//        break;
//    case ERR_IDC_PARAM_ERR:
//    case ERR_IDC_READERROR:
//        fwDevice = WFS_IDC_DEVHWERROR;
//        dwHWAct = WFS_ERR_ACT_NOACTION;
//        break;
//    case ERR_IDC_COMM_ERR:
//    case ERR_IDC_OFFLINE:
//    case ERR_IDC_NO_DEVICE:
//    case ERR_IDC_READTIMEOUT:
//        fwDevice = WFS_IDC_DEVOFFLINE;
//        dwHWAct = WFS_ERR_ACT_NOACTION;
//        break;
//    case ERR_IDC_STATUS_ERR:
//        fwDevice = WFS_IDC_DEVHWERROR;
//        dwHWAct = WFS_ERR_ACT_RESET;
//        break;
//    case ERR_IDC_JAMMED:
//        fwDevice = WFS_IDC_DEVHWERROR;
//        dwHWAct = WFS_ERR_ACT_RESET;
//        m_bJamm = TRUE;
//        break;
//    case ERR_IDC_NOT_OPEN:
//    case ERR_IDC_HWERR:
//    case ERR_IDC_OTHER:
//        fwDevice = WFS_IDC_DEVHWERROR;
//        dwHWAct = WFS_ERR_ACT_HWCLEAR;
//        break;
//    case ERR_IDC_RETAINBINFULL:
//        fwDevice = WFS_IDC_DEVONLINE;
//        break;
//    case ERR_IDC_USERERR:
//        fwDevice = WFS_IDC_DEVUSERERROR;
//        break;
//    default:
//        fwDevice = WFS_IDC_DEVHWERROR;
//        dwHWAct = WFS_ERR_ACT_RESET;
//        break;
//    }

    if (iRet < 0)
    {
        m_bNeedRepair = TRUE;
    }

    if (m_Status.fwDevice != fwDevice)
    {
        FireStatusChanged(fwDevice);
        if (fwDevice != WFS_IDC_DEVONLINE && fwDevice != WFS_IDC_DEVBUSY)
        {
            FireHWEvent(dwHWAct, nullptr);
            m_bNeedRepair = TRUE;
        }
    }
    m_Status.fwDevice = fwDevice;
}

void CXFS_BCR_IDC::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Status.fwChipPower  = WFS_IDC_CHIPNOTSUPP;
    m_Status.fwMedia = WFS_IDC_MEDIANOTSUPP;
    m_Status.fwSecurity = WFS_IDC_SECNOTSUPP;
    m_Status.usCards = 0;
    m_Status.lpszExtra = NULL;
    m_Status.fwRetainBin = WFS_IDC_RETAINNOTSUPP;

    return;
}

void CXFS_BCR_IDC::InitCaps()
{
    m_Caps.wClass = WFS_SERVICE_CLASS_IDC;
    m_Caps.fwType = WFS_IDC_TYPECONTACTLESS;
    m_Caps.bCompound = FALSE;
    m_Caps.fwReadTracks = WFS_IDC_NOTSUPP;
    m_Caps.fwWriteTracks = WFS_IDC_NOTSUPP;
    // 新增IC卡操作
    m_Caps.fwChipProtocols = WFS_IDC_CHIPT0 | WFS_IDC_CHIPT1;
    m_Caps.fwChipPower = WFS_IDC_CHIPPOWERCOLD | WFS_IDC_CHIPPOWERWARM | WFS_IDC_CHIPPOWEROFF;
    m_Caps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_Caps.fwPowerOnOption = WFS_IDC_NOACTION;
    m_Caps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_Caps.fwWriteMode = WFS_IDC_NOTSUPP;
    m_Caps.usCards                        = 0;
    m_Caps.bFluxSensorProgrammable        = FALSE;
    m_Caps.bReadWriteAccessFollowingEject = FALSE;
    m_Caps.lpszExtra = NULL;

    m_cCapExtra.AddExtra("VRTCount", "2");
    m_cCapExtra.AddExtra("VRTDetail[00]", "0000000000000000");             // SP版本程序名称8位+版本8位
    m_cCapExtra.AddExtra("VRTDetail[01]", "");
}

void CXFS_BCR_IDC::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
{
    WFSIDCCARDDATA data;
    data.wDataSource    = wSource;
    data.wStatus        = wStatus;
    data.ulDataLength   = uLen;
    data.lpbData        = pData;
    data.fwWriteMethod  = 0;

    m_CardDatas.SetAt(wSource, data);
    return;
}

bool CXFS_BCR_IDC::GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho)
{
    LPWFSIDCCARDDATA pCardData = m_CardDatas.GetAt(wSource);
    if (!pCardData)
    {
        *pLen = 0;
        return FALSE;
    }

    if (*pLen < pCardData->ulDataLength)
    {
        *pLen = pCardData->ulDataLength;
        return false;
    }

    *pLen           = pCardData->ulDataLength;
    *pWriteMetho    = pCardData->fwWriteMethod;
    memcpy(pData, pCardData->lpbData, *pLen);
    return true;
}

void CXFS_BCR_IDC::InitConfig()
{
  return;
}

// 基本接口
HRESULT CXFS_BCR_IDC::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    ReadConfig();
    InitCaps();
    InitStatus();

    m_strPort = m_cXfsReg.GetValue("CONFIG", "Port", "");
    std::string strDevDll = m_cXfsReg.GetValue("DriverDllName", "");
    if (strDevDll.empty() || m_strPort.empty())
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName或Port配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 加载DEV
    HRESULT hRet = m_pDev.Load(strDevDll.c_str(), "CreateIDevBCR", DEVTYPE);
    if (0 != hRet)
    {
        Log(ThisModule, __LINE__, "加载%s失败, hRet=%d", strDevDll.c_str(), hRet);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 打开连接
    hRet = m_pDev->Open(m_strPort.c_str());
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败！, Port=%s", m_strPort.c_str());
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 复位设备
    hRet = m_pDev->Reset();
    if (hRet != 0)
    {
        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 更新扩展状态
    char szDevVer[256] = { 0 };
    m_pDev->GetDevInfo(szDevVer);
    m_cCapExtra.AddExtra("VRTDetail[01]", szDevVer);
    Log(ThisModule, 0, "设备固件版本信息：%s", szDevVer);

    // 更新一次状态
    OnStatus();

    return WFS_SUCCESS;
}

HRESULT CXFS_BCR_IDC::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
    {
        m_pDev->CancelRead();// 退出前，取消一次
        m_pDev->Close();
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR_IDC::OnStatus()
{
    UpdateCardStatus();
    if(m_Status.fwDevice == WFS_IDC_DEVOFFLINE){
        //断线自动重连
        m_pDev->Close();
        m_pDev->Open(m_strPort.c_str());
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR_IDC::OnWaitTaken()
{
    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    WaitItemTaken();
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR_IDC::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
        m_pDev->CancelRead();
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR_IDC::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// 查询命令
HRESULT CXFS_BCR_IDC::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Status.lpszExtra = (LPSTR)m_cStaExtra.GetExtra();
    lpStatus = &m_Status;
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR_IDC::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_Caps.lpszExtra = (LPSTR)m_cCapExtra.GetExtra();
    lpCaps = &m_Caps;
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR_IDC::GetFormList(LPSTR &lpFormList)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
//    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
//    {
//        hRes = WFS_SUCCESS;
//    }
//    lpFormList = (LPSTR)(LPCSTR)m_FormNames;
//    //LPSTR szTest = (LPSTR)lpFormList;
//    return hRes;
    return WFS_ERR_UNSUPP_CATEGORY;
}

HRESULT CXFS_BCR_IDC::GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

//    HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
//    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
//    {
//        if (lpFormName == nullptr)
//        {
//            hRes = WFS_ERR_INVALID_DATA;
//        }
//        else
//        {
//            SP_IDC_FORM *pForm = FLFind(m_FormList, lpFormName);
//            if (!pForm)
//                hRes = WFS_ERR_IDC_FORMNOTFOUND;
//            else if (!pForm->bLoadSucc)
//                hRes = WFS_ERR_IDC_FORMINVALID;
//            else
//            {
//                hRes = CheackFormInvalid(m_FormList, (char *)pForm->FormName.c_str(), pForm->fwAction);
//                if (hRes == WFS_SUCCESS)
//                {
//                    m_LastForm.lpszFormName             = (char *)pForm->FormName.c_str();
//                    m_LastForm.cFieldSeparatorTrack1    = pForm->cFieldSeparatorTracks[0];
//                    m_LastForm.cFieldSeparatorTrack2    = pForm->cFieldSeparatorTracks[1];
//                    m_LastForm.cFieldSeparatorTrack3    = pForm->cFieldSeparatorTracks[2];
//                    m_LastForm.fwAction                 = pForm->fwAction;
//                    m_LastForm.lpszTracks                   = (char *)pForm->sTracks.c_str();
//                    m_LastForm.bSecure                  = pForm->bSecures[0] | pForm->bSecures[1] | pForm->bSecures[2];
//                    m_LastForm.lpszTrack1Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[0];
//                    m_LastForm.lpszTrack2Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[1];
//                    m_LastForm.lpszTrack3Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[2];
//                    lpForm = &m_LastForm;
//                }
//            }
//        }
//    }
//    return hRes;
    return WFS_ERR_UNSUPP_CATEGORY;
}


HRESULT CXFS_BCR_IDC::WriteTrack(const LPWFSIDCWRITETRACK lpWriteData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR_IDC::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

long CXFS_BCR_IDC::RetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR_IDC::RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}


HRESULT CXFS_BCR_IDC::ResetCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR_IDC::SetKey(const LPWFSIDCSETKEY lpKeyData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR_IDC::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wOption = *(WORD *)lpReadData;
    m_CardDatas.Clear();
    long hRes = AcceptAndReadTrack(wOption, dwTimeOut);
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, -1, "AcceptAndReadTrack failed");
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_CardDatas;
    return hRes;
}

HRESULT CXFS_BCR_IDC::ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR_IDC::ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

#ifdef CARD_REJECT_GD_MODE
//读卡器新增扩展部分
HRESULT CXFS_BCR_IDC::ReduceCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::SetCount(LPWORD lpwCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::IntakeCardBack()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMEjectCard(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMSetCardData(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMRetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMReduceCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMSetCount(LPWORD lpwCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMEmptyCard(LPCSTR lpszCardBox)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_BCR_IDC::CMReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}
#endif

HRESULT CXFS_BCR_IDC::WriteRawData(const LPWFSIDCCARDDATA *lppCardData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR_IDC::ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();


    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR_IDC::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = m_pDev->Reset();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "复位失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
//    m_pDev->SetRFIDCardReaderLED(LEDTYPE_BLUE, LEDACTION_CLOSE);

//    WORD wAction = *lpResetIn;
//    // 没有动作参数时由SP决定
//    if (wAction == 0)
//    {
//        wAction = WFS_IDC_RETAIN;
//    }

//    if (wAction < WFS_IDC_NOACTION || wAction > WFS_IDC_RETAIN)
//    {
//        return WFS_ERR_INVALID_DATA;
//    }

//    CardAction ActFlag;
//    if (wAction == WFS_IDC_RETAIN)
//    {
//        ActFlag = CARDACTION_RETRACT;
//    }
//    else if (wAction == WFS_IDC_NOACTION)
//    {
//        ActFlag = CARDACTION_NOACTION;
//    }
//    else if (wAction == WFS_IDC_EJECT)
//    {
//        ActFlag = CARDACTION_EJECT;
//    }
//    else
//    {
//        return WFS_ERR_INVALID_DATA;
//    }

//    long nRet = m_pDev->Init(ActFlag, (WobbleAction)m_stConfig.usNeedWobble);
//    UpdateDevStatus(nRet);
//    if (nRet >= 0)
//    {
//        return WFS_SUCCESS;
//    }
//    else
//    {
//        return WFS_ERR_HARDWARE_ERROR;
//    }
}

HRESULT CXFS_BCR_IDC::ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();


    return WFS_ERR_UNSUPP_COMMAND;
}

void CXFS_BCR_IDC::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_BCR_IDC::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_BCR_IDC::FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

void CXFS_BCR_IDC::FireCardInvalidMedia()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDMEDIA, nullptr);
}

void CXFS_BCR_IDC::FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

void CXFS_BCR_IDC::FireMediaRetained()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIARETAINED, nullptr);
}

void CXFS_BCR_IDC::FireRetainBinThreshold(WORD wReBin)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_IDC_RETAINBINTHRESHOLD, (LPVOID)&wReBin);
}

void CXFS_BCR_IDC::FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

void CXFS_BCR_IDC::FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}

// 读取ini配置项目
bool CXFS_BCR_IDC::ReadConfig()
{
    THISMODULE(__FUNCTION__);

//    QString strINIFile("FIDCConfig.ini");
//    #ifdef QT_WIN32
//        strINIFile.prepend("C:/CFES/ETC/");
//    #else
//        strINIFile.prepend("/usr/local/CFES/ETC/");
//    #endif

//    CINIFileReader cINIFile;
//    // read FIDCConfig.ini
//    cINIFile.LoadINIFile(strINIFile.toLocal8Bit().data());

    return true;
}
