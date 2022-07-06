#include "XFS_FIDC.h"

static const char *DEVTYPE = "FIDC";
static const char *ThisFile = "XFS_FIDC.cpp";
//////////////////////////////////////////////////////////////////////////
#define TIMEID_UPDATE_STATUS                1789
#define RETAIN_CARD_COUNT                   "card_num"
#define RETAIN_CARD_MAXCOUNT                "bin_size"
#define FLUXABLE                            "flux_able"
#define WRACCFOLLEJECT                      "wr_acc_folleject"
//////////////////////////////////////////////////////////////////////////
CXFS_FIDC::CXFS_FIDC() : m_bNeedRepair(FALSE)
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    m_nResetFailedTimes = 0;
    m_bJamm = FALSE;
    m_bChipPowerOff = FALSE;
    m_bMultiCard = FALSE;
    m_ulTakeMonitorStartTime = 0;
}

CXFS_FIDC::~CXFS_FIDC()
{

}


long CXFS_FIDC::StartRun()
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

void CXFS_FIDC::ControlLED(BYTE byLedCtrl)
{
    if (!m_stConfig.bLedOper)
        return;
    if (byLedCtrl == 1) {
        m_pDev->SetRFIDCardReaderLED(LEDTYPE_BLUE, LEDACTION_OPEN);
    } else if(byLedCtrl == 0) {
        m_pDev->SetRFIDCardReaderLED(LEDTYPE_BLUE, LEDACTION_CLOSE);
    } else if(byLedCtrl == 2) {
        m_pDev->SetRFIDCardReaderLED(LEDTYPE_BLUE, LEDACTION_FLASH);
    }
}

long CXFS_FIDC::AcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_bChipPowerOff = FALSE;
    //读卡时不选任何参数值返回DATAINVALID
    if ((dwReadOption & 0x803F) == 0)
    {
        Log(ThisModule, -1, "接收卡参数无效");
        return WFS_ERR_INVALID_DATA;
    }

    if (dwReadOption & WFS_IDC_FLUXINACTIVE)
    {
        return WFS_ERR_INVALID_DATA;
    }

    if (m_Status.fwDevice == WFS_IDC_DEVHWERROR ||
        m_Status.fwDevice == WFS_IDC_DEVOFFLINE ||
        m_Status.fwDevice == WFS_IDC_DEVPOWEROFF)
        return WFS_ERR_HARDWARE_ERROR;

    int nRet = UpdateCardStatus();
    if (nRet < 0)
        return WFS_ERR_HARDWARE_ERROR;

    {
//        CAutoControlLed AutoCtrLed(this);
        //打开非接灯，闪烁
        ControlLED(2);
        BOOL bMagneticCard = FALSE;
        nRet = m_pDev->AcceptCard(dwTimeOut, bMagneticCard);
        UpdateDevStatus(nRet);
        if (nRet < 0)
        {
            //GetMediaPresentState(m_pDriver->DetectCard(IDCstatus));// 更新一次介质状态
            //UpdateCardStatus();

            Log(ThisModule, 1, "进卡失败，返回码nRet = %d, %s", nRet, ProcessReturnCode(nRet));
            ControlLED(0);
            return Convert2XFSErrCode(nRet);
        }
        else if (nRet == ERR_IDC_INSERT_TIMEOUT)
        {
            ControlLED(0);
            return WFS_ERR_TIMEOUT;
        }
        else if (nRet == ERR_IDC_USER_CANCEL)
        {
            ControlLED(0);
            return WFS_ERR_CANCELED;
        }
        else
        {
            m_pDev->SetRFIDCardReaderBeep(1);               //30-00-00-00(FT#0041)
            //非接灯变为常亮
            ControlLED(1);
            Log(ThisModule, 1, "进卡成功");
            m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
            FireCardInserted();
            m_WaitTaken = WTF_TAKEN;
        }
    }

    m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
//    m_Status.fwChipPower = WFS_IDC_CHIPONLINE;

    bool bChip = (dwReadOption & WFS_IDC_CHIP);
    if (!bChip)
    {
        return WFS_ERR_UNSUPP_DATA;
    }

    int nChipRet = ReadChip();
    if (nChipRet == WFS_ERR_IDC_INVALIDMEDIA)
    {
        FireCardInvalidMedia();
    }

    if (nChipRet != WFS_SUCCESS)
    {
        SetTrackInfo(WFS_IDC_CHIP, WFS_IDC_DATAINVALID, 0, NULL);
    }

    return nChipRet;
}

long CXFS_FIDC::Convert2XFSErrCode(long lIDCErrCode)
{
    switch (lIDCErrCode)
    {
    case ERR_IDC_SUCCESS:                 return WFS_SUCCESS;
    case ERR_IDC_INSERT_TIMEOUT:          return WFS_ERR_TIMEOUT;
    case ERR_IDC_USER_CANCEL:             return WFS_ERR_CANCELED;
    case ERR_IDC_COMM_ERR:                return WFS_ERR_CONNECTION_LOST;
    case ERR_IDC_JAMMED:                  return WFS_ERR_IDC_MEDIAJAM;
    case ERR_IDC_OFFLINE:                 return WFS_ERR_CONNECTION_LOST;
    case ERR_IDC_NOT_OPEN:                return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_RETAINBINFULL:           return WFS_ERR_IDC_RETAINBINFULL;
    case ERR_IDC_HWERR:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_STATUS_ERR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_UNSUP_CMD:               return WFS_ERR_UNSUPP_COMMAND;
    case ERR_IDC_PARAM_ERR:               return WFS_ERR_INVALID_DATA;
    case ERR_IDC_READTIMEOUT:             return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_WRITETIMEOUT:            return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_READERROR:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_WRITEERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_INVALIDCARD:             return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_IDC_NOTRACK:                 return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_IDC_CARDPULLOUT:             return WFS_ERR_USER_ERROR;
    case ERR_IDC_CARDTOOSHORT:            return WFS_ERR_IDC_CARDTOOSHORT;
    case ERR_IDC_CARDTOOLONG:             return WFS_ERR_IDC_CARDTOOLONG;
    case ERR_IDC_WRITETRACKERROR:         return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_PROTOCOLNOTSUPP:         return WFS_ERR_UNSUPP_DATA;
    case ERR_IDC_ICRW_ERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_NO_DEVICE:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_OTHER:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_USERERR:                 return WFS_ERR_USER_ERROR;
    case ERR_IDC_TAMPER:                  return WFS_ERR_USER_ERROR;
    case ERR_IDC_CONFLICT:                return WFS_ERR_IDC_INVALIDMEDIA;
    default:                              return WFS_ERR_HARDWARE_ERROR;
    }
}

const char *CXFS_FIDC::ProcessReturnCode(int nCode)
{
    switch (nCode)
    {
    case ERR_IDC_SUCCESS:       return "操作成功";
    case ERR_IDC_INSERT_TIMEOUT: return "进卡超时";
    case ERR_IDC_USER_CANCEL:   return "用户取消";
    case ERR_IDC_PARAM_ERR:     return "参数错误";
    case ERR_IDC_COMM_ERR:      return "通讯错误";
    case ERR_IDC_STATUS_ERR:    return "读卡器状态出错";
    case ERR_IDC_JAMMED:        return "读卡器堵卡";
    case ERR_IDC_OFFLINE:       return "读卡器脱机";
    case ERR_IDC_NOT_OPEN:      return "没有调用Open";
    case ERR_IDC_RETAINBINFULL: return "回收箱满";
    case ERR_IDC_HWERR:         return "硬件故障";
    case ERR_IDC_READTIMEOUT:   return "读数据超时";
    case ERR_IDC_READERROR:     return "读数据错误";
    case ERR_IDC_INVALIDCARD:   return "无效芯片，或磁道数据无效";
    case ERR_IDC_NOTRACK:       return "非磁卡，未检测到磁道";
    case ERR_IDC_CARDPULLOUT:   return "接收卡时，卡被拖出";
    case ERR_IDC_CARDTOOSHORT:  return "卡太短";
    case ERR_IDC_CARDTOOLONG:   return "卡太长";
    case ERR_IDC_PROTOCOLNOTSUPP: return "不支持的IC通讯协议";
    case ERR_IDC_NO_DEVICE:     return "指定名的设备不存在";
    case ERR_IDC_OTHER:         return "其它错误，如调用API错误等";
    default:                    return "未定义错误码";
    }
}

long CXFS_FIDC::WaitItemTaken()
{
    UpdateCardStatus();
    return 0;
}

long CXFS_FIDC::UpdateCardStatus()
{
    THISMODULE(__FUNCTION__);

    WORD fwMedia = WFS_IDC_MEDIAUNKNOWN;
    WORD fwChipPower = WFS_IDC_CHIPUNKNOWN;

    m_Cardstatus = IDCSTATUS_UNKNOWN;
    int iRetStatus =  m_pDev->DetectCard(m_Cardstatus);

    if (iRetStatus >= 0)
    {
        switch (m_Cardstatus)
        {
        case IDCSTAUTS_NOCARD:
            {
                fwChipPower = WFS_IDC_CHIPNOCARD;
                fwMedia = WFS_IDC_MEDIANOTPRESENT;
                break;
            }
        case IDCSTAUTS_INTERNAL:
            {
                fwMedia = WFS_IDC_MEDIAPRESENT;
                fwChipPower = WFS_IDC_CHIPONLINE;
                if (m_bChipPowerOff)
                {
                    fwChipPower = WFS_IDC_CHIPPOWEREDOFF;
                }
                break;
            }
        case IDCSTAUTS_ENTERING:
            {
                fwMedia = WFS_IDC_MEDIAENTERING;
                fwChipPower = WFS_IDC_CHIPPOWEREDOFF;
                break;
            }
        default:
            {
                fwMedia = WFS_IDC_MEDIAUNKNOWN;
                fwChipPower = WFS_IDC_CHIPUNKNOWN;
                break;
            }
        }
    }
    else
    {
        Log(ThisModule, -1, "检测卡出错:%s", ProcessReturnCode(iRetStatus));
//        UpdateDevStatus(iRetStatus);
    }
    UpdateDevStatus(iRetStatus);

    WORD wLastMedia = m_Status.fwMedia;
    m_Status.fwChipPower = fwChipPower;
    m_Status.fwMedia = fwMedia;
//    m_Status.lpszExtra = (LPSTR)m_cStaExtra.GetExtra();

    if (fwMedia == WFS_IDC_MEDIANOTPRESENT && (wLastMedia == WFS_IDC_MEDIAENTERING
                                               || wLastMedia == WFS_IDC_MEDIAPRESENT))
    {
        //关闭非接灯
        ControlLED(0);
        m_ulTakeMonitorStartTime = 0;
        FireMediaRemoved();
        Log(ThisModule, 1, "用户取走卡");
        m_WaitTaken = WTF_NONE;
    }

    if(m_ulTakeMonitorStartTime > 0 && m_iTakeCardTimeout > 0){
        if(CQtTime::GetSysTick() - m_ulTakeMonitorStartTime > (ULONG)(m_iTakeCardTimeout * 1000)){
            //关闭非接灯
            ControlLED(0);
            m_ulTakeMonitorStartTime = 0;
        }
    }

    return iRetStatus;
}

void CXFS_FIDC::UpdateDevStatus(int iRet)
{
    THISMODULE(__FUNCTION__);

    WORD fwDevice = WFS_IDC_DEVHWERROR;
    DWORD dwHWAct = WFS_ERR_ACT_NOACTION;

    if (ERR_IDC_UNSUP_CMD ==      iRet ||
        ERR_IDC_USER_CANCEL ==    iRet ||
        ERR_IDC_INSERT_TIMEOUT == iRet ||
        ERR_IDC_CONFLICT == iRet)
    {
        iRet = ERR_IDC_SUCCESS;
    }

    switch (iRet)
    {
    case ERR_IDC_SUCCESS:
        fwDevice = WFS_IDC_DEVONLINE;
        break;
    case ERR_IDC_PARAM_ERR:
    case ERR_IDC_READERROR:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_NOACTION;
        break;
    case ERR_IDC_COMM_ERR:
    case ERR_IDC_OFFLINE:
    case ERR_IDC_NO_DEVICE:
    case ERR_IDC_READTIMEOUT:
        fwDevice = WFS_IDC_DEVOFFLINE;
        dwHWAct = WFS_ERR_ACT_NOACTION;
        break;
    case ERR_IDC_STATUS_ERR:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_RESET;
        break;
    case ERR_IDC_JAMMED:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_RESET;
        m_bJamm = TRUE;
        break;
    case ERR_IDC_NOT_OPEN:
    case ERR_IDC_HWERR:
    case ERR_IDC_OTHER:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_HWCLEAR;
        break;
    case ERR_IDC_RETAINBINFULL:
        fwDevice = WFS_IDC_DEVONLINE;
        break;
    case ERR_IDC_USERERR:
        fwDevice = WFS_IDC_DEVUSERERROR;
        break;
    default:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_RESET;
        break;
    }

    if (iRet < 0)
    {
        m_bNeedRepair = TRUE;
    }

    if (m_Status.fwDevice != fwDevice && iRet != -0x3009)
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

WORD CXFS_FIDC::GetMediaPresentState(IDC_IDCSTAUTS CardStatus)
{
    switch (CardStatus)
    {
    case IDCSTAUTS_NOCARD:
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPNOCARD;
            m_Status.fwMedia = WFS_IDC_MEDIANOTPRESENT;
            break;
        }
    case IDCSTAUTS_ENTERING:    m_Status.fwMedia = WFS_IDC_MEDIAENTERING;   break;
    case IDCSTAUTS_INTERNAL:    m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;    break;

    default:
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPUNKNOWN;
            m_Status.fwMedia = WFS_IDC_MEDIAUNKNOWN;
            break;
        }
    }
    return  m_Status.fwMedia;
}

long CXFS_FIDC::ReadChip()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 激活IC卡
    char szATRInfo[MAX_LEN_ATRINFO] = {0};
    unsigned int nATRLen = sizeof(szATRInfo);
    int nRet = m_pDev->ICCActive(szATRInfo, nATRLen);


    if(nRet != 0){                                                          //30-00-00-00(FT#0041)
         m_Status.fwChipPower  = WFS_IDC_CHIPNODEVICE;                      //30-00-00-00(FT#0041)
        if (nRet == ERR_IDC_MULTICARD)
        {
            m_bMultiCard = TRUE;
            m_cStaExtra.AddExtra("ICErrorDetail", "0000");
            return WFS_ERR_IDC_INVALIDMEDIA;
        } if(nRet == ERR_IDC_INVALIDCARD){                                  //30-00-00-00(FT#0041)
            return Convert2XFSErrCode(nRet);                                //30-00-00-00(FT#0041)
        } else {                                                            //30-00-00-00(FT#0041)
            UpdateDevStatus(nRet);
            Log("ICCActive", -1, "ReadChip失败:%s", ProcessReturnCode(nRet));
            return Convert2XFSErrCode(nRet);
        }
    } else {
        m_Status.fwChipPower  = WFS_IDC_CHIPONLINE;
        m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
    }

    SetTrackInfo(WFS_IDC_CHIP, WFS_IDC_DATAOK, nATRLen, (BYTE *)szATRInfo);
    return WFS_SUCCESS;
}


int CXFS_FIDC::Init()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    ControlLED(0);
    m_bChipPowerOff = FALSE;
    int nRet = m_pDev->Init(CARDACTION_NOACTION, (WobbleAction)m_stConfig.usNeedWobble);
    UpdateDevStatus(nRet);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "初始化读卡器出错1");
        return WFS_ERR_HARDWARE_ERROR;
    }
    else
    {
        Log(ThisModule, 0, "初始化读卡器成功");
    }
    return WFS_SUCCESS;
}

int CXFS_FIDC::GetFWVersion()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    char szFWVersion[MAX_LEN_FWVERSION] = {0};
    unsigned int uLen = sizeof(szFWVersion);
    int nRet = m_pDev->GetFWVersion(szFWVersion, uLen);
    UpdateDevStatus(nRet);
    if (nRet < 0)
        return WFS_ERR_HARDWARE_ERROR;

    m_cCapExtra.AddExtra("VRTDetail[01]", szFWVersion);
    Log(ThisModule, 0, "设备固件版本信息：%s", szFWVersion);
    return nRet;
}

void CXFS_FIDC::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 新增IC卡操作
    m_Status.fwChipPower  = WFS_IDC_CHIPNODEVICE;
    m_Status.fwMedia = WFS_IDC_MEDIANOTPRESENT;
    m_Status.fwSecurity = WFS_IDC_SECNOTSUPP;
    m_Status.usCards = m_cXfsReg.GetValue("CONFIG", RETAIN_CARD_COUNT,  m_Status.usCards);
    m_Status.lpszExtra = NULL;

    if (m_Caps.usCards == 0)
    {
        m_Status.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    }
    else
    {
        if (m_Status.usCards < (m_Caps.usCards * 3 / 4))
        {
            m_Status.fwRetainBin = WFS_IDC_RETAINBINOK;
        }
        else if (m_Status.usCards < m_Caps.usCards)
        {
            m_Status.fwRetainBin = WFS_IDC_RETAINBINHIGH;
        }
        else
        {
            m_Status.fwRetainBin = WFS_IDC_RETAINBINFULL;
        }
    }   
}

void CXFS_FIDC::InitCaps()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

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
    m_Caps.fwWriteMode = m_stConfig.bCanWriteTrack ? (WFS_IDC_LOCO | WFS_IDC_HICO | WFS_IDC_AUTO) : WFS_IDC_NOTSUPP;
    m_Caps.usCards                        = 0;
    m_Caps.bFluxSensorProgrammable        = FALSE;
    m_Caps.bReadWriteAccessFollowingEject = FALSE;
    m_Caps.lpszExtra = NULL;

    m_cCapExtra.AddExtra("VRTCount", "2");
    m_cCapExtra.AddExtra("VRTDetail[00]", "0000000000000000");             // SP版本程序名称8位+版本8位
    m_cCapExtra.AddExtra("VRTDetail[01]", "");
}

void CXFS_FIDC::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
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

bool CXFS_FIDC::GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho)
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

long CXFS_FIDC::ReadTrackData(DWORD dwReadOption)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

bool CXFS_FIDC::LoadDevDll(LPCSTR ThisModule)
{
    if (m_pDev == nullptr)
    {
        char szDevDllName[256] = { 0 };
        char szDevType[256] = {0};
        strcpy(szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));
        int iDevType = m_cXfsReg.GetValue("DevType", (DWORD)0);
        switch(iDevType){
        case 0:
            strcpy(szDevType, "MT50");
            break;
        case 1:
            strcpy(szDevType, "CJ201");
            break;
        case 2:
            strcpy(szDevType, "TMZ");
            break;
        default:
            strcpy(szDevType, "MT50");
            break;
        }

        if (0 != m_pDev.Load(szDevDllName, "CreateIDevIDC", szDevType))
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s", szDevDllName);
            return false;
        }
    }
    return (m_pDev != nullptr);
}

void CXFS_FIDC::InitConfig()
{
    m_stConfig.dwTotalTimeOut =  m_cXfsReg.GetValue("CONFIG", "TotalTimeOut", 50);
    m_stConfig.CardInitAction = (CardAction)m_cXfsReg.GetValue("CONFIG", "InitAction", 4);
    m_stConfig.bLedOper =  m_cXfsReg.GetValue("CONFIG", "LedOper", 1) != 0;
    m_stConfig.strComType =  m_cXfsReg.GetValue("CONFIG", "Port", "");
    m_stConfig.bPostRemovedAftEjectFixed = m_cXfsReg.GetValue("CONFIG", "PostRemovedAftEjectFixed", 1) == 1;
}

// 基本接口
HRESULT CXFS_FIDC::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    if (!LoadDevDll(ThisModule))
    {
        return WFS_ERR_HARDWARE_ERROR;
    }

    ReadConfig();
    InitCaps();
    InitStatus();
    InitConfig();

    int nRet = m_pDev->Open(m_stConfig.strComType.c_str());
    if (nRet < 0)
    {
        Log(ThisModule, -1, "Open fail");
        return WFS_ERR_HARDWARE_ERROR;
    }

    nRet = Init();
    if (nRet < 0)
        return nRet;

    nRet = GetFWVersion();
    if(nRet < 0)
        return nRet;

    //更新状态
    OnStatus();

    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    ControlLED(0);
    if (m_pDev != nullptr)
        m_pDev->Close();
    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::OnStatus()
{
    UpdateCardStatus();
    if(m_Status.fwDevice == WFS_IDC_DEVOFFLINE){
        //断线自动重连
        m_pDev->Close();
        m_pDev->Open(m_stConfig.strComType.c_str());
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::OnWaitTaken()
{
    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    WaitItemTaken();
    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
        m_pDev->CancelReadCard();
    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// 查询命令
HRESULT CXFS_FIDC::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_bMultiCard)
    {
        m_bMultiCard = FALSE;               
    } else {
        m_cStaExtra.Clear();
    }

    m_Status.lpszExtra = (LPSTR)m_cStaExtra.GetExtra();
    lpStatus = &m_Status;
    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_Caps.lpszExtra = (LPSTR)m_cCapExtra.GetExtra();
    lpCaps = &m_Caps;
    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::GetFormList(LPSTR &lpFormList)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
    {
        hRes = WFS_SUCCESS;
    }
    lpFormList = (LPSTR)(LPCSTR)m_FormNames;
    //LPSTR szTest = (LPSTR)lpFormList;
    return hRes;
}

HRESULT CXFS_FIDC::GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
    {
        if (lpFormName == nullptr)
        {
            hRes = WFS_ERR_INVALID_DATA;
        }
        else
        {
            SP_IDC_FORM *pForm = FLFind(m_FormList, lpFormName);
            if (!pForm)
                hRes = WFS_ERR_IDC_FORMNOTFOUND;
            else if (!pForm->bLoadSucc)
                hRes = WFS_ERR_IDC_FORMINVALID;
            else
            {
                hRes = CheackFormInvalid(m_FormList, (char *)pForm->FormName.c_str(), pForm->fwAction);
                if (hRes == WFS_SUCCESS)
                {
                    m_LastForm.lpszFormName             = (char *)pForm->FormName.c_str();
                    m_LastForm.cFieldSeparatorTrack1    = pForm->cFieldSeparatorTracks[0];
                    m_LastForm.cFieldSeparatorTrack2    = pForm->cFieldSeparatorTracks[1];
                    m_LastForm.cFieldSeparatorTrack3    = pForm->cFieldSeparatorTracks[2];
                    m_LastForm.fwAction                 = pForm->fwAction;
                    m_LastForm.lpszTracks                   = (char *)pForm->sTracks.c_str();
                    m_LastForm.bSecure                  = pForm->bSecures[0] | pForm->bSecures[1] | pForm->bSecures[2];
                    m_LastForm.lpszTrack1Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[0];
                    m_LastForm.lpszTrack2Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[1];
                    m_LastForm.lpszTrack3Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[2];
                    lpForm = &m_LastForm;
                }
            }
        }
    }
    return hRes;
}


HRESULT CXFS_FIDC::WriteTrack(const LPWFSIDCWRITETRACK lpWriteData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_FIDC::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    IDC_IDCSTAUTS IDCstatus;
    int iRet = UpdateCardStatus();
    if (iRet < 0)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }    

    if (m_Cardstatus == IDCSTAUTS_NOCARD)//没有检测到卡
    {
        if(m_stConfig.bPostRemovedAftEjectFixed){
            FireMediaRemoved();
        }
        Log(ThisModule, 1, "退卡时没有检测到卡");
        return WFS_SUCCESS;
    }
    else if (m_Cardstatus == IDCSTAUTS_ENTERING ||
             m_Cardstatus == IDCSTAUTS_INTERNAL)//卡在读卡器口
    {
        int nRet = m_pDev->EjectCard();
        UpdateDevStatus(nRet);
        if (nRet >= 0)
        {
            m_Status.fwMedia = WFS_IDC_MEDIAENTERING;
            m_WaitTaken = WTF_TAKEN;
            m_ulTakeMonitorStartTime = CQtTime::GetSysTick();
            return WFS_SUCCESS;
        }
        else
        {
            Log(ThisModule, -1, "退卡时:%s", ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    return WFS_ERR_HARDWARE_ERROR;
}

long CXFS_FIDC::RetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_FIDC::RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}


HRESULT CXFS_FIDC::ResetCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_FIDC::SetKey(const LPWFSIDCSETKEY lpKeyData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_FIDC::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_ulTakeMonitorStartTime = 0;
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

HRESULT CXFS_FIDC::ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_FIDC::ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

#ifdef CARD_REJECT_GD_MODE
//读卡器新增扩展部分
HRESULT CXFS_FIDC::ReduceCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::SetCount(LPWORD lpwCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::IntakeCardBack()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMEjectCard(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMSetCardData(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMRetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMReduceCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMSetCount(LPWORD lpwCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMEmptyCard(LPCSTR lpszCardBox)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_FIDC::CMReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}
#endif

HRESULT CXFS_FIDC::WriteRawData(const LPWFSIDCCARDDATA *lppCardData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_FIDC::ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_ChipIO.wChipProtocol = lpChipIOIn->wChipProtocol;
    m_ChipIO.ulChipDataLength = MAX_CHIP_IO_LEN;

    ICCardProtocol eProFlag;
    if (lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT0  && lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT1)
    {
        Log(ThisModule, -1, "wChipProtocol=%d", lpChipIOIn->wChipProtocol);
        return WFS_ERR_IDC_PROTOCOLNOTSUPP;
    }

    if (lpChipIOIn->wChipProtocol == WFS_IDC_CHIPT0)
    {
        eProFlag = ICCARD_PROTOCOL_T0;
    }
    else if (lpChipIOIn->wChipProtocol == WFS_IDC_CHIPT1)
    {
        eProFlag = ICCARD_PROTOCOL_T1;
    }

    if (m_Status.fwMedia == WFS_IDC_MEDIANOTPRESENT)//没有检测到卡
    {
        Log(ThisModule, -1, "ChipIO时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }

    if (m_Status.fwMedia == WFS_IDC_MEDIAENTERING)
    {
        return WFS_ERR_IDC_ATRNOTOBTAINED;
    }

    if (m_Status.fwChipPower  != WFS_IDC_CHIPONLINE)
    {
        return WFS_ERR_IDC_INVALIDMEDIA;
    }

    char szDataInOut[512] = {0};
    memcpy(szDataInOut, lpChipIOIn->lpbChipData, lpChipIOIn->ulChipDataLength);
    unsigned int nInOutLen = lpChipIOIn->ulChipDataLength;
    int nRet = m_pDev->ICCChipIO(eProFlag, szDataInOut, nInOutLen, sizeof(szDataInOut));
    if (ERR_IDC_PROTOCOLNOTSUPP == nRet)
    {
        return WFS_ERR_IDC_INVALIDDATA;
    }
    if (ERR_IDC_INVALIDCARD == nRet)
    {
        return WFS_ERR_IDC_INVALIDMEDIA;
    }

    UpdateDevStatus(nRet);
    if (0 != nRet)
    {
        Log(ThisModule, -1, "ChipIO失败，返回：%d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_ChipIO.ulChipDataLength = nInOutLen;
    memcpy(m_ChipIO.lpbChipData, szDataInOut, nInOutLen);
    lpChipIOOut = &m_ChipIO;
    return 0;
}

HRESULT CXFS_FIDC::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_pDev->SetRFIDCardReaderLED(LEDTYPE_BLUE, LEDACTION_CLOSE);

    WORD wAction = *lpResetIn;
    // 没有动作参数时由SP决定
    if (wAction == 0)
    {
        wAction = WFS_IDC_RETAIN;
    }

    if (wAction < WFS_IDC_NOACTION || wAction > WFS_IDC_RETAIN)
    {
        return WFS_ERR_INVALID_DATA;
    }

    CardAction ActFlag;
    if (wAction == WFS_IDC_RETAIN)
    {
        ActFlag = CARDACTION_RETRACT;
    }
    else if (wAction == WFS_IDC_NOACTION)
    {
        ActFlag = CARDACTION_NOACTION;
    }
    else if (wAction == WFS_IDC_EJECT)
    {
        ActFlag = CARDACTION_EJECT;
    }
    else
    {
        return WFS_ERR_INVALID_DATA;
    }

    long nRet = m_pDev->Init(ActFlag, (WobbleAction)m_stConfig.usNeedWobble);
    UpdateDevStatus(nRet);
    if (nRet >= 0)
    {
        return WFS_SUCCESS;
    }
    else
    {
        return WFS_ERR_HARDWARE_ERROR;
    }
}

HRESULT CXFS_FIDC::ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_bChipPowerOff = FALSE;
    m_ChipPowerRes.Clear();

    WORD wPower = * lpChipPower;
    if ((WFS_IDC_CHIPPOWERCOLD != wPower) &&
        (WFS_IDC_CHIPPOWERWARM != wPower) &&
        (WFS_IDC_CHIPPOWEROFF  != wPower))
    {
        return WFS_ERR_INVALID_DATA;
    }

    int iTmpRet = UpdateCardStatus();
    if (iTmpRet < 0)
    {
        Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(iTmpRet));
        return WFS_ERR_HARDWARE_ERROR;
    }
    else if (m_Cardstatus == IDCSTAUTS_NOCARD)//没有检测到卡
    {
        Log(ThisModule, -1, "ChipPower时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }

    //////////////////////////////////////////////////////////////////////////
    char szATRInfo[100] = {0};
    unsigned int nATRLen = 0;
    int nRet = 0;

    if (m_Status.fwChipPower  == WFS_IDC_CHIPNODEVICE)
    {
        Log(ThisModule, -1, "ChipPower时卡没有芯片或芯片有问题");
        return WFS_ERR_IDC_INVALIDMEDIA;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPNOCARD)
    {
        Log(ThisModule, -1, "ChipPower时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPHWERROR ||
             m_Status.fwChipPower  == WFS_IDC_CHIPUNKNOWN)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPNOTSUPP)
    {
        Log(ThisModule, -1, "ChipPower时设备不支持IC卡操作");
        return WFS_ERR_UNSUPP_COMMAND;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPONLINE ||
             m_Status.fwChipPower  == WFS_IDC_CHIPBUSY)
    {
        if (WFS_IDC_CHIPPOWERCOLD == wPower)
        {
            nRet = m_pDev->ICCDeActivation();
            if (0 != nRet)
            {
                Log(ThisModule, -1, "IC冷复位（当前IC卡为已激活状态，现在正尝试反激活）失败，复位参数：%d", wPower);
                if (ERR_IDC_INVALIDCARD == nRet)
                {
                    return WFS_ERR_IDC_INVALIDMEDIA;
                }
                //SetFailState(nRet);
                if (ERR_IDC_UNSUP_CMD == nRet)
                {
                    return WFS_ERR_UNSUPP_COMMAND;
                }
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    }
    else // WFS_IDC_CHIPPOWEREDOFF
    {
        if (WFS_IDC_CHIPPOWERWARM == wPower)
        {
            nRet = m_pDev->ICCReset(ICCARDRESET_COLD, szATRInfo, nATRLen);
            UpdateDevStatus(nRet);
            if (0 != nRet)
            {
                Log(ThisModule, -1, "IC复位（当前IC卡为未激活状态，现在正尝试激活）失败，复位参数：%d", wPower);
                if (ERR_IDC_INVALIDCARD == nRet)
                {
                    return WFS_ERR_IDC_INVALIDMEDIA;
                }
                if (ERR_IDC_UNSUP_CMD == nRet)
                {
                    return WFS_ERR_UNSUPP_COMMAND;
                }
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
        else if (WFS_IDC_CHIPPOWEROFF == wPower)
        {
            m_ChipPowerRes.SetData(szATRInfo, nATRLen);
            lpData = m_ChipPowerRes.GetData();
            return WFS_SUCCESS;
        }
    }

    memset(szATRInfo, 0, sizeof(szATRInfo));
    //////////////////////////////////////////////////////////////////////////

    if (wPower == WFS_IDC_CHIPPOWERCOLD)
    {
        nRet = m_pDev->ICCReset(ICCARDRESET_COLD, szATRInfo, nATRLen);
        if (nRet == 0)
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPONLINE;
        }
    }
    else if (wPower == WFS_IDC_CHIPPOWERWARM)
    {
        nRet = m_pDev->ICCReset(ICCARDRESET_WARM, szATRInfo, nATRLen);
        if (nRet == 0)
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPONLINE;
        }
    }
    else if (wPower == WFS_IDC_CHIPPOWEROFF)
    {
        nRet = m_pDev->ICCDeActivation();
        if (nRet == 0)
        {
            m_bChipPowerOff = TRUE;
            m_Status.fwChipPower  = WFS_IDC_CHIPPOWEREDOFF;
        }
    }

    if (0 != nRet)
    {
        Log(ThisModule, -1, "IC复位失败，复位参数：%d", wPower);
        if (ERR_IDC_INVALIDCARD == nRet)
        {
            return WFS_ERR_IDC_INVALIDMEDIA;
        }
        if (ERR_IDC_UNSUP_CMD == nRet)
        {
            return WFS_ERR_UNSUPP_COMMAND;
        }
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_ChipPowerRes.SetData(szATRInfo, nATRLen);
    lpData = m_ChipPowerRes.GetData();

    return WFS_SUCCESS;
}

void CXFS_FIDC::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_FIDC::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_FIDC::FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

void CXFS_FIDC::FireCardInvalidMedia()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDMEDIA, nullptr);
}

void CXFS_FIDC::FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

void CXFS_FIDC::FireMediaRetained()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIARETAINED, nullptr);
}

void CXFS_FIDC::FireRetainBinThreshold(WORD wReBin)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_IDC_RETAINBINTHRESHOLD, (LPVOID)&wReBin);
}

void CXFS_FIDC::FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

void CXFS_FIDC::FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}

// 读取ini配置项目
bool CXFS_FIDC::ReadConfig()
{
    THISMODULE(__FUNCTION__);
    m_iTakeCardTimeout = 0;


    QString strINIFile("FIDCConfig.ini");
    #ifdef QT_WIN32
        strINIFile.prepend("C:/CFES/ETC/");
    #else
        strINIFile.prepend("/usr/local/CFES/ETC/");
    #endif

    CINIFileReader cINIFile;
    // read FIDCConfig.ini
    if(!cINIFile.LoadINIFile(strINIFile.toLocal8Bit().data()))
    {
        Log(ThisModule, __LINE__, "Load FIDCConfig.ini fail.");
        return false;
    }

    CINIReader cINI = cINIFile.GetReaderSection("FIDCInfo");
    m_iTakeCardTimeout = (int)cINI.GetValue("TakeCardTimeout", 0);

    return true;
}
