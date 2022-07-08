/***************************************************************
* 文件名称：XFS_MSR.cpp
* 文件描述：刷折器模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_MSR.h"

//------------------------------------处理流程------------------------------------

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

// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_MSR::StartOpen()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    // Open前下传初始参数
    if (m_bReCon == FALSE)
    {
        if (strlen(m_stMsrIniConfig.szSDKPath) > 0)
        {
            m_pDev->SetData(m_stMsrIniConfig.szSDKPath, DTYPE_LIB_PATH);
        }
        m_pDev->SetData(&(m_stMsrIniConfig.stMsrInitParamInfo), DATATYPE_INIT);
    }

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
        HRESULT hRet = InnerReset();
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
    m_cExtra.AddExtra("VRTCount", "2");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);

    // 获取固件版本
    CHAR szFWVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);    
    if (strlen(szFWVer) == 0)   // 获取失败等待2秒再次获取
    {
        sleep(1);
        m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);
    }
    if (strlen(szFWVer) > 0)
    {
        m_cExtra.AddExtra("VRTCount", "3");
        m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
    }

    CHAR szSWVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szSWVer, sizeof(szSWVer) - 1, 3);
    if (strlen(szSWVer) > 0)
    {
        if (strlen(szFWVer) > 0)
        {
            m_cExtra.AddExtra("VRTCount", "4");
            m_cExtra.AddExtra("VRTDetail[03]", szSWVer);
        } else
        {
            m_cExtra.AddExtra("VRTCount", "3");
            m_cExtra.AddExtra("VRTDetail[02]", szSWVer);
        }
    }

    // 更新一次状态
    OnStatus();

    if (m_bReCon == FALSE)
    {
        Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "断线重连,打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
        m_bReCon = FALSE;
    }


    return WFS_SUCCESS;
}

// 读卡子处理1
HRESULT CXFS_MSR::InnerReadRawData(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STTRACK_INFO    trackInfo;

    if (m_Status.fwDevice == WFS_IDC_DEVHWERROR ||
        m_Status.fwDevice == WFS_IDC_DEVOFFLINE ||
        m_Status.fwDevice == WFS_IDC_DEVPOWEROFF)
    {
        Log(ThisModule, __LINE__, "读卡子处理: 取设备状态: m_Status.fwDevice = %d, Return: %d.",
            m_Status.fwDevice, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // ReadRawData前先将磁道初始化避免KAL出错崩溃
    if (dwReadOption & WFS_IDC_TRACK1)
    {
        SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAINVALID, 0, NULL);
        trackInfo.TrackData[0].bTrackOK = true; // 标记需要读1磁
    }
    if (dwReadOption & WFS_IDC_TRACK2)
    {
        SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAINVALID, 0, NULL);
        trackInfo.TrackData[1].bTrackOK = true; // 标记需要读2磁
    }
    if (dwReadOption & WFS_IDC_TRACK3)
    {
        SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAINVALID, 0, NULL);
        trackInfo.TrackData[2].bTrackOK = true; // 标记需要读3磁
    }

    // 调用设备接口
    int nRet = m_pDev->ReadTracks(trackInfo, dwTimeOut);
    if (nRet == ERR_MSR_SUCCESS)    // 读成功,数据写入应答结构体
    {
        Log(ThisModule, __LINE__, "读卡子处理: 调用设备接口: ->ReadTracks()调用成功.");
        m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
        if (dwReadOption & WFS_IDC_TRACK1)
        {
            if (trackInfo.TrackData[0].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[0].szTrack),
                             (BYTE *)trackInfo.TrackData[0].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK2)
        {
            if (trackInfo.TrackData[1].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[1].szTrack),
                             (BYTE *)trackInfo.TrackData[1].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK3)
        {
            if (trackInfo.TrackData[2].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[2].szTrack),
                             (BYTE *)trackInfo.TrackData[2].szTrack);
            }
        }
        FireCardInserted(); // 入卡事件
        //if(!trackInfo.TrackData[0].bTrackOK && !trackInfo.TrackData[1].bTrackOK && !trackInfo.TrackData[2].bTrackOK){
        //    return WFS_ERR_IDC_INVALIDMEDIA;
        //}
    } else
    {
        // 返回错误非以下值时,认定已刷卡,上报InsertCard事件
        if (nRet != ERR_MSR_INSERT_TIMEOUT &&   // 进卡超时
            nRet != ERR_MSR_USER_CANCEL &&      // 用户取消
            nRet != ERR_MSR_PARAM_ERR &&        // 参数错误
            nRet != ERR_MSR_RETAINBINFULL &&    // 回收箱满
            nRet != ERR_MSR_READTIMEOUT &&      // 读数据超时
            nRet != ERR_MSR_WRITETIMEOUT &&     // 写数据超时
            nRet != ERR_MSR_HWERR &&            // 硬件故障
            nRet != ERR_MSR_NO_DEVICE &&        // 指定名的设备不存在
            nRet != ERR_MSR_OTHER)              // 其它错误，如调用API错误等
        {
            FireCardInserted(); // 入卡事件
        }
        Log(ThisModule, __LINE__, "读卡子处理: 调用设备接口: ->ReadTracks()出错, RetCode:%d|%s, Return:%d.",
            nRet, GetErrorStr(nRet), hErrCodeChg(nRet));
        return hErrCodeChg(nRet);
    }

    return WFS_SUCCESS;
}

// 读卡数据结构体变量初始化
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
HRESULT CXFS_MSR::InnerReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = 0;

    nRet = m_pDev->Reset();
    if (nRet != ERR_MSR_SUCCESS) // 复位动作失败
    {
        Log(ThisModule, __LINE__, "复位子处理: 调用设备复位接口: ->Reset()失败, RetCode:%d|%s, Return:%d.",
            nRet, GetErrorStr(nRet), hErrCodeChg(nRet));
        return hErrCodeChg(nRet);
    } else
    {
        return WFS_SUCCESS;
    }
}

// 状态更新子处理
long CXFS_MSR::UpdateStatus()
{
    //THISMODULE(__FUNCTION__);

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
        case ERR_MSR_READTIMEOUT:             return WFS_ERR_TIMEOUT;
        case ERR_MSR_WRITETIMEOUT:            return WFS_ERR_TIMEOUT;
        case ERR_MSR_READERROR:               return WFS_ERR_IDC_INVALIDDATA;
        case ERR_MSR_WRITEERROR:              return WFS_ERR_HARDWARE_ERROR;
        case ERR_MSR_INVALIDCARD:             return WFS_SUCCESS/*WFS_ERR_IDC_INVALIDMEDIA*/;
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
    m_stMsrIniConfig.wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (DWORD)0);

    // Open时是否执行Reset动作(0不执行/1执行,缺省0)
    m_stMsrIniConfig.wOpenResetSupp = (WORD)m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetSupp", (DWORD)0);
    if (m_stMsrIniConfig.wOpenResetSupp != 0 &&
        m_stMsrIniConfig.wOpenResetSupp != 1)
    {
        m_stMsrIniConfig.wOpenResetSupp = 0;
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stMsrIniConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stMsrIniConfig.nOpenFailRet != 0 && m_stMsrIniConfig.nOpenFailRet != 1)
    {
        m_stMsrIniConfig.nOpenFailRet = 0;
    }

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_OPENMODE_%d", m_stMsrIniConfig.wDeviceType);

    // 设备SDK库路径
    strcpy(m_stMsrIniConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    // 读设备Open相关参数
    if (m_stMsrIniConfig.wDeviceType == DEV_TYPE_WBCS10 ||
        m_stMsrIniConfig.wDeviceType == DEV_TYPE_WBT2172)
    {
        STDEVICEOPENMODE    stDevOpenModeTmp;
        STDEVICELOG         stDeviceLogTmp;

        // 打开方式(0串口/1USBHID,缺省0)
        stDevOpenModeTmp.wOpenMode = (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenMode", (DWORD)0);
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
        // 是否启用动态库底层处理日志记录(0不启用/1启用,缺省0)
        stDeviceLogTmp.wEnableLog = (WORD)m_cXfsReg.GetValue(szIniAppName, "IsEnableDevLog", (INT)1);
        if (stDeviceLogTmp.wEnableLog < 0 && stDeviceLogTmp.wEnableLog > 1)
        {
            stDeviceLogTmp.wEnableLog = 0;
        }

        // 动态库底层处理日志记录级别(缺省4)
        // 0禁止显示写日志, 1:允许记录信息级日志, 2:允许记录调试级日志,
        // 3允许记录警告级日志, 4允许记录错误等级日志，记录发送接收数据
        stDeviceLogTmp.wLogLevel = (WORD)m_cXfsReg.GetValue(szIniAppName, "DeviceLogLevel", (INT)1);

        // 动态库底层处理日志路径(绝对路径,不设置则与动态库处于同一路径下)
        strcpy(stDeviceLogTmp.szLogPath, m_cXfsReg.GetValue(szIniAppName, "DeviceLog", ""));

        memcpy(&(m_stMsrIniConfig.stMsrInitParamInfo.stDeviceLog), &stDeviceLogTmp, sizeof(STDEVICELOG));
        memcpy(&(m_stMsrIniConfig.stMsrInitParamInfo.stDeviceOpenMode), &stDevOpenModeTmp, sizeof(STDEVICEOPENMODE));
    }

    // Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功,缺省0)
    m_stMsrIniConfig.wResetFailReturn = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetFailReturn", (DWORD)0);
    if (m_stMsrIniConfig.wResetFailReturn != 0 &&
        m_stMsrIniConfig.wResetFailReturn != 1)
    {
        m_stMsrIniConfig.wResetFailReturn = 0;
    }

    return WFS_SUCCESS;
}

