#include "XFS_MSR.h"

// MSR SP 版本号
BYTE    byVRTU[17] = {"HWMSRSTE00000100"};

static const char *DEVTYPE = "MSR";
static const char *ThisFile = "XFS_MSR.cpp";
#define LOG_NAME     "XFS_MSR.log"

//////////////////////////////////////////////////////////////////////////

CXFS_MSR::CXFS_MSR()
{
    // SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    m_stMsrIniConfig.clear();
    memset(&m_Status, 0x00, sizeof(CWfsIDCStatus));
    memset(&m_OldStatus, 0x00, sizeof(CWfsIDCStatus));
    memset(&m_Caps, 0x00, sizeof(CWfsIDCCap));
    m_CardDatas.Clear();
    m_cExtra.Clear();
}

CXFS_MSR::~CXFS_MSR()
{
    //
}

long CXFS_MSR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseMSR
    if (0 != m_pBase.Load("SPBaseMSR.dll", "CreateISPBaseMSR", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseMSR失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

// ------基本接口----------------------------
// 打开设备
HRESULT CXFS_MSR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // INI读取
    InitConfig();

    // 设备驱动动态库验证
    if (strlen(m_stMsrIniConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 加载设备驱动动态库
    if (m_pDev == NULL)
    {
        hRet = m_pDev.Load(m_stMsrIniConfig.szDevDllName,
                           "CreateIDevMSR", DEVTYPE_CHG(m_stMsrIniConfig.wDeviceType));
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__,
                "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%s",
                m_stMsrIniConfig.szDevDllName, m_stMsrIniConfig.wDeviceType,
                DEVTYPE_CHG(m_stMsrIniConfig.wDeviceType),
                m_pDev.LastError().toUtf8().constData());
            return hErrCodeChg(hRet);
        }
    }

    // Open前下传初始参数
    m_pDev->SetData(&(m_stMsrIniConfig.stMsrInitParamInfo), DATATYPE_INIT);

    // 打开连接
    hRet = m_pDev->Open(nullptr);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败．ReturnCode:%d", hRet);
        return hErrCodeChg(hRet);
    }

    // Reset
    if (m_stMsrIniConfig.wOpenResetSupp == 1)
    {
        HRESULT hRet = ResetExecute();
        if (hRet != WFS_SUCCESS)
        {

            if (m_stMsrIniConfig.wResetFailReturn == 0)
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Return Error. ");
                return hRet;
            } else
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Not Return Error. ");
            }
        }
    }

    // 更新扩展状态
    CHAR szDevMSRVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szDevMSRVer, sizeof(szDevMSRVer) - 1, 1);

    CHAR szFWVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    CHAR szSWVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szSWVer, sizeof(szSWVer) - 1, 3);

    m_cExtra.AddExtra("VRTCount", "4");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", szDevMSRVer);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
    m_cExtra.AddExtra("VRTDetail[03]", szSWVer);

    // Capabilities Status　初始化
    InitCaps();
    InitStatus();

    // 更新一次状态
    OnStatus();

    Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());

    return WFS_SUCCESS;
}

// 关闭设备
HRESULT CXFS_MSR::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
        m_pDev->Close();

    return WFS_SUCCESS;
}

// 状态更新
HRESULT CXFS_MSR::OnStatus()
{
    UpdateStatus();
    if(m_Status.fwDevice == WFS_IDC_DEVOFFLINE){
        m_pDev->Close();
        CQtTime::Sleep(500);
        m_pDev->Open(nullptr);
    }

    return WFS_SUCCESS;
}

// Taken上报
HRESULT CXFS_MSR::OnWaitTaken()
{
    return WFS_SUCCESS;
}

HRESULT CXFS_MSR::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
        m_pDev->Cancel();
    return WFS_SUCCESS;
}

HRESULT CXFS_MSR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// ------查询命令----------------------------

// 状态查询
HRESULT CXFS_MSR::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpStatus = &m_Status;

    return WFS_SUCCESS;
}

// 能力值查询
HRESULT CXFS_MSR::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_Caps;

    return WFS_SUCCESS;
}

// ------执行命令----------------------------

// 读卡
HRESULT CXFS_MSR::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wOption = *(WORD *)lpReadData;

    // 读卡时不选任何参数值返回DATAINVALID
    //if ((wOption & 0x803F) == 0)
    WORD nTrack = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
    if (wOption & nTrack == 0)
    {
        Log(ThisModule, __LINE__, "接收ReadRawData参数无效");
        return WFS_ERR_INVALID_DATA;
    }

    m_CardDatas.Clear();
    long hRes = ReadRawDataExecute(wOption, dwTimeOut);
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "AcceptAndReadTrack failed");
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_CardDatas;
    return hRes;
}

// 复位
HRESULT CXFS_MSR::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = ResetExecute();
    if (m_stMsrIniConfig.wResetFailReturn == 0)
    {
        return hRet;
    } else
    {
        return WFS_SUCCESS;
    }
}

//  ------处理流程----------------------------

// 状态初始化
void CXFS_MSR::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Status.fwDevice    = WFS_IDC_DEVNODEVICE;
    m_Status.fwMedia     = WFS_IDC_MEDIAUNKNOWN;
    m_Status.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    m_Status.fwSecurity  = WFS_IDC_SECNOTSUPP;
    m_Status.usCards     = 0;
    m_Status.fwChipPower = WFS_IDC_CHIPNOTSUPP;
    m_Status.lpszExtra   = nullptr;

    m_OldStatus.fwDevice    = m_Status.fwDevice;
    m_OldStatus.fwMedia     = m_Status.fwMedia;
    m_OldStatus.fwRetainBin = m_Status.fwRetainBin;
    m_OldStatus.fwSecurity  = m_Status.fwSecurity;
    m_OldStatus.usCards     = m_Status.usCards;
    m_OldStatus.fwChipPower = m_Status.fwChipPower;
    m_OldStatus.lpszExtra   = m_Status.lpszExtra;
}

// 能力值初始化
void CXFS_MSR::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.wClass = WFS_SERVICE_CLASS_IDC;
    m_Caps.fwType = WFS_IDC_TYPESWIPE;
    m_Caps.bCompound = FALSE;
    m_Caps.fwReadTracks = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
    m_Caps.fwWriteTracks = WFS_IDC_NOTSUPP;
    m_Caps.usCards                        = 0;  // 指定回收盒可以容纳的最大卡张数(如果不可用则为0)
    m_Caps.fwChipProtocols = WFS_IDC_NOTSUPP;
    m_Caps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_Caps.fwPowerOnOption = WFS_IDC_NOACTION;
    m_Caps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_Caps.bFluxSensorProgrammable        = FALSE;
    m_Caps.bReadWriteAccessFollowingEject = FALSE;
    m_Caps.fwWriteMode = WFS_IDC_NOTSUPP;
    m_Caps.fwChipPower = WFS_IDC_NOTSUPP;
    m_Caps.lpszExtra = nullptr;//(LPSTR)m_cExtra.GetExtra();
}

// 读卡子处理1
HRESULT CXFS_MSR::ReadRawDataExecute(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_Status.fwDevice == WFS_IDC_DEVHWERROR ||
        m_Status.fwDevice == WFS_IDC_DEVOFFLINE ||
        m_Status.fwDevice == WFS_IDC_DEVPOWEROFF)
    {
        Log(ThisModule, __LINE__, "m_Status.fwDevice = %d",m_Status.fwDevice);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return ReadTrackData(dwReadOption, dwTimeOut);
}

// 读卡子处理2
long CXFS_MSR::ReadTrackData(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STTRACK_INFO    trackInfo;
    // ReadRawData前先将磁道初始化避免KAL出错崩溃
    if (dwReadOption & WFS_IDC_TRACK1)
    {
        SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAINVALID, 0, NULL);
        trackInfo.TrackData[0].bTrackOK = true;
    }
    if (dwReadOption & WFS_IDC_TRACK2)
    {
        SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAINVALID, 0, NULL);
        trackInfo.TrackData[1].bTrackOK = true;
    }
    if (dwReadOption & WFS_IDC_TRACK3)
    {
        SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAINVALID, 0, NULL);
        trackInfo.TrackData[2].bTrackOK = true;
    }

    int nRet = m_pDev->ReadTracks(trackInfo, dwTimeOut);
    if (nRet == 0)
    {
        Log("ReadTrack", __LINE__, "ReadTracks()调用成功");
        m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
        if (dwReadOption & WFS_IDC_TRACK1)
        {
            if (trackInfo.TrackData[0].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[0].szTrack), (BYTE *)trackInfo.TrackData[0].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK2)
        {
            if (trackInfo.TrackData[1].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[1].szTrack), (BYTE *)trackInfo.TrackData[1].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK3)
        {
            if (trackInfo.TrackData[2].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[2].szTrack), (BYTE *)trackInfo.TrackData[2].szTrack);
            }
        }
        FireCardInserted(); // 入卡事件
        //if(!trackInfo.TrackData[0].bTrackOK && !trackInfo.TrackData[1].bTrackOK && !trackInfo.TrackData[2].bTrackOK){
        //    return WFS_ERR_IDC_INVALIDMEDIA;
        //}
    }
    else if (nRet == ERR_MSR_NOTRACK)
    {
        FireCardInserted(); // 入卡事件
        return WFS_ERR_IDC_INVALIDMEDIA;
    }
    else if (nRet == ERR_MSR_CARDTOOSHORT)
    {
        FireCardInserted(); // 入卡事件
        return WFS_ERR_IDC_CARDTOOSHORT;
    }
    else if (nRet == ERR_MSR_CARDTOOLONG)
    {
        FireCardInserted(); // 入卡事件
        return WFS_ERR_IDC_CARDTOOLONG;
    }
    else if (nRet == ERR_MSR_READERROR)
    {
        FireCardInserted(); // 入卡事件
        return WFS_ERR_IDC_INVALIDDATA;
    }
    else if (nRet == ERR_MSR_INVALIDCARD)
    {
        FireCardInserted(); // 入卡事件
        return WFS_SUCCESS;
    } else if(nRet == ERR_MSR_READTIMEOUT)   // 读数据超时
    {
        Log("ReadTrack", -1, "ReadTrack TimeOut:%s", GetErrorStr(nRet));
        return WFS_ERR_TIMEOUT;
    } else if(nRet == ERR_MSR_USER_CANCEL)   // 读数据取消
    {
        Log("ReadTrack", -1, "ReadTrack Canceled:%s", GetErrorStr(nRet));
        return WFS_ERR_CANCELED;
    }
    else
    {
        Log("ReadTrack", -1, "ReadTrack时出错:%s", GetErrorStr(nRet));
        FireCardInserted(); // 入卡事件
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

// 读卡子处理3
void CXFS_MSR::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
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

// 复位子处理
HRESULT CXFS_MSR::ResetExecute()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = 0;

    nRet = m_pDev->Reset();
    if (nRet < 0) // 复位动作失败
    {
        Log(ThisModule, __LINE__, "Reset失败: %s", GetErrorStr(nRet));
        if (nRet == ERR_MSR_JAMMED)
        {
            return WFS_ERR_IDC_MEDIAJAM;
        }
        else
        if (nRet == ERR_MSR_INVALIDCARD)
        {
            return WFS_ERR_IDC_INVALIDMEDIA;
        }
        else
        {
            return WFS_ERR_HARDWARE_ERROR;
        }
    } else
    {
        return WFS_SUCCESS;
    }
}

// 状态更新子处理
long CXFS_MSR::UpdateStatus()
{
    THISMODULE(__FUNCTION__);


    int nRet = 0;

    DWORD dwHWAct = WFS_ERR_ACT_NOACTION;

    DEVMSRSTATUS stDevMsrStat;
    nRet = m_pDev->GetStatus(stDevMsrStat);
    if (nRet != ERR_MSR_SUCCESS)
    {
        switch (nRet)
        {
            case ERR_MSR_PARAM_ERR:
            case ERR_MSR_READERROR:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_MSR_COMM_ERR:
            case ERR_MSR_OFFLINE:
            case ERR_MSR_NO_DEVICE:
            case ERR_MSR_READTIMEOUT:
            case ERR_MSR_NOT_OPEN:
                m_Status.fwDevice = WFS_IDC_DEVOFFLINE;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_MSR_STATUS_ERR:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
            case ERR_MSR_JAMMED:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                //m_bJamm = TRUE;
                break;
//            case ERR_MSR_NOT_OPEN:
            case ERR_MSR_HWERR:
            case ERR_MSR_OTHER:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_HWCLEAR;
                break;
            case ERR_MSR_RETAINBINFULL:
                m_Status.fwDevice = WFS_IDC_DEVONLINE;
                break;
            case ERR_MSR_USERERR:
                m_Status.fwDevice = WFS_IDC_DEVUSERERROR;
                break;
            default:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
        }
    } else
    {
        m_Status.fwDevice = wConvertDeviceStatus(stDevMsrStat.wDevice);
        m_Status.fwMedia = wConvertMediaStatus(stDevMsrStat.wMedia);
    }


    if (m_Status.fwDevice != m_OldStatus.fwDevice)
    {
        FireStatusChanged(m_Status.fwDevice);
        if (m_Status.fwDevice != WFS_IDC_DEVONLINE && m_Status.fwDevice != WFS_IDC_DEVBUSY)
        {
            FireHWEvent(dwHWAct, nullptr);
        }
    }

    m_Status.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    m_OldStatus.fwDevice = m_Status.fwDevice;
    m_OldStatus.fwMedia = m_Status.fwMedia;

    return WFS_SUCCESS;
}

// Device状态转换为WFS标准
WORD CXFS_MSR::wConvertDeviceStatus(WORD wDevIdxStat)
{
    switch(wDevIdxStat)
    {
        case DEV_ONLINE:
            return WFS_IDC_DEVONLINE;
        case DEV_OFFLINE:
            return WFS_IDC_DEVOFFLINE;
        case DEV_POWEROFF:
            return WFS_IDC_DEVPOWEROFF;
        case DEV_NODEVICE:
            return WFS_IDC_DEVNODEVICE;
        case DEV_HWERROR:
            return WFS_IDC_DEVHWERROR;
        case DEV_USERERROR:
            return WFS_IDC_DEVUSERERROR;
        case DEV_BUSY:
            return WFS_IDC_DEVBUSY;
    }

    return (0);
}

// Media状态转换为WFS标准
WORD CXFS_MSR::wConvertMediaStatus(WORD wDevIdxStat)
{
    switch(wDevIdxStat)
    {
        case MEDIA_PRESENT:
            return WFS_IDC_MEDIAPRESENT;
        case MEDIA_NOTPRESENT:
            return WFS_IDC_MEDIANOTPRESENT;
        case MEDIA_JAMMED:
            return WFS_IDC_MEDIAJAMMED;
        case MEDIA_NOTSUPP:
            return WFS_IDC_MEDIANOTSUPP;
        case MEDIA_UNKNOWN:
            return WFS_IDC_MEDIAUNKNOWN;
        case MEDIA_ENTERING:
            return WFS_IDC_MEDIAENTERING;
    }

    return (0);
}

// DevMSR返回值转换为WFS标准
HRESULT CXFS_MSR::hErrCodeChg(LONG lDevCode)
{
    switch (lDevCode)
    {
    case ERR_MSR_SUCCESS:                 return WFS_SUCCESS;
    case ERR_MSR_INSERT_TIMEOUT:          return WFS_ERR_TIMEOUT;
    case ERR_MSR_USER_CANCEL:             return WFS_ERR_CANCELED;
    case ERR_MSR_COMM_ERR:                return WFS_ERR_CONNECTION_LOST;
    case ERR_MSR_JAMMED:                  return WFS_ERR_IDC_MEDIAJAM;
    case ERR_MSR_OFFLINE:                 return WFS_ERR_CONNECTION_LOST;
    case ERR_MSR_NOT_OPEN:                return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_RETAINBINFULL:           return WFS_ERR_IDC_RETAINBINFULL;
    case ERR_MSR_HWERR:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_STATUS_ERR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_UNSUP_CMD:               return WFS_ERR_UNSUPP_COMMAND;
    case ERR_MSR_PARAM_ERR:               return WFS_ERR_INVALID_DATA;
    case ERR_MSR_READTIMEOUT:             return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_WRITETIMEOUT:            return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_READERROR:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_WRITEERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_INVALIDCARD:             return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_MSR_NOTRACK:                 return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_MSR_CARDPULLOUT:             return WFS_ERR_USER_ERROR;
    case ERR_MSR_CARDTOOSHORT:            return WFS_ERR_IDC_CARDTOOSHORT;
    case ERR_MSR_CARDTOOLONG:             return WFS_ERR_IDC_CARDTOOLONG;
    case ERR_MSR_WRITETRACKERROR:         return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_PROTOCOLNOTSUPP:         return WFS_ERR_UNSUPP_DATA;
    case ERR_MSR_ICRW_ERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_NO_DEVICE:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_OTHER:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_MSR_USERERR:                 return WFS_ERR_USER_ERROR;
    case ERR_MSR_TAMPER:                  return WFS_ERR_USER_ERROR;
    default:                              return WFS_ERR_HARDWARE_ERROR;
    }
}

// DevMSR返回值转换为明文解释
LPCSTR CXFS_MSR::GetErrorStr(int nCode)
{
    switch (nCode)
    {
    case ERR_MSR_SUCCESS:       return "操作成功";
    case ERR_MSR_INSERT_TIMEOUT: return "进卡超时";
    case ERR_MSR_USER_CANCEL:   return "用户取消";
    case ERR_MSR_PARAM_ERR:     return "参数错误";
    case ERR_MSR_COMM_ERR:      return "通讯错误";
    case ERR_MSR_STATUS_ERR:    return "读卡器状态出错";
    case ERR_MSR_JAMMED:        return "读卡器堵卡";
    case ERR_MSR_OFFLINE:       return "读卡器脱机";
    case ERR_MSR_NOT_OPEN:      return "没有调用Open";
    case ERR_MSR_RETAINBINFULL: return "回收箱满";
    case ERR_MSR_HWERR:         return "硬件故障";
    case ERR_MSR_READTIMEOUT:   return "读数据超时";
    case ERR_MSR_READERROR:     return "读数据错误";
    case ERR_MSR_INVALIDCARD:   return "无效芯片，或磁道数据无效";
    case ERR_MSR_NOTRACK:       return "非磁卡，未检测到磁道";
    case ERR_MSR_CARDPULLOUT:   return "接收卡时，卡被拖出";
    case ERR_MSR_CARDTOOSHORT:  return "卡太短";
    case ERR_MSR_CARDTOOLONG:   return "卡太长";
    case ERR_MSR_PROTOCOLNOTSUPP: return "不支持的IC通讯协议";
    case ERR_MSR_NO_DEVICE:     return "指定名的设备不存在";
    case ERR_MSR_OTHER:         return "其它错误，如调用API错误等";
    default:                    return "未定义错误码";
    }
}

// Taken子处理
long CXFS_MSR::WaitItemTaken()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();


    return 0;
}


// ------事件处理----------------------------

// 硬件故障事件
void CXFS_MSR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 状态变化事件
void CXFS_MSR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 进卡事件
void CXFS_MSR::FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

// 退卡/移走卡事件
void CXFS_MSR::FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

// 复位时检测到卡事件
void CXFS_MSR::FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

// 出现无效磁道事件
void CXFS_MSR::FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}


// ------INI处理----------------------------
// 读INI
int CXFS_MSR::InitConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD    wTmp = 0;
    CHAR    szTmp[BUF_SIZE256];
    CHAR    szIniAppName[BUF_SIZE256];

    // 底层设备控制动态库名
    memset(&m_stMsrIniConfig, 0x00, sizeof(ST_MSR_INI_CONFIG));
    strcpy(m_stMsrIniConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));


    // 设备类型(0:WBT-2172-ZD/1:WB-CS)
    m_stMsrIniConfig.wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (INT)0);

    // Open时是否执行Reset动作(0不执行/1执行,缺省0)
    m_stMsrIniConfig.wOpenResetSupp = (WORD)m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetSupp", (INT)0);
    if (m_stMsrIniConfig.wOpenResetSupp != 0 &&
        m_stMsrIniConfig.wOpenResetSupp != 1)
    {
        m_stMsrIniConfig.wOpenResetSupp = 0;
    }

    // 读设备Open相关参数
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_OPENMODE_%d", m_stMsrIniConfig.wDeviceType);
    STDEVICEOPENMODE    stDevOpenModeTmp;
    //STWBT2172DEVLOG     stWbt2172DevLogTmp;

    // 打开方式(0串口/1USBHID,缺省0)
    stDevOpenModeTmp.wOpenMode = (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenMode", (INT)0);
    if (stDevOpenModeTmp.wOpenMode < 0 || stDevOpenModeTmp.wOpenMode > 1)
    {
        stDevOpenModeTmp.wOpenMode = 0;
    }
    // 设备路径(适用于串口和USBHID,缺省空)
    strcpy(stDevOpenModeTmp.szDevPath, m_cXfsReg.GetValue(szIniAppName, "DevPath", ""));
    // 波特率(适用于串口,缺省9600)
    stDevOpenModeTmp.wBaudRate = (WORD)m_cXfsReg.GetValue(szIniAppName, "BaudRate", (INT)9600);
    // 设备VID(适用于USBHID,4位16进制字符,缺省空)
    strcpy(stDevOpenModeTmp.szHidVid, m_cXfsReg.GetValue(szIniAppName, "VendorId", ""));
    // 设备PID(适用于USBHID,4位16进制字符,缺省空)
    strcpy(stDevOpenModeTmp.szHidPid, m_cXfsReg.GetValue(szIniAppName, "ProductId", ""));
    // 是否启用动态库底层处理日志记录(0不启用/1启用,缺省1)
    /*stWbt2172DevLogTmp.wIsEnableDevLog = (WORD)m_cXfsReg.GetValue(szIniAppName, "IsEnableDevLog", (INT)1);
    if (stWbt2172DevLogTmp.wIsEnableDevLog != 1 && stWbt2172DevLogTmp.wIsEnableDevLog != 2)
    {
        stWbt2172DevLogTmp.wIsEnableDevLog = 1;
    }*/
    // 动态库底层处理日志记录级别(缺省4)
    // 0禁止显示写日志, 1:允许记录信息级日志, 2:允许记录调试级日志,
    // 3允许记录警告级日志, 4允许记录错误等级日志，记录发送接收数据
    /*stWbt2172DevLogTmp.wDeviceLogLevel = (WORD)m_cXfsReg.GetValue(szIniAppName, "DeviceLogLevel", (INT)4);
    if (stWbt2172DevLogTmp.wDeviceLogLevel < 0 || stWbt2172DevLogTmp.wDeviceLogLevel > 4)
    {
        stWbt2172DevLogTmp.wDeviceLogLevel = 4;
    }
    // 动态库底层处理日志路径(绝对路径,不设置则与动态库处于同一路径下)
    strcpy(stWbt2172DevLogTmp.szDeviceLog, m_cXfsReg.GetValue(szIniAppName, "DeviceLog", ""));


    memcpy(&(m_stMsrIniConfig.stMsrInitParamInfo.stWBT2172DevLog), &stWbt2172DevLogTmp, sizeof(STWBT2172DEVLOG));
*/
    memcpy(&(m_stMsrIniConfig.stMsrInitParamInfo.stDeviceOpenMode), &stDevOpenModeTmp, sizeof(STDEVICEOPENMODE));
    // Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功,缺省0)
    m_stMsrIniConfig.wResetFailReturn = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetFailReturn", (INT)0);
    if (m_stMsrIniConfig.wResetFailReturn != 0 &&
        m_stMsrIniConfig.wResetFailReturn != 1)
    {
        m_stMsrIniConfig.wResetFailReturn = 0;
    }

    return WFS_SUCCESS;
}

